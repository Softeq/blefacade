#include <advertisement.hh>

#include <algorithm>

using namespace softeq::ble::bluez;

namespace
{
static std::map<Advertisement::Type, std::string> advertisementType = {{Advertisement::Type::Broadcast, "broadcast"},
                                                                       {Advertisement::Type::Peripheral, "peripheral"}};
}

Advertisement::Advertisement(DBus::Connection &connection, const DBus::Path &objectPath)
    : DBus::ObjectAdaptor(connection, objectPath)
{
}

void Advertisement::setServiceUUIDs(const std::vector<softeq::ble::core::UUID> &uuids)
{
    std::vector<std::string> stringUUIDs;

    std::transform(uuids.begin(), uuids.end(), std::back_inserter(stringUUIDs),
                   [](const softeq::ble::core::UUID &uuid) { return uuid.getPrintable(); });

    ServiceUUIDs = stringUUIDs;
}

void Advertisement::setSolicitUUIDs(const std::vector<softeq::ble::core::UUID> &uuids)
{
    std::vector<std::string> stringUUIDs;

    std::transform(uuids.begin(), uuids.end(), std::back_inserter(stringUUIDs),
                   [](const softeq::ble::core::UUID &uuid) { return uuid.getPrintable(); });

    SolicitUUIDs = stringUUIDs;
}

void Advertisement::setManufacturerData(const std::map<std::string, DBus::Variant> &manufacturerData)
{
    ManufacturerData = manufacturerData;
}

void Advertisement::setServiceData(const std::map<std::string, DBus::Variant> &serviceData)
{
    ServiceData = serviceData;
}

void Advertisement::setIncludeTxPower(bool isIncludeTxPower)
{
    IncludeTxPower = isIncludeTxPower;
}

void Advertisement::setLocalName(const std::string &localName)
{
    LocalName = localName;
}

void Advertisement::setType(Advertisement::Type type)
{
    org::bluez::LEAdvertisement1_adaptor::Type = advertisementType.at(type);
}

void Advertisement::Release(DBus::Error &)
{
    /* TODO: Set the error if something went wrong.

    Description from https://git.kernel.org/pub/scm/bluetooth/bluez.git/tree/doc/advertising-api.txt#n29 :
    This method gets called when the service daemon
	removes the Advertisement. A client can use it to do
	cleanup tasks. There is no need to call
	UnregisterAdvertisement because when this method gets
	called it has already been unregistered.
    */
}

std::map<DBus::Path, std::map<std::string, std::map<std::string, DBus::Variant>>>
Advertisement::GetManagedObjects(DBus::Error &)
{
    return {{path(),
             {{org::bluez::LEAdvertisement1_adaptor::name(),
               *org::bluez::LEAdvertisement1_adaptor::get_all_properties()}}}};
}
