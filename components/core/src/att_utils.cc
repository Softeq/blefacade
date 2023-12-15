#include <blefacade/core/GATT/att_utils.hh>
#include <blefacade/core/GATT/uuid_defs.hh>

namespace softeq
{
namespace ble
{
namespace core
{
Attribute *findAttributeByHandle(Profile &prof, uint16_t handle)
{
    for (size_t i = 0; i < prof.getServiceNum(); ++i)
    {
        Service &srv = prof.getService(i);
        for (size_t j = 0; j < srv.getCharNum(); ++j)
        {
            Characteristic &chr = srv.getChar(j);
            if (chr.getHandle() == handle)
            {
                return &chr;
            }
            for (size_t k = 0; k < chr.getDescrNum(); ++k)
            {
                Descriptor &decr = chr.getDescr(k);
                if (decr.getHandle() == handle)
                {
                    return &decr;
                }
            }
        }
    }
    return nullptr;
}

Service *findServiceByUuid(Profile &prof, const UUID &uuid)
{
    for (size_t i = 0; i < prof.getServiceNum(); ++i)
    {
        Service &srv = prof.getService(i);
        if (srv.getUuid() == uuid)
        {
            return &srv;
        }
    }
    return nullptr;
}

Attribute *findAttributeByUuid(Profile &prof, const UUID &uuid)
{
    for (size_t i = 0; i < prof.getServiceNum(); ++i)
    {
        Service &srv = prof.getService(i);
        for (size_t j = 0; j < srv.getCharNum(); ++j)
        {
            Characteristic &chr = srv.getChar(j);
            if (chr.getUuid() == uuid)
            {
                return &chr;
            }
            for (size_t k = 0; k < chr.getDescrNum(); ++k)
            {
                Descriptor &decr = chr.getDescr(k);
                if (decr.getUuid() == uuid)
                {
                    return &decr;
                }
            }
        }
    }
    return nullptr;
}

bool profileContainsAttribute(const Profile &prof, const Attribute &att)
{
    for (size_t i = 0; i < prof.getServiceNum(); ++i)
    {
        const Service &srv = prof.getService(i);
        for (size_t j = 0; j < srv.getCharNum(); ++j)
        {
            const Characteristic &chr = srv.getChar(j);
            if (static_cast<const Attribute *>(&chr) == &att)
            {
                return true;
            }
        }
    }
    return false;
}

Descriptor *findCccd(Characteristic &parent)
{
    for (size_t k = 0; k < parent.getDescrNum(); ++k)
    {
        Descriptor &decr = parent.getDescr(k);
        if (decr.getUuid().getLength() == UUID::Length::LEN16 &&
            decr.getUuid().getValue16() == static_cast<uint16_t>(DType::ClientCharacteristicConfiguration))
        {
            return &decr;
        }
    }
    return nullptr;
}

} // namespace core
} // namespace ble
} // namespace softeq
