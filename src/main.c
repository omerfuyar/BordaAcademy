#include <stdio.h>
#include "devices/lcdDriver.h"
#include "devices/soundDriver.h"
#include "rtosCommon.h"
#include "esp_log.h"
#include "esp_err.h"

#define LOG_HEADER "TEST_APP"

#define SENSOR_HUMIDITY_ADDRESS 0x38
#define SENSOR_BAROMETER_ADDRESS 0x77
#define SENSOR_LIGHT_ADDRESS 0x23

#define LCD_STRING_BUFFER_SIZE 16
#define haltIfError(function, ...)                                                                            \
    do                                                                                                        \
    {                                                                                                         \
        int _err = (function);                                                                                \
        if (_err != ESP_OK)                                                                                   \
        {                                                                                                     \
            ESP_LOGE(LOG_HEADER, "Error in function: %s -> %s (%d)", #function, esp_err_to_name(_err), _err); \
            __VA_ARGS__                                                                                       \
            return;                                                                                           \
        }                                                                                                     \
    } while (0)

void app_main(void)
{
    ESP_LOGI(LOG_HEADER, "Initializing I2C Master...");
    haltIfError(i2cInitMaster());

    ESP_LOGI(LOG_HEADER, "Initializing LCD...");
    haltIfError(lcdInitialize());
    haltIfError(lcdMoveCursor(0, 0));
    haltIfError(lcdSendString("Sound Level:"));

    ESP_LOGI(LOG_HEADER, "Initializing ADC for Sound Sensor...");
    haltIfError(soundSensorInit());

    int soundSensorRawVal = 0;
    char lcdStrBuf[LCD_STRING_BUFFER_SIZE] = {0};

    ESP_LOGI(LOG_HEADER, "Starting main loop...");
    while (1)
    {
        haltIfError(soundSensorRead(&soundSensorRawVal));

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "Value: %-4d", soundSensorRawVal);
        haltIfError(lcdMoveCursor(1, 0));
        haltIfError(lcdSendString(lcdStrBuf));

        vTaskDelay(tfms(1));
    }
}