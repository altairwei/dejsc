cmake_minimum_required(VERSION 3.20)
project(PackageTest CXX)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

set(DEJSC_SOURCES
    src/main.cpp
    src/utils.cpp
    src/shell.cpp
    src/runner.cpp
    src/cache.cpp
    src/deserializer.cpp
)

set(DEJSC_HEADERS
    src/utils.h
    src/shell.h
    src/runner.h
    src/cache.h
    src/deserializer.h
)

add_subdirectory("src/3rdparty/v8-cmake/")
include_directories("src/3rdparty/v8-cmake/")
include_directories("src/3rdparty/v8-cmake/v8")
include_directories("src/3rdparty/")

# Workaroun bug https://gitlab.kitware.com/cmake/cmake/-/issues/18399
cmake_policy(SET CMP0118 NEW)

add_executable(dejsc ${DEJSC_SOURCES} ${DEJSC_HEADERS})

target_link_libraries(dejsc
  PRIVATE
    v8_base_without_compiler
    v8_compiler
    v8_snapshot
    v8_inspector
    v8_libplatform
)

set_target_properties(dejsc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
