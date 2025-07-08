cmake_minimum_required(VERSION 3.10)

project(wasm_library)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/PokeFinder/CMake")
add_subdirectory(lib/PokeFinder/Source/Core)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_VERBOSE_MAKEFILE on)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(spinda_search
    main.cpp
)

target_link_libraries(spinda_search PRIVATE PokeFinderCore)
