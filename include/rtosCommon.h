#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ticks from micro seconds
#define tfus(n) (n / portTICK_PERIOD_MS)

// requires a int variable 'result', variadic is additional instructions to execute before return
#define returnIfError(function, ...)     \
    if ((result = (function)) != ESP_OK) \
    {                                    \
        __VA_ARGS__                      \
        return result;                   \
    }
