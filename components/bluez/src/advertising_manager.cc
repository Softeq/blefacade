#include <advertising_manager.hh>

#include <iostream>

namespace
{
const char *serviceName = "org.bluez";
} // namespace

using namespace softeq::ble::bluez;

AdvertisingManager::AdvertisingManager(DBus::Connection &connection, const DBus::Path &objectPath)
    : DBus::ObjectProxy(connection, objectPath, serviceName)
    , _promise()
    , _advertisement()
{
}

AdvertisingManager::~AdvertisingManager()
{
    unregisterAdvertisement();
}

bool AdvertisingManager::registerAdvertisement(std::unique_ptr<Advertisement> advertisement,
                                               const std::map<std::string, DBus::Variant> &options)
{
    unregisterAdvertisement();

    _promise = std::promise<bool>();

    RegisterAdvertisementAsync(advertisement->path(), options, nullptr);

    auto future = _promise.get_future();

    bool res = future.get();
    if (res)
        _advertisement = std::move(advertisement);

    return res;
}

void AdvertisingManager::unregisterAdvertisement()
{
    if (_advertisement)
    {
        UnregisterAdvertisement(_advertisement->path());
        _advertisement.reset(nullptr);
    }
}

void AdvertisingManager::RegisterAdvertisementCallback(const DBus::Error &error, void *)
{
    if (error.is_set())
    {
        std::cout << std::string("Failed to register advertisement: ") + error.what() << std::endl;
        _promise.set_value(false);
    }

    _promise.set_value(true);
}
