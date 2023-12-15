#ifndef TEST_UTILS_HH
#define TEST_UTILS_HH
#include <blefacade/core/backend/ble_backend_if.hh>

namespace softeq
{
namespace ble
{
namespace core
{
class MockedBackend : public BleBackendIf
{
public:
    MockedBackend() = default;
    bool configureProfile(Profile &profile) override{};
    bool indicate(Characteristic &chr, uint8_t *data = nullptr, size_t len = 0) override{};
    void addEventHandler(std::function<gattEventHandler>) override{};
};

struct MockHandler
{
    size_t readCallNum = 0;
    size_t writeCallNum = 0;
    ErrorCode onReadHandler(BleBackendIf &bck, Attribute &att, uint8_t *data, size_t &len, const ReadParam &param)
    {
        (void)bck;
        readCallNum++;
        return Error::ERR_OK;
    }
    ErrorCode onWriteHandler(BleBackendIf &bck, Attribute &att, const uint8_t *data, size_t len,
                             const WriteParam &param)
    {
        (void)bck;
        writeCallNum++;
        return Error::ERR_OK;
    }
};
} // namespace core
} // namespace ble
} // namespace softeq

#endif // TEST_UTILS_HH