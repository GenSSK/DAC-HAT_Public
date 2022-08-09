//
// Created by Genki Sasaki on 2020/12/10.
//
#include "mbed.h"
#define __STDC_FORMAT_MACROS
#include "AD5754R.h"
#include "DAC_HAT.h"

int main() {
    printf("--------TEST ST@RT--------\n\r");

	/* I/O Configuration */
	DigitalOut my_led1(LED1);
	DigitalOut my_led2(LED2);
	DigitalOut my_led3(LED3);

	/* DAC-HAT settings */
	DAC_HAT hat{};
    hat.init(); // must call this initialize function

	float float_value = 0.0;
	float value[3] = {0.0f};
	float sinValue[3] = {0.0f};
	int count = 0;
	int val = 0;

    while (1) {
		/* signal generation */
	    value[0] += 1.0f;
	    value[1] += 1.1f;
	    value[2] += 1.0f;
	    sinValue[0] = sin(value[0]);
	    sinValue[1] = cos(value[1]);
	    sinValue[2] = sin(value[2]);


		/* DAC output setting with raw value */
	    if (hat.sw1) {
	        rawValue += 100;
	    }
	    if (hat.sw2) {
	        rawValue -= 100;
	    }
	    if (hat.sw1 and hat.sw2) {
	        rawValue = 0x0000;
	    }
	    hat.set_raw_value(4, rawValue); // set raw value to DAC-HAT

		/* DAC output setting with float value */
	    hat.set_value(0, sin(value[0] * 0.1));  // set float value to DAC-HAT channel A
	    hat.set_value(1, sin(value[0] * 0.2));  // set float value to DAC-HAT channel B
	    hat.set_value(2, sin(value[0] * 0.3));  // set float value to DAC-HAT channel C
	    hat.set_value(3, sin(value[0] * 0.4));  // set float value to DAC-HAT channel D
	    hat.set_value(4, sin(value[0] * 0.5));  // set float value to DAC-HAT All channels


		/* LED sample */
	    my_led1 = sinValue[0];
	    my_led2 = sinValue[0];
	    my_led3 = sinValue[0];
	    hat.rgb_r = sinValue[0];    // set RGB LED
	    hat.rgb_g = sinValue[1];    // set RGB LED
	    hat.rgb_b = sinValue[2];    // set RGB LED
	    hat.set_rgb_led(sinValue[0], sinValue[1], sinValue[2]); // set RGB LED
	    if (count > 100) {
	        hat.rled = val; // set red LED
	        hat.gled = !val;    // set green LED
	        hat.bled = val; // set blue LED
	        count = 0;
	        val = !val;
	    }

        wait_us(1000);
	    count++;
    }

    return 0;
}