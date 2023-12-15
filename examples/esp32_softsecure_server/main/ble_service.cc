#include "ble_service.hh"
#include <blefacade/core/GATT/handlers.hh>
#include <blefacade/core/GATT/profile.hh>
#include <blefacade/core/GATT/service.hh>
#include <blefacade/core/GATT/uuid.hh>
#include <blefacade/core/GATT/uuid_defs.hh>

using namespace softeq::ble::core;
using namespace softeq::ble::security;

BleService::BleService(std::function<gattWriteHandler> wHandler, std::function<gattReadHandler> rHandler)
{
    // Create a hierarchy of services, characteristics
    // clang-format off
    auto &authServ = _authProfile.addService().uuid(0x1234)
                                              .primary(true);

    auto &authAtt = authServ.addCharacteristic().uuid(0xabcd)
                                                .permission(Permission::write);

    auto &protServ = _protectedProfile.addService().uuid(0x5678)
                                                   .primary(true);

    auto &protectedAtt = protServ.addCharacteristic().uuid(0xabcd)
                                                     .permission(Permission::read | Permission::write)
                                                     .onReadHandler(rHandler)
                                                     .onWriteHandler(wHandler);
    // clang-format on

    _secService.confAuthChar(authAtt, _backend);
    _secService.protectProfile(_protectedProfile);
}

void BleService::init()
{
    _backend.init();
    _backend.configureProfile(_authProfile);
    _backend.configureProfile(_protectedProfile);
}