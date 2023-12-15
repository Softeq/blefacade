#pragma once

#include <byte_array.hh>
#include <flags.hh>
#include <generated/descriptor_adaptor.hh>

#include <blefacade/core/GATT/uuid.hh>

#include <functional>

namespace softeq
{
namespace ble
{
namespace bluez
{
class Descriptor final : public org::bluez::GattDescriptor1_adaptor, public DBus::ObjectAdaptor
{
public:
    Descriptor(DBus::Connection &connection, const std::string &objectPath);

    using OnReadHandler = std::function<ByteArray(const std::map<std::string, DBus::Variant> &)>;
    using OnWriteHandler = std::function<bool(const ByteArray &, const std::map<std::string, DBus::Variant> &)>;

    void setUuid(const softeq::ble::core::UUID &uuid);
    void setCharacteristic(const DBus::Path &path);
    void setFlags(DescriptorFlags flags);

    void addReadValueHandler(OnReadHandler handler);
    void addWriteValueHandler(OnWriteHandler handler);

private:
    OnReadHandler _onReadHandler;
    OnWriteHandler _onWriteHandler;

    std::vector<uint8_t> ReadValue(const std::map<std::string, DBus::Variant> &options, DBus::Error &error) override;
    void WriteValue(const std::vector<uint8_t> &value, const std::map<std::string, DBus::Variant> &options,
                    DBus::Error &error) override;
};
} // namespace bluez
} // namespace ble
} // namespace softeq
