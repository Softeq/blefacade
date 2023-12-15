#ifndef SERVICE_HH
#define SERVICE_HH
#include "attribute.hh"
#include "characteristic.hh"
#include <blefacade/core/utils/slist.hh>
#include <initializer_list>

namespace softeq
{
namespace ble
{
namespace core
{
class Service
{
public:
    enum class Primary : bool
    {
        yes = true,
        no = false,
    };

    Service(UUID &&uuid, Primary prim, std::initializer_list<Characteristic> &&chars)
        : _uuid(uuid)
        , _primary(static_cast<bool>(prim))
    {

        _chars.insert(_chars.begin(), chars.begin(), chars.end());
    }

    Service() = default;

    bool isPrimary()
    {
        return _primary;
    }

    Service &uuid(UUID uuid)
    {
        _uuid = uuid;
        return *this;
    }

    Service &primary(bool prim)
    {
        _primary = prim;
        return *this;
    }

    size_t getCharNum() const
    {
        return _chars.size();
    }

    const Characteristic &getChar(const size_t index) const
    {
        return _chars.get(index);
    }

    Characteristic &getChar(const size_t index)
    {
        return const_cast<Characteristic &>(const_cast<const Service *>(this)->getChar(index));
    }

    const UUID &getUuid() const
    {
        return _uuid;
    }

    uint16_t getHandle() const
    {
        return _handle;
    }
    void setHandle(uint16_t handle)
    {
        _handle = handle;
    }

    Characteristic &addCharacteristic()
    {
        _chars.emplace_back();
        return _chars.back();
    }

    ~Service() = default;

private:
    Slist<Characteristic> _chars;
    uint16_t _handle = 0;
    UUID _uuid;
    bool _primary;
};

} // namespace core
} // namespace ble
} // namespace softeq

#endif //SERVICE_HH