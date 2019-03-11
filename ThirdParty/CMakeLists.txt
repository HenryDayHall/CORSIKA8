
add_library (CORSIKAthirdparty INTERFACE)

target_include_directories (CORSIKAthirdparty SYSTEM
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
  $<INSTALL_INTERFACE:include/ThirdParty>
  )

install (DIRECTORY phys DESTINATION include/ThirdParty/)
install (DIRECTORY catch2 DESTINATION include/ThirdParty/)
