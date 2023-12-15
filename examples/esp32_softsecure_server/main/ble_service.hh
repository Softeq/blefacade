#ifndef BLE_SERVICE_HH
#define BLE_SERVICE_HH

#include "esp32_backend.hh"
#include "simple_softsec.hh"
#include <blefacade/core/GATT/profile.hh>

class BleService
{
private:
    softeq::ble::core::Profile _authProfile;
    softeq::ble::core::Profile _protectedProfile;
    SimpleSoftSecurity _secService;
    softeq::ble::esp::ESP32Backend _backend;

public:
    BleService(std::function<softeq::ble::core::gattWriteHandler> wHandler,
               std::function<softeq::ble::core::gattReadHandler> rHandler);
    void init();
};

#endif