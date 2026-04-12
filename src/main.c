#include <stdio.h>
#include "devices/lcdDriver.h"
#include "devices/soundDriver.h"
#include "devices/barometerDriver.h"
#include "devices/humTempDriver.h"
#include "devices/lightDriver.h"
#include "bleDriver.h"
#include "dataFilter.h"
#include "rtosCommon.h"
#include "esp_log.h"
#include "esp_err.h"

#define LOG_HEADER "APP"
#define LCD_STRING_BUFFER_SIZE 16

#define SHARED_DATA_QUEUE_LENGTH 16
#define PRODUCER_TASK_DELAY_MS 1000
#define CONSUMER_TASK_BLE_ADVERT_INTERVAL 30

#define panicIfError(function, ...)                                                                           \
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

typedef struct DataPack
{
    float temperature;
    float humidity;
    float pressure;
    float light;
    int sound;
} DataPack;

QueueHandle_t sharedDataQueue;

void taskProducer(void *pvParameters)
{
    TickType_t taskLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        DataPack newData;
        panicIfError(soundRead(&newData.sound));
        panicIfError(humTempRead(&newData.temperature, &newData.humidity));
        panicIfError(lightRead(&newData.light));
        panicIfError(barometerRead(&newData.pressure));

        // SHARED_DATA_QUEUE_LENGTH
        if (xQueueSend(sharedDataQueue, &newData, 0) != pdTRUE)
        {
            ESP_LOGW(LOG_HEADER, "Consumer is too slow, Queue is full. Dropping the oldest measurement.");
            DataPack droppedData;
            xQueueReceive(sharedDataQueue, &droppedData, 0);
            xQueueSend(sharedDataQueue, &newData, 0);
        }

        xTaskDelayUntil(&taskLastWakeTime, tfms(PRODUCER_TASK_DELAY_MS));
    }
}

