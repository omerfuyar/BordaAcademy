#include "devices/lightDriver.h"
#include "i2cDriver.h"
#include "rtosCommon.h"

int lightInit()
{
    vTaskDelay(tfms(100));
    return ESP_OK;
}

int lightRead(float *retIlluminance)
{
    *retIlluminance = SENSOR_LIGHT_MOCK_VALUE + SENSOR_LIGHT_MOCK_MAX_DEVIATION * (float)(rand() - RAND_MAX / 2) / (RAND_MAX / 2);
    vTaskDelay(tfms(100));

    return ESP_OK;
}
