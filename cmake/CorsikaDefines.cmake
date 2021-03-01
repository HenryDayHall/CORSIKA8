#
# Floating point exception support - select implementation to use
#
include (CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX ("fenv.h" HAS_FEENABLEEXCEPT)
if (HAS_FEENABLEEXCEPT) # FLOATING_POINT_ENVIRONMENT
    set (CORSIKA_HAS_FEENABLEEXCEPT 1)
    set_property (DIRECTORY ${CMAKE_HOME_DIRECTORY} APPEND PROPERTY COMPILE_DEFINITIONS "HAS_FEENABLEEXCEPT")
endif ()


#
# General OS Detection
#
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set (CORSIKA_OS_WINDOWS TRUE)
    set (CORSIKA_OS "Windows")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set (CORSIKA_OS_LINUX TRUE)
    set (CORSIKA_OS "Linux")
    # check for RedHat/CentOS compiler from the software collections (scl)
    string (FIND ${CMAKE_CXX_COMPILER} "/opt/rh/devtoolset-" index)
    if (${index} EQUAL 0)
      set (CORSIKA_SCL_CXX TRUE)
    endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set (CORSIKA_OS_MAC TRUE)  
    set (CORSIKA_OS "Mac") 
endif ()
