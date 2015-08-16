/******************************************************************************
 ** Filename: keygen.c
 ** Author: Tim Robinson
 ** Date created: 8/11/2015
 ** Last modification date: 8/15/2015 11:46 AM PDT
 *****************************************************************************/

#include "enc_dec.h"

int main(int argc, char *argv[]) {
	
	// This stores the number of key characters this program is to generate.
	int num_to_gen = -1;

	// These are iterators.
	int i = 0;
	int j = 0;

	// Seed the random generator using time.
	srand(time(NULL));

	// The user must provide the number of characters to create in this key.
	if(argc != 2) {
		fprintf(stderr, "Invalid syntax. Use \"./keygen <key length>\"\n");
		exit(1);
	}

	// If the key is negative or is larger than the MAX_FILE_SIZE, then we can
	// not use that key for encryption and decryption in this program. The
	// MAX_FILE_SIZE is limited by the 3-byte hexadecimal measure of a 
	// message's contents, used in reliably communicating whole messages
	// between server and client.
	if((atoi(argv[1]) < 0) || (atoi(argv[1]) > MAX_FILE_SIZE)) {
		fprintf(stderr, "Invalid key length.\n");
		exit(1);
	}

	// Store the command line argument in the number to generate.
	num_to_gen = atoi(argv[1]);

	// We'll add a newline character at the end, so the final key size will
	// actually be 1 value longer than the number of characters to be
	// generated.
	int key_size = num_to_gen + 1;

	// This stores the final key values.
	char *key = malloc(sizeof(char) * key_size);

	// This is used for intermediary calculation of a random key value.
	int tmp_key_val;

	// Create the number of characters given by the user.
	for(i = 0; i < num_to_gen; i++) {

		// Get a random number between 0-26. Add 64 to put the character in the
		// range [64, 90], which is ['@', 'A', ... , 'Z'].
		tmp_key_val = rand() % 27 + 64;

		// Convert any '@' characters to ' ' characters.
		if(tmp_key_val == 64) { tmp_key_val = 32; }

		// Store the random int as a character.
		char tmp_key_char = tmp_key_val;

		// Put the random character in the array of key values.
		key[i] = tmp_key_char;
	}

	// Append a newline character at the end of the key.
	key[num_to_gen] = '\n';

	// Write the entire key to stdout.
	write(1, key, key_size);

	return 0;
}