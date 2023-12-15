#define ESP32
#include "ble_service.hh"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>


#define MAIN_TAG "MAIN"

using namespace softeq::ble::core;

static char protectedData[] = "Hello world";

static ErrorCode dummyWriteHandler(BleBackendIf &bck, Attribute &att, const uint8_t *data, size_t len,
                                   const WriteParam &param)
{
    ESP_LOGW(MAIN_TAG, "DEBUG: dummyWriteHandler called");
    (void)bck;
    size_t dataLen = sizeof(protectedData) / sizeof(protectedData[0]);
    if (dataLen >= len)
    {
        memcpy(protectedData, data, len);
        return Error::ERR_OK;
    }
    else
    {
        // log issue here
        return Error::ERR_LEN_MISMATCH;
    }
};

static ErrorCode dummyReadHandler(BleBackendIf &bck, Attribute &att, uint8_t *data, size_t &len, const ReadParam &param)
{
    ESP_LOGW(MAIN_TAG, "DEBUG: dummyReadHandler called");
    (void)bck;
    len = sizeof(protectedData) / sizeof(protectedData[0]);
    ESP_LOGW(MAIN_TAG, "DEBUG: protected data length: %d", len);
    memcpy(data, protectedData, len);
    return Error::ERR_OK;
};

// static ESP32Backend backend{};
BleService bleService{dummyWriteHandler, dummyReadHandler};

extern "C" void app_main(void)
{
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
    ESP_LOGI(MAIN_TAG, "BT intialization started");

    bleService.init();
}
