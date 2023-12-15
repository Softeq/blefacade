#pragma once

#include <generated/advertisement_adaptor.hh>
#include <generated/object_manager_adaptor.hh>

#include <blefacade/core/GATT/uuid.hh>

namespace softeq
{
namespace ble
{
namespace bluez
{
class Advertisement final : public org::bluez::LEAdvertisement1_adaptor,
                            public org::freedesktop::DBus::ObjectManager_adaptor,
                            public DBus::ObjectAdaptor
{
public:
    enum class Type
    {
        Broadcast = 0,
        Peripheral
    };

    Advertisement(DBus::Connection &connection, const DBus::Path &objectPath);

    void setServiceUUIDs(const std::vector<softeq::ble::core::UUID> &uuids);
    void setSolicitUUIDs(const std::vector<softeq::ble::core::UUID> &uuids);
    void setManufacturerData(const std::map<std::string, DBus::Variant> &manufacturerData);
    void setServiceData(const std::map<std::string, DBus::Variant> &serviceData);
    void setIncludeTxPower(bool isIncludeTxPower);
    void setLocalName(const std::string &localName);
    void setType(Type type);

private:
    void Release(DBus::Error &error) override;

    std::map<DBus::Path, std::map<std::string, std::map<std::string, DBus::Variant>>>
    GetManagedObjects(DBus::Error &error) override;
};
} // namespace bluez
} // namespace ble
} // namespace softeq