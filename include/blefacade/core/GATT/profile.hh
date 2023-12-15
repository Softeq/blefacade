#ifndef PROFILE_HH
#define PROFILE_HH
#include "service.hh"
#include <blefacade/core/utils/slist.hh>
#include <initializer_list>

namespace softeq
{
namespace ble
{
namespace core
{
class Profile
{
private:
    Slist<Service> _services;

public:
    Profile() = default;

    Profile(std::initializer_list<Service> &&services)
    {
        _services.insert(_services.begin(), services.begin(), services.end());
    };

    Service &addService()
    {
        _services.emplace_back();
        return _services.back();
    }

    unsigned int getServiceNum() const
    {
        return _services.size();
    };

    Service &getService(const size_t index)
    {
        return _services.get(index);
    };

    const Service &getService(const size_t index) const
    {
        return _services.get(index);
    };
    ~Profile() = default;
};

} // namespace core
} // namespace ble
} // namespace softeq

#endif //PROFILE_HH