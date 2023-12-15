# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

get_filename_component(BLEFACADE_ORIGIN_DIR "../../"
                       REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(EXTRA_COMPONENT_DIRS ${BLEFACADE_ORIGIN_DIR}/components/esp32/blefacade)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(gatt_server_demo)
