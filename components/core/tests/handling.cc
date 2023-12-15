#include <blefacade/core/GATT/characteristic.hh>
#include <blefacade/core/GATT/descriptor.hh>
#include <blefacade/core/GATT/handlers.hh>
#include <blefacade/core/GATT/profile.hh>
#include <blefacade/core/GATT/service.hh>
#include <blefacade/core/GATT/uuid.hh>
#include <blefacade/core/GATT/uuid_defs.hh>
#include <blefacade/core/backend/ble_backend_if.hh>
#include <blefacade/core/utils/test_utils.hh>
#include <gtest/gtest.h>

using namespace softeq::ble::core;
using namespace std::placeholders;

class HandlingTest : public ::testing::Test
{
};

// Check if default handlers allow read and write in boundaries of bound data
TEST_F(HandlingTest, DefaultHandling)
{
    MockedBackend mback{};
    uint8_t buf[517] = {};
    size_t readLen = 0;

    uint8_t desc_text[] = "text";
    uint8_t char_abcd = 0xff;

    Profile profile{{
        Service{UUID{0x1234},
                Service::Primary::yes,
                {
                    Characteristic{UUID{0xabcd},
                                   Permission::read,
                                   Property::indicate | Property::notify,
                                   bindData(char_abcd),
                                   {
                                       Descriptor{DType::ClientCharacteristicConfiguration,
                                                  Permission::read | Permission::write, bindData(desc_text)},
                                   }},
                }},
    }};

    Characteristic &chr = profile.getService(0).getChar(0);
    Descriptor &descr = profile.getService(0).getChar(0).getDescr(0);

    ASSERT_EQ(Error::ERR_OK, chr.onReadEvent(mback, buf, readLen, ReadParam{}));
    ASSERT_EQ(readLen, sizeof(char_abcd));
    ASSERT_EQ(buf[0], char_abcd);

    ASSERT_EQ(Error::ERR_OK, descr.onReadEvent(mback, buf, readLen, ReadParam{}));
    ASSERT_EQ(readLen, sizeof(desc_text));
    for (size_t i = 0; i < sizeof(desc_text); ++i)
    {
        ASSERT_EQ(buf[i], desc_text[i]);
    }

    uint8_t testByteData = 0xaf;
    ASSERT_EQ(Error::ERR_OK, chr.onWriteEvent(mback, &testByteData, sizeof(testByteData), WriteParam()));
    ASSERT_EQ(Error::ERR_OK, chr.onReadEvent(mback, buf, readLen, ReadParam{}));
    ASSERT_EQ(readLen, sizeof(testByteData));
    ASSERT_EQ(buf[0], testByteData);

    // Try to write data shorter than bound data buffer
    // The read length shall be equal to written data
    uint8_t testTextShortData[] = "Hi";
    ASSERT_EQ(Error::ERR_OK, descr.onWriteEvent(mback, testTextShortData, sizeof(testTextShortData), WriteParam()));
    ASSERT_EQ(Error::ERR_OK, descr.onReadEvent(mback, buf, readLen, ReadParam{}));
    ASSERT_EQ(readLen, sizeof(testTextShortData));
    for (size_t i = 0; i < sizeof(testTextShortData); ++i)
    {
        ASSERT_EQ(buf[i], testTextShortData[i]);
    }

    // Try to write data wider than bound data buffer
    uint8_t testTextLongData[] = "Hi there, I'm Alex!";

    ASSERT_EQ(Error::ERR_LEN_MISMATCH,
              descr.onWriteEvent(mback, testTextLongData, sizeof(testTextLongData), WriteParam()));
    ASSERT_EQ(Error::ERR_OK, descr.onReadEvent(mback, buf, readLen, ReadParam{}));

    // With default handler, the bound data should not be updated
    ASSERT_EQ(readLen, sizeof(testTextShortData));
    for (size_t i = 0; i < sizeof(testTextShortData); ++i)
    {
        ASSERT_EQ(buf[i], testTextShortData[i]);
    }
}

