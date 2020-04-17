
set (sources
  TestMain.cpp
  testEnvironment.cpp
  )


add_executable (testEnvironment ${sources})

target_link_libraries (testEnvironment CORSIKA8 Catch2)

target_compile_options (testEnvironment PRIVATE -g) # do not skip asserts

target_include_directories (testEnvironment PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/test_outputs/)

if (CORSIKA_SANITIZERS_ENABLED)
  # -O1 is suggested in clang docs to get reasonable performance
  target_compile_options (${name} PRIVATE -O1 -fno-omit-frame-pointer -fsanitize=${sanitize} -fno-sanitize-recover=all)
  set_target_properties (${name} PROPERTIES LINK_FLAGS "-fsanitize=${sanitize}")
endif ()

add_test (
  NAME testEnvironment
  COMMAND testEnvironment -o ${PROJECT_BINARY_DIR}/test_outputs/junit-media.xml -s -r junit)
