add_library (PhysUnits INTERFACE)

target_compile_options (PhysUnits
  INTERFACE
  -I${CMAKE_SOURCE_DIR}/externals/phys_units
  )

set (PhysUnits_FOUND True)
