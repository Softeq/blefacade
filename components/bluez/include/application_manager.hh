#pragma once

#include <application.hh>
#include <generated/manager_proxy.hh>

#include <future>

namespace softeq
{
namespace ble
{
namespace bluez
{
class ApplicationManager final : private org::bluez::GattManager1_proxy,
                                 public DBus::IntrospectableProxy,
                                 public DBus::ObjectProxy
{
public:
    ApplicationManager(DBus::Connection &connection, const std::string &objectPath);
    ~ApplicationManager();

    bool registerApplication(std::shared_ptr<Application> application,
                             const std::map<std::string, ::DBus::Variant> &options);
    void unregisterApplications();

    std::list<std::shared_ptr<Application>> getApps()
    {
        return _registeredApplications;
    }

protected:
    void RegisterApplicationCallback(const DBus::Error &error, void *) override;

private:
    std::promise<bool> _promise;
    std::list<std::shared_ptr<Application>> _registeredApplications;
};
} // namespace bluez
} // namespace ble
} // namespace softeq
