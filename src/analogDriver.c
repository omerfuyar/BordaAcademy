#include "analogDriver.h"
#include "esp_adc/adc_oneshot.h"
#include "rtosCommon.h"

adc_oneshot_unit_handle_t SOUND_SENSOR_HANDLE = NULL;

int analogInitSoundSensor()
{
    adc_oneshot_unit_init_cfg_t adcUnitInitConfig = {
        .unit_id = SOUND_SENSOR_ADC_UNIT,
    };

    int result = 0;
    returnIfError(adc_oneshot_new_unit(&adcUnitInitConfig, &SOUND_SENSOR_HANDLE));

    adc_oneshot_chan_cfg_t adcChannelInitConfig = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    return adc_oneshot_config_channel(SOUND_SENSOR_HANDLE, SOUND_SENSOR_ADC_CHANNEL, &adcChannelInitConfig);
}

int analogReadSoundSensor(int *retValue)
{
    int result = 0;
    returnIfError(adc_oneshot_read(SOUND_SENSOR_HANDLE, SOUND_SENSOR_ADC_CHANNEL, retValue));
    return ESP_OK;
}