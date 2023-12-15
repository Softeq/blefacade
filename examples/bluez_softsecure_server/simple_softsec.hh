#pragma once
#include <blefacade/security/softsecurity.hh>

class SimpleSoftSecurity : public softeq::ble::security::SoftSecurity
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
    const uint8_t password[4] = {1, 1, 1, 1};
};