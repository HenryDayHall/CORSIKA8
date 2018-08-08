
add_library (CORSIKAthirdparty INTERFACE)

target_include_directories (CORSIKAthirdparty
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>
  $<INSTALL_INTERFACE:include/>
  )

install (DIRECTORY phys DESTINATION include/ThirdParty/)
