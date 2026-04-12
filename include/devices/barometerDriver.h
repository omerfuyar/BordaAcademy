#pragma once

#define SENSOR_BAROMETER_ADDRESS 0x77
#define SENSOR_BAROMETER_MOCK_VALUE 1013.25f
#define SENSOR_BAROMETER_MOCK_MAX_DEVIATION 10.0f

int barometerInitialize(void);

int barometerRead(float *retPressure);