void taskConsumer(void *pvParameters)
{
    FilterQueueF rawTemperatureData;
    FilterQueueF rawHumidityData;
    FilterQueueF rawPressureData;
    FilterQueueF rawLightData;
    FilterQueueF rawSoundData;

    FilterQueue_Initialize(rawTemperatureData);
    FilterQueue_Initialize(rawHumidityData);
    FilterQueue_Initialize(rawPressureData);
    FilterQueue_Initialize(rawLightData);
    FilterQueue_Initialize(rawSoundData);

    FilterQueueF filteredTemperatureData;
    FilterQueueF filteredHumidityData;
    FilterQueueF filteredPressureData;
    FilterQueueF filteredLightData;
    FilterQueueF filteredSoundData;

    FilterQueue_Initialize(filteredTemperatureData);
    FilterQueue_Initialize(filteredHumidityData);
    FilterQueue_Initialize(filteredPressureData);
    FilterQueue_Initialize(filteredLightData);
    FilterQueue_Initialize(filteredSoundData);

    char lcdStrBuf[LCD_STRING_BUFFER_SIZE] = {0};
    int bleCounter = 0;

    while (1)
    {
        DataPack receivedSensorData;
        xQueueReceive(sharedDataQueue, &receivedSensorData, portMAX_DELAY); // infinite wait for data

        // take the median of last raw data and put it in the filtered queue
        FilterQueueF_Enqueue(&filteredTemperatureData, FilterQueueF_Enqueue(&rawTemperatureData, receivedSensorData.temperature));
        FilterQueueF_Enqueue(&filteredHumidityData, FilterQueueF_Enqueue(&rawHumidityData, receivedSensorData.humidity));
        FilterQueueF_Enqueue(&filteredPressureData, FilterQueueF_Enqueue(&rawPressureData, receivedSensorData.pressure));
        FilterQueueF_Enqueue(&filteredLightData, FilterQueueF_Enqueue(&rawLightData, receivedSensorData.light));
        FilterQueueF_Enqueue(&filteredSoundData, FilterQueueF_Enqueue(&rawSoundData, (float)receivedSensorData.sound));

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "P%.0f T%.1f H%.0f", receivedSensorData.pressure, receivedSensorData.temperature, receivedSensorData.humidity);
        panicIfError(lcdMoveCursor(0, 0));
        panicIfError(lcdSendString(lcdStrBuf));

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "S:%d L:%.1f", receivedSensorData.sound, receivedSensorData.light);
        panicIfError(lcdMoveCursor(1, 0));
        panicIfError(lcdSendString(lcdStrBuf));

        bleCounter++;
        if (bleCounter >= CONSUMER_TASK_BLE_ADVERT_INTERVAL)
        {
            bleCounter = 0;

            QueueStats temperatureStat = FilterQueueF_CalculateStats(&filteredTemperatureData);
            QueueStats humidityStat = FilterQueueF_CalculateStats(&filteredHumidityData);
            QueueStats pressureStat = FilterQueueF_CalculateStats(&filteredPressureData);
            QueueStats lightStat = FilterQueueF_CalculateStats(&filteredLightData);
            QueueStats soundStat = FilterQueueF_CalculateStats(&filteredSoundData);

            struct __attribute__((packed))
            {
                uint8_t len;
                uint8_t type;
                uint16_t company_id;
                int8_t minTmp, maxTmp, medTmp, stdTmp;
                uint8_t minHum, maxHum, medHum, stdHum;
                uint8_t minPrs, maxPrs, medPrs, stdPrs;
                uint8_t minLig, maxLig, medLig, stdLig;
                uint8_t minSnd, maxSnd, medSnd, stdSnd;
            } blePayload;

            blePayload.len = sizeof(blePayload) - 1;
            blePayload.type = 0xFF;
            blePayload.company_id = 0xFFFF;

            blePayload.minTmp = (int8_t)temperatureStat.min;
            blePayload.maxTmp = (int8_t)temperatureStat.max;
            blePayload.medTmp = (int8_t)temperatureStat.median;
            blePayload.stdTmp = (int8_t)temperatureStat.stdDev;

            blePayload.minHum = (uint8_t)humidityStat.min;
            blePayload.maxHum = (uint8_t)humidityStat.max;
            blePayload.medHum = (uint8_t)humidityStat.median;
            blePayload.stdHum = (uint8_t)humidityStat.stdDev;

            // ~1000 so scaling (pressure - 900)
            blePayload.minPrs = (uint8_t)(pressureStat.min - 900);
            blePayload.maxPrs = (uint8_t)(pressureStat.max - 900);
            blePayload.medPrs = (uint8_t)(pressureStat.median - 900);
            blePayload.stdPrs = (uint8_t)pressureStat.stdDev;

            blePayload.minLig = (uint8_t)(lightStat.min > 255 ? 255 : lightStat.min);
            blePayload.maxLig = (uint8_t)(lightStat.max > 255 ? 255 : lightStat.max);
            blePayload.medLig = (uint8_t)(lightStat.median > 255 ? 255 : lightStat.median);
            blePayload.stdLig = (uint8_t)(lightStat.stdDev);

            blePayload.minSnd = (uint8_t)(soundStat.min > 255 ? 255 : soundStat.min);
            blePayload.maxSnd = (uint8_t)(soundStat.max > 255 ? 255 : soundStat.max);
            blePayload.medSnd = (uint8_t)(soundStat.median > 255 ? 255 : soundStat.median);
            blePayload.stdSnd = (uint8_t)(soundStat.stdDev);

            panicIfError(bleAdvertise((uint8_t *)&blePayload, sizeof(blePayload)));
        }
    }
}

void app_main(void)
{
    ESP_LOGI(LOG_HEADER, "Initializing Peripherals");
    panicIfError(i2cMasterInitialize());

    panicIfError(lcdInitialize());

    panicIfError(barometerInitialize());
    panicIfError(lightInitialize());
    panicIfError(humTempInitialize());

    panicIfError(soundInitialize());

    panicIfError(bleInitialize());

    sharedDataQueue = xQueueCreate(SHARED_DATA_QUEUE_LENGTH, sizeof(DataPack));

    ESP_LOGI(LOG_HEADER, "Launching Tasks...");
    xTaskCreate(taskProducer, "Producer", 4096, NULL, 2, NULL); // high priority for producer
    xTaskCreate(taskConsumer, "Consumer", 4096, NULL, 1, NULL);

    // app_main exit
}