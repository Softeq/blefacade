cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
make_softeq_component(bluez OBJECT)
################################### PROJECT SPECIFIC GLOBALS
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
################################### COMPONENT SOURCES
set(DBUS_CPP_INSTALL_DIR)
find_package(DBusCppChromium REQUIRED)

include(data/CMakeLists.txt)

target_sources(${PROJECT_NAME}
  PRIVATE
  ${DBUS_GENERATED_HEADERS}
  src/byte_array.cc
  src/flags.cc
  src/bluez_object_manager.cc
  src/descriptor.cc
  src/characteristic.cc
  src/service.cc
  src/application.cc
  src/application_manager.cc
  src/advertisement.cc
  src/advertising_manager.cc
  src/dbus_manager.cc
  src/bluez_backend.cc
  src/utils.cc
  )

include(FindPkgConfig)

target_include_directories(${PROJECT_NAME} PUBLIC ${DBUS_CPP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} 
  PUBLIC
  DBusCpp::DBusCpp
  ${DBUS_CPP_LIBRARIES}
  PRIVATE
  Threads::Threads
  )


################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  EXPORTS
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/blefacade/bluez/bluez_backend.hh
  INSTALL_PARAMS
  # does not exclude static lib
  ARCHIVE
  )
