add_subdirectory(cxroot)

add_library(conex_random STATIC conex_random.cpp)
target_link_libraries(conex_random PUBLIC modules_common)
target_link_libraries(CONEXsibyll conex_random)

#
# add to combined CORSIKA8 target
#
add_dependencies (CORSIKA8 CONEXsibyll)
target_link_libraries (CORSIKA8 INTERFACE CONEXsibyll)

#
# install and export
#  
install (
  TARGETS CONEXsibyll conex_random
  EXPORT CORSIKA8PublicTargets
  ARCHIVE DESTINATION lib/corsika
  PUBLIC_HEADER DESTINATION include/corsika_modules/conex
  )
