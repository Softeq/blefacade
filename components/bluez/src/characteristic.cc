#include <characteristic.hh>

using namespace softeq::ble::bluez;

namespace
{
const char *propertiesInterfaceName = "org.freedesktop.DBus.Properties";
const char *propertiesChangedSignal = "PropertiesChanged";
} // namespace

Characteristic::Characteristic(DBus::Connection &connection, const std::string &objectPath)
    : DBus::ObjectAdaptor(connection, objectPath)
    , _onReadHandler()
    , _onWriteHandler()
    , _notifySubscribersCnt(0)
    , _descriptorList()
{
    Notifying = false;
}

void Characteristic::setUuid(const softeq::ble::core::UUID &uuid)
{
    UUID = uuid.getPrintable();
}

std::string Characteristic::strUuid() // method is non-const as get_property is not constant
{
    return get_property("UUID")->operator std::string().c_str();
}

void Characteristic::setService(const DBus::Path &path)
{
    Service = path;
}

void Characteristic::setFlags(CharacteristicFlags flags)
{
    Flags = flagsToStrings(flags);
}

void Characteristic::updateValue(const ByteArray &value)
{
    Value = value.data();

    if (Notifying())
        emitPropertyChanged("Value");
}

void Characteristic::addReadValueHandler(Characteristic::OnReadHandler handler)
{
    _onReadHandler = handler;
}

void Characteristic::addWriteValueHandler(Characteristic::OnWriteHandler handler)
{
    _onWriteHandler = handler;
}

void Characteristic::addDescriptor(std::shared_ptr<Descriptor> descriptor)
{
    descriptor->setCharacteristic(path());

    _descriptorList.emplace_back(descriptor);
}

std::list<std::shared_ptr<Descriptor>> Characteristic::getDescriptors() const
{
    return _descriptorList;
}

std::vector<uint8_t> Characteristic::ReadValue(const std::map<std::string, DBus::Variant> &options, DBus::Error &error)
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

void Characteristic::WriteValue(const std::vector<uint8_t> &value, const std::map<std::string, DBus::Variant> &options,
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

void Characteristic::StartNotify(DBus::Error &)
{
    _notifySubscribersCnt++;

    Notifying = true;
}

void Characteristic::StopNotify(DBus::Error &)
{
    if (_notifySubscribersCnt > 0)
        _notifySubscribersCnt--;

    if (_notifySubscribersCnt == 0)
        Notifying = false;
}

void Characteristic::emitPropertyChanged(const std::string &propertyName)
{
    std::map<std::string, DBus::Variant> propertyInfo = {{propertyName, *get_property(propertyName)}};

    DBus::SignalMessage signal(path().c_str(), propertiesInterfaceName, propertiesChangedSignal);

    auto writer = signal.writer();
    writer << name();
    writer << propertyInfo;

    emit_signal(signal);
}
