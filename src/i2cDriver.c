#include "i2cDriver.h"
#include "rtosCommon.h"
#include "driver/i2c_master.h"

static i2c_master_bus_handle_t I2C_BUS_HANDLE = NULL;
static i2c_master_dev_handle_t I2C_DEVICE_HANDLES[128] = {0};

int i2cInitMaster()
{
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    int result = 0;
    returnIfError(i2c_new_master_bus(&i2c_mst_config, &I2C_BUS_HANDLE));

    return ESP_OK;
}

int i2cWriteToSlave(uint8_t deviceAddress, uint8_t *data, size_t dataLen)
{
    if (I2C_DEVICE_HANDLES[deviceAddress] == NULL)
    {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN,
            .device_address = deviceAddress,
            .scl_speed_hz = I2C_MASTER_FREQ_HZ};

        int result = 0;
        returnIfError(i2c_master_bus_add_device(I2C_BUS_HANDLE, &dev_cfg, &I2C_DEVICE_HANDLES[deviceAddress]));
    }

    return i2c_master_transmit(I2C_DEVICE_HANDLES[deviceAddress], data, dataLen, I2C_MASTER_TIMEOUT_MS);
}

int i2cReadFromSlave(uint8_t deviceAddress, uint8_t *data, size_t dataLen)
{
    if (I2C_DEVICE_HANDLES[deviceAddress] == NULL)
    {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = deviceAddress,
            .scl_speed_hz = I2C_MASTER_FREQ_HZ};

        int result = 0;
        returnIfError(i2c_master_bus_add_device(I2C_BUS_HANDLE, &dev_cfg, &I2C_DEVICE_HANDLES[deviceAddress]));
    }

    return i2c_master_receive(I2C_DEVICE_HANDLES[deviceAddress], data, dataLen, I2C_MASTER_TIMEOUT_MS);
}
