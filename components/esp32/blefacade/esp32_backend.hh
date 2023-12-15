#pragma once

#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"

#include <blefacade/core/GATT/handlers.hh>
#include <blefacade/core/GATT/profile.hh>
#include <blefacade/core/backend/ble_backend_if.hh>
#include <blefacade/core/utils/slist.hh>

namespace softeq
{
namespace ble
{
namespace esp
{
class ESP32Backend : public softeq::ble::core::BleBackendIf
{
private:
    softeq::ble::core::Slist<softeq::ble::core::Profile *> _profiles;
    void gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    void gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    softeq::ble::core::Slist<std::function<softeq::ble::core::gattEventHandler>> extEventHandlers;
    bool configured = false;

public:
    ESP32Backend() = default;
    bool init();
    bool configureProfile(softeq::ble::core::Profile &profile) override;
    bool indicate(softeq::ble::core::Characteristic &chr, uint8_t *data = nullptr, size_t len = 0) override;
    void addEventHandler(std::function<softeq::ble::core::gattEventHandler> h) override
    {
        extEventHandlers.push_back(h);
    };
    ~ESP32Backend();
};
} // namespace esp
} // namespace ble
} // namespace softeq
