#pragma once
#include <stdint.h>

int bleInitialize(void);

int bleAdvertise(const uint8_t *data, uint32_t length);