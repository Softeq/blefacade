#ifndef BLE_BACKEND_HH
#define BLE_BACKEND_HH
#include <blefacade/core/GATT/characteristic.hh>
#include <blefacade/core/GATT/handlers.hh>
#include <blefacade/core/GATT/profile.hh>

namespace softeq
{
namespace ble
{
namespace core
{
class BleBackendIf
{
public:
    virtual bool configureProfile(Profile &profile) = 0;
    virtual bool indicate(Characteristic &chr, uint8_t *data = nullptr, size_t len = 0) = 0;
    virtual void addEventHandler(std::function<gattEventHandler>) = 0;
};

} // namespace core
} // namespace ble
} // namespace softeq


#endif //BLE_BACKEND_HH