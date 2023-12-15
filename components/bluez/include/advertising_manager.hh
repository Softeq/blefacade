#pragma once

#include <advertisement.hh>
#include <generated/advertising_manager_proxy.hh>

#include <future>

namespace softeq
{
namespace ble
{
namespace bluez
{
class AdvertisingManager final : private org::bluez::LEAdvertisingManager1_proxy, public DBus::ObjectProxy
{
public:
    AdvertisingManager(DBus::Connection &connection, const DBus::Path &objectPath);
    ~AdvertisingManager();

    bool registerAdvertisement(std::unique_ptr<Advertisement> advertisement,
                               const std::map<std::string, ::DBus::Variant> &options);
    void unregisterAdvertisement();

protected:
    void RegisterAdvertisementCallback(const DBus::Error &error, void *data) override;

private:
    std::promise<bool> _promise;
    std::unique_ptr<Advertisement> _advertisement;
};
} // namespace bluez
} // namespace ble
} // namespace softeq
