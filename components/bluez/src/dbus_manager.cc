#include <dbus_manager.hh>

#include <iostream>

using namespace softeq::ble::bluez;

DBusManager::DBusManager()
{
    DBus::default_dispatcher = &_dispatcher;

    _connection.reset(new DBus::Connection(DBus::Connection::SystemBus()));

    _dispatcherThread = std::thread([this] { _dispatcher.enter(); });
}

DBusManager::~DBusManager()
{
    _dispatcher.leave();

    try
    {
        if (_dispatcherThread.joinable())
        {
            _dispatcherThread.join();
        }
    }
    catch (const std::exception &ex)
    {
        std::cout << "An error when DBus dispatcher thread joined. Reason: " << ex.what() << std::endl;
    }
}

DBus::Connection DBusManager::connection()
{
    return *_connection;
}