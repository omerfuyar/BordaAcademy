#pragma once

#define SENSOR_LIGHT_ADDRESS 0x23
#define SENSOR_LIGHT_MOCK_VALUE 500.0f
#define SENSOR_LIGHT_MOCK_MAX_DEVIATION 50.0f

int lightInitialize(void);

int lightRead(float *retIlluminance);
