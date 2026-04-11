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

#define SENSOR_DATA_QUEUE_LENGTH 16
#define PRODUCER_TASK_DELAY_MS 1000
#define CONSUMER_TASK_BLE_ADVERT_INTERVAL 30

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

typedef struct
{
    float temperature;
    float humidity;
    float pressure;
    float light;
    int sound;
} DataPack;

QueueHandle_t sensorDataQueue;

void producer_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        DataPack newData;
        haltIfError(soundSensorRead(&newData.sound));
        haltIfError(humTempRead(&newData.temperature, &newData.humidity));
        haltIfError(lightRead(&newData.light));
        haltIfError(barometerRead(&newData.pressure));

        // SENSOR_DATA_QUEUE_LENGTH
        if (xQueueSend(sensorDataQueue, &newData, 0) != pdTRUE)
        {
            ESP_LOGW(LOG_HEADER, "Consumer is too slow, Queue is full. Dropping latest measurement.");
        }

        xTaskDelayUntil(&xLastWakeTime, tfms(PRODUCER_TASK_DELAY_MS));
    }
}

void consumer_task(void *pvParameters)
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
        xQueueReceive(sensorDataQueue, &receivedSensorData, portMAX_DELAY); // infinite wait for data

        // take the median of last raw data and put it in the filtered queue
        FilterQueueF_Enqueue(&filteredTemperatureData, FilterQueueF_Enqueue(&rawTemperatureData, receivedSensorData.temperature));
        FilterQueueF_Enqueue(&filteredHumidityData, FilterQueueF_Enqueue(&rawHumidityData, receivedSensorData.humidity));
        FilterQueueF_Enqueue(&filteredPressureData, FilterQueueF_Enqueue(&rawPressureData, receivedSensorData.pressure));
        FilterQueueF_Enqueue(&filteredLightData, FilterQueueF_Enqueue(&rawLightData, receivedSensorData.light));
        FilterQueueF_Enqueue(&filteredSoundData, FilterQueueF_Enqueue(&rawSoundData, (float)receivedSensorData.sound));

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "P%.0f T%.1f H%.0f", receivedSensorData.pressure, receivedSensorData.temperature, receivedSensorData.humidity);
        haltIfError(lcdMoveCursor(0, 0));
        haltIfError(lcdSendString(lcdStrBuf));

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "S:%d L:%.1f", receivedSensorData.sound, receivedSensorData.light);
        haltIfError(lcdMoveCursor(1, 0));
        haltIfError(lcdSendString(lcdStrBuf));

        bleCounter++;
        if (bleCounter >= CONSUMER_TASK_BLE_ADVERT_INTERVAL)
        {
            bleCounter = 0;

            QueueStats temperatureStat = FilterQueueF_CalculateStats(&filteredTemperatureData);
            QueueStats humidityStat = FilterQueueF_CalculateStats(&filteredHumidityData);
            QueueStats pressureStat = FilterQueueF_CalculateStats(&filteredPressureData);
            QueueStats lightStat = FilterQueueF_CalculateStats(&filteredLightData);
            QueueStats soundStat = FilterQueueF_CalculateStats(&filteredSoundData);

            ESP_LOGW(LOG_HEADER, "--- 30s BLE PAYLOAD GENERATED ---");
            ESP_LOGW(LOG_HEADER, "Temp Stats | Min: %5.1f | Max: %5.1f | Median: %5.1f | StdDev: %5.2f",
                     temperatureStat.min, temperatureStat.max, temperatureStat.median, temperatureStat.stdDev);
            ESP_LOGW(LOG_HEADER, "Hum  Stats | Min: %5.1f | Max: %5.1f | Median: %5.1f | StdDev: %5.2f",
                     humidityStat.min, humidityStat.max, humidityStat.median, humidityStat.stdDev);
            ESP_LOGW(LOG_HEADER, "Press Stats | Min: %5.1f | Max: %5.1f | Median: %5.1f | StdDev: %5.2f",
                     pressureStat.min, pressureStat.max, pressureStat.median, pressureStat.stdDev);
            ESP_LOGW(LOG_HEADER, "Light Stats | Min: %5.1f | Max: %5.1f | Median: %5.1f | StdDev: %5.2f",
                     lightStat.min, lightStat.max, lightStat.median, lightStat.stdDev);
            ESP_LOGW(LOG_HEADER, "Sound Stats | Min: %5.1f | Max: %5.1f | Median: %5.1f | StdDev: %5.2f",
                     soundStat.min, soundStat.max, soundStat.median, soundStat.stdDev);
            ESP_LOGW(LOG_HEADER, "-----------------------------------");
        }
    }
}

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

    ESP_LOGI(LOG_HEADER, "Initializing RTOS Primitives...");
    sensorDataQueue = xQueueCreate(SENSOR_DATA_QUEUE_LENGTH, sizeof(DataPack));

    ESP_LOGI(LOG_HEADER, "Launching Tasks...");
    xTaskCreate(producer_task, "Producer", 4096, NULL, 2, NULL); // high priority for producer
    xTaskCreate(consumer_task, "Consumer", 4096, NULL, 1, NULL);

    // app_main exit
}