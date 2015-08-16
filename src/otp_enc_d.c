/******************************************************************************
 ** Filename: otp_enc_d.c
 ** Author: Tim Robinson
 ** Date created: 8/11/2015
 ** Last modification date: 8/15/2015 11:45 AM PDT
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
	fprintf(stderr, "Incorrect syntax. Use \"./otp_enc_d <port>\"\n");
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
	int new_fd = -1;

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

	// These are iterators.
	int i;
	int j;

	// This stores the client's address that will send us data to encrypt.
	struct sockaddr_storage their_addr;

	// These structs are used to hold address information about the sockets we
	// are to create and process. The first will aid in setting our server 
	// socket data. The res variable is a pointer to a linked list of addrinfo
	// structs returned by the getaddrinfo function. The "p" variable is an 
	// iterator, used to pick out successful links in the "res" list.
	struct addrinfo servSet, *res, *p;

	// This is used to pass the IP address size in the accept function.
	socklen_t addr_size;

	// The only argument should be a user-provided port number. Validate the
	// number of arguments provided.
	if(argc != 2) { incorrect_syntax(); }

	// Validate that the port number provided is a valid port number.
	if((atoi(argv[1]) > 65535) || (atoi(argv[1]) < 0)) {
		fprintf(stderr, "Invalid port number\n");
		exit(1);

	// Assign the argument to our PORT variable.
	} else {
		PORT = argv[1];
	}

	// Clear the data at servSet.
	memset(&servSet, 0, sizeof servSet);

	// Use IPv4 addressing for this socket.
	servSet.ai_family = AF_INET;

	// We will use TCP using SOCK_STREAM.
	servSet.ai_socktype = SOCK_STREAM;

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
		if((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			fprintf(stderr, "socket error: %s\n", strerror(s));
			continue;
		}

		// Allow the port used for the socket to be reused.
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

		// Assign the socket address to the socket referred to by s.
		if(bind(s, p->ai_addr, p->ai_addrlen) == -1) {
			fprintf(stderr, "bind error: %s\n", strerror(s));
			continue;
		}

		// Stop looping through the addrinfo struct list if we have succesfully
		// returned from the bind function.
		break;
	}

	// Free the linked list of addrinfo structs since we have the socket file
	// descriptor.
	freeaddrinfo(res);

	// Open the port for connections.
	if(listen(s, 5) == -1) { fprintf(stderr, "listen error"); }

	// Always accept new connections.
	while(1) {

		// Accept a new connection when one is requested on socket s. The 
		// new_fd variable will store the connection file descriptor.
		new_fd = accept(s, (struct sockaddr *)&their_addr, &addr_size);

		// Validate the file descriptor was accepted succesfully.
		if(new_fd == -1) { fprintf(stderr, "acceptance failed\n"); }

		// Fork off a new process so the parent can continue to accept new
		// connections on the socket while the child process completes the
		// encryption.
		pid_t spawnpid;
		spawnpid = fork();

		// Child process does the encryption...
		if(spawnpid == 0) {

			// This stores all of the incoming data, from which we get the file
			// contents to encrypt and the key we'll use for encryption.
			char * data = malloc(sizeof(char) * MAX_FILE_SIZE);
			
			// Initialize the malloc'd location.
			for(i = 0; i < strlen(data); i++) { data[i] = '\0'; }

			
			// This ensures that all of the data was reliably received by the 
			// server using the file size of the data received. See the 
			// receive_all function description in enc_dec.h and in the
			// readme.txt to see how that is accomplished.

			// n stores the number of bytes of data received by the server.
			int n = receive_all(data, new_fd, enc_flag);

			// Validate that receiving the data was successful.
			if(n == -1) {
				fprintf(stderr, "No decryption here.\n");
				char * error = malloc(4);
				error = "-1";

				// Send an error message to the client so the client knows the
				// encryption failed or was not allowed.
				int errormessage = send(new_fd, error, 4, 0);
				
				exit(1);
			}
		
			// This stores all the data we are to encrypt. Initialize it.
			char full_data[70004];
			for(i = 0; i < 70004; i++) { full_data[i] = 0; }

			// This stores all of the key values we are to use to do the
			// encryption. Initialize it.
			char full_key[70004];		
			for(i = 0; i < 70004; i++) { full_key[i] = 0; }
		
			// This is used to parse the message incoming from the client for
			// encryption. While iterating through the client's raw data, while
			// the data_flag is 0, we'll store that data in full_data. When a
			// '^' character is encountered, change the data_flag to a 1. While
			// the data_flag is 1, we'll store that data in full_key.
			int data_flag = 0;
			
			// This is used to iterate through the array storing key values.
			j = 0;
			for(i = 0; i < n; i++) {

				// If we are iterating through plaintext...
				if(data_flag == 0) {

					// Don't store the '^' character in either full_data or
					// full_key. We are done iterating through the plaintext 
					// and will now iterate through the key.
					if(data[i] == '^') {
						data_flag = 1;
						continue;
					}

					full_data[i] = data[i];

				// If we are iterating through the key...
				} else {
					full_key[j] = data[i];
					j += 1;
				}
			}

			// This loop does the encryption now that have both the plaintext
			// and the key stored.
			for(i = 0; i < strlen(full_data); i++) {

				// Change a ' ' character to an '@' character in both the 
				// plaintext and the key for the purposes of cipher 
				// calculation.
				if(full_data[i] == 32) { full_data[i] = 64; }
				if(full_key[i] == 32) { full_key[i] = 64; }

				// Store the character's values on a basis of 0-26 in order to
				// calculate the ciphertext.
				int x = full_data[i] - 64;
				int y = full_key[i] - 64;

				// Add the values and take the modulo of 27 of the result.
				int z = x + y;
				z = z % 27;

				// Convert the character to the range [64, 90], which is:
				//		['@', 'A', ... , 'Z']
				z += 64;

				// Convert any '@' characters to ' ' characters.
				if(z == 64) { z = 32; }

				// Replace the plaintext value with the encrypted value.
				full_data[i] = z;
			}

			// Use the '^' character prepended to the server's encryption so
			// the client knows that it is indeed encrypted.
			char * circum_char = malloc(1);
			circum_char[0] = '^';

			// Get the size of the encrypted message.
			int size_to_send = strlen(full_data);

			// Add 4 bytes to the size, which will store the message preamble
			// of the '^' character and 3 bytes of the encrypted file's size.
			int enc_size = size_to_send + 4;
			char * enc_to_send = malloc(sizeof(char) * enc_size);

			// This will store the encrypted file's size in hexadecimal.
			unsigned char bytes[3];

			// This is the file size that we will convert to hexadecimal and 
			// prepend to our response to the client. 
			unsigned long m = size_to_send;

			// Initialize it.
			for(i = 0; i < 3; i++) { bytes[1] = 0x00; }

			// Convert m to hexadecimal and store it in bytes.
			bytes[0] = (m >> 16) & 0xFF;
			bytes[1] = (m >> 8) & 0xFF;
			bytes[2] = m & 0xFF;

			// This file will be uesd to construct our the ciphertext message
			// to the client.
			FILE * tmpfile3;
			tmpfile3 = fopen("tmpfile3", "w+");

			// Prepend the file with '^' character to indicate that it is
			// encrypted.
			fwrite(circum_char, 1, 1, tmpfile3);

			// Add the encrypted file's size in hexadecimal to the tmpfile.
			// Again, the first 4 bytes consist of the '^' character and then
			// 3 bytes storing the file size in hexadecimal. The full protocol
			// is described in detail in the readme.txt file.
			fwrite(bytes, sizeof *bytes, sizeof bytes, tmpfile3);

			// Add the encrypted data to the first 4 bytes.
			fwrite(full_data, strlen(full_data), 1, tmpfile3);

			// Reset the file pointer to now read in from the tmpfile.
			rewind(tmpfile3);

			// Read in the the file to get a string of the full response to
			// be given to the client.
			fread(enc_to_send, enc_size, 1, tmpfile3);

			// Send encrypted data prepended with the 4 byte preamble to the
			// client.
			int newmessage = send(new_fd, enc_to_send, enc_size, 0);

			break;

		// Parent process continues accepting connections...
		} else if(spawnpid > 0) { 
			continue;
		} else { 
			printf("So the fork failed...\n");
		}
	}

	return 0;
}