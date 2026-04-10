#include "devices/lcdDriver.h"
#include "rtosCommon.h"

static int lcdSendNibble(uint8_t nibble_command)
{
    uint8_t data_u = (nibble_command & 0xF0);

    uint8_t data_t[2] = {
        data_u | 0x0C,
        data_u | 0x08};

    return i2cWriteToSlave(LCD_I2C_ADDRESS, data_t, 2);
}

int lcdInitialize(void)
{
    int result = 0;

    // try for 3 times
    for (int i = 0; i < 3; i++)
    {
        returnIfError(lcdSendNibble(LCDCommand_Function8B1L));
        vTaskDelay(tfms(5));
    }

    returnIfError(lcdSendNibble(LCDCommand_Function4B2L));
    vTaskDelay(tfms(5));

    returnIfError(lcdSendCommand(LCDCommand_Function4B2L));
    vTaskDelay(tfms(5));

    returnIfError(lcdSendCommand(LCDCommand_DisplayOff));
    returnIfError(lcdSendCommand(LCDCommand_ClearDisplay));
    vTaskDelay(tfms(50));

    returnIfError(lcdSendCommand(LCDCommand_EntryModeIncrement));
    returnIfError(lcdSendCommand(LCDCommand_DisplayOn));
    vTaskDelay(tfms(5));

    return ESP_OK;
}

int lcdSendCommand(uint8_t cmd)
{
    uint8_t data_u = (cmd & 0xF0);
    uint8_t data_l = ((cmd << 4) & 0xF0);

    // todo remove bl from here
    uint8_t data_t[4] = {
        data_u | 0x0C, // 0000 1100 : Backlight=1, En=1, , Rs=0
        data_u | 0x08, // 0000 1000 : Backlight=1, En=0, , Rs=0
        data_l | 0x0C, // 0000 1100 : Backlight=1, En=1, , Rs=0
        data_l | 0x08  // 0000 1000 : Backlight=1, En=0, , Rs=0
    };

    int result = 0;
    returnIfError(i2cWriteToSlave(LCD_I2C_ADDRESS, data_t, 4));

    vTaskDelay(tfms(2));

    return ESP_OK;
}

int lcdSendByte(uint8_t byte)
{
    uint8_t data_u = (byte & 0xF0);        // 1111 0000 : upper half
    uint8_t data_l = ((byte << 4) & 0xF0); // 0000 1111 : lower half

    uint8_t data_t[4] = {
        data_u | 0x0D, // 0000 1101 : Backlight=1, En=1, , Rs=1
        data_u | 0x09, // 0000 1001 : Backlight=1, En=0, , Rs=1
        data_l | 0x0D, // 0000 1101 : Backlight=1, En=1, , Rs=1
        data_l | 0x09  // 0000 1001 : Backlight=1, En=0, , Rs=1
    };

    int result = 0;
    returnIfError(i2cWriteToSlave(LCD_I2C_ADDRESS, data_t, 4));

    vTaskDelay(tfms(2));

    return ESP_OK;
}

int lcdSendString(const char *str)
{
    int i = 0;
    int result = 0;
    while (str[i] != '\0')
    {
        returnIfError(lcdSendByte(str[i]));
        i++;
    }

    return ESP_OK;
}

int lcdMoveCursor(uint8_t row, uint8_t col)
{
    uint8_t cmd = (row == 0) ? (0x80 | col) : (0xC0 | col);

    return lcdSendCommand(cmd);
}
