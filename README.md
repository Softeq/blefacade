
# BLE facade
[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](https://choosealicense.com/licenses/mit/)
[![GPLv3 License](https://img.shields.io/badge/License-GPL%20v3-yellow.svg)](https://opensource.org/licenses/)
The library is used for establishment an interface to BLE host and interaction with remote devices through GATT.

The library is supposed to use on different platforms. The main ones are bare metal, RTOS and Linux. Thus, the design of the library is made as static as possible. In the meantime, functionality with dynamic nature is sacrificed as the first priority is portability and bare-metal systems execution.

### Internal details

The library consists of two parts: Generic library itself and a platform-specific part.

Generic part serves GATT construction purposes.

Platform-specific part is implemented for ESP32 only. There is a backend mock planned to implement.
`service_declaration.cc` file contains the structure of GATT profiles and services. The same structure is implemented in test sources.

### Prerequisites
Setup libtool:
```
sudo apt-get install libtool-bin
```
Setup ctemplate:
```
git clone https://github.com/OlafvdSpek/ctemplate
./autogen.sh && ./configure  
make install 
```
To refresh shaled libraries cache registry:
```
ldconfig 
```
Setup chromiuim dbus-cplusplus:
```
export BLEFACADE_DIR=<specify location of blefacade lib>
git clone https://chromium.googlesource.com/chromiumos/third_party/dbus-cplusplus/ -b release-R75-12105.B --single-branch
cd dbus-cplusplus/
./bootstrap && ./configure --prefix='${BLEFACADE_DIR}/third_party/libdbus-c++-chromium/' --enable-shared --disable-static --disable-glib --disable-ecore
make install
```

### Build

```bash
mkdir build
cd build
cmake ..
make
```

### Service declaration
The blefacade library allows to use a bit different approaches to configure the ble services.
First approach is a preferrable one and looks like a chain of configuration calls
```cpp
// Create a hierarchy of services, characteristics

auto &service = profile.addService().uuid(0x1234)
									.primary(true);  
auto &characteristic = authServ.addCharacteristic().uuid(0xabcd)
					.permission(Permission::read | Permission::write)
					.property(Property::notify | Property::indicate);
```
This approach gives more traditional view on configuration and may be done in runtime. You may find an example of such configuring in esp32_softsecure_server project.
In the meantime, the other static-like declaration gives less flexibility but may be done in compile-time strictly:
```cpp
Profile profile{{
        Service{UUID{0x1234},
                Service::Primary::yes,
                {
                    Characteristic{UUID{0xabcd},
                                   Permission::read | Permission::write
                                   Property::notify | Property::indicate,
                 },
        }}
}};
```
It looks more like a tree structure, but less clear than the first one.
Let's review what configuration capabilities present here.

#### Profile
Profile contains one or more sevices inside. 
`addService()` - adds a new empty service
`getServiceNum()` - retrieves number of services
`getService(const size_t index)` - retreives a service by the index
Services are the root elements for ESP32 backend to start registration with.

#### Service
Service contains one or more characteristics. Service carries own handle and uuid as well.
`isPrimary()` - retreives if service is primary
`primary(bool prim)` - specifies if a service is primary or secondary
`uuid(UUID uuid)` - sets an uuid
`getUuid()` - retieves an uuid
`getCharNum()` - retrieves number of characteristics
`getChar(const size_t index)` - retrieves a characteristic by index
`getHandle()` - retrieves a handle
`setHandle(uint16_t handle)` - sets a fandle
`addCharacteristic()` - adds new characteristic

#### Characteristic
Characteristic is an attribute which may be associated with some data and read/write handlers. So, it is an entity which provides basic interaction abilities.
`uuid(UUID uuid)` - sets an uuid
`permission(const Permission &permissions)` - sets a permission
`property(const Property &props)` - sets a property (broadcast, indicate, notify)
`data(const CBData &data)` - binds data
`handle(uint16_t handle)` - sets a fandle
`onWriteHandler(const std::function<gattWriteHandler> &handler)` - sets write handler
`onReadHandler(const std::function<gattReadHandler> &handler)` - sets read handler
If a characteristic has bound data, but custom read/write handlers are not specified, there is default access to the data is arranged accordingly to the permissions.
If custom handlers are provided, all control flow is passed to those handlers upon read/write operations.
All meaningful characteristic uuids are described in uuid_defs.hh

#### Descriptor
Descriptor is also an attribute tied to the data and may serve configuration purposes.
The descriptor carries almost the same set of methods as Characteristic does except properties. All meaningful descriptor uuids are described in uuid_defs.hh

### Soft security
Soft security serves for purposes when standard BLE security options are not available. For example, you may need to grant access to a user, who knows the only secret key. Protection in such case is arranged in the following way:
1. Implement a derived class from SoftSecurity specifying at least authenticate method.
2. Configure a BLE service and dedicate some characteristic as an authentication interface to your implementation of SoftSecurity class:
```cpp
auto &authServ = _authProfile.addService().uuid(0x1234).primary(true);
auto &authAtt = authServ.addCharacteristic().uuid(0xabcd).permission(Permission::read | Permission::write);
// Bind security handlers
authAtt.onReadHandler(bindReadCMethod(&SimpleSoftSecurity::readHandler, &_secService));
authAtt.onWriteHandler(bindWriteCMethod(&SimpleSoftSecurity::authHandler, &_secService));
_backend.addEventHandler(bindGattEventHandler(&SimpleSoftSecurity::clientRegHandler, &_secService));
```

There is also a helper method that makes registration in one call:
```cpp
confAuthChar(Attribute &att, BleBackendIf &bck)
```
3. add handlers protected by SoftSecurity module using .protectCall() method to register them in the security module:
```cpp
// Bind protected handlers
protectedAtt.onReadHandler(_secService.protectCall(rHandler));
protectedAtt.onWriteHandler(_secService.protectCall(wHandler));
```
There are also aletrnative helper methods that protect read and write calls for Profile, Service or Attribute respectively:
```cpp
protectProfile(Profile &prof)
protectService(Service &srv)
protectAttribute(Attribute &att)
```

Configurtion is done on this step. 

Now any unauthenticated client is unable to reach protected services. To authenticate, the client has to pass some data to softsecurity write handler that would pass authenticate function.
Once it's done the client is authorized and granted access to the protected handlers. Authorization lasts till the disconnection. There is no way to persistently keep clients authorized throughout multiple connection sesstions at the moment.

### Indication and notification

Indication or notification has to be configured as a characteristic property:
```cpp
Property::notify
Property::indicate
```
PLease note, indicate property prevailes if they set together.
Once it is done, the notification or indication event can be sent using the following backend call:
```cpp
bool indicate(ble::Characteristic &chr, uint8_t *data = nullptr, size_t len = 0)
```
If both optional parameters are not set, the BLE facade will try to get data which are bound to ble::Characteristic object if any.
If no proper data is set or target characteristic is not found by UUID, the call returns false.