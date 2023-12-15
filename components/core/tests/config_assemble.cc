#include <blefacade/core/GATT/characteristic.hh>
#include <blefacade/core/GATT/descriptor.hh>
#include <blefacade/core/GATT/handlers.hh>
#include <blefacade/core/GATT/profile.hh>
#include <blefacade/core/GATT/service.hh>
#include <blefacade/core/GATT/uuid.hh>
#include <gtest/gtest.h>

using namespace softeq::ble::core;

class GattConfig : public ::testing::Test
{
};

TEST_F(GattConfig, ServiceAssemble)
{
    Profile profile{{
        Service{UUID{0x1234},
                Service::Primary::yes,
                {
                    Characteristic{UUID{0xabcd},
                                   Permission::read,
                                   Property::nop,
                                   {
                                       Descriptor{UUID{0xab00}, Permission::read},
                                       Descriptor{UUID{0xab01}, Permission::read | Permission::write},
                                   }},
                    Characteristic{UUID{0xfeca}, Permission::read | Permission::write, Property::nop, {}},
                    Characteristic{UUID{0xfecd}, Permission::read | Permission::write, Property::nop, {}},

                }},
        Service{UUID{0x12345678},
                Service::Primary::no,
                {
                    Characteristic{UUID{0xabc1}, Permission::read, Property::nop, {}},
                    Characteristic{UUID{0xabc2}, Permission::read, Property::nop, {}},
                }},

    }};

    ASSERT_EQ(profile.getServiceNum(), 2);

    ASSERT_EQ(profile.getService(0).getUuid().getLength(), UUID::Length::LEN16);
    ASSERT_EQ(profile.getService(0).getUuid().getValue16(), 0x1234);
    ASSERT_EQ(profile.getService(0).isPrimary(), true);

    ASSERT_EQ(profile.getService(1).getUuid().getLength(), UUID::Length::LEN32);
    ASSERT_EQ(profile.getService(1).getUuid().getValue32(), 0x12345678);
    ASSERT_EQ(profile.getService(1).isPrimary(), false);
}
