//
// Created by Kota Kobayashi on 2020/07/09.
//

#include "AD5754R.h"

AD5754R::AD5754R(PinName _mosi, PinName _miso, PinName _sclk, PinName _sync, PinName _ldac, PinName _clr) :
    SPI(_mosi, _miso, _sclk),
    sync(_sync),
    ldac(_ldac),
    clr(_clr) {
  ldac = 0;
  clr = 1;
//     TODO: make the frequency in an argument?
  frequency(20000000);
//     8 bits a frame
//     mode 2: ClockPolarity 1 ClockPhase 1
  format(8, 2);
  disableChipSelect();
  clear();
}

/***************************************************************************//**
 * @brief Enables/disables indicated DAC channels.
 *
 * @param dacChannel - DAC channel or channels to be enabled/disabled.<BR>
 *                     Example: PU_A - Channel A.<BR>
 *                              PU_B - Channel B.<BR>
 *                              PU_C - Channel C.<BR>
 *                              PU_D - Channel D.<BR>
 *                              PU_ALL - Channel ALL.<BR>
 *                              PU_REF - Internal Reference.<BR>
 *                              PU_ALL_REF - Channel ALL and Internal Reference.
 * @param isON - Enable or disable channel.<BR>
 *                Example: false - Off.<BR>
 *                         true  - On.
 *
 * @return None.
*******************************************************************************/
void AD5754R::SetPower(PU_registers dacChannel, bool isON) {
  uint16_t oldPowerCtl;
  uint16_t newPowerCtl;
  oldPowerCtl = GetRegisterValue(REG_POWER_CONTROL, 0x00);
  newPowerCtl = oldPowerCtl & ~dacChannel;
  newPowerCtl = newPowerCtl | (isON * dacChannel);
  SetRegisterValue(REG_POWER_CONTROL, static_cast<registers>(0x00), newPowerCtl);
}

/***************************************************************************//**
 * @brief Set the output range for the addresses.<BR>
 *        example : setOutputRange( AD5754R::ADDRESS_ALL, AD5754R::BIPOLAR_10V );
 *
 * @param address - DAC channel or channels to be written.
 * @param range - Range value in outputRnges
 *
 * @return None.
*******************************************************************************/
void AD5754R::setOutputRange(channels address, outputRanges range) {
  uint8_t a = (uint) 8 | address;
  uint8_t b = 0;
  uint8_t c = range; // range & 0x7;
  send(a, b, c);
}

/***************************************************************************//**
 * @brief Writes data into a DAC channel register.
 *
 * @param dacChannel - DAC channel or channels to be written.
 * @param dacValue - Value written in DAC register with 16bit value
 *                   Example : Bipolar +-10V<br>
 *                             0xFFFF - +10.0v<br>
 *                             0x8000 - +-0.0v<br>
 *                             0x0000 - -10.0v
 *
 * @return None.
*******************************************************************************/
void AD5754R::setRawValue(channels address, uint16_t value) {
  send(address, (uint8_t) (value >> (uint) 8) & (uint) 0xFF, (uint8_t) (value) & (uint) 0xFF);
}

/***************************************************************************//**
 * @brief Writes -1.0 to 1.0 data into a DAC channel register.
 *
 * @param dacChannel - DAC channel or channels to be written.
 * @param dacValue - Value written in DAC register with float or double
 *
 * @return None.
*******************************************************************************/
void AD5754R::setValue(channels address, float float_value) {
  uint16_t value = 0;
#ifndef UNIPOLAR
#ifndef OFSET_BIN
  /* Twos comp and Bipolar */
  if (float_value >= 1.0f) {
    value = 0x7FFF;
  } else if (float_value <= -1.0f) {
    value = 0x8000;
  } else if (float_value >= 0.0f) {
    value = (uint16_t) ((float_value) * 0x7FFF);
  } else {
    value = (uint16_t) ((fabs(float_value)) * 0x7FFF);
    value = 0xFFFF - value;
  }

#else //OFSET_BIN
  /* Binary and Bipolar */
  if (float_value >= 1.0f) {
    value = 0xFFFF;
  } else if (float_value <= -1.0f) {
    value = 0x0000;
  } else {
    value = (uint16_t)((float_value + 1.0f) * 0x8000);
  }
#endif //OFSET_BIN

#else //UNIPOLAR
  /* Unipolar */
  if (float_value >= 1.0f) {
      value = 0xFFFF;
  } else if (float_value <= 0.0f) {
      value = 0x0000;
  } else {
      value = (uint16_t)((float_value + 1.0f) * 0xFFFF);
  }
#endif //UNIPOLAR
  send(address, (uint8_t) (value >> 8) & 0xFF, (uint8_t) (value) & 0xFF);
}

