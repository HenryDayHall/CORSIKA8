cmake_minimum_required (VERSION 3.4.3)

set (CMAKE_CXX_STANDARD 14)

project (CORSIKA VERSION 8.0.0.0 DESCRIPTION "CORSIKA C++ project")

find_package (Boost 1.40 COMPONENTS program_options REQUIRED)
find_package (Eigen3 3.3 REQUIRED)

# include_directories (${Boost_INCLUDE_DIR})
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ADD_EXECUTABLE( anyExecutable myMain.cpp )
# TARGET_LINK_LIBRARIES( anyExecutable LINK_PUBLIC ${Boost_LIBRARIES} )

add_subdirectory (Framework)
