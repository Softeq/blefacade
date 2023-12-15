#include <blefacade/bluez/bluez_backend.hh>
#include <blefacade/core/GATT/att_utils.hh>
#include <chrono>
#include <iostream>

using namespace softeq::ble::core;

extern Profile profileMain;
extern Profile profileAux;

int main()
{
    softeq::ble::bluez::BluezBackend bb;

    bb.configureProfile(profileMain);
    bb.configureProfile(profileAux);

    std::cout << "Hello, i'm bluez-gatt-server!" << std::endl;

    Attribute *att = findAttributeByUuid(profileMain, UUID{0xabcd});
    uint8_t tmp = 0xfa;
    auto lastUpdate = std::chrono::system_clock::now();
    const std::chrono::duration<int64_t> timeout = std::chrono::seconds(5);
    while (true)
    {
        auto now = std::chrono::system_clock::now();
        if (att && now > (lastUpdate + timeout))
        {
            lastUpdate = now;
            bb.indicate(*static_cast<Characteristic *>(att), &tmp, 1);
            tmp++;
        }
    }

    return 0;
}