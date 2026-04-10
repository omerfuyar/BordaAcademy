#pragma once
#include "hal/i2c_types.h"
#include "hal/gpio_types.h"

// master is preconfigured for this project
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SCL_IO GPIO_NUM_22
#define I2C_MASTER_SDA_IO GPIO_NUM_21
#define I2C_MASTER_FREQ_HZ 100000

int i2cInitMaster();
