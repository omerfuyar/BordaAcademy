#pragma once
#include "i2cDriver.h"

// lcd is preconfigured for this project
#define I2C_ADDR_LCD 0x27 // LCD1602 display PCF8574 controller (i2c expander)

int lcdSendCommand(uint8_t cmd);

int lcdSendByte(uint8_t byte);

int lcdSendString(const char *str);

int lcdInitialize(void);

int lcdMoveCursor(int row, int col);