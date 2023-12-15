#ifndef UUID_HH
#define UUID_HH
#include <array>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdint.h>
#include <string>

#include <blefacade/core/GATT/uuid_defs.hh>

namespace softeq
{
namespace ble
{
namespace core
{
typedef std::array<uint8_t, 16> Uuid128;

class UUID
{
public:
    enum class Length
    {
        LEN16,
        LEN32,
        LEN128,
    };

    UUID() = default;

    UUID(SType serviceType)
    {
        _uuid.len16 = static_cast<uint16_t>(serviceType);
        _len = Length::LEN16;
    }

    UUID(CType characteristicType)
    {
        _uuid.len16 = static_cast<uint16_t>(characteristicType);
        _len = Length::LEN16;
    }

    UUID(DType descriptorType)
    {
        _uuid.len16 = static_cast<uint16_t>(descriptorType);
        _len = Length::LEN16;
    }

    UUID(const char *uuidStr)
    {
        std::regex uuid128_expr("[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}");
        std::regex uuid32_expr("[a-fA-F0-9]{8}");
        std::regex uuid16_expr("[a-fA-F0-9]{4}");

        if (std::regex_match(uuidStr, uuid128_expr))
        {
            _len = Length::LEN128;

            std::string uuid(uuidStr);
            for (auto it = uuid.begin(); it != uuid.end(); it++)
            {
                if (*it == '-')
                {
                    uuid.erase(it);
                }
            }

            auto i = uuid.begin();
            for (auto it = _uuid.len128.begin(); it != _uuid.len128.end(); it++)
            {
                char hexStr[3]{*i, *(i + 1), '\0'};

                uint16_t temp;
                std::stringstream ss;
                ss << std::hex << hexStr;
                ss >> temp;

                *it = temp;

                i = i + 2;
            }
        }
        else if (std::regex_match(uuidStr, uuid32_expr))
        {
            std::stringstream ss;
            ss << std::hex << uuidStr;
            ss >> _uuid.len32;
            _len = Length::LEN32;
        }
        else if (std::regex_match(uuidStr, uuid16_expr))
        {
            std::stringstream ss;
            ss << std::hex << uuidStr;
            ss >> _uuid.len16;
            _len = Length::LEN16;
        }
        else
        {
            assert(true);
        }
    }

    UUID(const uint32_t uuid, const UUID::Length len)
    {
        assert(len != Length::LEN128); // 128 are supported in another constructor
        _uuid.len32 = uuid;
        _len = len;
    }

    UUID(const uint32_t uuid)
    {
        _uuid.len32 = uuid;
        if ((uuid >> 16) == 0) // if high octets are empty
        {
            _len = Length::LEN16;
        }
        else
        {
            _len = Length::LEN32;
        }
    }

    UUID(const Uuid128 &uuid)
    {
        _uuid.len128 = uuid;
        _len = Length::LEN128;
    }

    UUID(const UUID &uuid)
    {
        _uuid = uuid._uuid;
        _len = uuid._len;
    }

    UUID(UUID &&uuid) = default;

    uint16_t getValue16() const
    {
        return _uuid.len16;
    }

    uint32_t getValue32() const
    {
        return _uuid.len32;
    }

    uint8_t getValue128(size_t i) const
    {
        return _uuid.len128[i];
    }

    const Length &getLength() const
    {
        return _len;
    }

    UUID &operator=(UUID rhs)
    {
        std::swap(_len, rhs._len);
        std::swap(_uuid, rhs._uuid);
        return *this;
    }

    ~UUID() = default;

    std::string getPrintable() const
    {
        std::string uuid;

        switch (_len)
        {
        case Length::LEN16:
            uuid = getUuidStr(_uuid.len16);
            break;

        case Length::LEN32:
            uuid = getUuidStr(_uuid.len32);
            break;

        case Length::LEN128: {
            std::stringstream uuidStream;
            for (size_t i = 0; i < _uuid.len128.size(); i++)
            {
                if (i == 4 || i == 6 || i == 8 || i == 10)
                {
                    uuidStream << "-";
                }
                uuidStream << std::setfill('0') << std::setw(2) << std::hex << uint32_t(_uuid.len128[i]);
            }

            uuid = uuidStream.str();
        }
        break;


        default:
            break;
        }

        return uuid;
    }

private:
    Length _len = Length::LEN16;
    union uuid_t
    {
        uint16_t len16;
        uint32_t len32;
        Uuid128 len128;
    } _uuid = {0};

    std::string getUuidStr(uint32_t uuid) const
    {
        const uint32_t bluetoothBaseUUIDPrefix = 0x00000000;
        const std::string bluetoothBaseUUIDSuffix = "0000-1000-8000-00805f9b34fb";

        uint32_t uuidPrefix = bluetoothBaseUUIDPrefix | uuid;

        std::stringstream uuidStream;
        uuidStream << std::setfill('0') << std::setw(sizeof(uint32_t) * 2) << std::hex << uuidPrefix << '-'
                   << bluetoothBaseUUIDSuffix;

        return uuidStream.str();
    }
};

inline bool operator==(const UUID &lhs, const UUID &rhs)
{
    if (lhs.getLength() == rhs.getLength())
    {
        if (lhs.getLength() == UUID::Length::LEN16 && lhs.getValue16() == rhs.getValue16())
        {
            return true;
        }
        else if (lhs.getLength() == UUID::Length::LEN32 && lhs.getValue32() == rhs.getValue32())
        {
            return true;
        }
        else if (lhs.getLength() == UUID::Length::LEN128)
        {
            for (unsigned i = 0; i < 16; ++i)
            {
                if (lhs.getValue128(i) != rhs.getValue128(i))
                {
                    return false;
                }
            }
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
} // namespace core
} // namespace ble
} // namespace softeq

#endif //UUID_HH