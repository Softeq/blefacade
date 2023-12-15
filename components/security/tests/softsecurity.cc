#include <blefacade/core/GATT/characteristic.hh>
#include <blefacade/core/GATT/descriptor.hh>
#include <blefacade/core/GATT/handlers.hh>
#include <blefacade/core/GATT/profile.hh>
#include <blefacade/core/GATT/service.hh>
#include <blefacade/core/GATT/uuid.hh>
#include <blefacade/core/GATT/uuid_defs.hh>
#include <blefacade/core/backend/ble_backend_if.hh>
#include <blefacade/core/utils/test_utils.hh>
#include <blefacade/security/softsecurity.hh>
#include <gtest/gtest.h>

using namespace softeq::ble::security;
using namespace softeq::ble::core;
using namespace std::placeholders;

class SimpleSoftSecurity : public SoftSecurity
{
protected:
    bool authenticate(const uint8_t *data, size_t len) override
    {
        if (len == sizeof(password) / sizeof(password[0]))
        {
            if (memcmp(data, password, len) == 0)
            {
                return true;
            }
        }
        return false;
    }

private:
    const uint8_t password[4] = {1, 2, 3, 4};
};
class SoftSecurityTest : public ::testing::Test
{
public:
    SoftSecurityTest()
    {
        authReadParam.conn_id = authConnId;
        authWriteParam.conn_id = authConnId;
        unauthReadParam.conn_id = unauthConnId;
        unauthWriteParam.conn_id = unauthConnId;
    }
    static const uint64_t authConnId = 0xffaa;
    static const uint64_t unauthConnId = 0x55aa;
    ReadParam authReadParam;
    WriteParam authWriteParam;
    ReadParam unauthReadParam;
    WriteParam unauthWriteParam;
    static MockedBackend mback;
    static SimpleSoftSecurity secService;
    static uint8_t buf[517]; // 517 bytes - maximum buffer needed for a single transaction
    static size_t readLen;

    static uint8_t protectedData[5];

