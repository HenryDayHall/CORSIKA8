
add_library(CORSIKA INTERFACE)

target_compile_coptions(CORSIKA
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR})
