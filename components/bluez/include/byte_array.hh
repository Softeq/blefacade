#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace softeq
{
namespace ble
{
namespace bluez
{
class ByteArray final
{
public:
    explicit ByteArray(const std::string &str);
    explicit ByteArray(const std::tm *time);
    explicit ByteArray(const std::vector<uint8_t> &data);

    std::vector<uint8_t> data() const;
    std::string toString() const;

private:
    std::vector<uint8_t> _data;
};
} // namespace bluez
} // namespace ble
} // namespace softeq
