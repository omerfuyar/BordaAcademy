#include "bleDriver.h"
#include "rtosCommon.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_bt_main.h"

int bleInitialize(void)
{
    int result;

    returnIfError(nvs_flash_init(), if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init(); });

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    returnIfError(esp_bt_controller_init(&bt_cfg));

    returnIfError(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    returnIfError(esp_bluedroid_init());

    returnIfError(esp_bluedroid_enable());

    return ESP_OK;
}

int bleAdvertise(const uint8_t *data, uint32_t length)
{
    int result;

    returnIfError(esp_ble_gap_config_adv_data_raw((uint8_t *)data, length));

    // Standard advertising parameters for a broadcaster beacon
    static esp_ble_adv_params_t adv_params = {
        .adv_int_min = 0x0200,            // min interval
        .adv_int_max = 0x0200,            // max interval
        .adv_type = ADV_TYPE_NONCONN_IND, // non-connectable undirected
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };

    // Note: stopping advertising before starting just in case it's already active
    esp_ble_gap_stop_advertising();
    returnIfError(esp_ble_gap_start_advertising(&adv_params));

    return ESP_OK;
}