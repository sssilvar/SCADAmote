/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "stdlib.h"
#include "string.h"
#include "test.h"

#include <math.h>


#define ADC_GRP1_NUM_CHANNELS   1
#define ADC_GRP1_BUF_DEPTH      4
#define ADC_ARRAY_LENGH					1024

static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
adcsample_t data1 [ADC_ARRAY_LENGH];


static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;
}

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 * Channels:    IN0.
 */
static const ADCConversionGroup adcgrpcfg1 = {
  FALSE,                        //circular buffer mode
  ADC_GRP1_NUM_CHANNELS,        //Number of the analog channels
  NULL,                         //Callback function (not needed here)
  adcerrorcallback,             //Error callback
  0,                                        /* CR1 */
  ADC_CR2_SWSTART,                          /* CR2 */
  ADC_SMPR2_SMP_AN1(ADC_SAMPLE_3),         //sample times ch10-18
  0,                                        //sample times ch0-9
  ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS),   //SQR1: Conversion group sequence 13...16 + sequence length
  0,                                        //SQR2: Conversion group sequence 7...12
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN1)          //SQR3: Conversion group sequence 1...6
};


/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palSetPad(GPIOC, 13);       /* Orange.  */
    chThdSleepMilliseconds(1000);
    palClearPad(GPIOC, 13);     /* Orange.  */
    chThdSleepMilliseconds(1000);
  }
}

/*
 * DAQ Task
 * Using ADC
 */
static THD_WORKING_AREA(waADCThread1, 256);
static THD_FUNCTION(ADCThread1, arg) {

  (void)arg;
  int i=0;
  chRegSetThreadName("Va Samples");
  
	for(i=0; i<ADC_ARRAY_LENGH; i++) {
		adcStartConversion(&ADCD1, &adcgrpcfg1,samples1, ADC_GRP1_BUF_DEPTH);
		chThdSleepMicroseconds(1);
		data1[i] = (samples1[0] + samples1[1] + samples1[2] + samples1[3])/4;
	  }
   chThdSleepMilliseconds(5000);
}


/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  
  
  /*
   * Enabling interrupts, initialization done.
   */
  osalSysEnable();
  palSetPadMode(GPIOC, 13, PAL_MODE_OUTPUT_PUSHPULL);

  /*
   * Activates the serial driver 2 using the driver default configuration.
   * PA2(TX) and PA3(RX) are routed to USART2.
   */
  sdStart(&SD1, NULL);
   palSetPadMode(GPIOB, 6, PAL_MODE_ALTERNATE(7)); // UART1 TX
   palSetPadMode(GPIOB, 7, PAL_MODE_ALTERNATE(7)); // UART1 RX

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waADCThread1, sizeof(waADCThread1), NORMALPRIO, ADCThread1, NULL);
  
  
  /*
   * Initializes the ADC driver 1 and enable the thermal sensor.
   * The pin PC0 on the port GPIOC is programmed as analog input.
   */
  adcStart(&ADCD1, NULL);
  adcSTM32EnableTSVREFE();
  palSetPadMode(GPIOA, 1, PAL_MODE_INPUT_ANALOG);
  
	
    
  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
	  int i=0;
    
	for(i=0; i<ADC_ARRAY_LENGH; i++){
		//~ chprintf((BaseSequentialStream *) &SD1, "%d\r\n", (int)sqrt(pow((x[i]), 2) + pow(y[i],2)));
		chprintf((BaseSequentialStream *) &SD1, "%d\r\n", data1[i]);
		chThdSleepMilliseconds(10);
	}
	while(1);
}
}
