cmake_minimum_required(VERSION 3.1)
project(PackageTest CXX)

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/3rdparty/v8-cmake/cmake"
)

set(DEJSC_SOURCES
    src/main.cpp
    src/utils.cpp
    src/shell.cpp
    src/runner.cpp
)

set(DEJSC_HEADERS
    src/utils.h
    src/shell.h
    src/runner.h
)

add_subdirectory("src/3rdparty/v8-cmake/")
include_directories("src/3rdparty/v8-cmake/v8/")
include_directories("src/3rdparty/")

add_executable(dejsc ${DEJSC_SOURCES} ${DEJSC_HEADERS})

target_include_directories(dejsc
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/3rdparty/v8-cmake/v8/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/3rdparty/v8-cmake/v8>
)

target_link_libraries(dejsc
  PRIVATE
    v8_base_without_compiler
)

set_target_properties(dejsc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
