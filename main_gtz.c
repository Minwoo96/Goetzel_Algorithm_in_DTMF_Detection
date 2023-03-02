/*
 *  ======== gtz.c ========
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

void clk_SWI_Generate_DTMF(UArg arg0);

void clk_SWI_GTZ_0697Hz(UArg arg0); //function prototypes for the Goertzel algorithm functions: one to detect each frequency
void clk_SWI_GTZ_0770Hz(UArg arg0);
void clk_SWI_GTZ_0852Hz(UArg arg0);
void clk_SWI_GTZ_0941Hz(UArg arg0);
void clk_SWI_GTZ_1209Hz(UArg arg0);
void clk_SWI_GTZ_1336Hz(UArg arg0);
void clk_SWI_GTZ_1477Hz(UArg arg0);
void clk_SWI_GTZ_1633Hz(UArg arg0);


extern void task0_dtmfGen(void);
extern void task1_dtmfDetect(void);

extern char digit;
extern int sample, mag1, mag2, freq1, freq2, gtz_out[8];
extern short coef[8];

/*
 *  ======== main ========
 */void main(void) {
    System_printf("\n I am in main :\n");
    System_flush();
	/* Create a Clock Instance */
    Clock_Params clkParams;

    /* Initialise Clock Instance with time period and timeout (system ticks) */
    Clock_Params_init(&clkParams);
    clkParams.period = 1;
    clkParams.startFlag = TRUE;

    /* Instantiate ISR for tone generation  */
	Clock_create(clk_SWI_Generate_DTMF, TIMEOUT, &clkParams, NULL);

    /* Instantiate 8 parallel ISRs for each of the eight Goertzel coefficients */
	Clock_create(clk_SWI_GTZ_0697Hz, TIMEOUT, &clkParams, NULL);
	Clock_create(clk_SWI_GTZ_0770Hz, TIMEOUT, &clkParams, NULL);
	Clock_create(clk_SWI_GTZ_0852Hz, TIMEOUT, &clkParams, NULL);
	Clock_create(clk_SWI_GTZ_0941Hz, TIMEOUT, &clkParams, NULL);
	Clock_create(clk_SWI_GTZ_1209Hz, TIMEOUT, &clkParams, NULL);
	Clock_create(clk_SWI_GTZ_1336Hz, TIMEOUT, &clkParams, NULL);
	Clock_create(clk_SWI_GTZ_1477Hz, TIMEOUT, &clkParams, NULL);
	Clock_create(clk_SWI_GTZ_1633Hz, TIMEOUT, &clkParams, NULL);

	/* Start SYS_BIOS */
    BIOS_start();
}

/*
 *  ====== clk0Fxn =====
 *  Dual-Tone Generation
 *  ====================
 */
void clk_SWI_Generate_DTMF(UArg arg0) {
	static int tick;

	tick = Clock_getTicks();

	sample = (int) 32768.0*sin(2.0*PI*freq1*TICK_PERIOD*tick) + 32768.0*sin(2.0*PI*freq2*TICK_PERIOD*tick); //Frequencies determined by input key. (See util.c)
	sample = sample >> 12;
}

/*
 *  ====== clk_SWI_GTZ =====
 *  gtz sweep
 *  ====================
 */
