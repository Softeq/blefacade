#include <flags.hh>

#include <map>

namespace softeq
{
namespace ble
{
namespace bluez
{
static std::map<CharacteristicFlags, std::string> _characteristicFlagToString{{CharacteristicFlags::Read, "read"},
                                                                              {CharacteristicFlags::Write, "write"},
                                                                              {CharacteristicFlags::Notify, "notify"},
                                                                              {CharacteristicFlags::Indicate, "indicate"}};

static std::map<DescriptorFlags, std::string> _descriptorFlagToString{{DescriptorFlags::Read, "read"},
                                                                      {DescriptorFlags::Write, "write"}};

CharacteristicFlags operator|(CharacteristicFlags lhs, CharacteristicFlags rhs)
{
    return static_cast<CharacteristicFlags>(static_cast<uint>(lhs) | static_cast<uint>(rhs));
}

bool operator&(CharacteristicFlags lhs, CharacteristicFlags rhs)
{
    return static_cast<uint>(lhs) & static_cast<uint>(rhs);
}

DescriptorFlags operator|(DescriptorFlags lhs, DescriptorFlags rhs)
{
    return static_cast<DescriptorFlags>(static_cast<uint>(lhs) | static_cast<uint>(rhs));
}

bool operator&(DescriptorFlags lhs, DescriptorFlags rhs)
{
    return static_cast<uint>(lhs) & static_cast<uint>(rhs);
}

template <typename T>
static std::vector<std::string> _flagsToStrings(T flags, std::map<T, std::string> &flagToString)
{
    std::vector<std::string> stringFlags;

    for (size_t i = 1; i <= static_cast<size_t>(T::Count); i++)
    {
        T testFlag = static_cast<T>(1 << i);
        if (flags & testFlag)
        {
            stringFlags.push_back(flagToString.at(testFlag));
        }
    }

    return stringFlags;
}

std::vector<std::string> flagsToStrings(CharacteristicFlags flags)
{
    return _flagsToStrings<CharacteristicFlags>(flags, _characteristicFlagToString);
}

std::vector<std::string> flagsToStrings(DescriptorFlags flags)
{
    return _flagsToStrings<DescriptorFlags>(flags, _descriptorFlagToString);
}

} // namespace bluez
} // namespace ble
} // namespace softeq
