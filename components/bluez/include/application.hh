#pragma once

#include <generated/object_manager_adaptor.hh>
#include <service.hh>

#include <memory>

namespace softeq
{
namespace ble
{
namespace bluez
{
class Application final : public org::freedesktop::DBus::ObjectManager_adaptor, public DBus::ObjectAdaptor
{
public:
    Application(DBus::Connection &connection, const std::string &objectPath);

    std::map<DBus::Path, std::map<std::string, std::map<std::string, DBus::Variant>>>
    GetManagedObjects(DBus::Error &) override;

    void addService(std::shared_ptr<Service> service);
    std::list<std::shared_ptr<Service>> getServices();

private:
    std::list<std::shared_ptr<Service>> _serviceList;
};
} // namespace bluez
} // namespace ble
} // namespace softeq
