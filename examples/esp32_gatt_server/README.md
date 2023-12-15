# ESP32 GATT server example

This example contains simple usage of BLE facade in using ESP32 IDF framework.

GATT services structure is described in `service_declaration.cc` file.
The backend platform-specific part consumes the constructed profiles from the description and configures ESP32 host GATT services. Current implementation supports handling of simple READ/WRITE operations that are done over attributes.

The advertisment (GAP) part is hard-coded and subject to be implemented configurable as GATT.

### Build:
```bash
idf.py build
```

### Flash:
```bash
idf.py -p <servial_device> flash
```
for example, `idf.py -p /dev/ttyACM0 flash`

### Get logs:
```bash
idf.py monitor
```
