#include <stdio.h>
#include "lcdDriver.h"
#include "analogDriver.h"
#include "esp_log.h"

#define LCD_STRING_BUFFER_SIZE 16

const char *LOG_HEADER = "TEST_APP";

void app_main(void)
{
    ESP_LOGI(LOG_HEADER, "Initializing I2C Master...");

    i2cInitMaster();

    ESP_LOGI(LOG_HEADER, "Initializing LCD...");

    lcdInitialize();
    lcdMoveCursor(0, 0);
    lcdSendString("Sound Level:");

    ESP_LOGI(LOG_HEADER, "Initializing ADC for Sound Sensor...");

    int soundSensorRawVal = 0;
    char lcdStrBuf[LCD_STRING_BUFFER_SIZE] = {0};

    while (1)
    {
        ESP_ERROR_CHECK(analogReadSoundSensor(&soundSensorRawVal));

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "Value: %-4d", soundSensorRawVal);
        lcdMoveCursor(1, 0);
        lcdSendString(lcdStrBuf);

        vTaskDelay();
    }
}