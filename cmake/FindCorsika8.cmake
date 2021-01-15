#
# FindCorsika8
#
# This module tries to find the Corsika8 header files and extrats their version.  It
# sets the following variables.
#
# CORSIKA8_FOUND       -  Set ON if Corsika8 headers are found, otherwise OFF.
#
# CORSIKA8_INCLUDE_DIR -  Include directory for hydra header files.  (All header
#                       files will actually be in the Corsika8 subdirectory.)
# CORSIKA8_VERSION     -  Version of hydra in the form "major.minor.patch".
#

find_path( CORSIKA8_INCLUDE_DIR
  PATHS
    ${CMAKE_SOURCE_DIR}
    /usr/include
    /usr/local/include
    ${CORSIKA8_DIR}
  NAMES corsika/corsika.h
  DOC "Corsika8 headers"
  )


if( CORSIKA8_INCLUDE_DIR )
  list( REMOVE_DUPLICATES CORSIKA8_INCLUDE_DIR )
endif( CORSIKA8_INCLUDE_DIR )

# Find hydra version
if (CORSIKA8_INCLUDE_DIR)
  file( STRINGS ${CORSIKA_INCLUDE_DIR}/corsika/corsika.h
    version
    REGEX "#define CORSIKA_VERSION[ \t]+([0-9x]+)"
    )
  string( REGEX REPLACE
    "#define CORSIKA_VERSION[ \t]+"
    ""
    version
    "${version}"
    )

  string( REGEX MATCH "^[0-9]" major ${version} )
  string( REGEX REPLACE "^${major}00" "" version "${version}" )
  string( REGEX MATCH "^[0-9]" minor ${version} )
  string( REGEX REPLACE "^${minor}0" "" version "${version}" )
  set( CORSIKA_VERSION "${major}.${minor}.${version}")
  set( CORSIKA_MAJOR_VERSION "${major}")
  set( CORSIKA_MINOR_VERSION "${minor}")
endif()

# Check for required components
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Corsika8
  FOUND_VAR CORSIKA8_FOUND
  REQUIRED_VARS CORSIKA8_INCLUDE_DIR
  VERSION_VAR CORSIKA8_VERSION
  )

if(CORSIKA8_FOUND)
  set(CORSIKA8_INCLUDE_DIRS ${CORSIKA8_INCLUDE_DIR})
endif()

mark_as_advanced(CORSIKA8_INCLUDE_DIR)
