// Modified: February 17 2014
//                            
// Encoder / Decoder software implementation of (7,4) encoding
// Creates 7 bit codeword and then decodes it
// 1errorcodec.cpp 

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <tchar.h>
#include "Serial.h"
#include <vector>
#include <iostream>

using namespace std;

// Create 7 bit codeword from 4 bit message g(x) = 1 + x + x^3
// input: 4 bit message
// output: 7 bit codeword
char encode1bit(char x) {
	// declare all variables
	char codeWord = 0x00;
	char parity = 0x00;
	char v0, v1, v2 = 0;

	// v2 = v6 + v4 + v3
	v2 = ((x >> 2) & 1) + ((x >> 1) & 1) + (x & 1);
	v2 &= 0x01;
	// v1 = v5 + v4 + v3
	v1 = ((x >> 3) & 1) + ((x >> 2) & 1) + ((x >> 1) & 1);
	v1 &= 0x01;
	// v0 = v4 + v3 + v2
	v0 = ((x >> 3) & 1) + ((x >> 2) & 1) + v2;
	v0 &= 0x01;

	codeWord = (v0 << 6) | (v1 << 5) | (v2 << 4) | x;
	return codeWord;
}

// Create 511 bit codeword from 493 bit message g(x) = 1 + x + x^3
// input: 493 bit message
// output: 511 bit codeword
vector<char> encode2bit(vector<char> message) {
	// declare all variables
	vector<char> codeWord;
	vector<char> parity(18, 0);
	
	// Generate the parity bit according to formula
	for (int i = 0; i < 492; i++){
		parity[17] = parity[16];
		parity[16] = parity[15];
		parity[15] = parity[14] ^ message[i];
		parity[14] = parity[13];
		parity[13] = parity[12];
		parity[12] = parity[11] ^ message[i];
		parity[11] = parity[10];
		parity[10] = parity[9] ^ message[i];
		parity[9] = parity[8];
		parity[8] = parity[7] ^ message[i];
		parity[7] = parity[6] ^ message[i];
		parity[6] = parity[5] ^ message[i];
		parity[5] = parity[4];
		parity[4] = parity[3];
		parity[3] = parity[2] ^ message[i];
		parity[2] = parity[1];
		parity[1] = parity[0] ^ message[i];
		parity[0] = message[i];
	}

	// Add parity bit on the end of the message and return the finished codeword
	codeWord.reserve(message.size() + parity.size());
	codeWord.insert(codeWord.end(), message.begin(), message.end());
	codeWord.insert(codeWord.end(), parity.begin(), parity.end());
	return codeWord;
}

// decode 7 bit codeword and extract 4 bit message
// input: 7 bit codeword
// output: 4 bit message
char decode1bit(char x){
	// declare all variables
	char message = 0x00;
	char syndrome = 0x00;
	char s0, s1, s2, s0next, s1next, s2next = 0x00;
	char detection = 0x00;
	char reMessage = 0x00;

	// build up the syndrome for the codeword
	for (int i = 0; i < 7; i++){
		// collect individual bits of the syndrom
		s0 = (syndrome >> 2) & 1;
		s1 = (syndrome >> 1) & 1;
		s2 = syndrome & 1;

		// build next iteration of the syndrome
		s0next = (((x >> i) & 1) + s2) & 1;
		s1next = (s0 + s2) & 1;
		s2next = s1;
		syndrome = (s0next << 2) | (s1next << 1) | s2next;
	}

	// correct code using syndrome
	for (int i = 0; i < 7; i++){
		// collect individual bits of the syndrome
		s0 = (syndrome >> 2) & 1;
		s1 = (syndrome >> 1) & 1;
		s2 = syndrome & 1;
		
		// detection logic from syndrome
		detection = (s0 & !(s1)&s2) & 1;

		// build up message
		message = (message << 1) | ((((x >> i) & 1) + detection) & 1);

		// build next iteration of the syndrome
		s0next = (0 + s2) & 1;
		s1next = (s0 + s2) & 1;
		s2next = s1;
		syndrome = (s0next << 2) | (s1next << 1) | s2next;
	}

	// Reverse order of the message
	message = ((message >> 6) & 1) | ((message >> 5) & 1) << 1 | ((message >> 4) & 1) << 2 | ((message >> 3) & 1) << 3 | ((message >> 2) & 1) << 4 | ((message >> 1) & 1) << 5 | (message & 1) << 6;

	// Cut off parity bits and return the original message
	return 0xf & message;
}

// function to flip a single bit in a codeword
// input: codeword of 7 bits
// output: codeword of 7 bits with 1 error
char errorGen1bit(char x){
	char codeWord = 0x00;
	char error = rand()%6;
	char bit = 0x00;

	for (int i = 0; i < 7; i++){
		bit = (x >> (6 - i)) & 1;

		// flip the random bit
		if (error == i){
			bit = ~bit & 1;
		}

		codeWord = (codeWord << 1) | bit;
	}

	return codeWord;
}

int main(void){

	// Declare variables needed for serial connection
	LPCWSTR com = _T("COM4");

	srand(time(NULL));
	int correctCount = 0;
	char code = 0x00;
	vector<char> input(493,0);
	vector<char> encoded;
	unsigned char sendout;
	unsigned char testout = 0x00;
	char nopoint = 0x00;
	int counter = 0;		// Used to cut vector into characters

	// Create a vector of random bits
	for (int i = 0; i < input.size(); i++){
		input[i] = rand() % 2;
		printf("%x", input[i]);
	}
	printf("\n\n");
	// Encode the message
	encoded = encode2bit(input);	
	
	// Show what was final product of encoding is
	for (int i = 0; i < encoded.size(); i++){
		printf("%x", encoded[i]);
	}

	printf("\n\nSize of encoded:%i\n", encoded.size());

	// Small function to display in hex what will be sent.
	for (int i = 0; i < encoded.size(); i++){
		
		// If a full character has been built, print it off / send it out and reset the counters
		if (counter == 8){
			counter = 0;
			printf("%x ", testout);
			testout = 0x00;
		}
		// Build the character to be send
		testout = (testout | (encoded[i] << counter));

		//printf("\ninput:%i  inputshift:%i  testout:%x   i:%i",input[i],input[i] << counter, testout,i);
		counter++;

	}
	printf("%x\n\n", testout);

	
	// Instantiate serial connection
	//Serial fpga(com);

	// Wait for it to connect
	//while (!fpga.IsConnected());

	printf("done");
	getchar();
}