// Check if custom handlers are set and called
TEST_F(HandlingTest, CustomHandling)
{
    MockedBackend mback{};
    uint8_t buf[517] = {};
    size_t readLen = 0;

    Profile profile{{
        Service{UUID{0x1234},
                Service::Primary::yes,
                {
                    Characteristic{
                        UUID{0xabcd},
                        Permission::read,
                        Property::notify,
                        {
                            Descriptor{DType::CharacteristicUserDescription, Permission::read | Permission::write},
                        }},
                }},
    }};

    Characteristic &chr = profile.getService(0).getChar(0);
    Descriptor &descr = profile.getService(0).getChar(0).getDescr(0);
    uint8_t testByteData = 0xaf;

    // Default handlers are failed to work with no data
    ASSERT_EQ(Error::NO_BOUND_DATA, chr.onWriteEvent(mback, &testByteData, sizeof(testByteData), WriteParam()));
    ASSERT_EQ(Error::NO_BOUND_DATA, chr.onReadEvent(mback, buf, readLen, ReadParam{}));

    MockHandler mockHandler;
    chr.replaceReadHandler(std::bind(&MockHandler::onReadHandler, &mockHandler, _1, _2, _3, _4, _5));
    chr.replaceWriteHandler(std::bind(&MockHandler::onWriteHandler, &mockHandler, _1, _2, _3, _4, _5));

    ASSERT_EQ(mockHandler.readCallNum, 0);
    ASSERT_EQ(mockHandler.writeCallNum, 0);

    ASSERT_EQ(Error::ERR_OK, chr.onWriteEvent(mback, &testByteData, sizeof(testByteData), WriteParam()));
    ASSERT_EQ(mockHandler.writeCallNum, 1);

    ASSERT_EQ(Error::ERR_OK, chr.onReadEvent(mback, buf, readLen, ReadParam{}));
    ASSERT_EQ(mockHandler.readCallNum, 1);
}

// Check if custom handlers are set and called with new declaration style
TEST_F(HandlingTest, CustomHandlingAltDecl)
{
    MockedBackend mback{};
    uint8_t buf[517] = {};
    size_t readLen = 0;
    Profile profile;
    uint8_t testByteData = 0xaf;

    auto &service = profile.addService().uuid(0x1234).primary(true);

    auto &chr = service.addCharacteristic().uuid(0xabcd).permission(Permission::read).property(Property::notify);

    auto &descr = chr.addDescriptor().uuid(DType::ClientCharacteristicConfiguration).permission(Permission::read);

    ASSERT_EQ(profile.getServiceNum(), 1);
    ASSERT_EQ(profile.getService(0).getCharNum(), 1);

    // Default handlers are failed to work with no data
    ASSERT_EQ(Error::NO_BOUND_DATA, chr.onWriteEvent(mback, &testByteData, sizeof(testByteData), WriteParam()));
    ASSERT_EQ(Error::NO_BOUND_DATA, chr.onReadEvent(mback, buf, readLen, ReadParam{}));

    MockHandler mockHandler;
    chr.replaceReadHandler(bindReadCMethod(&MockHandler::onReadHandler, &mockHandler));
    chr.replaceWriteHandler(bindWriteCMethod(&MockHandler::onWriteHandler, &mockHandler));

    ASSERT_EQ(mockHandler.readCallNum, 0);
    ASSERT_EQ(mockHandler.writeCallNum, 0);

    ASSERT_EQ(Error::ERR_OK, chr.onWriteEvent(mback, &testByteData, sizeof(testByteData), WriteParam{}));
    ASSERT_EQ(mockHandler.writeCallNum, 1);

    ASSERT_EQ(Error::ERR_OK, chr.onReadEvent(mback, buf, readLen, ReadParam{}));
    ASSERT_EQ(mockHandler.readCallNum, 1);
}