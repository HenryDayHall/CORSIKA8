
add_library (CORSIKAthirdparty INTERFACE)

target_include_directories (CORSIKAthirdparty SYSTEM
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
  $<INSTALL_INTERFACE:include/ThirdParty>
  )

install (DIRECTORY phys DESTINATION include/ThirdParty/)
install (DIRECTORY catch2 DESTINATION include/ThirdParty/)

##########################################################################################
# Boost subset
# ============
#
# A subset of header-only Boost-1.70 libraries was installed with the bcp tool.
# Instructions to update Boost:
#
# - Download the latest tarball from www.boost.org
# - Unpack source code somewhere, go to into the folder, then run
#
#   ./bootstrap && ./b2 tools/bcp
#   ./dist/bin/bcp mp11 iterator core format interval optional ./dist
#   mv ./dist/boost <CORSIKA-path>/ThirdParty/boost
#
install (DIRECTORY boost DESTINATION include/ThirdParty/boost/)