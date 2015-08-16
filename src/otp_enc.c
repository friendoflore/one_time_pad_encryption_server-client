/******************************************************************************
 ** Filename: otp_enc.c
 ** Author: Tim Robinson
 ** Date created: 8/11/2015
 ** Last modification date: 8/15/2015 11:46 AM PDT
 *****************************************************************************/

#include "enc_dec.h"

/******************************************************************************
 ** Function: incorrect_syntax
 ** Description: This function prints the correct syntax to run this program
 ** 	from the command line to stderr.
 ** Parameters: None.
 ** Preconditions: None.
 ** Postconditions: This function terminates with an error message printed to
 **		stderr. The error message contains the correct sytax to run this
 **		program.
 *****************************************************************************/

void incorrect_syntax() {
	fprintf(stderr, "Incorrect syntax. Use \"./otp_enc <plaintext file> <key file> <port>\"\n");
	exit(1);
}

int main(int argc, char *argv[]) {

	// This stores the port number to listen on.
	char *PORT;

	// This stores the status of the getaddrinfo function.
	int status;

	// This stores the file descriptor for the socket created using the 
	// provided port number.
	int s;

	// This stores the file descriptor for the connection created using the 
	// accept function.
	int new_fd;

	// This is used to send a 1 as an address while setting the sockopt 
	// options.
	int yes = 1;

	// This is used to govern whether we should check for data meant for
	// encryption or data meant for decryption in the receive_all function.
	// If the enc_flag is 1, the data received will be checked for a prepended 
	// '^', meaning the data received is meant for the encryption process.
	// If the enc_flag is 0, the data received will be checked for a prepended
	// '#', meaning the data received is meant for the decryption process.
	int enc_flag = 1;

	// This stores the size of the original plaintext file.
	int file_size;

	// This stores the size of the key file. 
	int key_size;

	// This stores the length of the plaintext and key files combined, with a
	// character to separate them.
	int size_to_send;

	// This is an iterator.
	int i;

	// This stores the server's address that will send us encrypted data.
	struct sockaddr_storage their_addr;

	// These structs are used to hold address information about the sockets we
	// are to create and process. The first will aid in setting our server 
	// socket data. The res variable is a pointer to a linked list of addrinfo
	// structs returned by the getaddrinfo function. The "p" variable is an 
	// iterator, used to pick out successful links in the "res" list.	
	struct addrinfo servSet, *res, *p;

	// The user must provide a plaintext file, a key file, and a port number
	// as command line arguments.
	if(argc != 4) { incorrect_syntax(); }

	// Validate that the port number provided is a valid port number.
	if((atoi(argv[3]) > 65535) || (atoi(argv[3]) < 0)) {
		fprintf(stderr, "Invalid port number\n");
		exit(1);
	
	// Assign the argument to our PORT variable.
	} else {
		PORT = argv[3];
	}

	// This stores the plaintext file name.
	char *enc_file = malloc(sizeof(char) * (strlen(argv[1])));

	// This stores the key file name.
	char *key_file = malloc(sizeof(char) * (strlen(argv[2])));

	// Store the command line arguments in the file name variables.
	strcpy(enc_file, argv[1]);
	strcpy(key_file, argv[2]);

	// Get and store the size of both the plaintext file and the key file.
	struct stat st_file;
	struct stat st_key;
	stat(enc_file, &st_file);
	stat(key_file, &st_key);
	file_size = st_file.st_size;
	key_size = st_key.st_size;

	// If the size of the key file is smaller than the file size, then we 
	// cannot reliably encrypt the plaintext file. Print an error and exit.
	if(file_size > (key_size)) {
		fprintf(stderr, "The key is not large enough.\n");
		exit(1);
	}

	// This stores the plaintext file contents. Initialize it.
	char * file_string = malloc(sizeof(char) * file_size);
	for(i = 0; i < file_size; i++) { file_string[i] = 0; }

	// This stores the key file contents.
	char * key_string = malloc(sizeof(char) * key_size);

	// The first byte of our message to the server is a '^' character. This
	// Indicates that we have connected to the correct server. '^' indicates
	// a message to be encrypted while '#' indicates a message to be decrypted.
	char * circum_char = malloc(1);
	circum_char[0] = '^';

	// Open the plaintext and key files for reading.
	FILE * fp = fopen(enc_file, "r");
	FILE * kp = fopen(key_file, "r");

	// These tmpfiles are used to build our full message to the server. We read
	// in the plaintext file and write it to tmpfile1, followed by a '^' 
	// character, followed by the key file contents.
	FILE * tmpfile1 = fopen("tmpfile1", "w");

	// We write a '^' character and then write the file size in hexadecimal,
	// always occupying 3 bytes. This is how the MAX_FILE_SIZE is set in 
	// enc_dec.h. Then write the contents of tmpfile1.
	FILE * tmpfile2 = fopen("tmpfile2", "w");

	// Read in the plaintext and key file contents.
	fread(file_string, file_size, 1, fp);
	fread(key_string, key_size, 1, kp);

	// Strip the newline character at the end of the file while validating the
	// file contents of the plaintext file.
	for(i = 0; i < file_size; i++) {

		// Replace the newline with a null terminator. In each plaintext file,
		// there is only one newline character.
		if(file_string[i] == '\n') {
			file_string[i] = '\0';
			break;
		}
		
		// If the current character is not between ['A', ..., 'Z']...
		if((file_string[i] < 65) || (file_string[i] > 90)) {

			// If the current, non uppercase alphabetic character is not a ' '
			// character, then we have found an invalid character. Exit the 
			// program with a value of 1.
			if(file_string[i] != 32) {
				fprintf(stderr, "Invalid characters in file %s.\n", enc_file);
				exit(1);
			}
		}
	}

	// Strip the newline character at the end of the key file.
	for(i = 0; i < file_size; i++) {

		// Replace the newline with a null terminator. In each key file, there
		// is only one newline character.
		if(key_string[i] == '\n') {
			key_string[i] = '\0';
		}
	}

	// Write the plaintext file contents.
	fwrite(file_string, strlen(file_string), 1, tmpfile1);

	// Write the '^' character as a division between plaintext and key values.
	fwrite(circum_char, 1, 1, tmpfile1);

	// Write the key file contents.
	fwrite(key_string, strlen(key_string), 1, tmpfile1);

	// Close the source files for reading.
	fclose(fp);
	fclose(kp);

	// Close the tmpfile for writing.
	fclose(tmpfile1);

	// Get the size of the tmpfile and store it. This is the value that will, 
	// with the '^' character as the first byte, occupy the 4-byte preamble to
	// all messages to the encoding server.
	struct stat st_send;
	stat("tmpfile1", &st_send);
	size_to_send = st_send.st_size;

	// If the size_to_send (so the sum of the plaintext file size with the key 
	// file size with the single circumflex character) is greater than the max
	// file size, then we cannot be sure that the encryption server will 
	// receive all of the data it needs to complete the encryption. The max
	// file size refers to the 3-byte hexadecimal limit given from the 4-byte
	// message preamble.
	if(size_to_send > MAX_FILE_SIZE) {
		fprintf(stderr, "This program cannot reliably encrypt a file of that size!\n");
		exit(1);
	}

	// This stores the final message to the server. Initialize it.
	char * send_string = malloc(sizeof(char) * size_to_send);
	for(i = 0; i < size_to_send; i++) { send_string[i] = '\0'; }

	// This will store the tmpfile's size in hexadecimal. This is the size of
	// the whole message to the server less the 4-byte preamble.
	unsigned char bytes[3];

	// This is the file size that we will convert to hexadecimal and prepend
	// our message to the server. 
	unsigned long n = size_to_send;

	// Convert n to hexadecimal and store it in 3 bytes.
	bytes[0] = (n >> 16) & 0xFF;
	bytes[1] = (n >> 8) & 0xFF;
	bytes[2] = n & 0xFF;

	// Open the tmpfile that we wrote to previously for reading. 
	tmpfile1 = fopen("tmpfile1", "r");

	// Read in all of its contents into the send_string variable.
	fread(send_string, size_to_send, 1, tmpfile1);

	// Write the 4-byte preamble, consisting of a '^', followed by exactly 3
	// bytes that give the size the server should expect to receive after
	// receiving the preamble.
	fwrite(circum_char, 1, 1, tmpfile2);
	fwrite(bytes, sizeof *bytes, sizeof bytes, tmpfile2);

	// Write the post-preamble message contents to the tmpfile.
	fwrite(send_string, strlen(send_string), 1, tmpfile2);

	// Close the tmpfile for writing.
	fclose(tmpfile2);

	// Reopen the tmpfile for reading.
	tmpfile2 = fopen("tmpfile2", "r");

	// The full message size is the size_to_send plus the 4-byte preamble.
	int full_size = size_to_send + 4;

	// This stores the entire message to be sent to the encryption server.
	// Initialize it.
	char * full_message = malloc(sizeof(char) * full_size);
	for(i = 0; i < strlen(full_message); i++) { full_message[i] = '\0'; }

	// Read in the full message from tmpfile2.
	fread(full_message, full_size, 1, tmpfile2);

	// Now, set up the client's socket. Clear the data at servSet and
	// their_addr.
	memset(&servSet, 0, sizeof servSet);
	memset(&their_addr, 0, sizeof their_addr);


	// We will use TCP using SOCK_STREAM.
	servSet.ai_socktype = SOCK_STREAM;

	// Use IPv4 addressing for this socket.	
	servSet.ai_family = AF_INET;

	// Allow the address information to fill in for us in the getaddrinfo 
	// function.
	servSet.ai_flags = AI_PASSIVE;

	// The "res" variable points to a linked list of addrinfo structs created 
	// using the address information in the servSet and PORT. The NULL is used
	// here to use the localhost as the server location.	
	if((status = getaddrinfo(NULL, PORT, &servSet, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	// Loop through the linked list of addrinfo structs and create a socket
	// using the first successful link in the list.
	for(p = res; p != NULL; p = p->ai_next) {

		// Create a socket and using the current addrinfo struct. If it is not
		// successful, try the next struct in the list. Refer to the socket
		// using s, the file descriptor.		
		if((s = socket(AF_INET, p->ai_socktype, p->ai_protocol)) == -1) {
			fprintf(stderr, "socket error: %s\n", strerror(s));
			continue;
		}

		// Allow the port used for the socket to be reused.
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

		// Try to connect the local socket with the server's socket.
		if(connect(s, p->ai_addr, p->ai_addrlen) < 0) {
			perror("connection error");
			exit(2);
		}

		// Stop looping through the addrinfo struct list if we have succesfully
		// returned from the connect function.
		break;
	}

	// Free the linked list of addrinfo structs since we have the socket file
	// descriptor.
	freeaddrinfo(res);

	// If the first character is not a '^', then we have failed at creating a
	// valid message for the server to parse and encrypt.
	if(full_message[i] != 94) { 
		fprintf(stderr, "File not marked for encryption.\n");
	}

	// Send the full message to the server for encryption.
	int newmessage = send(s, full_message, full_size, 0);

	// Free the full message memory after sending.
	free(full_message);

	// We know that the size of the ecnryption message to receive back from the
	// server must be the size of the plaintext file plus the 4-byte preamble 
	// that also prepends messages from the server to the client.
	int size_to_receive = file_size + 4;

	// This stores the response from the server. This stores the final 
	// encrypted message. Initialize it.
	char data[size_to_receive];
	for(i = 0; i < size_to_receive; i++) { data[i] = 0; }


	// This ensures that all of the data was reliably received by the 
	// client using the file size of the data received. See the 
	// receive_all function description in enc_dec.h and in the
	// readme.txt to see how that is accomplished.

	// m stores the number of bytes of data received by the client.
	int m = receive_all(data, s, enc_flag);

	// Validate that receiving the data was successful.
	if(m == -1) {
		fprintf(stderr, "File received not marked as encrypted.\n");
	}

	// Print the ciphertext to stdout, followed by a newline character.
	printf("%s\n", data);

	return 0;
}