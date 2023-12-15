#ifndef SOFT_SECURITY
#define SOFT_SECURITY

#include <blefacade/core/GATT/handlers.hh>
#include <blefacade/core/GATT/service.hh>
#include <blefacade/core/backend/ble_backend_if.hh>
#include <blefacade/core/utils/slist.hh> // TODO: to replace with static list when it comes to static design
#include <functional>
#include <tuple>

namespace softeq
{
namespace ble
{
namespace security
{
class SoftSecurity
{
public:
    SoftSecurity() = default;

    void onClientDisconnect(uint64_t connectionID);

    bool isAuthorized(uint64_t connectionID)
    {
        return std::find(authClients.cbegin(), authClients.cend(), connectionID) != authClients.cend();
    }

    void confAuthChar(softeq::ble::core::Attribute &att, softeq::ble::core::BleBackendIf &bck);

    void protectProfile(softeq::ble::core::Profile &prof);
    void protectService(softeq::ble::core::Service &srv);
    void protectAttribute(softeq::ble::core::Attribute &att);

    // Handlers may be defined by users
    virtual softeq::ble::core::ErrorCode authHandler(softeq::ble::core::BleBackendIf &bck,
                                                     softeq::ble::core::Attribute &att, const uint8_t *data, size_t len,
                                                     const softeq::ble::core::WriteParam &param);
    virtual softeq::ble::core::ErrorCode readHandler(softeq::ble::core::BleBackendIf &bck,
                                                     softeq::ble::core::Attribute &att, uint8_t *data, size_t &len,
                                                     const softeq::ble::core::ReadParam &param);
    virtual softeq::ble::core::ErrorCode clientRegHandler(softeq::ble::core::BleBackendIf &bck,
                                                          const softeq::ble::core::Event &event,
                                                          const softeq::ble::core::EventParam &param);

    // Wrappers for locked read and write handlers
    std::function<softeq::ble::core::gattWriteHandler>
    protectCall(std::function<softeq::ble::core::gattWriteHandler> hndlr);
    std::function<softeq::ble::core::gattReadHandler>
    protectCall(std::function<softeq::ble::core::gattReadHandler> hndlr);

    void dropAll()
    {
        authClients.clear();
    }

protected:
    virtual bool authenticate(const uint8_t *, size_t) = 0; // To implement by a concrete class
    void authorize(uint64_t connectionID);

private:
    softeq::ble::core::Slist<uint64_t> authClients;
    softeq::ble::core::Attribute *_authAtt = nullptr;
};
} // namespace security
} // namespace ble
} // namespace softeq

#endif // SOFT_SECURITY