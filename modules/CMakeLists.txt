cmake_minimum_required (VERSION 3.9)

project (
  corsika_modules
  VERSION 8.0.0
  DESCRIPTION "CORSIKA modules"
  LANGUAGES CXX
  )

include (FeatureSummary)

# as long as there still are modules using it:
enable_language (Fortran)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_EXTENSIONS OFF)

# enable warnings and disallow non-standard language
# configure the various build types here, too
# FYI: optimizer flags: -O2 would not trade speed for size, neither O2/3 use fast-math
# debug: O0, relwithdebinfo: 02, release: O3, minsizerel: Os (all defaults)
set (CMAKE_CXX_FLAGS "-Wall -pedantic -Wextra -Wno-ignored-qualifiers")
set (CMAKE_Fortran_FLAGS "-std=legacy -Wfunction-elimination")

# clang produces a lot of unecessary warnings without this:
add_compile_options ("$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-nonportable-include-path>")

# order of subdirectories
add_subdirectory (Sibyll)
#add_subdirectory (Pythia)
add_subdirectory (QGSJetII)
add_subdirectory (UrQMD)
