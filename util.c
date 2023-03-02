/*
 *  ======== util.c ========
 */

#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gtz.h"

#define threshold 1000

UInt32 time1, time2;

char digit;

int freq1, freq2;
int mag1, mag2;

int sample, gtz_out[8];

short coef[8] = {0x6D02, 0x68AD, 0x63FC, 0x5EE7, 0x4A70, 0x4090, 0x3290, 0x23CE}; // goertzel coefficients


void task1_dtmfDetect(void) {
	//System runs continuously
	while (1) {

		/*
		 * Read key from user input, output correct frequencies and amplitudes to the receiver.
		 * Error message present when invalid key is entered.
		*/
		char symbol;
		int read_symbol = 0; //Signal for whether a key has been read, resets after each cycle.

		while(read_symbol == 0) {

			System_printf("\nPlease enter a key (0 to 9 or A, B, C, D, * or #):\n");
			System_flush();
			scanf("%s",&symbol); //Key input

			switch(symbol) {
			case '0':
				read_symbol = 1; //Signals that a valid key has been read.
				freq1 = 941;
				freq2 = 1335;
				break;
			case '1':
				read_symbol = 1;
				freq1 = 697;
				freq2 = 1209;
				break;
			case '2':
				read_symbol = 1;
				freq1 = 697;
				freq2 = 1335;
				break;
			case '3':
				read_symbol = 1;
				freq1 = 697;
				freq2 = 1477;
				break;
			case '4':
				read_symbol = 1;
				freq1 = 770;
				freq2 = 1209;
				break;
			case '5':
				read_symbol = 1;
				freq1 = 770;
				freq2 = 1335;
				break;
			case '6':
				read_symbol = 1;
				freq1 = 770;
				freq2 = 1477;
				break;
			case '7':
				read_symbol = 1;
				freq1 = 852;
				freq2 = 1209;
				break;
			case '8':
				read_symbol = 1;
				freq1 = 852;
				freq2 = 1335;
				break;
			case '9':
				read_symbol = 1;
				freq1 = 852;
				freq2 = 1477;
				break;
			case 'A':
				read_symbol = 1;
				freq1 = 697;
				freq2 = 1633;
				break;
			case 'B':
				read_symbol = 1;
				freq1 = 770;
				freq2 = 1633;
				break;
			case 'C':
				read_symbol = 1;
				freq1 = 852;
				freq2 = 1633;
				break;
			case 'D':
				read_symbol = 1;
				freq1 = 941;
				freq2 = 1633;
				break;
			case '*':
				read_symbol = 1;
				freq1 = 941;
				freq2 = 1209;
				break;
			case '#':
				read_symbol = 1;
				freq1 = 941;
				freq2 = 1477;
				break;
			default:
				mag1 = 00000.0;
				mag2 = 00000.0;
				freq1 = 000;
				freq2 = 0000;
				System_printf("\nInvalid symbol, please try again. Note that symbols are case sensitive.\n");
				System_flush();
			}
		}
		mag1 = 32768.0;
		mag2 = 32768.0;
		System_printf("\nPlease wait while the system processes your input.\n");
		System_flush();

		/*				 Corresponding frequencies and amplitudes for each key (for reference)
		 	 	 	 	 '0' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 941, freq2 = 1335
						 '1' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 697, freq2 = 1209,
						 '2' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 697, freq2 = 1335,
						 '3' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 697, freq2 = 1477,
						 '4' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 770, freq2 = 1209,
						 '5' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 770, freq2 = 1335,
						 '6' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 770, freq2 = 1477,
						 '7' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 852, freq2 = 1209,
						 '8' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 852, freq2 = 1335,
						 '9' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 852, freq2 = 1477,
						 '*' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 941, freq2 = 1209,
						 '#' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 941, freq2 = 1477,
						 'A' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 697, freq2 = 1633,
						 'B' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 770, freq2 = 1633,
						 'C' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 852, freq2 = 1633,
						 'D' : mag1 = 32768.0, mag2 = 32768.0, freq1 = 941, freq2 = 1633,
						otherwise : mag1 = 00000.0, mag2 = 00000.0, freq1 = 000, freq2 = 0000  */

		Task_sleep(500);

		/*
		 * Processes the data from the Goertzel outputs, identifies the key and its respective frequencies
		*/

		//System_printf("\nThe GTZ of 697Hz, 770Hz, 852Hz, 941Hz, 1209Hz, 1336Hz, 1477Hz and 1633Hz are:\n");
		//System_printf("%d, %d, %d, %d, %d, %d, %d, %d\n", gtz_out[0], gtz_out[1], gtz_out[2], gtz_out[3], gtz_out[4], gtz_out[5], gtz_out[6], gtz_out[7]);
		//System_flush();

		int i, F_out1 = 0, F_out2 = 0, F1 = 0, F2 = 0;
		int F_list[8] = {697, 770, 852, 941, 1209, 1336, 1477, 1633}; //Array of possible frequencies to reference from
		char key_out; //Detected key

		//Identifies the two component frequencies.
		//Ensures F1 is always the smaller frequency. If all GTZ are 0, then the F_outs stay at 0.
		for(i = 0 ; i < 8 ; ++i) {
			if( (gtz_out[i] > F1) || (gtz_out[i] > F2) ) {
				if(F2 > F1) {
					F1 = F2;
					F_out1 = F_out2;
					F2 = gtz_out[i];
					F_out2 = F_list[i];
				}
				else if(F1 >= F2) {
					F2 = gtz_out[i];
					F_out2 = F_list[i];
				}
			}
		}

		//System_printf("\nThe two frequencies are:\n");
		//System_printf("%dHz and %dHz\n", F_out1, F_out2);
		//System_flush();


		//Using the two identified frequencies, selects the coresponding key.
		switch(F_out1) {
		case 697:
			switch(F_out2) {
			case 1209:
				key_out = '1';
				break;
			case 1336:
				key_out = '2';
				break;
			case 1477:
				key_out = '3';
				break;
			case 1633:
				key_out = 'A';
				break;
			}
			break;
		case 770:
			switch(F_out2) {
			case 1209:
				key_out = '4';
				break;
			case 1336:
				key_out = '5';
				break;
			case 1477:
				key_out = '6';
				break;
			case 1633:
				key_out = 'B';
				break;
			}
			break;
		case 852:
			switch(F_out2) {
			case 1209:
				key_out = '7';
				break;
			case 1336:
				key_out = '8';
				break;
			case 1477:
				key_out = '9';
				break;
			case 1633:
				key_out = 'C';
				break;
			}
			break;
		case 941:
			switch(F_out2) {
			case 1209:
				key_out = '*';
				break;
			case 1336:
				key_out = '0';
				break;
			case 1477:
				key_out = '#';
				break;
			case 1633:
				key_out = 'D';
				break;
			}
			break;
		}

		System_printf("\nKey detected: %c\n", key_out);
		System_printf("Frequencies detected: %dHz and %dHz\n", F_out1, F_out2);
		System_flush();
	}
}
