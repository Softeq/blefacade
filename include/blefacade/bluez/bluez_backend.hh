#ifndef BLUEZ_BACKEND_HH
#define BLUEZ_BACKEND_HH

#include <memory>
#include <vector>

#include <dbus-c++/dbus.h>

#include <advertising_manager.hh>
#include <application_manager.hh>
#include <blefacade/core/backend/ble_backend_if.hh>
#include <bluez_object_manager.hh>
#include <dbus_manager.hh>

namespace softeq
{
namespace ble
{
namespace bluez
{
class BluezBackend : public softeq::ble::core::BleBackendIf
{
public:
    BluezBackend();
    ~BluezBackend();

    bool configureProfile(softeq::ble::core::Profile &profile) override;
    bool indicate(softeq::ble::core::Characteristic &iChar, uint8_t *value = nullptr, size_t len = 0) override;
    void addEventHandler(std::function<softeq::ble::core::gattEventHandler>) override {}; //Left empty for compatibility purposes. To be implemented if needed.
private:
    DBusManager _dbusManager;
    std::unique_ptr<BluezObjectManager> _bluezObjectManager;
    std::unique_ptr<ApplicationManager> _applicationManager;
    std::unique_ptr<AdvertisingManager> _advertisingManager;

    int _profilesCount;
    std::vector<softeq::ble::core::UUID> _servicesUuids;
    bool _advertisementIsConfigured;

    bool configureAdvertisement();
    void convertService(DBus::Connection &connection, softeq::ble::core::Service &bleService,
                        std::shared_ptr<Service> &bluezService);
    void convertCharacteristic(DBus::Connection &connection, softeq::ble::core::Characteristic &bleChar,
                               std::shared_ptr<Characteristic> &bluezChar);
    void convertDescriptor(softeq::ble::core::Descriptor &bleDesc, std::shared_ptr<Descriptor> &bluezDesc);
};
} // namespace bluez
} // namespace ble
} // namespace softeq


#endif //BLUEZ_BACKEND_HH