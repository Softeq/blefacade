#include "simple_softsec.hh"
#include <blefacade/bluez/bluez_backend.hh>
#include <blefacade/core/GATT/att_utils.hh>
#include <iostream>

using namespace softeq::ble::core;
using namespace softeq::ble::security;

static char protectedData[] = "Hello world";

static ErrorCode dummyWriteHandler(BleBackendIf &bck, Attribute &att, const uint8_t *data, size_t len,
                                   const WriteParam &param)
{
    (void)bck;
    (void)att;
    size_t dataLen = sizeof(protectedData) / sizeof(protectedData[0]);
    if (dataLen >= len)
    {
        memcpy(protectedData, data, len);
        return Error::ERR_OK;
    }
    else
    {
        // log issue here
        return Error::ERR_LEN_MISMATCH;
    }
};

static ErrorCode dummyReadHandler(BleBackendIf &bck, Attribute &att, uint8_t *data, size_t &len, const ReadParam &param)
{
    (void)bck;
    (void)att;
    len = sizeof(protectedData) / sizeof(protectedData[0]);
    memcpy(data, protectedData, len);
    return Error::ERR_OK;
};


int main()
{
    softeq::ble::bluez::BluezBackend backend;
    SimpleSoftSecurity secService;

    Profile authProfile{{
        Service{UUID{0x1234},
                Service::Primary::yes,
                {
                    Characteristic{UUID{0xabcd}, Permission::read | Permission::write, Property::nop, {}},
                }},
    }};

    Profile protectedProfile{{
        Service{UUID{0x5678},
                Service::Primary::yes,
                {
                    Characteristic{UUID{0xef10}, Permission::read | Permission::write, Property::nop, {}},
                }},
    }};

    Attribute *authAtt = findAttributeByUuid(authProfile, UUID{0xabcd});
    if (authAtt)
    {
        secService.confAuthChar(*authAtt, backend);
    }
    else
    {
        std::cout << "Proper attribute is not found" << std::endl;
    }

    Attribute *protectedAtt = findAttributeByUuid(protectedProfile, UUID{0xef10});
    if (protectedAtt)
    {
        protectedAtt->replaceWriteHandler(secService.protectCall(dummyWriteHandler));
        protectedAtt->replaceReadHandler(secService.protectCall(dummyReadHandler));
    }
    else
    {
        std::cout << "Proper attribute is not found" << std::endl;
    }

    backend.configureProfile(authProfile);
    backend.configureProfile(protectedProfile);

    std::cout << "Hello, i'm bluez-gatt-server!" << std::endl;
    std::cout << "Enter 'q' to quit" << std::endl;

    char ch;
    while (ch != 'q')
    {
        std::cin >> ch;
    }

    return 0;
}