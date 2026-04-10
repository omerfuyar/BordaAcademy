#include "devices/barometerDriver.h"
#include "i2cDriver.h"
#include "rtosCommon.h"

int barometerInit(void)
{
    vTaskDelay(tfms(100));
    return ESP_OK;
}

int barometerRead(float *retPressure)
{
    *retPressure = SENSOR_BAROMETER_MOCK_VALUE + SENSOR_BAROMETER_MOCK_MAX_DEVIATION * (float)(rand() - RAND_MAX / 2) / (RAND_MAX / 2);
    vTaskDelay(tfms(100));
    return ESP_OK;
}
