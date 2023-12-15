cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(bluez-softsecure-server)

add_executable(${PROJECT_NAME} main.cc)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(${PROJECT_NAME} blefacade-core blefacade-bluez blefacade-security)
