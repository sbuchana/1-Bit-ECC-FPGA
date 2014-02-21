// Modified: February 17 2014
//                            
// Encoder / Decoder software implementation of (7,4) encoding
// Creates 7 bit codeword and then decodes it
// 1errorcodec.cpp 

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

using namespace std;

// Create 7 bit codeword from 4 bit message g(x) = 1 + x + x^3
// input: 4 bit message
// output: 7 bit codeword
char encode(char x) {
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

// decode 7 bit codeword and extract 4 bit message
// input: 7 bit codeword
// output: 4 bit message
char decode(char x){
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
char errorGen(char x){
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
	srand(time(NULL));
	char code = 0x00;

	for (int i = 0; i < 15; i++){
		code = rand() % 15;
		printf("\nsent: %#4x   ", code);
		code = encode(code);
		printf("Codeword: %#4x   ", code);
		code = errorGen(code);
		printf("error: %#4x   ", code);
		printf("returned: %#4x", decode(code));
	}

	getchar();
}
