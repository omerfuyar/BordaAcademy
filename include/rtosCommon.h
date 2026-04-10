#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Convert milliseconds to ticks
#define tfms(n) ((TickType_t)(((n) + (portTICK_PERIOD_MS - 1)) / portTICK_PERIOD_MS))

// requires a int variable 'result', variadic is additional instructions to execute before return
#define returnIfError(function, ...)     \
    if ((result = (function)) != ESP_OK) \
    {                                    \
        __VA_ARGS__                      \
        return result;                   \
    }
