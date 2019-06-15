
add_library (CORSIKAthirdparty INTERFACE)

target_include_directories (CORSIKAthirdparty SYSTEM
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
  $<INSTALL_INTERFACE:include/ThirdParty>
  )

install (DIRECTORY phys DESTINATION include/ThirdParty/)
install (DIRECTORY catch2 DESTINATION include/ThirdParty/)
install (DIRECTORY boost DESTINATION include/ThirdParty/boost/)

if (NOT WITH_EIGEN3)
  set (_LOCAL_Eigen3_VERSION "eigen-eigen-b3f3d4950030")
  if ("${CMAKE_CURRENT_SOURCE_DIR}/${_LOCAL_Eigen3_VERSION}.tar.bz2"
      IS_NEWER_THAN
      "${CMAKE_CURRENT_SOURCE_DIR}/${_LOCAL_Eigen3_VERSION}")
    message ("Unpacking ThirdParty/eigen3")
    execute_process (
      COMMAND ${CMAKE_COMMAND} -E tar xjf ${_LOCAL_Eigen3_VERSION}.tar.bz2
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      )
  endif ()
  set (LOCAL_Eigen3_VERSION ${CMAKE_CURRENT_SOURCE_DIR}/eigen-eigen-b3f3d4950030 PARENT_SCOPE)
endif (NOT WITH_EIGEN3)
