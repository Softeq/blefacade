#ifndef ATT_UTLS_HH
#define ATT_UTLS_HH
#include "attribute.hh"
#include "profile.hh"
#include <cstdint>

namespace softeq
{
namespace ble
{
namespace core
{
// TODO : add constant methods
Attribute *findAttributeByHandle(Profile &prof, uint16_t handle);
Attribute *findAttributeByUuid(Profile &prof, const UUID &uuid);
Service *findServiceByUuid(Profile &prof, const UUID &uuid);
bool profileContainsAttribute(const Profile &prof, const Attribute &att);
Descriptor *findCccd(Characteristic &parent);

} // namespace core
} // namespace ble
} // namespace softeq


#endif //ATT_UTLS_HH