void clk_SWI_GTZ_0697Hz(UArg arg0) {

	/*
	 * Feedback Component
	 * Qn = x(n) + coef * Q(n-1) - Q(n-2)
	 * coef = 2 * cos(2 * pi * k / 205 )
	 * coef are pre-calculated and stored in coef[8] in util.c
	*/
   	static int N = 0; //Number of feedback iterations
   	static int Goertzel_Value = 0; //Reset Goertzel Value

   	static short delay; //Q(n)
   	static short delay_1 = 0; //Q(n-1)
   	static short delay_2 = 0; //Q(n-2)
   	//Static variable used so there is "memory" between iterations, while the variable are only initialised once
   	//These variable are not global!

   	int prod1, prod2, prod3;
   	short input, coef_1 = coef[0]; // For detecting 697 Hz

   	input = (short) sample; 				//Stores x(n) to input
   	prod1 = (delay_1*coef_1)>>14; 			//Finds coef * Q(n-1)
   	delay = input + (short)prod1 - delay_2; //Finds Q(n)
   	delay_2 = delay_1; 						//Saves Q(n-1) onto Q(n-2) for next cycle
   	delay_1 = delay; 						//Saves Q(n) onto Q(n-1) for next cycle
   	N++;									//Updates cycle number

   	/*
	 * Feedforward Component
	 * Y(n)^2 = Q(n)^2 + Q(n-1)^2 - coeff * Q(n) * Q(n - 1)
	 */
   	if (N==206) {										//Check if iterations are complete
   	prod1 = (delay_1 * delay_1);						//Finds Q(n)^2
   	prod2 = (delay_2 * delay_2);						//Finds Q(n-1)^2
   	prod3 = (delay_1 * coef_1)>>14;						//Finds coeff * Q(n) * Q(n - 1)
   	prod3 = prod3 * delay_2;							//^^^
   	Goertzel_Value = (prod1 + prod2 - prod3) >> 15;		//Finds Y(n)^2
   	Goertzel_Value <<= 8; 								//Scale up value for sensitivity
   	N = 0;
   	delay_1 = delay_2 = 0;
   	}

	gtz_out[0] = Goertzel_Value;
}

void clk_SWI_GTZ_0770Hz(UArg arg0) {
   	static int N = 0;
   	static int Goertzel_Value = 0;

   	static short delay;
   	static short delay_1 = 0;
   	static short delay_2 = 0;

   	int prod1, prod2, prod3;
   	short input, coef_1 = coef[1]; // For detecting 770 Hz

   	input = (short) sample;
   	prod1 = (delay_1*coef_1)>>14;
   	delay = input + (short)prod1 - delay_2;
   	delay_2 = delay_1;
   	delay_1 = delay;
   	N++;

   	if (N==206){
   	prod1 = (delay_1 * delay_1);
   	prod2 = (delay_2 * delay_2);
   	prod3 = (delay_1 * coef_1)>>14;
   	prod3 = prod3 * delay_2;
   	Goertzel_Value = (prod1 + prod2 - prod3) >> 15;
   	Goertzel_Value <<= 8;
   	N = 0;
   	delay_1 = delay_2 = 0;
   	}

	gtz_out[1] = Goertzel_Value;
}

void clk_SWI_GTZ_0852Hz(UArg arg0) {
   	static int N = 0;
   	static int Goertzel_Value = 0;

   	static short delay;
   	static short delay_1 = 0;
   	static short delay_2 = 0;

   	int prod1, prod2, prod3;
   	short input, coef_1 = coef[2]; // For detecting 852 Hz

   	input = (short) sample;
   	prod1 = (delay_1*coef_1)>>14;
   	delay = input + (short)prod1 - delay_2;
   	delay_2 = delay_1;
   	delay_1 = delay;
   	N++;

   	if (N==206){
   	prod1 = (delay_1 * delay_1);
   	prod2 = (delay_2 * delay_2);
   	prod3 = (delay_1 * coef_1)>>14;
   	prod3 = prod3 * delay_2;
   	Goertzel_Value = (prod1 + prod2 - prod3) >> 15;
   	Goertzel_Value <<= 8;
   	N = 0;
   	delay_1 = delay_2 = 0;
   	}

	gtz_out[2] = Goertzel_Value;
}

void clk_SWI_GTZ_0941Hz(UArg arg0) {
   	static int N = 0;
   	static int Goertzel_Value = 0;

   	static short delay;
   	static short delay_1 = 0;
   	static short delay_2 = 0;

   	int prod1, prod2, prod3;
   	short input, coef_1 = coef[3]; // For detecting 941 Hz

   	input = (short) sample;
   	prod1 = (delay_1*coef_1)>>14;
   	delay = input + (short)prod1 - delay_2;
   	delay_2 = delay_1;
   	delay_1 = delay;
   	N++;

   	if (N==206){
   	prod1 = (delay_1 * delay_1);
   	prod2 = (delay_2 * delay_2);
   	prod3 = (delay_1 * coef_1)>>14;
   	prod3 = prod3 * delay_2;
   	Goertzel_Value = (prod1 + prod2 - prod3) >> 15;
   	Goertzel_Value <<= 8;
   	N = 0;
   	delay_1 = delay_2 = 0;
   	}

	gtz_out[3] = Goertzel_Value;
}

