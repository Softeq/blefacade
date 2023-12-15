#include <bluez_object_manager.hh>

using namespace softeq::ble::bluez;

namespace
{
const char *serviceName = "org.bluez";
const char *objectPath = "/";
} // namespace

BluezObjectManager::BluezObjectManager(DBus::Connection &connection)
    : DBus::ObjectProxy(connection, objectPath, serviceName)
{
}

DBus::Path BluezObjectManager::gattManagerPath()
{
    for (const auto &object : GetManagedObjects())
    {
        for (const auto &interface : object.second)
        {
            if (interface.first.compare("org.bluez.GattManager1") == 0)
            {
                return object.first;
            }
        }
    }

    return DBus::Path();
}

DBus::Path BluezObjectManager::leAdvertisingManagerPath()
{
    for (const auto &object : GetManagedObjects())
    {
        for (const auto &interface : object.second)
        {
            if (interface.first.compare("org.bluez.LEAdvertisingManager1") == 0)
            {
                return object.first;
            }
        }
    }

    return DBus::Path();
}
