#pragma once
#include <stddef.h>
#include "hal/i2c_types.h"
#include "hal/gpio_types.h"

#define I2C_ADDR_BIT_LEN 7

// master is preconfigured for this project
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TIMEOUT_MS 1000

int i2cInitMaster();

int i2cWriteToSlave(uint8_t deviceAddress, uint8_t *data, size_t dataLen, uint32_t ticksToWait, bool mockWrite);

int i2cReadFromSlave(uint8_t deviceAddress, uint8_t *data, size_t dataLen, uint32_t ticksToWait, bool mockRead);
