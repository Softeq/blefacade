#pragma once

#include <generated/object_manager_proxy.hh>

namespace softeq
{
namespace ble
{
namespace bluez
{
class BluezObjectManager final : public org::freedesktop::DBus::ObjectManager_proxy,
                                 public DBus::IntrospectableProxy,
                                 public DBus::ObjectProxy
{
public:
    explicit BluezObjectManager(DBus::Connection &connection);

    DBus::Path gattManagerPath();
    DBus::Path leAdvertisingManagerPath();
};
} // namespace bluez
} // namespace ble
} // namespace softeq
