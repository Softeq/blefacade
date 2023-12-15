set(DBUS_GENERATED_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/generated)
set(DBUS_CPP_BIN_DIR "${DBUS_CPP_INSTALL_DIR}/bin")

set(DBUS_GENERATED_HEADERS
  ${DBUS_GENERATED_HEADERS_DIR}/object_manager_proxy.hh
  ${DBUS_GENERATED_HEADERS_DIR}/object_manager_adaptor.hh
  ${DBUS_GENERATED_HEADERS_DIR}/manager_proxy.hh
  ${DBUS_GENERATED_HEADERS_DIR}/service_adaptor.hh
  ${DBUS_GENERATED_HEADERS_DIR}/characteristic_adaptor.hh
  ${DBUS_GENERATED_HEADERS_DIR}/descriptor_adaptor.hh
  ${DBUS_GENERATED_HEADERS_DIR}/advertisement_adaptor.hh
  ${DBUS_GENERATED_HEADERS_DIR}/advertising_manager_proxy.hh
)

add_custom_command(OUTPUT ${DBUS_GENERATED_HEADERS}
  COMMAND mkdir -p ${DBUS_GENERATED_HEADERS_DIR}
  COMMAND ${DBUS_CPP_BIN_DIR}/dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/object-manager.xml --proxy=${DBUS_GENERATED_HEADERS_DIR}/object_manager_proxy.hh
  COMMAND ${DBUS_CPP_BIN_DIR}/dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/object-manager.xml --adaptor=${DBUS_GENERATED_HEADERS_DIR}/object_manager_adaptor.hh
  COMMAND ${DBUS_CPP_BIN_DIR}/dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/gatt-manager.xml --async --proxy=${DBUS_GENERATED_HEADERS_DIR}/manager_proxy.hh
  COMMAND ${DBUS_CPP_BIN_DIR}/dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/gatt-service.xml --adaptor=${DBUS_GENERATED_HEADERS_DIR}/service_adaptor.hh
  COMMAND ${DBUS_CPP_BIN_DIR}/dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/gatt-characteristic.xml --adaptor=${DBUS_GENERATED_HEADERS_DIR}/characteristic_adaptor.hh
  COMMAND ${DBUS_CPP_BIN_DIR}/dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/gatt-descriptor.xml --adaptor=${DBUS_GENERATED_HEADERS_DIR}/descriptor_adaptor.hh
  COMMAND ${DBUS_CPP_BIN_DIR}/dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/le-advertisement.xml --adaptor=${DBUS_GENERATED_HEADERS_DIR}/advertisement_adaptor.hh
  COMMAND ${DBUS_CPP_BIN_DIR}/dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/data/le-advertising-manager.xml --async --proxy=${DBUS_GENERATED_HEADERS_DIR}/advertising_manager_proxy.hh
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  COMMENT "Generating headers from xml files"
  VERBATIM
)