void clk_SWI_GTZ_1209Hz(UArg arg0) {
   	static int N = 0;
   	static int Goertzel_Value = 0;

   	static short delay;
   	static short delay_1 = 0;
   	static short delay_2 = 0;

   	int prod1, prod2, prod3;
   	short input, coef_1 = coef[4]; // For detecting 1209 Hz

   	input = (short) sample;
   	prod1 = (delay_1*coef_1)>>14;
   	delay = input + (short)prod1 - delay_2;
   	delay_2 = delay_1;
   	delay_1 = delay;
   	N++;

   	if (N==206){
   	prod1 = (delay_1 * delay_1);
   	prod2 = (delay_2 * delay_2);
   	prod3 = (delay_1 * coef_1)>>14;
   	prod3 = prod3 * delay_2;
   	Goertzel_Value = (prod1 + prod2 - prod3) >> 15;
   	Goertzel_Value <<= 8;
   	N = 0;
   	delay_1 = delay_2 = 0;
   	}

	gtz_out[4] = Goertzel_Value;
}

void clk_SWI_GTZ_1336Hz(UArg arg0) {
   	static int N = 0;
   	static int Goertzel_Value = 0;

   	static short delay;
   	static short delay_1 = 0;
   	static short delay_2 = 0;

   	int prod1, prod2, prod3;
   	short input, coef_1 = coef[5]; // For detecting 1336 Hz

   	input = (short) sample;
   	prod1 = (delay_1*coef_1)>>14;
   	delay = input + (short)prod1 - delay_2;
   	delay_2 = delay_1;
   	delay_1 = delay;
   	N++;

   	if (N==206){
   	prod1 = (delay_1 * delay_1);
   	prod2 = (delay_2 * delay_2);
   	prod3 = (delay_1 * coef_1)>>14;
   	prod3 = prod3 * delay_2;
   	Goertzel_Value = (prod1 + prod2 - prod3) >> 15;
   	Goertzel_Value <<= 8;
   	N = 0;
   	delay_1 = delay_2 = 0;
   	}

	gtz_out[5] = Goertzel_Value;
}

void clk_SWI_GTZ_1477Hz(UArg arg0) {
   	static int N = 0;
   	static int Goertzel_Value = 0;

   	static short delay;
   	static short delay_1 = 0;
   	static short delay_2 = 0;

   	int prod1, prod2, prod3;
   	short input, coef_1 = coef[6]; // For detecting 1477 Hz

   	input = (short) sample;
   	prod1 = (delay_1*coef_1)>>14;
   	delay = input + (short)prod1 - delay_2;
   	delay_2 = delay_1;
   	delay_1 = delay;
   	N++;

   	if (N==206){
   	prod1 = (delay_1 * delay_1);
   	prod2 = (delay_2 * delay_2);
   	prod3 = (delay_1 * coef_1)>>14;
   	prod3 = prod3 * delay_2;
   	Goertzel_Value = (prod1 + prod2 - prod3) >> 15;
   	Goertzel_Value <<= 8;
   	N = 0;
   	delay_1 = delay_2 = 0;
   	}

	gtz_out[6] = Goertzel_Value;
}

void clk_SWI_GTZ_1633Hz(UArg arg0) {
   	static int N = 0;
   	static int Goertzel_Value = 0;

   	static short delay;
   	static short delay_1 = 0;
   	static short delay_2 = 0;

   	int prod1, prod2, prod3;
   	short input, coef_1 = coef[7]; // For detecting 1633 Hz

   	input = (short) sample;
   	prod1 = (delay_1*coef_1)>>14;
   	delay = input + (short)prod1 - delay_2;
   	delay_2 = delay_1;
   	delay_1 = delay;
   	N++;

   	if (N==206){
   	prod1 = (delay_1 * delay_1);
   	prod2 = (delay_2 * delay_2);
   	prod3 = (delay_1 * coef_1)>>14;
   	prod3 = prod3 * delay_2;
   	Goertzel_Value = (prod1 + prod2 - prod3) >> 15;
   	Goertzel_Value <<= 8;
   	N = 0;
   	delay_1 = delay_2 = 0;
   	}

	gtz_out[7] = Goertzel_Value;
}
