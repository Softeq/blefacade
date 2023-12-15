#pragma once

#include <characteristic.hh>
#include <generated/object_manager_adaptor.hh>
#include <generated/service_adaptor.hh>

#include <blefacade/core/GATT/uuid.hh>

#include <memory>

namespace softeq
{
namespace ble
{
namespace bluez
{
class Service final : public org::bluez::GattService1_adaptor, public DBus::ObjectAdaptor
{
public:
    Service(DBus::Connection &connection, const std::string &objectPath);

    void setUuid(const softeq::ble::core::UUID &uuid);
    void setPrimary(bool isPrimary);
    softeq::ble::core::UUID strUuid();

    void addCharacteristic(std::shared_ptr<Characteristic> characteristic);
    std::list<std::shared_ptr<Characteristic>> getCharacteristics() const;

private:
    std::list<std::shared_ptr<Characteristic>> _characteristicList;
};
} // namespace bluez
} // namespace ble
} // namespace softeq
