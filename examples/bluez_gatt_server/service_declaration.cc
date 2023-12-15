#include <blefacade/core/GATT/handlers.hh>
#include <blefacade/core/GATT/profile.hh>
#include <blefacade/core/GATT/service.hh>
#include <blefacade/core/GATT/uuid.hh>
#include <blefacade/core/GATT/uuid_defs.hh>

using namespace softeq::ble::core;

static uint16_t char_abcd;
static uint16_t desc_cccd;
static char desc_text[] = "Hello world";

Profile profileMain{
    {Service{UUID{0x1234},
             Service::Primary::yes,
             {
                 Characteristic{UUID{0xabcd},
                                Permission::read,
                                Property::notify | Property::indicate,
                                bindData(char_abcd),
                                {
                                    Descriptor{DType::ClientCharacteristicConfiguration,
                                               Permission::read | Permission::write, bindData(desc_cccd)},
                                    Descriptor{DType::CharacteristicUserDescription,
                                               Permission::read | Permission::write, bindData(desc_text)},
                                }},
                 Characteristic{UUID{0xfeca}, Permission::read | Permission::write, Property::nop, {}},

             }},
     Service{UUID{0x0345},
             Service::Primary::no,
             {
                 Characteristic{UUID{0xabc1}, Permission::read, Property::nop, {}},
                 Characteristic{UUID{0xabc2}, Permission::read, Property::nop, {}},
             }}

    }};

Profile profileAux{{Service{UUID{0x0345},
                            Service::Primary::yes,
                            {
                                Characteristic{UUID{0xabc3}, Permission::read, Property::nop, {}},
                                Characteristic{UUID{0xabc4}, Permission::read, Property::nop, {}},
                            }}}};