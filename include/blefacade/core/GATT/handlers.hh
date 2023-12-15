#ifndef HANDLERS_HH
#define HANDLERS_HH
#include <functional>
#include <stdint.h>
// #include <blefacade/core/backend/ble_backend_if.hh>

// TODO: remake errors as enum classes

namespace softeq
{
namespace ble
{
namespace core
{
enum class Error
{
    ERR_OK,
    ERR_LEN_MISMATCH,
    NO_HANDLER,
    NO_BOUND_DATA,
    ERR_NOT_PERMITTED,
    ERR_AUTH,
};

class Attribute;
class BleBackendIf;

// New callbacks implementation
struct WriteParam
{
    // TODO: Define
    uint64_t conn_id;
    bool seq_write;
    uint16_t offset;
};

struct ReadParam
{
    // TODO: Define
    uint64_t conn_id;
};

struct EventParam
{
    // TODO: Define
    uint64_t conn_id;
};

enum class Event
{
    // TODO: Define
    CONNECT,
    DISCONNECT,
    EXEC_WRITE,
};

using ErrorCode = Error;

typedef ErrorCode(gattWriteHandler)(BleBackendIf &bck, Attribute &att, const uint8_t *data, size_t len,
                                    const WriteParam &param);
typedef ErrorCode(gattReadHandler)(BleBackendIf &bck, Attribute &att, uint8_t *data, size_t &len,
                                   const ReadParam &param);
typedef ErrorCode(gattEventHandler)(BleBackendIf &bck, const Event &event, const EventParam &param);

template <typename C, typename O>
std::function<gattWriteHandler> bindWriteCMethod(C classMethod, O object)
{
    return std::bind(classMethod, object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                     std::placeholders::_4, std::placeholders::_5);
}

template <typename C, typename O>
std::function<gattReadHandler> bindReadCMethod(C classMethod, O object)
{
    return std::bind(classMethod, object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                     std::placeholders::_4, std::placeholders::_5);
}

template <typename C, typename O>
std::function<gattEventHandler> bindGattEventHandler(C classMethod, O object)
{
    return std::bind(classMethod, object, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}
} // namespace core
} // namespace ble
} // namespace softeq

#endif // HANDLERS_HH