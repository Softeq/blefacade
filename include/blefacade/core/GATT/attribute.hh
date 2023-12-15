#ifndef ATTRIBUTE_HH
#define ATTRIBUTE_HH
#include "handlers.hh"
#include "uuid.hh"
#include <blefacade/core/utils/bound_data.hh>
#include <cstring>
#include <functional>

namespace softeq
{
namespace ble
{
namespace core
{
enum Permission : uint8_t // TODO : add another permissions
{
    read = (1 << 0),
    write = (1 << 1),
    authSignedWr = (1 << 2),
    authSignedRd = (1 << 3),
};

inline Permission operator|(const Permission &lhs, const Permission &rhs)
{
    return Permission(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}
class AttFlags // TODO : template it
{
private:
    uint8_t _flags;

public:
    AttFlags()
        : _flags(0)
    {
    }
    AttFlags(uint8_t flag)
        : _flags(flag)
    {
    }
    void setFlag(uint8_t flag)
    {
        _flags |= flag;
    }
    bool isSet(uint8_t flag) const
    {
        return _flags & flag;
    }
};
class Attribute
{
protected:
    uint16_t _handle = 0;
    std::function<gattWriteHandler> _onWriteHandler = defaultWriteHandler;
    std::function<gattReadHandler> _onReadHandler = defaultReadHandler;
    UUID _uuid;
    Permission _perms;
    CBData _data = {};

public:
    Attribute() = default;

    Attribute(const UUID &uuid, const Permission &permissions)
        : _uuid(uuid)
        , _perms(permissions)
    {
    }

    Attribute(const UUID &uuid, const Permission &permissions, const CBData &data)
        : _uuid(uuid)
        , _perms(permissions)
        , _data(data)
    {
    }

    const CBData &getData() const
    {
        return _data;
    }

    CBData &getData()
    {
        return _data;
    }

    void setData(const CBData &data)
    {
        _data = data;
    }

    uint16_t getHandle() const
    {
        return _handle;
    }

    void setHandle(uint16_t handle)
    {
        _handle = handle;
    }

    void replaceWriteHandler(const std::function<gattWriteHandler> &handler)
    {
        _onWriteHandler = handler;
    }

    void replaceReadHandler(const std::function<gattReadHandler> &handler)
    {
        _onReadHandler = handler;
    }

    AttFlags getPermissions() const
    {
        return _perms;
    }

    std::function<gattWriteHandler> getOnWriteHandler()
    {
        return _onWriteHandler;
    }

    std::function<gattReadHandler> getOnReadHandler()
    {
        return _onReadHandler;
    }

    ErrorCode onReadEvent(BleBackendIf &bck, uint8_t *data, size_t &len, const ReadParam &param)
    {
        if (_onReadHandler)
        {
            return _onReadHandler(bck, *this, data, len, param);
        }
        else
        {
            // log issue here
            return Error::NO_HANDLER;
        }
    }

    ErrorCode onWriteEvent(BleBackendIf &bck, const uint8_t *data, size_t len, const WriteParam &param)
    {
        if (_onWriteHandler)
        {
            return _onWriteHandler(bck, *this, data, len, param);
        }
        else
        {
            // log issue here
            return Error::NO_HANDLER;
        }
    }

    const UUID &getUuid() const
    {
        return _uuid;
    }

protected:
    static ErrorCode defaultWriteHandler(BleBackendIf &bck, Attribute &att, const uint8_t *data, size_t len,
                                         const WriteParam &param)
    {
        (void)bck;
        (void)param;
        if (att.getData().data)
        {
            if (att.getData().maxLen >= len)
            {
                memcpy(att.getData().data, data, len);
                att.getData().len = len;
                return Error::ERR_OK;
            }
            else
            {
                // log issue here
                return Error::ERR_LEN_MISMATCH;
            }
        }
        else
        {
            // log issue here
            return Error::NO_BOUND_DATA;
        }
    }

    static ErrorCode defaultReadHandler(BleBackendIf &bck, Attribute &att, uint8_t *data, size_t &len,
                                        const ReadParam &param)
    {
        (void)bck;
        (void)param;
        if (att.getData().data)
        {
            len = att.getData().len;
            memcpy(data, att.getData().data, len);
            return Error::ERR_OK;
        }
        else
        {
            // log issue here
            return Error::NO_BOUND_DATA;
        }
    }
};

template <typename T>
class AttributeProxy : public Attribute
{
public:
    AttributeProxy(const UUID &uuid, const Permission &permissions)
        : Attribute(uuid, permissions)
    {
    }

    AttributeProxy(const UUID &uuid, const Permission &permissions, const CBData &data)
        : Attribute(uuid, permissions, data)
    {
    }

    AttributeProxy() = default;

    T &uuid(UUID uuid)
    {
        _uuid = uuid;
        return retVal();
    }

    T &permission(const Permission &permissions)
    {
        _perms = permissions;
        return retVal();
    }

    T &data(const CBData &data)
    {
        setData(data);
        return retVal();
    }

    T &handle(uint16_t handle)
    {
        setHandle(handle);
        return retVal();
    }

    T &onWriteHandler(const std::function<gattWriteHandler> &handler)
    {
        replaceWriteHandler(handler);
        return retVal();
    }

    T &onReadHandler(const std::function<gattReadHandler> &handler)
    {
        replaceReadHandler(handler);
        return retVal();
    }

protected:
    virtual T &retVal() = 0;
};

} // namespace core
} // namespace ble
} // namespace softeq


#endif // ATTRIBUTE_HH