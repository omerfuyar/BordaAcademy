#include "lcdDriver.h"
#include "rtosCommon.h"
#include "driver/i2c.h"

int lcdSendCommand(uint8_t cmd)
{
    uint8_t data_l = (cmd & 0xF);
    uint8_t data_u = ((cmd >> 4) & 0xF);

    // todo remove bl from here
    uint8_t data_t[4] = {
        data_u | 0x0C, // 0000 1100 : Backlight=1, En=1, , Rs=0
        data_u | 0x08, // 0000 1000 : Backlight=1, En=0, , Rs=0
        data_l | 0x0C, // 0000 1100 : Backlight=1, En=1, , Rs=0
        data_l | 0x08  // 0000 1000 : Backlight=1, En=0, , Rs=0
    };

    int result = 0;
    returnIfError(i2c_master_write_to_device(I2C_MASTER_NUM, I2C_ADDR_LCD, data_t, 4, tfus(1000)));

    vTaskDelay(tfus(2000));

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
    returnIfError(i2c_master_write_to_device(I2C_MASTER_NUM, I2C_ADDR_LCD, data_t, 4, tfus(1000)));

    vTaskDelay(tfus(2000));

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

int lcdInitialize(void)
{
    vTaskDelay(tfus(50000));

    uint8_t data[3] = {
        0x30 | 0x0C,  // 0000 1100 : Backlight=1, En=1,
        0x30 | 0x08}; // 0000 1000 : Backlight=1, En=0,

    vTaskDelay(tfus(5000));

    // Command 0x20 (Set to 4-bit mode)
    uint8_t data_4bit[3] =
        {0x20 | 0x0C,
         0x20 | 0x08};

    vTaskDelay(tfus(200));

    int result = 0;
    returnIfError(lcdSendCommand(0x28)); // 2 lines, 5x8 style
    returnIfError(lcdSendCommand(0x08)); // Display off
    returnIfError(lcdSendCommand(0x01)); // Clear display (this command takes the longest!)
    vTaskDelay(tfus(2500));              // Mandatory 2ms delay after clear

    returnIfError(lcdSendCommand(0x06)); // Entry mode (increment cursor)
    returnIfError(lcdSendCommand(0x0C)); // Display on, cursor off

    return ESP_OK;
}

int lcdMoveCursor(int row, int col)
{
    uint8_t cmd = (row == 0) ? (0x80 | col) : (0xC0 | col);

    return lcdSendCommand(cmd);
}