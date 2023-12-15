#include <algorithm>
#include <blefacade/bluez/bluez_backend.hh>
#include <utils.hh>

using namespace softeq::ble::bluez;

BluezBackend::BluezBackend()
    : _profilesCount(0)
    , _advertisementIsConfigured(false)
{
    DBus::Connection connection = _dbusManager.connection();

    _bluezObjectManager = std::unique_ptr<BluezObjectManager>(new BluezObjectManager(connection));

    _applicationManager =
        std::unique_ptr<ApplicationManager>(new ApplicationManager(connection, _bluezObjectManager->gattManagerPath()));
    _advertisingManager = std::unique_ptr<AdvertisingManager>(
        new AdvertisingManager(connection, _bluezObjectManager->leAdvertisingManagerPath()));
}

BluezBackend::~BluezBackend()
{
    _advertisingManager->unregisterAdvertisement();
    _applicationManager->unregisterApplications();
}

bool BluezBackend::configureProfile(ble::core::Profile &profile)
{
    auto idx = _profilesCount++;

    DBus::Connection connection = _dbusManager.connection();

    auto app = std::make_shared<Application>(connection, _bluezObjectManager->gattManagerPath() + "/application_" +
                                                             std::to_string(idx));

    for (unsigned int i = 0; i < profile.getServiceNum(); i++)
    {
        auto bluezService = std::make_shared<Service>(connection, app->path() + "/service_" + std::to_string(i));
        auto &bleService = profile.getService(i);

        convertService(connection, bleService, bluezService);

        app->addService(bluezService);

        _servicesUuids.push_back(bluezService->strUuid());
    }

    if (!_applicationManager->registerApplication(app, {}))
    {
        std::cout << "Failed to register gatt application" << std::endl;
        return false;
    }

    if (!configureAdvertisement())
        return false;

    return true;
}

bool BluezBackend::configureAdvertisement()
{
    if (_advertisementIsConfigured)
    {
        _advertisingManager->unregisterAdvertisement();
    }

    DBus::Connection connection = _dbusManager.connection();

    auto advertisementData =
        std::unique_ptr<Advertisement>(new Advertisement(connection, _advertisingManager->path() + "/advertisement_1"));
    advertisementData->setType(Advertisement::Type::Peripheral);
    advertisementData->setLocalName("bluez-gatt-server");
    advertisementData->setIncludeTxPower(true);
    advertisementData->setServiceUUIDs(_servicesUuids);

    if (!_advertisingManager->registerAdvertisement(std::move(advertisementData), {}))
    {
        _applicationManager->unregisterApplications();
        std::cout << "Failed to register advertisement" << std::endl;
        return false;
    }

    _advertisementIsConfigured = true;

    return true;
}

void BluezBackend::convertService(DBus::Connection &connection, ble::core::Service &bleService,
                                  std::shared_ptr<Service> &bluezService)
{
    bluezService->setUuid(bleService.getUuid());
    bluezService->setPrimary(bleService.isPrimary());

    for (size_t j = 0; j < bleService.getCharNum(); j++)
    {
        auto bluezChar =
            std::make_shared<Characteristic>(connection, bluezService->path() + "/characteristic_" + std::to_string(j));
        auto &bleChar = bleService.getChar(j);

        convertCharacteristic(connection, bleChar, bluezChar);

        bluezService->addCharacteristic(bluezChar);
    }
}

