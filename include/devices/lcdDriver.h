#pragma once
#include "i2cDriver.h"

// lcd is preconfigured for this project
#define LCD_I2C_ADDRESS 0x27 // LCD1602 display PCF8574 controller (i2c expander)

typedef enum LCDCommand
{
    // LCD Commands
    LCDCommand_ClearDisplay = 0x01,
    LCDCommand_ReturnHome = 0x02,

    // Entry Mode
    LCDCommand_EntryModeDecrement = 0x04,
    LCDCommand_EntryModeIncrement = 0x06,
    LCDCommand_EntryModeIncrementShift = 0x07,

    // Display Control
    LCDCommand_DisplayOff = 0x08,
    LCDCommand_DisplayOn = 0x0C,
    LCDCommand_DisplayOnCursor = 0x0E,
    LCDCommand_DisplayOnCursorBlink = 0x0F,

    // Cursor/Display Shift
    LCDCommand_CursorShiftLeft = 0x10,
    LCDCommand_CursorShiftRight = 0x14,
    LCDCommand_DisplayShiftLeft = 0x18,
    LCDCommand_DisplayShiftRight = 0x1C,

    // Function Set
    LCDCommand_Function4B2L = 0x28, // 4-bit mode, 2 lines, 5x8 dots, default
    LCDCommand_Function8B1L = 0x30  // wake up
} LCDCommand;

int lcdInitialize(void);

int lcdSendCommand(uint8_t cmd);

int lcdSendByte(uint8_t byte);

int lcdSendString(const char *str);

int lcdMoveCursor(uint8_t row, uint8_t col);
