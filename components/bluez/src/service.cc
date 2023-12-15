#include <service.hh>

using namespace softeq::ble::bluez;

Service::Service(DBus::Connection &connection, const std::string &objectPath)
    : DBus::ObjectAdaptor(connection, objectPath)
    , _characteristicList()
{
}

void Service::setUuid(const softeq::ble::core::UUID &uuid)
{
    UUID = uuid.getPrintable();
}

void Service::setPrimary(bool isPrimary)
{
    Primary = isPrimary;
}

softeq::ble::core::UUID Service::strUuid()
{
    return softeq::ble::core::UUID(get_property("UUID")->operator std::string().c_str());
}

void Service::addCharacteristic(std::shared_ptr<Characteristic> characteristic)
{
    characteristic->setService(path());

    _characteristicList.emplace_back(characteristic);
}

std::list<std::shared_ptr<Characteristic>> Service::getCharacteristics() const
{
    return _characteristicList;
}
