# One-Time Pad Encryption & Decryption Servers and Clients
## I. File list
otp_enc_d.c&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Encryption server implementation<br />
otp_enc.c&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Encryption client implementation<br />
otp_dec_d.c&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Decryption server implementation<br />
otp_dec.c&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Decryption client implementaiton<br />
keygen.c&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Encryption/Decryption key generation implementation<br />
enc_dec.h&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Header file for all *.c files associated with encryption and decryption.<br />
compileall&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Shell (bash) script for compiling all *.c files.<br />

## II. Program instructions

#### A. Brief program overview

This program allows for the encryption and decryption of a plaintext file. The
only allowable characters that can be encrypted by this program are as follows:
	
	[' ', 'A', ... , 'Z']

The maximum file that can be encrypted by this program is roughly 8.3 MB, or
8388602 Bytes to be exact, due to the limitation imposed by the communication
protocol used in server-client communication. This is addressed further below 
in II.H and II.G.

This program implements a one-time pad (OTP) encryption, for which this program
generates a one-time use key to be used by an encryption client and server and
a decryption client and server. 

This program encrypts plaintext files by running the encryption client 
program, which sends data to be encrypted to the encryption server program over
a TCP connection. The server then returns the encrypted plaintext data, the  
ciphertext data, to the client, which the client outputs to standard output.

This program decrypts ciphertext files by running the decryption client 
program, which sends data to be decrypted to the decryption server program over
a TCP connection. The server then returns the decrypted ciphertext data to the
client, which the client outputs to standard output.

The encryption is accomplished by using the key provided by the key generation
program, which generates keys of a size specified by the user. The same key
must be used by both the encryption client/server and the decryption 
client/server in order to successfully encrypt and decrypt files. The same key
should not be used more than once to prevent a compromised encryption.

In order for the encryption client to produce an encrypted file to be used by 
the decryption client, the user should redirect the encryption client's output
to a plaintext file that will contain the ciphertext.

The provided shell script "compileall" can be used to compile all of the 
necessary files associated with key generation, encryption server and client,
and decryption server and client. The syntax used for compiling can be seen in 
the compileall file. Compiling the program files is the only purpose of the 
compileall script. The object files referred to in this file are as follows, 
and self apparent:
	
	otp_enc_d		Encryption server object file
	otp_enc 		Encryption client object file
	otp_dec_d 		Decryption server object file
	otp_dec 		Decryption client object file
	keygen 			Key generation object file

#### B. Running the key generation program

The key generation program will produce a random set of characters among:

	[' ', 'A', ... , 'Z']

The number of characters are provided by the user in the command line as 
follows (for the rest of this document, in running object files ignore the '"' 
characters):

	"keygen <key size>"

This program generates the key and outputs it to standard output. In order to
generate a key to be used by the encryption/decryption clients, redirect the
output to a key file.

#### C. Running the encryption server

The encryption server accepts a TCP connection from the encryption client. The
user must specify the port on which to wait for connections from the client.
Every time a client connects to the server, the server forks off a child 
process that completes the encryption while the parent process continues to 
wait for more TCP connections from other clients.

The encryption server accepts a stream of data from the client that contains
data related to the information and communication (specified in II.G and II.H 
below), the data to be encrypted, and the key to use for encryption. The
encryption procedure used by the encryption server is specified in II.J below.

The encryption server returns a stream of data to the client that contains data
related to the information and communication (specified in II.G and II.H 
below) and the encrypted message, granted the encryption is successful. If the
encryption is not successful, either from an erroneous connection to the server
or an error, the server sends an error message, i.e. "-1", to the client.

The encryption server can be run in the foreground or the background, using the
respective syntaxes as follows:

	"otp_enc_d <port number>"
	"otp_enc_d <port number> &"

where the latter is used to run the encryption server in the background. The
encryption client must also specify a port number in the command line program
call, where the client's specified port must match the server port number.

#### D. Running the decryption server

The decryption server accepts a TCP connection from the decryption client. The
user must specify the port on which to wait for connections from the client.
Every time a client connects to the server, the server forks off a child 
process that completes the decryption while the parent process continues to 
wait for more TCP connections from other clients.

The decryption server accepts a stream of data from the client that contains
data related to the information and communication (specified in II.G and II.H 
below), the data to be decrypted, and the key to use for decryption. The
decryption procedure used by the decryption server is specified in II.K below.

