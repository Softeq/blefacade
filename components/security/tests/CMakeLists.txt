make_softeq_component(security-tests EXECUTABLE)
################################### PROJECT SPECIFIC GLOBALS
gtest_discover_tests(${PROJECT_NAME})

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  main.cc
  softsecurity.cc
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  GTest::GTest
  GTest::Main
  blefacade-core
  ${PARENT_COMPONENT_NAME}
  $<TARGET_PROPERTY:${PARENT_COMPONENT_NAME},LINK_LIBRARIES>
  )

################################### SUBCOMPONENTS

################################### INSTALLATION
