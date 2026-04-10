#pragma once
#include "hal/adc_types.h"

// sound sensor is preconfigured for this project
#define SOUND_SENSOR_ADC_UNIT ADC_UNIT_1
#define SOUND_SENSOR_ADC_CHANNEL ADC_CHANNEL_4

int soundSensorInit();

int soundSensorRead(int *retValue);