The decryption server returns a stream of data to the client that contains data
related to the information and communication (specified in II.G and II.H 
below) and the decrypted message, granted the decryption is successful. If the
decryption is not successful, either from an erroneous connection to the server
or an error, the server sends an error message, i.e. "-1", to the client.

The decryption server can be run in the foreground or the background, using the
respective syntaxes as follows:

	"otp_dec_d <port number>"
	"otp_dec_d <port number> &"

where the latter is used to run the decryption server in the background. The
decryption client must also specify a port number in the command line program
call, where the client's specified port must match the server port number.

#### E. Running the encryption client

The encryption client connects to the encryption server with a TCP connection.
The user must specify the port at which the server is waiting for connections.
Once connected to the server, the client pieces together a message to send to
the server containing a 4-byte preamble of data relevant for the data 
transfer, the data to be encrypted, the key to be used for encryption, and a
separating character between the data and the key.

The plaintext data that can be encrypted must be stored in a plaintext file 
and contain only the following characters: 

	[' ', 'A', ..., 'Z']

If any other characters are found while the client is processing the plaintext
file, an error is printed to stderr and no encryption is done. Upon a 
succesful encryption by the ecryption server, the client receives the encrypted
message with a 4-byte preamble. The preamble contents are specified in II.H. 
Upon an unsuccessful encryption by the encryption server, the client receives a
message beginning with "-1". This indicates that either there was a problem 
with the encryption or that the client attempted to use the decryption server 
instead of the encryption server for encryption.

The client can be run in the foreground or background. The encryption client 
program can be run using the following syntax:

	"otp_enc <plaintext file> <key file> <port number>"
	"otp_enc <plaintext file> <key file> <port number> > <ciphertext file>"
	"otp_enc <plaintext file> <key file> <port number> > <ciphertext file> &"

where the first will print the ciphertext to standard output, the second will
create a ciphertext file that can be used by the decryption client, and the 
third creates the ciphertext file but runs in the background instead of the
foreground.

#### F. Running the decryption client

The decryption client connects to the decryption server with a TCP connection.
The user must specify the port at which the server is waiting for connections.
Once connected to the server, the client pieces together a message to send to
the server containing a 4-byte preamble of data relevant for the data 
transfer, the data to be decrypted, the key to be used for decryption, and a
separating character between the data and the key.

The ciphertext data that can be decrypted must be stored in a plaintext file 
and contain only the following characters: 

	[' ', 'A', ..., 'Z']

If any other characters are found while the client is processing the plaintext
file, an error is printed to stderr and no decryption is done. Upon a 
succesful decryption by the deryption server, the client receives the decrypted
message with a 4-byte preamble. The preamble contents are specified in II.H. 
Upon an unsuccessful decryption by the decryption server, the client receives a
message beginning with "-1". This indicates that either there was a problem 
with the decryption or that the client attempted to use the encryption server 
instead of the decryption server for decryption.

The client can be run in the foreground or background. The decryption client 
program can be run using the following syntax:

	"otp_dec <ciphertext file> <key file> <port number>"
	"otp_dec <ciphertext file> <key file> <port number> > <deciphered file>"
	"otp_dec <ciphertext file> <key file> <port number> > <deciphered file> &"

where the first will print the deciphered text to standard output, the second 
will create a deciphered text file, and the third creates the derciphered text 
file but runs in the background instead of the foreground.

#### G. Communication between server and client

The client and server for both encryption and decryption communicate over a
TCP connection. Each message sent is prepended with a 4-byte preamble
specified in II.H. Messages from the clients to the servers contain a
character dividing data to be encrypted or decrypted from the key to be used, 
specified in II.I.

Communication between the server and client alternate with one another, where
the client sends the first communication after a connection is established.

#### H. Protocol for sending data between server and client

In creating a message to be sent from either the client or server, a 4-byte
preamble prepends all other data sent. The first byte of data sent contains
either a '^' or a '#' character, where the '^' indicates that the 
communication is meant for either encryption program and the '#' indicates that
the communication is meant for either decryption program.

Following the first byte of the encryption/decryption indicator are 3 bytes
containing a hexadecimal measure of the size of communication to be received
after the 4-byte preamble. In receiving a message, each program strips off the
first 4 bytes of the message, validates that the message is correctly
identified as an encryption or decryption message, and then continues to
receive data until the correct amount of data specified in the 3-byte 
hexadecimal measure is received. This procedure ensures that all of the data
intended to be received by the sender is received. The program imposes a limit
on the data allowed to be encrypted, governed by the hexadecimal measure that
can fit in the 3-byte measurement.

