cmake_minimum_required(VERSION 3.23)
# 3.23 Needed for file sets to collect SpeakEasy2 headers.

project(
  speakeasyr
  DESCRIPTION "R bindings for the SpeakEasy2 C library."
  LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF)

# From rigraph
execute_process(COMMAND bash "-c" "Rscript -e 'cat(R.home(\"include\"))'" OUTPUT_VARIABLE R_INCLUDE)
execute_process(COMMAND bash "-c" "Rscript -e 'x <- desc::desc_get_deps(); pkgs <- x$package[x$type == \"LinkingTo\"]; paths <- file.path(.libPaths()[[1]], pkgs, \"include\"); cat(paths, sep = \";\")'" OUTPUT_VARIABLE R_LIBRARIES_INCLUDES)

include_directories(${R_INCLUDE} ${R_LIBRARIES_INCLUDES})

add_compile_definitions(USING_R)
add_subdirectory(tools/se2)

# To create compile_commands.json. Not otherwise needed as R will handle
# compiling. Specify -DDEVELOP=1 when configuring.
if ("${DEVELOP}")
  add_library(RSE2 src/speakeasy2.c)
  target_link_libraries(RSE2 SpeakEasy2)
endif()

target_sources(SpeakEasy2 PUBLIC FILE_SET HEADERS
  BASE_DIRS ${PROJECT_SOURCE_DIR}/tools/se2/include
  FILES ${PROJECT_SOURCE_DIR}/tools/se2/include/speak_easy_2.h)

install(TARGETS SpeakEasy2
  DESTINATION .
  RUNTIME_DEPENDENCY_SET SE2_deps)

install(TARGETS SpeakEasy2 FILE_SET HEADERS)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  install(RUNTIME_DEPENDENCY_SET SE2_deps
    DIRECTORIES
    "C:\\msys64\\mingw64\\bin" "C:\\msys64\\ucrt64\\bin"
    "D:\\msys64\\mingw64\\bin" "D:\\msys64\\ucrt64\\bin"
    PRE_EXCLUDE_REGEXES "api-ms-*" "libmx" "libmex" "ext-ms-*"
    DESTINATION lib)
endif()
