#ifndef HEADER_ENC_DEC
#define HEADER_ENC_DEC

/******************************************************************************
 ** Filename: enc_dec.h
 ** Author: Tim Robinson
 ** Date created: 8/11/2015
 ** Last modification date: 8/15/2015 11:47 AM PDT
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#define MAX_FILE_SIZE 16777215

/******************************************************************************
 ** Function: receive_all
 ** Description: This function ensures that all of the data sent by one host is
 **		received by the other host. This function allows the connection to 
 **		remain open and continue to expect data for exactly as long as it
 **		should based on the file size being sent. This function does this by
 **		stripping off the first 4 bytes recieved by the host, reading the
 **		first byte to validate the action to be taken with the data received,
 **		and then using the following 3 bytes to set how long the host should
 **		continue to receive data. I refer to these 4 bytes as the message
 **		preamble throughout this program's documentation. The incoming message
 **		size, less the 4-byte preamble, is stored as hexadecimal values in the
 **		latter 3 bytes of the preamble.
 **			This function can be used by the encryption server and client, as
 **		well as the decryption server and client. All communications must
 **		use the protocol with the 4-byte preamble.
 ** Parameters: char * data, int new_fd, int enc_dec. The data variable will
 **		store all of the received data, less the preamble. The new_fd variable
 **		is the file descriptor for the connection over which we are receiving
 **		data. The enc_dec variable is used to tell us whether this function was
 **		called by an encryption program or a decryption program, which will
 **		change what we will look for in the first byte of the communication.
 ** Preconditions: The data variable must be large enough for the received
 **		message, which is the responsibility of the program calling this 
 **		function, and initialized to zero. The new_fd variable must be a valid
 **		file descriptor for an open TCP connection. The enc_dec variable must
 **		be a 1 if an encryption program called this function, a 0 if a
 **		decryption program called this function.
 **			The incoming message must be prepended with the 4 byte preamble 
 **		specified in the description of this program. The first byte received 
 **		must be either '^' for an encryption program or a "#" for a decryption
 **		program. All other first-byte values will return an error (-1) from
 **		this function. The size of the incoming message less the preamble must
 **		be greater than zero, otherwise this function's behavior is undefined.
 **		The following 3 bytes of the 4-byte preamble must be the hexadecimal 
 **		size of the incoming message, even if the first 2 bytes are 0x00.
 ** Postconditions: This function returns the size of the data received by the
 **		host less the preamble. This function returns the data received in the
 **		data variable with the preamble characters stripped from the front of
 **		data received.
 *****************************************************************************/

int receive_all(char * data, int new_fd, int enc_dec) {
	
	// These are iterators.
	int i;
	int j;

	// This stores the individual incoming packets of communication over the
	// TCP connection. This is used to build the final data received variable.
	// Initialize it.
	char * packet = malloc(sizeof(char) * 1024);
	for(i = 0; i < 1024; i++) { packet[i] = '\0'; }


	// This flag is set to 1 when we are to check the 4 byte preamble while
	// receiving data over the connection.
	int flag = 1;

	// This stores the number of bytes that we will receive in total over the
	// connection, less the preamble. We start with a value of 4 so that we
	// receive just the 4-byte preamble first.
	uint32_t n = 4;

	// This stores the status of the recv function.
	int received;

	// Continue to accept data while there is data left to be received.
	while(strlen(data) < n) {

		// Receive n - strlen(data) bytes of data and store it in the packet
		// variable. Note that n - strlen(data) is equal to 4 for the first
		// transfer of data over the connection, while after the first 
		// transfer, the function attempts to receive the remainder of the data
		// to be received. We stop receiving data when the strlen(data) equals
		// n, the length of data to be received.
		received = recv(new_fd, packet, (n - strlen(data)), 0);

		// If we are checking the 4-byte preamble...
		if(flag) {

			// If an encryption program called this function...
			if(enc_dec) {

				// Return an error (-1) if the first byte is not the '^' 
				// character.
				if(packet[0] != '^') { return -1; }
			
			// If a decryption program called this function...
			} else {

				// Return an error (-1) if the first byte is not the '^' 
				// character.
				if(packet[0] != '#') { return -1; }
			}

			// Overwrite the encryption/decryption indicator byte with a 
			// leading 0 to convert from hex to an unsigned integer.
			packet[0] = 0;

			// Convert the hexadecimal to an unsigned integer.
			n = (uint8_t)packet[0] << 24  |
				(uint8_t)packet[1] << 16  |
			    (uint8_t)packet[2] << 8   |
				(uint8_t)packet[3];

			// We are done processing the 4-byte preamble.
			flag = 0;

		// If we are not processing the 4-byte preamble and are receiving the
		// message data...
		} else {

			// Add the newly received packet from the recv function to the data
			// variable.
			strcat(data, packet);
		}
	}

	// Return the total number of bytes of the message received by the host.
	return n;
}

#endif