If a "-1" message is received by any program, this indicates an error has been
sent by the corresponding client or server. A "-1" message is designed to
indicate whether the encryption client tried to encrypt a message using the
decryption server or if the decryption client tried to decrypt a message
using the encryption server.

#### I. Protocol for parsing data between client and server

When a client prepares a message to send to its server, the message uses the
4-byte preamble specified above and uses a character to divide the data to be
encrypted or decrypted from the key to be used for the encryption or 
decryption.

For the encryption client, the '^' character is used as the divider, while the
decryption client uses the '#' character as the divider. The servers then parse
the data so they can use the data separate from the key received by the client.

#### J. Encryption procedure

The encryption server, once the data to be encrypted is separated from the key
to use for encryption, does a one-time pad encryption on the data using the 
key.

Because there are 27 allowable characters in the plaintext file, the procedure
for encrypting a single character is the following:

	1. Convert ' ' characters (ASCII 32) to '@' characters (ASCII 64)
	2. Reduce data[i] and key[i] from the ASCII range [64-90] to [0-26]
	3. Add data[i] and key[i]
	4. Take the modulo 27 of the sum
	5. Increase the modulo from the ASCII range [0-26] to [64-90]
	6. Convert '@' characters (ASCII 64) to ' ' characters (ASCII 32)

Do this procedure for all values i in the data received and return the
encrypted data prepended with a 4-byte preamble specified in II.H.

#### K. Decryption procedure

The decryption server, once the data to be decrypted is separated from the key
to use for decryption, does a one-time pad decryption on the data using the 
key.

Because there are 27 allowable characters in the ciphertext file, the procedure
for decrypting a single character is the following:

	1. Convert ' ' characters (ASCII 32) to '@' characters (ASCII 64)
	2. Reduce cipher[i] and key[i] from the ASCII range [64-90] to [0-26]
	3. Subtract key[i] from cipher[i]
	4. Take the modulo 27 of the difference
	5. Ensure all values are the positive modulo (add 27 if necessary)
	6. Increase the modulo from the ASCII range [0-26] to [64-90]
	7. Convert '@' characters (ASCII 64) to ' ' characters (ASCII 32)

Do this procedure for all values i in the ciphertext received and return the
decrypted data prepended with a 4-byte preamble specified in II.H. The
decryption must use the same key as was used in the encryption procedure.


## III. References
#####A. Beej's Guide to Network Programming | Using Internet Sockets
Brian "Beej Jorgensen" Hall<br />
http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html

	1. This resource was used for proper syntax and structure for a program
	implementing internet sockets to accomplish network programming tasks.

	2. This resource aided in the understanding, creation, and use of various
	structs involved in network programming in C. This includes the handling of
	addrinfo structs, sockaddr structs, and sockaddr_in structs.

	3. This resource aided in the understanding, creation, and use of various
	functions involved in network programming in C. This includes the handling
	of the getaddrinfo() function, the socket() function, the bind() function,
	the listen() function, the accept() function, the inet_ntop() function, the
	recv() function, the send() function, the connect() function, and the 
	close() function.

	4. This resource aided in the understanding and use of the various structs
	and functions listed in III.A.2 and III.A.3 in conjunction with one 
	another. Much of the structure of this program was aided by the 
	information offered in this resource, though the program was ultimately 
	built to accomplish the operation outlined in II.A-K. 

##### B. How can I get a file's size in C? [duplicate]
Responding user: Greg Hewgill on October 26, 2008<br />
http://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c

	1. This resource was used for the proper syntax and structure for a program
	obtaining the size of a specific file.

	2. This resource was used in the understanding, creation, and use of the 
	stat struct, the stat function, and the st_size attribute of the stat 
	struct. These were used to obtain the integer measure of the size of the
	files relevant for plaintext, ciphertext, keys, and temporary files used 
	in communication between the servers and clients. This is instrumental in 
	the reliable transfer of the requested file from the server to the client.

	3. This resource aided in the understanding and use of the struct and 
	function in III.B.2 in conjunction with one another to obtain the file size
	of the client's requested file.

