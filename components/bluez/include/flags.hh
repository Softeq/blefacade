#pragma once

#include <string>
#include <vector>

namespace softeq
{
namespace ble
{
namespace bluez
{
enum class CharacteristicFlags : unsigned
{
    Unknown = 1 << 0,
    Read = 1 << 1,
    Write = 1 << 2,
    Notify = 1 << 3,
    Indicate = 1 << 4,
    Count = 5
};

CharacteristicFlags operator|(CharacteristicFlags lhs, CharacteristicFlags rhs);
bool operator&(CharacteristicFlags lhs, CharacteristicFlags rhs);

enum class DescriptorFlags : unsigned
{
    Unknown = 1 << 0,
    Read = 1 << 1,
    Write = 1 << 2,
    Count = 3
};

DescriptorFlags operator|(DescriptorFlags lhs, DescriptorFlags rhs);
bool operator&(DescriptorFlags lhs, DescriptorFlags rhs);

std::vector<std::string> flagsToStrings(CharacteristicFlags flags);
std::vector<std::string> flagsToStrings(DescriptorFlags flags);
} // namespace bluez
} // namespace ble
} // namespace softeq
