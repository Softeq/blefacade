#pragma once

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>

class DevicePathConverter
{
private:
    static const std::string key;
    static const char delimeter;

    static std::string getDeviceId(const std::string &str)
    {
        auto n = str.find(key);
        std::string id = str.substr(n + key.length());
        id.erase(remove(id.begin(), id.end(), delimeter), id.end());
        return id;
    }

    static uint64_t hexToUint(const std::string &str)
    {
        std::istringstream converter(str);
        uint64_t value;
        converter >> std::hex >> value;
        return value;
    }

public:
    static uint64_t toUint64(const std::string &str)
    {
        return hexToUint(getDeviceId(str));
    }
};