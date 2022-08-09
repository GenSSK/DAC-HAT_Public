//
// Created by Genki Sasaki on 2020/12/10.
//

#ifndef MBED_OS2__DAC_BOARD_H_
#define MBED_OS2__DAC_BOARD_H_

#include "AD5754R.h"

class DAC_HAT {
 private:
    AD5754R dac;
 public:
    DAC_HAT();
    void init();
    void set_value(int set_address, float value);
    void set_raw_value(int set_address, uint16_t raw_value);
    void set_rgb_led(double red, double green, double blue);

    /* PIN Settings */
    DigitalOut rled;
    DigitalOut gled;
    DigitalOut bled;
    DigitalOut uled;
    PwmOut rgb_r;
    PwmOut rgb_g;
    PwmOut rgb_b;
    DigitalIn sw1;
    DigitalIn sw2;
};

#endif //MBED_OS2__DAC_BOARD_H_
