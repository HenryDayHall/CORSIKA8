
add_library (CORSIKAthirdparty INTERFACE)

target_include_directories (CORSIKAthirdparty SYSTEM
  INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
  $<INSTALL_INTERFACE:include/ThirdParty>
  )

install (DIRECTORY phys DESTINATION include/ThirdParty/)
install (DIRECTORY catch2 DESTINATION include/ThirdParty/)
install (DIRECTORY boost DESTINATION include/ThirdParty/boost/)

if (THIRDPARTY_Eigen3)
  set (LOCAL_Eigen3_VERSION "eigen-eigen-b3f3d4950030") 
  execute_process (
    COMMAND ${CMAKE_COMMAND} -E tar xjf ${LOCAL_Eigen3_VERSION}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )  
  set (EIGEN3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${LOCAL_Eigen3_VERSION}" PARENT_SCOPE)
endif (THIRDPARTY_Eigen3)
