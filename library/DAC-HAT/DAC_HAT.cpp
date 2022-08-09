//
// Created by Genki Sasaki on 2020/12/10.
//
#include <stdio.h>
#include "mbed.h"
#include "DAC_HAT.h"
#include "AD5754R.h"

DAC_HAT::DAC_HAT() :
    rled(PF_2),
    gled(PE_5),
    bled(PE_4),
    uled(PE_2),
    rgb_r(PE_6),
    rgb_g(PF_9),
    rgb_b(PF_8),
    sw1(PG_3),
    sw2(PG_2),
    dac(PC_12, PC_11, PC_10, PA_0, PH_1, PA_15) //rev1.03
//AD5754R(PinName _mosi, PinName _miso, PinName _sclk, PinName _sync, PinName _ldac, PinName _clr);
    {

}

void DAC_HAT::init(){

    /* set like this */
    dac.ControlFunction(AD5754R::CLAMP_ENABLE, 0x00);
    dac.ControlFunction(AD5754R::TSD_ENABLE, 0x00);

    /* confirm like this */
    printf(dac.getControlString());

    /* Set coding mode */
    dac.clear();

    /* Power Up like this */
    dac.SetPower(static_cast<AD5754R::PU_registers>(AD5754R::PU_A | AD5754R::PU_B
                                                    | AD5754R::PU_REF), true);
    printf(dac.getPowerControlString());

    /* dac Normal Initialization */
    dac.setControl();
    dac.SetPower(AD5754R::PU_ALL_REF, true);
    dac.setOutputRange(AD5754R::ADDRESS_ALL, AD5754R::BIPOLAR_10V);

    printf("\n\r");
    printf(dac.getPowerControlString());
    printf(dac.getAlertString());
    printf(dac.getControlString());
    printf(dac.getOutputRangeString(AD5754R::ADDRESS_A));
}

void DAC_HAT::set_value(int set_address, float value){
    switch (set_address) {
        case 0:
            dac.setValue(AD5754R::ADDRESS_A, value);
            break;
        case 1:
            dac.setValue(AD5754R::ADDRESS_B, value);
            break;
        case 2:
            dac.setValue(AD5754R::ADDRESS_C, value);
            break;
        case 3:
            dac.setValue(AD5754R::ADDRESS_D, value);
            break;
        case 4:
            dac.setValue(AD5754R::ADDRESS_ALL, value);
            break;
        default:
            printf("The set address is not allowed.\n\r");
            break;

    }
}

void DAC_HAT::set_raw_value(int set_address, uint16_t raw_value){
    switch (set_address) {
        case 0:
            dac.setRawValue(AD5754R::ADDRESS_A, raw_value);
            break;
        case 1:
            dac.setRawValue(AD5754R::ADDRESS_B, raw_value);
            break;
        case 2:
            dac.setRawValue(AD5754R::ADDRESS_C, raw_value);
            break;
        case 3:
            dac.setRawValue(AD5754R::ADDRESS_D, raw_value);
            break;
        case 4:
            dac.setRawValue(AD5754R::ADDRESS_ALL, raw_value);
            break;
        default:
            printf("The set address is not allowed.\n\r");
            break;

    }
}

void DAC_HAT::set_rgb_led(double red, double green, double blue) {
    if(red > 1.0 || green > 1.0 || blue > 1.0)
        printf("Set value is out of range");

    rgb_r = 1.0 - red;
    rgb_g = 1.0 - green;
    rgb_b = 1.0 - blue;
}