/***************************************************************************//**
 * @brief Read alert bits form Power Control Register.
 *
 * @param None.
 *
 * @return powerCtlStatus - Result of the Power Control Register read.<br>
 *                          Contains alert bits: OCd, OCc, OCb, OCa and TSD.<br>
 *                          Example: 0x01 - TSD is set<br>
 *                                   0x05 - OCa and TSD are set<br>
 *                                   0x3C - OCd, OCc, OCb, OCa are set.
 *
*******************************************************************************/
uint8_t AD5754R::ReadAlertBits() {
  uint16_t powerCtlStatus = 0;

  powerCtlStatus = GetRegisterValue(REG_POWER_CONTROL,
                                    0x00);
  powerCtlStatus = (powerCtlStatus >> 5);
  powerCtlStatus &= 0x003F;               // Only bits 5 to 0 are required.
  return ((uint8_t) powerCtlStatus);
}

/***************************************************************************//**
 * @brief Writes data into a register.
 *
 * @param regBits - Address of the register.
 * @param adrBits - Address of a channel or of an option.
 * @param registerValue - Data value to write.
 *
 * @return None.
*******************************************************************************/
void AD5754R::SetRegisterValue(registers regBits, registers adrBits, uint16_t registerValue) {
  //TODO: uint8_t to enum (for type safety)
  uint8_t registerWord[4] = {0};
  registerWord[0] = 0x01;   // AD5754R CS.
  registerWord[1] = (regBits << (uint) 3) + adrBits;
  registerWord[2] = (registerValue & (uint) 0xFF00) >> (uint) 8;
  registerWord[3] = (registerValue & (uint) 0x00FF) >> (uint) 0;
//  printf("send3 : %lx|%lx|%lx\n\r", registerWord[1], registerWord[2], registerWord[3]);
  send(registerWord[1], registerWord[2], registerWord[3]);
}

/***************************************************************************//**
 * @brief Reads the value of a register.
 *
 * @param registerAddress - Address of the register.
 *
 * @return *dataPointer - Value of the register.
*******************************************************************************/
uint32_t AD5754R::GetRegisterValue(uint8_t regBits, uint8_t adrBits) {
  //TODO: uint8_t to enum (for type safety)
  uint8_t registerWord[3] = {0};
  registerWord[0] = ((uint) 1 << (uint8_t) 7) + (regBits << (uint) 3) + adrBits;
  registerWord[1] = 0;
  registerWord[2] = 0;
  send(registerWord[0], registerWord[1], registerWord[2]);
  return (transferNop());
}

/***************************************************************************//**
 * @brief Selects an option of Control Register.
 *
 * @param bitValue - indicates a set or a clear of the following
 *                   bits: SDO disable
 *                         CRL select
 *                         Clamp enable
 *                         TSD enable
 * @param option - set value 0x00 or 0x01
 *
 * @return None.
*******************************************************************************/
void AD5754R::ControlFunction(control option, uint8_t bitValue) {
  unsigned char adrBits = option & 0xF0;
  unsigned char dataBits = option & 0X0F;
  unsigned short oldControl = 0;
  unsigned short newControl = 0;
  adrBits = adrBits >> 4;
  if (adrBits == 0x01) {
    oldControl = GetRegisterValue(REG_CONTROL, 0x01);
    newControl = oldControl & ~(dataBits);
    newControl = newControl | (bitValue * dataBits);
  }
  SetRegisterValue(REG_CONTROL, static_cast<registers>(adrBits), newControl);
}

/***************************************************************************//**
 * @brief To enable the thermal shutdown feature and the current limit clamp.
 * @return None.
*******************************************************************************/
void AD5754R::setControl() {
  SetRegisterValue(REG_CONTROL, AD5754R::REG_A0, 0xC);
  // TSD termal shutdown + clamp enable
}

/***************************************************************************//**
 * @brief Shutdown all of Dac. And clear registers.
 * @return None.
*******************************************************************************/
void AD5754R::clear() {
  //TODO:何故かクリアできない．直せたら直そう
  SetRegisterValue(REG_CONTROL, REG_A2, 0x00);
  SetRegisterValue(REG_POWER_CONTROL, static_cast<registers>(0x00), 0x00);
}

//Private Methods

