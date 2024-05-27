cmake_minimum_required(VERSION 3.18)

project(
  speakeasyR
  DESCRIPTION "R bindings for the SpeakEasy2 C library."
  LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF)

include_directories("${R_INCLUDE_DIRS}")

add_compile_definitions(USING_R)
add_subdirectory(se2)
set_property(TARGET igraph PROPERTY C_STANDARD 17)

if (${DEVELOP})
  # To create compile_commands.json. Not otherwise needed as R will handle
  # compiling. Specify -DDEVELOP=1 when configuring.
  add_library(RSE2 ../src/speakeasyR.c)
  target_link_libraries(RSE2 SpeakEasy2)
endif ()

install(TARGETS SpeakEasy2
  DESTINATION .
  RUNTIME_DEPENDENCY_SET SE2_deps)

install(FILES ${PROJECT_SOURCE_DIR}/se2/include/speak_easy_2.h
  DESTINATION include)
