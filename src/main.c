#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define LED_GPIO 2

static const char *TAG = "BLINK_APP";

void app_main(void)
{
    ESP_LOGI(TAG, "Starting blink program!");

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    int led_state = 0;

    while (1)
    {
        ESP_LOGI(TAG, "Turning the LED %s!", led_state == 1 ? "ON" : "OFF");
        gpio_set_level(LED_GPIO, led_state);

        led_state = !led_state;

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}