/***************************************************************************//**
 * @brief Send 3 bytes to the dac. Discard returned values.
 * @return None.
*******************************************************************************/
void AD5754R::send(uint8_t a, uint8_t b, uint8_t c) {
  enableChipSelect();
  write(a);
  write(b);
  write(c);
  disableChipSelect();
}

/***************************************************************************//**
 * @brief Send a NOP to receive the output of the previous command.
 * @return: a 32bit unsigned int with the received 3 bytes.
*******************************************************************************/
uint32_t AD5754R::transferNop() {
  enableChipSelect();
  auto aa = (uint8_t) write(0x18);
  auto ab = (uint8_t) write(0x00);
  auto ac = (uint8_t) write(0x00);

  auto result = (uint32_t) aa;
  result = result << (uint) 8;
  result |= (uint32_t) ab;
  result = result << (uint) 8;
  result |= ac;
  disableChipSelect();
  return result;
}

/***************************************************************************//**
 * @brief Enable chip select.(SYNC)
 * @return: None.
*******************************************************************************/
void AD5754R::enableChipSelect() {
  sync = 0;
}

/***************************************************************************//**
 * @brief Disable chip select.(SYNC)
 * @return: None.
*******************************************************************************/
void AD5754R::disableChipSelect() {
  sync = 1;
}

//Debug String Methods
/***************************************************************************//**
 * @brief returns the PowerUP Dac string.
 * @return char[100]
 *         Print like this printf(dac.getPowerControlString());
*******************************************************************************/
char *AD5754R::getPowerControlString() {
  /* AD5754R https://www.analog.com/media/en/technical-documentation/data-sheets/AD5724R_5734R_5754R.pdf
  |R/W |Zero|Reg2|Reg1|Reg0 | A2| A1| A0| | DB15-DB11|10 |  9|  8| |  7|  6|  5|    4|  3|  2|  1|  0|
  |0   |0   |0   |1   |0    |0  |0  |0  | | ---------|OCd|OCb|OCb| |OCa|0  |TSD|PUref|PUd|PUa|PUb|PUa|
  */
  uint16_t readBits = GetRegisterValue(AD5754R::REG_POWER_CONTROL, 0x00);
  static char message[200];
  memset(message, '\0', sizeof(message));
  sprintf(message, "PowerUp:");
  if (readBits & (uint) 0x1) {
    sprintf(message, "%s A|", message);
  }
  if (readBits & (uint) 0x2) {
    sprintf(message, "%s B|", message);
  }
  if (readBits & (uint) 0x4) {
    sprintf(message, "%s C|", message);
  }
  if (readBits & (uint) 0x8) {
    sprintf(message, "%s D|", message);
  }
  if (readBits & (uint) 0x10) {
    sprintf(message, "%s Ref", message);
  } else {
    sprintf(message,
            "%s\n\rInternal Reference is disabled.\n\rIf not to use external reference check power up ref.",
            message);
  }
  sprintf(message, "%s\n\r", message);
  return message;
}

/***************************************************************************//**
 * @brief returns the OverCurrent Alert dac.
 * @return char[100]
 *         Print like this printf(dac.getAlertString());
*******************************************************************************/
char *AD5754R::getAlertString() {
  /* AD5754R https://www.analog.com/media/en/technical-documentation/data-sheets/AD5724R_5734R_5754R.pdf
|R/W |Zero|Reg2|Reg1|Reg0 | A2| A1| A0| | DB15-DB11|10 |  9|  8| |  7|  6|  5|    4|  3|  2|  1|  0|
|0   |0   |0   |1   |0    |0  |0  |0  | | ---------|OCd|OCb|OCb| |OCa|0  |TSD|PUref|PUd|PUa|PUb|PUa|
*/
  uint16_t readBits = GetRegisterValue(AD5754R::REG_POWER_CONTROL, 0x00);
  static char message[100];
  memset(message, '\0', sizeof(message));
  sprintf(message, "Alert:");
  if (readBits & (uint) 0x40) {
    sprintf(message, "%s A|", message);
  }
  if (readBits & (uint) 0x80) {
    sprintf(message, "%s B|", message);
  }
  if (readBits & (uint) 0x100) {
    sprintf(message, "%s C|", message);
  }
  if (readBits & (uint) 0x200) {
    sprintf(message, "%s D|", message);
  }
  if (readBits & (uint) 0x20) {
    sprintf(message, "%s Thermal shutdown!!", message);
  }
  sprintf(message, "%s\n\r", message);
  return message;
}

