#
# Floating point exception support - select implementation to use
#
try_compile (HAS_FEENABLEEXCEPT "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/try_feenableexcept.cc")
if (HAS_FEENABLEEXCEPT) # FLOATING_POINT_ENVIRONMENT
    set (CORSIKA_HAS_FEENABLEEXCEPT 1)
    set_property(DIRECTORY ${CMAKE_HOME_DIRECTORY} APPEND PROPERTY COMPILE_DEFINITIONS "HAS_FEENABLEEXCEPT")
endif ()



if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CORSIKA_OS_WINDOWS TRUE)
     set (CORSIKA_OS "Windows")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CORSIKA_OS_LINUX TRUE)
     set (CORSIKA_OS "Linux")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CORSIKA_OS_MAC TRUE)  
    set (CORSIKA_OS "Mac") 
endif()