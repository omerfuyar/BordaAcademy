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
#define CONSUMER_TASK_DELAY_MS 30000

typedef struct
{
    float temp;
    float hum;
    float pres;
    float light;
    int sound;
} SensorData_t;

QueueHandle_t sensorDataQueue;

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

void producer_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        SensorData_t newData;
        haltIfError(soundSensorRead(&newData.sound));
        humTempRead(&newData.temp, &newData.hum);
        lightRead(&newData.light);
        barometerRead(&newData.pres);

        // Wait instantly (0 timeout). If fails, buffer is full (consumer is slow).
        if (xQueueSend(sensorDataQueue, &newData, 0) != pdTRUE)
        {
            ESP_LOGW(LOG_HEADER, "Consumer is too slow, Queue is full. Dropping latest measurement.");
        }

        xTaskDelayUntil(&xLastWakeTime, tfms(PRODUCER_TASK_DELAY_MS));
    }
}

void consumer_task(void *pvParameters)
{
    FilterQueueF temperatureData;
    FilterQueueF humidityData;
    FilterQueueF pressureData;
    FilterQueueF lightData;

    FilterQueue_Initialize(temperatureData);
    FilterQueue_Initialize(humidityData);
    FilterQueue_Initialize(pressureData);
    FilterQueue_Initialize(lightData);

    char lcdStrBuf[LCD_STRING_BUFFER_SIZE] = {0};

    while (1)
    {
        SensorData_t popData;
        xQueueReceive(sensorDataQueue, &popData, portMAX_DELAY); // infinite wait for data

        popData.temp = FilterQueueF_Enqueue(&temperatureData, popData.temp);
        popData.hum = FilterQueueF_Enqueue(&humidityData, popData.hum);
        popData.pres = FilterQueueF_Enqueue(&pressureData, popData.pres);
        popData.light = FilterQueueF_Enqueue(&lightData, popData.light);

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "P%.0f T%.1f H%.0f", popData.pres, popData.temp, popData.hum);
        haltIfError(lcdMoveCursor(0, 0));
        haltIfError(lcdSendString(lcdStrBuf));

        snprintf(lcdStrBuf, sizeof(lcdStrBuf), "S:%d L:%.1f", popData.sound, popData.light);
        haltIfError(lcdMoveCursor(1, 0));
        haltIfError(lcdSendString(lcdStrBuf));
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
    sensorDataQueue = xQueueCreate(SENSOR_DATA_QUEUE_LENGTH, sizeof(SensorData_t));

    ESP_LOGI(LOG_HEADER, "Launching Tasks...");
    xTaskCreate(producer_task, "Producer", 4096, NULL, 2, NULL); // high priority for producer
    xTaskCreate(consumer_task, "Consumer", 4096, NULL, 1, NULL);

    // app_main exit
}