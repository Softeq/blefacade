cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
make_softeq_component(core OBJECT)

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
target_sources(${PROJECT_NAME}
  PRIVATE
  src/att_utils.cc
  )
################################### SUBCOMPONENTS
if (BUILD_TESTING)
  add_subdirectory(tests)
endif ()

################################### INSTALLATION
install(TARGETS ${PROJECT_NAME}
  EXPORT "${GLOBAL_INSTALL_EXPORT_NAME}"
  )

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
