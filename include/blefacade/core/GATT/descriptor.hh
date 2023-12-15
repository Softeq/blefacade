#ifndef DESCRIPTOR_HH
#define DESCRIPTOR_HH
#include "attribute.hh"

namespace softeq
{
namespace ble
{
namespace core
{
class Descriptor : public AttributeProxy<Descriptor> // TODO : inherit from permissions class
{
private:
public:
    Descriptor(const Descriptor &descr) = default;

    Descriptor() = default;

    Descriptor(Descriptor &&descr) = default;

    Descriptor(UUID &&uuid, const Permission &permissions)
        : AttributeProxy(std::move(uuid), permissions)
    {
    }

    Descriptor(UUID &&uuid, const Permission &permissions, const CBData &data)
        : AttributeProxy(std::move(uuid), permissions, data)
    {
    }

    Descriptor(UUID &&uuid, const Permission &permissions, CBData &data)
        : AttributeProxy(std::move(uuid), permissions, data)
    {
    }

    virtual Descriptor &retVal() override
    {
        return *this;
    };

    ~Descriptor() = default;
};

} // namespace core
} // namespace ble
} // namespace softeq

#endif //DESCRIPTOR_HH