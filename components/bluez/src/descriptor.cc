#include <descriptor.hh>

using namespace softeq::ble::bluez;

Descriptor::Descriptor(DBus::Connection &connection, const std::string &objectPath)
    : DBus::ObjectAdaptor(connection, objectPath)
    , _onReadHandler()
    , _onWriteHandler()
{
}

void Descriptor::setUuid(const softeq::ble::core::UUID &uuid)
{
    UUID = uuid.getPrintable();
}

void Descriptor::setCharacteristic(const DBus::Path &path)
{
    Characteristic = path;
}

void Descriptor::setFlags(DescriptorFlags flags)
{
    Flags = flagsToStrings(flags);
}

void Descriptor::addReadValueHandler(Descriptor::OnReadHandler handler)
{
    _onReadHandler = handler;
}

void Descriptor::addWriteValueHandler(Descriptor::OnWriteHandler handler)
{
    _onWriteHandler = handler;
}

std::vector<uint8_t> Descriptor::ReadValue(const std::map<std::string, DBus::Variant> &options, DBus::Error &error)
{
    try
    {
        return _onReadHandler(options).data();
    }
    catch (const std::bad_function_call &callError)
    {
        error = DBus::ErrorNotSupported(callError.what());

        return std::vector<uint8_t>();
    }
}

void Descriptor::WriteValue(const std::vector<uint8_t> &value, const std::map<std::string, DBus::Variant> &options,
                            DBus::Error &error)
{
    try
    {
        if (!_onWriteHandler(ByteArray(value), options))
            error = DBus::ErrorFailed("Failed to write value");
    }
    catch (const std::bad_function_call &callError)
    {
        error = DBus::ErrorNotSupported(callError.what());
    }
}
