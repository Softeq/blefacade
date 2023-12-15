#pragma once

#include <descriptor.hh>
#include <generated/characteristic_adaptor.hh>

#include <blefacade/core/GATT/uuid.hh>

#include <list>
#include <memory>

namespace softeq
{
namespace ble
{
namespace bluez
{
class Characteristic final : public org::bluez::GattCharacteristic1_adaptor, public DBus::ObjectAdaptor
{
public:
    Characteristic(DBus::Connection &connection, const std::string &objectPath);

    using OnReadHandler = std::function<ByteArray(const std::map<std::string, DBus::Variant> &)>;
    using OnWriteHandler = std::function<bool(const ByteArray &, const std::map<std::string, DBus::Variant> &)>;

    void setUuid(const softeq::ble::core::UUID &uuid);
    void setService(const DBus::Path &path);
    void setFlags(CharacteristicFlags flags);
    void updateValue(const ByteArray &value);
    std::string strUuid();

    void addReadValueHandler(OnReadHandler handler);
    void addWriteValueHandler(OnWriteHandler handler);

    void addDescriptor(std::shared_ptr<Descriptor> descriptor);
    std::list<std::shared_ptr<Descriptor>> getDescriptors() const;

private:
    OnReadHandler _onReadHandler;
    OnWriteHandler _onWriteHandler;
    std::size_t _notifySubscribersCnt;
    std::list<std::shared_ptr<Descriptor>> _descriptorList;

    std::vector<uint8_t> ReadValue(const std::map<std::string, DBus::Variant> &options, DBus::Error &error) override;
    void WriteValue(const std::vector<uint8_t> &value, const std::map<std::string, DBus::Variant> &options,
                    DBus::Error &error) override;
    void StartNotify(DBus::Error &error) override;
    void StopNotify(DBus::Error &error) override;

    void emitPropertyChanged(const std::string &propertyName);
};
} // namespace bluez
} // namespace ble
} // namespace softeq
