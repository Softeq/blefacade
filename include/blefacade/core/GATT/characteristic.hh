#ifndef CHARACTERISTIC_HH
#define CHARACTERISTIC_HH
#include "attribute.hh"
#include "descriptor.hh"
#include <blefacade/core/utils/slist.hh>
#include <initializer_list>

namespace softeq
{
namespace ble
{
namespace core
{
enum Property : uint8_t
{
    broadcast = (1 << 0),
    writeWoResp = (1 << 1),
    notify = (1 << 2),
    indicate = (1 << 3),
    nop = (0),
};

inline Property operator|(const Property &lhs, const Property &rhs)
{
    return Property(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

class Characteristic : public AttributeProxy<Characteristic>
{

private:
    Property _props;
    Slist<Descriptor> _descrs;

public:
    Characteristic(UUID &&uuid, const Permission &permissions, const Property &properties,
                   std::initializer_list<Descriptor> &&descrs)
        : AttributeProxy(std::move(uuid), permissions)
        , _props(properties)
    {
        _descrs.insert(_descrs.begin(), descrs.begin(), descrs.end());
    }

    Characteristic(UUID &&uuid, const Permission &permissions, const Property &properties, const CBData &data,
                   std::initializer_list<Descriptor> &&descrs)
        : AttributeProxy(std::move(uuid), permissions, data)
        , _props(properties)
    {
        _descrs.insert(_descrs.begin(), descrs.begin(), descrs.end());
    }

    Characteristic(UUID &&uuid, const Permission &permissions, const Property &properties, CBData &data,
                   std::initializer_list<Descriptor> &&descrs)
        : AttributeProxy(std::move(uuid), permissions, data)
        , _props(properties)
    {
        _descrs.insert(_descrs.begin(), descrs.begin(), descrs.end());
    }

    Characteristic(UUID &&uuid, const Permission &permissions, const Property &properties, CBData &data,
                   gattWriteHandler whandler, gattReadHandler rhandler, std::initializer_list<Descriptor> &&descrs)
        : Characteristic(std::move(uuid), permissions, properties, data, std::move(descrs))
    {
        replaceReadHandler(rhandler);
        replaceWriteHandler(whandler);
    }

    Characteristic(UUID &&uuid, const Permission &permissions, const Property &properties, CBData &data,
                   gattWriteHandler whandler, std::initializer_list<Descriptor> &&descrs)
        : Characteristic(std::move(uuid), permissions, properties, data, std::move(descrs))
    {
        replaceWriteHandler(whandler);
    }

    Characteristic(UUID &&uuid, const Permission &permissions, const Property &properties, const CBData &data,
                   gattReadHandler rhandler, std::initializer_list<Descriptor> &&descrs)
        : Characteristic(std::move(uuid), permissions, properties, data, std::move(descrs))
    {
        replaceReadHandler(rhandler);
    }

    Characteristic(UUID &&uuid, const Permission &permissions, const Property &properties, CBData &data,
                   gattReadHandler rhandler, std::initializer_list<Descriptor> &&descrs)
        : Characteristic(std::move(uuid), permissions, properties, data, std::move(descrs))
    {
        replaceReadHandler(rhandler);
    }

    Characteristic(UUID &&uuid, const Permission &permissions, const Property &properties,
                   std::function<gattWriteHandler> whandler, std::function<gattReadHandler> rhandler,
                   std::initializer_list<Descriptor> &&descrs)
        : Characteristic(std::move(uuid), permissions, properties, std::move(descrs))
    {
        replaceReadHandler(rhandler);
        replaceWriteHandler(whandler);
    }

    Characteristic() = default;

    const Descriptor &getDescr(const size_t index) const
    {
        return _descrs.get(index);
    }

    Descriptor &getDescr(const size_t index)
    {
        return const_cast<Descriptor &>(const_cast<const Characteristic *>(this)->getDescr(index));
    }

    unsigned int getDescrNum() const
    {
        return _descrs.size();
    }

    AttFlags getProperties() const
    {
        return _props;
    }

    virtual Characteristic &retVal() override
    {
        return *this;
    };

    Characteristic &property(const Property &props)
    {
        _props = props;
        return retVal();
    }

    Descriptor &addDescriptor()
    {
        _descrs.emplace_back();
        return _descrs.back();
    }

    ~Characteristic() = default;
};

} // namespace core
} // namespace ble
} // namespace softeq

#endif //CHARACTERISTIC_HH