/***************************************************************************//**
 * @brief returns the Dac Preferences info.
 * @return char[200]
 *         Print like this printf(dac.getControlString());
*******************************************************************************/
char *AD5754R::getControlString() {
  /* AD5754R https://www.analog.com/media/en/technical-documentation/data-sheets/AD5724R_5734R_5754R.pdf
|R/W |Zero|Reg2|Reg1|Reg0 | A2| A1| A0| | DB15-DB4 |         3|           2|         1|          0|
|1   |0   |0   |1   |1    |0  |0  |1  | | -------- |TSD enable|Clamp enable|CLR select|SDO disable|
*/
  printf("ReadControl\n\r");
  uint16_t readBits = GetRegisterValue(AD5754R::REG_CONTROL, AD5754R::REG_A0);
  static char message[200];
  memset(message, '\0', sizeof(message));
  sprintf(message, "Control Registers:");
  if (readBits & (uint) 0x1) {
    sprintf(message, "%s SDO disable|", message);
  }
  if (readBits & (uint) 0x4) {
    sprintf(message, "%s Clamp enable|", message);
  } else {
    sprintf(message, "%s Clamp disabled!!! This may cause damage to the Device. ", message);
  }
  if (readBits & (uint) 0x8) {
    sprintf(message, "%s TSD enable|", message);
  } else {
    sprintf(message, "%s TSD disabled!!! This may cause damage to the Device. ", message);
  }
  sprintf(message, "%s\n\r", message);
  if (readBits & (uint) 0x2) {
    sprintf(message, "%sUnipolar: Midscale Bipolar: Negative full scale", message);
  } else {
    sprintf(message, "%sUnipolar: 0V Bipolar: 0V", message);
  }
  sprintf(message, "%s\n\r", message);
  return message;
}

/***************************************************************************//**
 * @brief Query the output range of the given address.
 * @return char[200]
 *         Print like this printf(dac.getOutputRangeString());
*******************************************************************************/
char *AD5754R::getOutputRangeString(int address) {
//  TODO: めんどい&必要性がわからん
  uint16_t readBits = GetRegisterValue(AD5754R::REG_RANGE_SELECT, address);
  static char message[100];
  memset(message, '\0', sizeof(message));
  sprintf(message, "Range :");
  if (readBits & BIPOLAR_10V) {
    sprintf(message, "%s BIPOLAR_10V", message);
  }
  sprintf(message, "%s\n\r", message);
  return message;
}

uint32_t AD5754R::getPowerControl() {
  /* AD5754R https://www.analog.com/media/en/technical-documentation/data-sheets/AD5724R_5734R_5754R.pdf
|R/W |Zero|Reg2|Reg1|Reg0 | A2| A1| A0| | DB15-DB11|10 |  9|  8| |  7|  6|  5|    4|  3|  2|  1|  0|
|0   |0   |0   |1   |0    |0  |0  |0  | | ---------|OCd|OCb|OCb| |OCa|0  |TSD|PUref|PUd|PUa|PUb|PUa|
*/
  return GetRegisterValue(AD5754R::REG_POWER_CONTROL, 0x00);;
}

uint32_t AD5754R::getAlert() {
  /* AD5754R https://www.analog.com/media/en/technical-documentation/data-sheets/AD5724R_5734R_5754R.pdf
|R/W |Zero|Reg2|Reg1|Reg0 | A2| A1| A0| | DB15-DB11|10 |  9|  8| |  7|  6|  5|    4|  3|  2|  1|  0|
|0   |0   |0   |1   |0    |0  |0  |0  | | ---------|OCd|OCb|OCb| |OCa|0  |TSD|PUref|PUd|PUa|PUb|PUa|
*/
  return GetRegisterValue(AD5754R::REG_POWER_CONTROL, 0x00);
}

uint32_t AD5754R::getControl() {
  /* AD5754R https://www.analog.com/media/en/technical-documentation/data-sheets/AD5724R_5734R_5754R.pdf
|R/W |Zero|Reg2|Reg1|Reg0 | A2| A1| A0| | DB15-DB4 |         3|           2|         1|          0|
|1   |0   |0   |1   |1    |0  |0  |1  | | -------- |TSD enable|Clamp enable|CLR select|SDO disable|
*/
  return GetRegisterValue(AD5754R::REG_CONTROL, AD5754R::REG_A0);
}

uint32_t AD5754R::getOutputRange(int address) {
  return GetRegisterValue(AD5754R::REG_RANGE_SELECT, address);
}





