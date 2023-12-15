#pragma once

#include <dbus-c++/dbus.h>

#include <thread>

namespace softeq
{
namespace ble
{
namespace bluez
{
class DBusManager
{
public:
    DBusManager();
    ~DBusManager();

    DBus::Connection connection();

private:
    DBus::BusDispatcher _dispatcher;
    std::unique_ptr<DBus::Connection> _connection{nullptr};
    std::thread _dispatcherThread;
};
} // namespace bluez
} // namespace ble
} // namespace softeq