void BluezBackend::convertCharacteristic(DBus::Connection &connection, ble::core::Characteristic &bleChar,
                                         std::shared_ptr<Characteristic> &bluezChar)
{
    bluezChar->setUuid(bleChar.getUuid());

    CharacteristicFlags flags = CharacteristicFlags::Unknown; // TODO: remove unknown from flags
    if (bleChar.getPermissions().isSet(ble::core::Permission::read))
    {
        flags = flags | CharacteristicFlags::Read;
    }
    if (bleChar.getPermissions().isSet(ble::core::Permission::write))
    {
        flags = flags | CharacteristicFlags::Write;
    }
    if (bleChar.getProperties().isSet(ble::core::Property::notify))
    {
        flags = flags | CharacteristicFlags::Notify;
    }
    if (bleChar.getProperties().isSet(ble::core::Property::indicate))
    {
        flags = flags | bluez::CharacteristicFlags::Indicate;
    }
    bluezChar->setFlags(flags);

    bluezChar->addReadValueHandler([&](const std::map<std::string, DBus::Variant> &options) {
        // TODO: optimize that
        std::vector<uint8_t> ba(517); // TODO: remove magic number
        size_t len = 0;
        ble::core::ReadParam param;
        DBus::MessageIter it = options.at("device").reader();
        param.conn_id = DevicePathConverter::toUint64(it.get_path());
        bleChar.onReadEvent(*this, ba.data(), len, param);
        ba.resize(len);
        return ByteArray(ba);
    });

    bluezChar->addWriteValueHandler([&](const ByteArray &value, const std::map<std::string, DBus::Variant> &options) {
        ble::core::WriteParam param;
        DBus::MessageIter it = options.at("device").reader();
        param.conn_id = DevicePathConverter::toUint64(it.get_path());
        bleChar.onWriteEvent(*this, value.data().data(), value.data().size(), param);
        return true;
    });

    for (unsigned int k = 0; k < bleChar.getDescrNum(); k++)
    {
        auto bluezDesc =
            std::make_shared<Descriptor>(connection, bluezChar->path() + "/descriptor_" + std::to_string(k));
        auto &bleDesc = bleChar.getDescr(k);

        convertDescriptor(bleDesc, bluezDesc);

        bluezChar->addDescriptor(bluezDesc);
    }
}

void BluezBackend::convertDescriptor(ble::core::Descriptor &bleDesc, std::shared_ptr<Descriptor> &bluezDesc)
{
    bluezDesc->setUuid(bleDesc.getUuid());

    DescriptorFlags flags = DescriptorFlags::Unknown; // TODO: remove unknown from flags
    if (bleDesc.getPermissions().isSet(ble::core::Permission::read))
    {
        flags = flags | DescriptorFlags::Read;
    }
    if (bleDesc.getPermissions().isSet(ble::core::Permission::write))
    {
        flags = flags | DescriptorFlags::Write;
    }
    bluezDesc->setFlags(flags);

    bluezDesc->addReadValueHandler([&](const std::map<std::string, DBus::Variant> &options) {
        // TODO: optimize that
        std::vector<uint8_t> ba(517); // TODO: remove magic number
        size_t len = 0;
        ble::core::ReadParam param;
        DBus::MessageIter it = options.at("device").reader();
        param.conn_id = DevicePathConverter::toUint64(it.get_path());
        bleDesc.onReadEvent(*this, ba.data(), len, param);
        ba.resize(len);
        return ByteArray(ba);
    });

    bluezDesc->addWriteValueHandler([&](const ByteArray &value, const std::map<std::string, DBus::Variant> &options) {
        ble::core::WriteParam param;
        DBus::MessageIter it = options.at("device").reader();
        param.conn_id = DevicePathConverter::toUint64(it.get_path());
        bleDesc.onWriteEvent(*this, value.data().data(), value.data().size(), param);
        return true;
    });
}

bool BluezBackend::indicate(softeq::ble::core::Characteristic &iChar, uint8_t *value, size_t len)
{
    auto appsList = _applicationManager->getApps();
    for (auto app : appsList)
    {
        auto serviceList = app->getServices();
        for (auto service : serviceList)
        {
            auto charsList = service->getCharacteristics();
            for (auto chr : charsList)
            {
                const std::string UUID = chr->strUuid();
                if (UUID == iChar.getUuid().getPrintable()) // Find target characteristic
                {
                    if (!value || len == 0) // if any external data is not provided for notification
                    {
                        // try to get bound to the characteristic data
                        len = iChar.getData().len;
                        value = reinterpret_cast<uint8_t *>(iChar.getData().data);
                    }
                    if (value && len > 0) // check if we still have any valid data to send
                    {
                        std::vector<uint8_t> data(len);
                        memcpy(data.data(), value, len);
                        softeq::ble::bluez::ByteArray array{data};
                        chr->updateValue(array);
                        return true;
                    }                    
                }
            }
        }
    }
    return false;
};
