#include <application.hh>
#include <service.hh>

#include <iostream>

using namespace softeq::ble::bluez;

Application::Application(DBus::Connection &connection, const std::string &objectPath)
    : DBus::ObjectAdaptor(connection, objectPath.c_str(), REGISTER_NOW, USE_EXCEPTIONS)
    , _serviceList()
{
}

std::map<DBus::Path, std::map<std::string, std::map<std::string, DBus::Variant>>>
Application::GetManagedObjects(DBus::Error &)
{
    std::map<DBus::Path, std::map<std::string, std::map<std::string, DBus::Variant>>> res;

    for (const auto service : _serviceList)
    {
        res[service->path()] = {{service->name(), *service->get_all_properties()}};

        for (const auto &characteristic : service->getCharacteristics())
        {
            res[characteristic->path()] = {{characteristic->name(), *characteristic->get_all_properties()}};

            for (const auto &descriptor : characteristic->getDescriptors())
            {
                res[descriptor->path()] = {{descriptor->name(), *descriptor->get_all_properties()}};
            }
        }
    }

    return res;
}

void Application::addService(std::shared_ptr<Service> service)
{
    _serviceList.emplace_back(service);
}

std::list<std::shared_ptr<Service>> Application::getServices()
{
    return _serviceList;
}