##### C. Converting an int into a 4 byte char array (C)
Responding user: caf on September 24, 2010, edited by betabandido on June 7, 2012<br />
http://stackoverflow.com/questions/3784263/converting-an-int-into-a-4-byte-char-array-c

	1. This resource was used for the proper syntax and structure for a program
	implementing the conversion of an integer to a 3-byte char array. This 
	resource was used in the proper use of the unsigned char array to store the
	converted integer, the unsigned long variable to store the integer file 
	size, the bit-shift operator, and the conversion to hexadecimal in the
	appropriate part of the array.

	2. This resource aided in the understanding of how to convert an integer
	to a consistent 3-byte array so that the servers and clients could prepend 
	their communications with a 3-byte measure of the communication in 
	question.

	3. This resource was instrumental in implementing the the large file transfer
	protocol of adding a 4-byte preamble to all communications, where the latter 
	3 bytes of the preamble is a measure of the message size from server to client 
	or from client to server.

##### D. Python Socket Receive Large Amount of Data
Responding user: Adam Rosenfield on July 16, 2013<br />
http://stackoverflow.com/questions/17667903/python-socket-receive-large-amount-of-data

	1. This resource was the motivation for using an n-byte preamble of a
	hexadecimal measure to ensure reliable large file communication. The
	general structure of the protocol specified in this resource was
	influential to how the protocol with the 4-byte preamble is implemented
	in this program.

	2. This resource conceptually informed the receive_all function in its use 
	of a file's size to govern a while loop in continually accepting packets 
	to accomplish a more reliable transfer between the server and the client.
	This resource was used to conceptually understand prepending a message with
	data to be used by the receiver of data by unpacking the preamble and
	making each host read the first 4 bytes of the transmitted file first so 
	that the file size can be used to govern the reception of the entire 
	message.

	3. This resource was used in the understanding of the necessity and the 
	creation of a specific protocol that stores the requested file's size in 
	the communication between server and client. Using 3 bytes of the the 4-
	byte preamble of the transmission of the file to encode the file's size 
	allowed me to accomplish the  successful transfer of both small and large 
	files between the server and client.

	4. This resource was instrumental behind the implementation of the large
	file transfer functionality of the programs and the idea behind the 
	transmission of large files between them. This resource aided in building a 
	function that returns all of the received file's contents in one variable, 
	to be processed and parsed by the receiving server or client.

##### E. How to properly convert an unsigned char array into an uint32_t
Responding user: cnicutar on August 14, 2011, edited by larsmans on August 14, 2011<br />
http://stackoverflow.com/questions/7059299/how-to-properly-convert-an-unsigned-char-array-into-an-uint32-t

	1. This resource was used for the proper syntax and structure of unpacking
	an unsigned char array of hexadecimal values from the 4-byte preamble of
	communications to store the transmission size in an uint32_t variable.

	2. This resource was used to understand and execute the conversion from
	an unsigned char array to a uint32_t variable using bit shifts casting each
	array location to an int variable. I ended up casting each byte to a 
	uint8_t variable and then building the final uint32_t variable used to
	govern the message reception while loop.

	3. This resource was instrumental in implenting the large message transfer
	protocol using the 4-byte preamble, where this resource was used by
	receiving hosts to unpack the 3-byte file size in the 4-byte preamble.

##### F. https://oregonstate.instructure.com/courses/1524722/discussion_topics/7567876
&nbsp;&nbsp;&nbsp;(login required)<br />
Post by user Benjamin Brewster on August 6, 2015 at 8:49 AM PDT

	1. This resource was used to understand the necessity of prepending the
	communications between the encryption and decryption servers and clients in
	order to prevent erroneous encryption/decryption.

	2. This resource was used to inspire the method of using the circumflex
	character prepending encryption communications and the octothorpe character
	prepending decryption communications. This first byte to all 
	communications allowed the servers and clients to differentiate between 
	communications between the encryption server and client and the decryption 
	server and client.

	Special thanks to:
	Post by user Iam Dalrymple on August 7, 2015 at 6:00 AM PDT

	3. Thanks for provision of a template of clean up script used during
	implementation and testing of the programs.

	Post by user Jeffrey Mueller on August 11, 2015 5:47 AM PDT

	4. Thanks for provision of a script to allow the clean killing of only 
	parent processes, preventing defunct child processes from attempting to
	killed.

	5. This was used throughout implementation and testing to see more succint
	clean up upon killing the processes begun in the program run.
