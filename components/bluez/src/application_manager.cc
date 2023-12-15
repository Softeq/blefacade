#include <application_manager.hh>

#include <iostream>

using namespace softeq::ble::bluez;

namespace
{
const char *serviceName = "org.bluez";
} // namespace

ApplicationManager::ApplicationManager(DBus::Connection &connection, const std::string &objectPath)
    : DBus::ObjectProxy(connection, objectPath.c_str(), serviceName)
    , _promise()
    , _registeredApplications()
{
}

ApplicationManager::~ApplicationManager()
{
    unregisterApplications();
}

bool ApplicationManager::registerApplication(std::shared_ptr<Application> application,
                                             const std::map<std::string, DBus::Variant> &options)
{
    _promise = std::promise<bool>();

    RegisterApplicationAsync(application->path(), options, nullptr);

    auto future = _promise.get_future();

    bool res = future.get();
    if (res)
        _registeredApplications.emplace_back(application);

    return res;
}

void ApplicationManager::unregisterApplications()
{
    for (const auto &application : _registeredApplications)
    {
        UnregisterApplication(application->path());
    }

    _registeredApplications.clear();
}

void ApplicationManager::RegisterApplicationCallback(const DBus::Error &error, void *)
{
    if (error.is_set())
    {
        std::cout << std::string("Failed to register gatt application: ") + error.what() << std::endl;
        _promise.set_value(false);
    }

    _promise.set_value(true);
}
