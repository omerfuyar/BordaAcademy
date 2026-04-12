#pragma once

#define SENSOR_HT_ADDRESS 0x38
#define SENSOR_HT_MOCK_TEMPERATURE_VALUE 25.0f
#define SENSOR_HT_MOCK_HUMIDITY_VALUE 50.0f
#define SENSOR_HT_MOCK_MAX_DEVIATION 5.0f

int humTempInitialize(void);

int humTempRead(float *retTemperature, float *retHumidity);
