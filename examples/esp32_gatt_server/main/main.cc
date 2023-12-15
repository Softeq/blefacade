#define ESP32
#include "esp32_backend.hh"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <blefacade/core/GATT/att_utils.hh>
#include <blefacade/core/GATT/profile.hh>
#include <blefacade/core/backend/ble_backend_if.hh>
#include <stdio.h>


#define MAIN_TAG "MAIN"

using namespace softeq::ble::core;
using namespace softeq::ble::esp;
using namespace std::placeholders;

extern Profile profileMain;
extern Profile profileAux;

static ESP32Backend backend{};

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

    backend.init();
    backend.configureProfile(profileMain);
    backend.configureProfile(profileAux);

    Attribute *att = findAttributeByUuid(profileMain, UUID{0xabcd});
    if (att)
    {
        uint8_t tmp = 0xfa;
        while (true)
        {
            vTaskDelay(configTICK_RATE_HZ * 5); // 5s delay
            backend.indicate(*static_cast<Characteristic *>(att), &tmp, 1);
        }
    }
}
