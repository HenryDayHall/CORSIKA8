
set (sources
  TestMain.cpp
  testSuperStupidStack.cpp
  testNuclearStackExtension.cpp
  )


add_executable (testStack ${sources})

target_link_libraries (testStack CORSIKA8 Catch2)

target_compile_options (testStack PRIVATE -g) # do not skip asserts

target_include_directories (testStack PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

file (MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/test_outputs/)

if (CORSIKA_SANITIZERS_ENABLED)
  # -O1 is suggested in clang docs to get reasonable performance
  target_compile_options (${name} PRIVATE -O1 -fno-omit-frame-pointer -fsanitize=${sanitize} -fno-sanitize-recover=all)
  set_target_properties (${name} PROPERTIES LINK_FLAGS "-fsanitize=${sanitize}")
endif ()

add_test (
  NAME testStack
  COMMAND testStack -o ${PROJECT_BINARY_DIR}/test_outputs/junit-${name}.xml -s -r junit
  )
