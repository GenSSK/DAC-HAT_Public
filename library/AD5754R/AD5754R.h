//
// Created by Kota Kobayashi on 2020/07/09.
//

#ifndef MBED_OS2__AD5754R_H_
#define MBED_OS2__AD5754R_H_

#include "mbed.h"
#include <string>

class AD5754R : private SPI {
 public:
  enum outputRanges {
    UNIPOLAR_5V = 0x00, // 0..+5(V)
    UNIPOLAR_10V = 0x01, // 0..+10(V)
    UNIPOLAR_10_8V = 0x02, // 0..+10.8(V)
    BIPOLAR_5V = 0x03, // -5..+5(V)
    BIPOLAR_10V = 0x04, // -10...+10(V)
    BIPOLAR_10_8V = 0x05  // -10.8...+10.8(V)
  };
  enum channels {
    ADDRESS_A = 0x00,   // Address of channel A
    ADDRESS_B = 0x01,   // Address of channel B
    ADDRESS_C = 0x02,   // Address of channel C
    ADDRESS_D = 0x03,   // Address of channel D
    ADDRESS_ALL = 0x04,  // All four DACs
  };
  enum PU_registers {
    PU_A = 0x1,
    PU_B = 0x2,
    PU_C = 0x4,
    PU_D = 0x8,
    PU_REF = 0x10,
    PU_ALL = 0xF,
    PU_ALL_REF = 0x1F
  };
  enum registers {
    REG_A2 = 4,
    REG_A1 = 2,
    REG_A0 = 1,

    REG_DAC = 0x00, // DAC register
    REG_RANGE_SELECT = 0x01, // Output range select register
    REG_POWER_CONTROL = 0x02, // Power control register
    REG_CONTROL = 0x03,  // Control register
  };
//TODO:この辺のレジスタの使い方がまだ曖昧
  enum control {
    NOP_OPERATION = 0x00, // No operation instruction used in readback operations.
    SDO_DISABLE = 0x11, // Set by the user to disable the SDO output.
    CLR_SELECT = 0x12, // Selects the clear code: 0V, Midscale or Negative full scale.
    CLAMP_ENABLE = 0x14, // Set by the user to enable the current-limit clamp.
    TSD_ENABLE = 0x18, // Set by the user to enable the thermal shutdown feature.
    CLEAR = 0x40, // Sets the DAC registers to the clear code and updates the outputs.
    LOAD = 0x50 // Updates the DAC registers and, consequently, the DAC outputs.
  };

  AD5754R(PinName _mosi, PinName _miso, PinName _sclk, PinName _sync, PinName _ldac, PinName _clr);
  void SetPower(PU_registers dacChannel, bool isON);
  void setOutputRange(channels address, outputRanges range);
  void setRawValue(channels address, uint16_t value);
  void setValue(channels address, float float_value);
  void SetRegisterValue(registers regBits, registers adrBits, uint16_t registerValue);
  uint8_t ReadAlertBits();
  uint32_t GetRegisterValue(uint8_t regBits, uint8_t adrBits);
  void ControlFunction(control option, uint8_t bitValue);
  void setControl();
  void clear();
 private:
  void send(uint8_t a, uint8_t b, uint8_t c);
  uint32_t transferNop();
  void enableChipSelect();
  void disableChipSelect();

  DigitalOut sync;
  DigitalOut clr;
  DigitalOut ldac;
 public:
  //for Debug String
  char *getPowerControlString();
  char *getAlertString();
  char *getControlString();
  char *getOutputRangeString(int address);

  //TODO: make doxygen below
  uint32_t getPowerControl();
  uint32_t getAlert();
  uint32_t getControl();
  uint32_t getOutputRange(int address);
};

#endif //MBED_OS2__AD5754R_H_
