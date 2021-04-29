cmake_minimum_required(VERSION 3.1)
project(PackageTest CXX)

list(APPEND CMAKE_MODULE_PATH 
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

option(CONAN_INSTALL_MANUALLY "Run Conan install command manually." OFF)

if(NOT CONAN_INSTALL_MANUALLY)
    include(conan)
    conan_check(REQUIRED)
    conan_cmake_run(
        CONANFILE conanfile.py
        INSTALL_FOLDER ${CMAKE_BINARY_DIR}
        BUILD_TYPE ${CMAKE_BUILD_TYPE}
        BUILD missing
    )
    include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
endif()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(SOURCES
    src/main.cpp
    src/utils.cpp
)
set(HEADERS
    src/utils.h
)

add_executable(dejsc ${HEADERS} ${SOURCES})
target_link_libraries(dejsc ${CONAN_LIBS})