    static ErrorCode dummyWriteHandler(BleBackendIf &bck, Attribute &att, const uint8_t *data, size_t len,
                                const WriteParam &param)
    {
        (void)bck;
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

    static ErrorCode dummyReadHandler (BleBackendIf &bck, Attribute &att, uint8_t *data, size_t &len,
                                const ReadParam &param)
    {
        (void)bck;
        len = sizeof(protectedData) / sizeof(protectedData[0]);
        memcpy(data, protectedData, len);
        return Error::ERR_OK;
    };
};

uint8_t SoftSecurityTest::protectedData[] = "text";
uint8_t SoftSecurityTest::buf[] = {};
MockedBackend SoftSecurityTest::mback{};
SimpleSoftSecurity SoftSecurityTest::secService{};
size_t SoftSecurityTest::readLen = 0;

// Check if custom softsecurity service is able to protect another chracteristic
TEST_F(SoftSecurityTest, basicConfig)
{
    Profile authProfile{{
        Service{UUID{0x1234},
                Service::Primary::yes,
                {
                    Characteristic{UUID{0xabcd},
                                   Permission::read | Permission::write,
                                   Property::nop,
                                   bindWriteCMethod(&SimpleSoftSecurity::authHandler, &secService),
                                   bindReadCMethod(&SimpleSoftSecurity::readHandler, &secService),
                                   {}},
                }},
    }};

    Profile protectedProfile{{
        Service{UUID{0x5678},
                Service::Primary::yes,
                {
                    Characteristic{UUID{0xef10},
                                   Permission::read | Permission::write,
                                   Property::nop,
                                   secService.protectCall(dummyWriteHandler),
                                   secService.protectCall(dummyReadHandler),
                                   {}},
                }},
    }};

    Characteristic &authChr = authProfile.getService(0).getChar(0);
    Characteristic &protChr = protectedProfile.getService(0).getChar(0);

    // Check access from unauthorized client call
    ASSERT_EQ(Error::ERR_NOT_PERMITTED, protChr.onReadEvent(mback, buf, readLen, unauthReadParam));
    ASSERT_EQ(Error::ERR_OK, authChr.onReadEvent(mback, buf, readLen, unauthReadParam));
    ASSERT_EQ(readLen, 0);

    // Try to authorize with false password
    const uint8_t fakepassOne[] = {0, 0, 1, 2};
    ASSERT_EQ(Error::ERR_AUTH, authChr.onWriteEvent(mback, fakepassOne, sizeof(fakepassOne), unauthWriteParam));
    const uint8_t fakepassTwo[] = {1, 2, 3};
    ASSERT_EQ(Error::ERR_AUTH, authChr.onWriteEvent(mback, fakepassTwo, sizeof(fakepassTwo), unauthWriteParam));

    // Try to reach protected data by unauthorized calee
    const uint8_t dummyData[] = {1, 2, 3, 4, 5};
    ASSERT_EQ(Error::ERR_NOT_PERMITTED, protChr.onWriteEvent(mback, dummyData, sizeof(dummyData), unauthWriteParam));
    ASSERT_EQ(Error::ERR_NOT_PERMITTED, protChr.onReadEvent(mback, buf, readLen, unauthReadParam));

    // Try to authorize with true password
    const uint8_t truepass[] = {1, 2, 3, 4};
    ASSERT_EQ(Error::ERR_OK, authChr.onWriteEvent(mback, truepass, sizeof(truepass), authWriteParam));

    ASSERT_EQ(Error::ERR_OK, authChr.onReadEvent(mback, buf, readLen, authReadParam));
    ASSERT_EQ(readLen, 0);

    // Try to reach protected data by authorized calee
    ASSERT_EQ(Error::ERR_OK, protChr.onReadEvent(mback, buf, readLen, authReadParam));
    ASSERT_EQ(readLen, sizeof(protectedData));
    ASSERT_EQ(memcmp(buf, protectedData, sizeof(protectedData)), 0);

    ASSERT_EQ(Error::ERR_OK, protChr.onWriteEvent(mback, dummyData, sizeof(dummyData), authWriteParam));

    ASSERT_EQ(Error::ERR_OK, protChr.onReadEvent(mback, buf, readLen, authReadParam));
    ASSERT_EQ(readLen, sizeof(protectedData));
    ASSERT_EQ(memcmp(buf, dummyData, sizeof(protectedData)), 0);
}

TEST_F(SoftSecurityTest, ProfileSecured)
{
    Profile authProfile;
    auto &authService = authProfile.addService().uuid(0x1234).primary(true);
    auto &authChr = authService.addCharacteristic().uuid(0xabcd).permission(Permission::read | Permission::write);
    secService.confAuthChar(authChr, mback);

    Profile protectedProfile;
    auto &protService = protectedProfile.addService().uuid(0x5678).primary(true);
    auto &protChr = protService.addCharacteristic()
                        .uuid(0xef10)
                        .permission(Permission::read | Permission::write)
                        .onReadHandler(dummyReadHandler)
                        .onWriteHandler(dummyWriteHandler);
    secService.protectProfile(protectedProfile);

    // Check access from unauthorized client call
    ASSERT_EQ(Error::ERR_NOT_PERMITTED, protChr.onReadEvent(mback, buf, readLen, unauthReadParam));
    ASSERT_EQ(Error::ERR_OK, authChr.onReadEvent(mback, buf, readLen, unauthReadParam));
    ASSERT_EQ(readLen, 0);

    // Try to authorize with false password
    const uint8_t fakepassOne[] = {0, 0, 1, 2};
    ASSERT_EQ(Error::ERR_AUTH, authChr.onWriteEvent(mback, fakepassOne, sizeof(fakepassOne), unauthWriteParam));
    const uint8_t fakepassTwo[] = {1, 2, 3};
    ASSERT_EQ(Error::ERR_AUTH, authChr.onWriteEvent(mback, fakepassTwo, sizeof(fakepassTwo), unauthWriteParam));

    // Try to reach protected data by unauthorized calee
    const uint8_t dummyData[] = {1, 2, 3, 4, 5};
    ASSERT_EQ(Error::ERR_NOT_PERMITTED, protChr.onWriteEvent(mback, dummyData, sizeof(dummyData), unauthWriteParam));
    ASSERT_EQ(Error::ERR_NOT_PERMITTED, protChr.onReadEvent(mback, buf, readLen, unauthReadParam));

    // Try to authorize with true password
    const uint8_t truepass[] = {1, 2, 3, 4};
    ASSERT_EQ(Error::ERR_OK, authChr.onWriteEvent(mback, truepass, sizeof(truepass), authWriteParam));

    ASSERT_EQ(Error::ERR_OK, authChr.onReadEvent(mback, buf, readLen, authReadParam));
    ASSERT_EQ(readLen, 0);

    // Try to reach protected data by authorized calee
    ASSERT_EQ(Error::ERR_OK, protChr.onReadEvent(mback, buf, readLen, authReadParam));
    ASSERT_EQ(readLen, sizeof(protectedData));
    ASSERT_EQ(memcmp(buf, protectedData, sizeof(protectedData)), 0);

    ASSERT_EQ(Error::ERR_OK, protChr.onWriteEvent(mback, dummyData, sizeof(dummyData), authWriteParam));

    ASSERT_EQ(Error::ERR_OK, protChr.onReadEvent(mback, buf, readLen, authReadParam));
    ASSERT_EQ(readLen, sizeof(protectedData));
    ASSERT_EQ(memcmp(buf, dummyData, sizeof(protectedData)), 0);
}

TEST_F(SoftSecurityTest, ServiceSecured)
{
    Profile authProfile;
    auto &authService = authProfile.addService().uuid(0x1234).primary(true);
    auto &authChr = authService.addCharacteristic().uuid(0xabcd).permission(Permission::read | Permission::write);
    secService.confAuthChar(authChr, mback);

    Profile protectedProfile;
    auto &protService = protectedProfile.addService().uuid(0x5678).primary(true);
    auto &protChr = protService.addCharacteristic()
                        .uuid(0xef10)
                        .permission(Permission::read | Permission::write)
                        .onReadHandler(dummyReadHandler)
                        .onWriteHandler(dummyWriteHandler);
    secService.protectService(protService);

    // Check access from unauthorized client call
    ASSERT_EQ(Error::ERR_NOT_PERMITTED, protChr.onReadEvent(mback, buf, readLen, unauthReadParam));
    ASSERT_EQ(Error::ERR_OK, authChr.onReadEvent(mback, buf, readLen, unauthReadParam));
    ASSERT_EQ(readLen, 0);

    // Try to reach protected data by unauthorized calee
    const uint8_t dummyData[] = {1, 2, 3, 4, 5};
    ASSERT_EQ(Error::ERR_NOT_PERMITTED, protChr.onWriteEvent(mback, dummyData, sizeof(dummyData), unauthWriteParam));
    ASSERT_EQ(Error::ERR_NOT_PERMITTED, protChr.onReadEvent(mback, buf, readLen, unauthReadParam));

    // Try to authorize with true password
    const uint8_t truepass[] = {1, 2, 3, 4};
    ASSERT_EQ(Error::ERR_OK, authChr.onWriteEvent(mback, truepass, sizeof(truepass), authWriteParam));

    ASSERT_EQ(Error::ERR_OK, authChr.onReadEvent(mback, buf, readLen, authReadParam));
    ASSERT_EQ(readLen, 0);

    // Try to reach protected data by authorized calee
    ASSERT_EQ(Error::ERR_OK, protChr.onReadEvent(mback, buf, readLen, authReadParam));
    ASSERT_EQ(readLen, sizeof(protectedData));
    ASSERT_EQ(memcmp(buf, protectedData, sizeof(protectedData)), 0);

    ASSERT_EQ(Error::ERR_OK, protChr.onWriteEvent(mback, dummyData, sizeof(dummyData), authWriteParam));

    ASSERT_EQ(Error::ERR_OK, protChr.onReadEvent(mback, buf, readLen, authReadParam));
    ASSERT_EQ(readLen, sizeof(protectedData));
    ASSERT_EQ(memcmp(buf, dummyData, sizeof(protectedData)), 0);
}