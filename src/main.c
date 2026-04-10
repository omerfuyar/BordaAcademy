#include <stdio.h>
#include "devices/lcdDriver.h"
#include "devices/soundDriver.h"
#include "devices/barometerDriver.h"
#include "devices/humTempDriver.h"
#include "devices/lightDriver.h"
#include "dataFilter.h"
#include "rtosCommon.h"
#include "esp_log.h"
#include "esp_err.h"

#define LOG_HEADER "TEST_APP"

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

    ESP_LOGI(LOG_HEADER, "Initializing Sensors...");
    haltIfError(barometerInit());
    haltIfError(lightInit());
    haltIfError(humTempInit());

    ESP_LOGI(LOG_HEADER, "Initializing ADC for Sound Sensor...");
    haltIfError(soundSensorInit());

    FilterQueueF temperatureData;
    FilterQueueF humidityData;
    FilterQueueF pressureData;
    FilterQueueF lightData;

    FilterQueue_Initialize(temperatureData);
    FilterQueue_Initialize(humidityData);
    FilterQueue_Initialize(pressureData);
    FilterQueue_Initialize(lightData);

    float temperatureFilteredVal = 0.0f;
    float humidityFilteredVal = 0.0f;
    float pressureFilteredVal = 0.0f;
    float lightFilteredVal = 0.0f;
    int soundSensorRawVal = 0;
    char lcdStrBuf[LCD_STRING_BUFFER_SIZE] = {0};

    ESP_LOGI(LOG_HEADER, "Starting main loop...");
    while (1)
    {
        haltIfError(soundSensorRead(&soundSensorRawVal));

        humTempRead(&temperatureFilteredVal, &humidityFilteredVal);
        temperatureFilteredVal = FilterQueueF_Enqueue(&temperatureData, temperatureFilteredVal);
        humidityFilteredVal = FilterQueueF_Enqueue(&humidityData, humidityFilteredVal);

        lightRead(&lightFilteredVal);
        lightFilteredVal = FilterQueueF_Enqueue(&lightData, lightFilteredVal);

        barometerRead(&pressureFilteredVal);
        pressureFilteredVal = FilterQueueF_Enqueue(&pressureData, pressureFilteredVal);

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "P%.0f T%.1f H%.0f", pressureFilteredVal, temperatureFilteredVal, humidityFilteredVal);
        haltIfError(lcdMoveCursor(0, 0));
        haltIfError(lcdSendString(lcdStrBuf));

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "S:%d L:%.1f", soundSensorRawVal, lightFilteredVal);
        haltIfError(lcdMoveCursor(1, 0));
        haltIfError(lcdSendString(lcdStrBuf));

        vTaskDelay(tfms(1));
    }
}