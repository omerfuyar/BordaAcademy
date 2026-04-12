#include "devices/humTempDriver.h"
#include "i2cDriver.h"
#include "rtosCommon.h"

int humTempInitialize(void)
{
    vTaskDelay(tfms(100));
    return ESP_OK;
}

int humTempRead(float *retTemperature, float *retHumidity)
{
    *retTemperature = SENSOR_HT_MOCK_TEMPERATURE_VALUE + SENSOR_HT_MOCK_MAX_DEVIATION * (float)(rand() - RAND_MAX / 2) / (RAND_MAX / 2);
    *retHumidity = SENSOR_HT_MOCK_HUMIDITY_VALUE + SENSOR_HT_MOCK_MAX_DEVIATION * (float)(rand() - RAND_MAX / 2) / (RAND_MAX / 2);

    vTaskDelay(tfms(100));

    return ESP_OK;
}
