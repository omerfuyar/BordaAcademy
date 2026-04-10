#include "i2cDriver.h"
#include "rtosCommon.h"
#include "driver/i2c.h"

int i2cInitMaster()
{
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    int result = 0;
    returnIfError(i2c_param_config(I2C_MASTER_NUM, &i2c_conf));
    returnIfError(i2c_driver_install(I2C_MASTER_NUM, i2c_conf.mode, 0, 0, 0));

    return ESP_OK;
}