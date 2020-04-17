add_library (Catch2 INTERFACE)

target_compile_options (Catch2
  INTERFACE
  -I${CMAKE_SOURCE_DIR}/externals/catch2
  )

set (Catch2_FOUND True)
