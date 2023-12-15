cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
set(PRODUCT_NAMESPACE "softeq")
project(blefacade VERSION 0.3.0 LANGUAGES CXX)

set(BUILD_ALL ON CACHE BOOL "Build everything")

include(tools/cmake/SofteqComponentsModel.cmake)

make_softeq_component(${PROJECT_NAME} STATIC)

add_library(${PRODUCT_NAMESPACE}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

################################### PROJECT SPECIFIC GLOBALS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# -O0            (optimization level: -O0, -O1, -O2, -O3, -Os, -Og, -Ofast)
# -Wall          (enables all compiler's warning messages)
# -Wextra        (enables some extra warning flags that are not enabled by -Wall)
# -Wpedantic     (issue all the warnings demanded by strict ISO C and ISO C++)
# -Wfatal-errors (compiler to abort compilation on the first error)
# -Werror        (warning == error)
# -pthread       (multithrading enable)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  LINKER_LANGUAGE CXX
  OUTPUT_NAME "${PRODUCT_NAMESPACE}${PROJECT_NAME}"
  VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
  SOVERSION ${PROJECT_VERSION_MAJOR}
  )

configure_file(include/blefacade/version.hh.in
  include/${COMPONENT_PATH}/version.hh
  )

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_BINARY_DIR}/include/${COMPONENT_PATH}
  )

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  src/version.cc
  )

set(PUBLIC_HEADERS
  ${CMAKE_BINARY_DIR}/include/${COMPONENT_PATH}/version.hh
  )

########################################### SUBCOMPONENTS

option(ENABLE_BLE "BLE" ${BUILD_ALL})
if (ENABLE_BLE)
  add_subdirectory(components/core)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    blefacade-core
    )
endif ()

option(ENABLE_BLUEZ "BLUEZ" ${BUILD_ALL})
if (ENABLE_BLUEZ)
  if (NOT ENABLE_BLE)
    message(FATAL_ERROR "BLE security module depends on BLE core")
  else()
    add_subdirectory(components/bluez)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      blefacade-bluez
      )
  endif ()
endif ()

option(ENABLE_SECURITY "BLE" ${BUILD_ALL})
if (ENABLE_SECURITY)
  if (NOT ENABLE_BLE)
    message(FATAL_ERROR "BLE security module depends on BLE core")
  else()
    add_subdirectory(components/security)
    target_link_libraries(${PROJECT_NAME}
      PRIVATE
      blefacade-security
      )
  endif ()
endif ()

option(BUILD_EXAMPLES "EXAMPLES" ${BUILD_ALL})
if (BUILD_EXAMPLES)
  if (ENABLE_BLUEZ)
    add_subdirectory(examples/bluez_gatt_server)
  else()
    message(WARNING "To build bluez example you should enable 'BLUEZ' target")
  endif()

  if (ENABLE_BLUEZ AND ENABLE_SECURITY)
    add_subdirectory(examples/bluez_softsecure_server)
  else()
    message(WARNING "To build bluez security example you should enable 'BLUEZ' target")
  endif()
endif ()

# add_softeq_testing()

########################################### INSTALLATION

install_softeq_framework(${PROJECT_NAME})