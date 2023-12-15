#include <algorithm>
#include <byte_array.hh>

namespace
{
static std::vector<uint8_t> toByteArray(const std::tm *time)
{
    const std::size_t arraySize = 10;
    std::vector<uint8_t> byteArray(arraySize, 0);

    uint16_t year = static_cast<uint16_t>(time->tm_year + 1900);
    uint8_t weekDay = static_cast<uint8_t>(time->tm_wday == 0 ? 7 : time->tm_wday);

    byteArray[0] = (year >> 0) & 0xff;
    byteArray[1] = (year >> 8) & 0xff;
    byteArray[2] = uint8_t(time->tm_mon + 1); // month (1-12)
    byteArray[3] = uint8_t(time->tm_mday);    // day (1-31)
    byteArray[4] = uint8_t(time->tm_hour);    // hour (0-23)
    byteArray[5] = uint8_t(time->tm_min);     // minute (0-59)
    byteArray[6] = uint8_t(time->tm_sec);     // seconds (0-59)
    byteArray[7] = weekDay;                   // weekday (1-7 where 1=Monday)
    byteArray[8] = 0;                         // Fractions (1/256th of second)
    byteArray[9] = 0;                         // Adjust reason bitmask (0 for testing)

    return byteArray;
}

static std::vector<uint8_t> toByteArray(const std::string &data)
{
    std::vector<uint8_t> byteArray;

    std::transform(data.begin(), data.end(), std::back_inserter(byteArray),
                   [](const char c) { return static_cast<uint8_t>(c); });

    return byteArray;
}

static std::string fromByteArray(const std::vector<uint8_t> &byteArray)
{
    std::string result;

    std::transform(byteArray.begin(), byteArray.end(), std::back_inserter(result),
                   [](const uint8_t byte) { return static_cast<char>(byte); });

    return result;
}
} // namespace

namespace softeq::ble::bluez
{
ByteArray::ByteArray(const std::string &str)
    : _data(toByteArray(str))
{
}

ByteArray::ByteArray(const std::tm *time)
    : _data(toByteArray(time))
{
}

ByteArray::ByteArray(const std::vector<uint8_t> &data)
    : _data(data)
{
}

std::vector<uint8_t> ByteArray::data() const
{
    return _data;
}

std::string ByteArray::toString() const
{
    return fromByteArray(_data);
}

} // namespace softeq::ble::bluez
