cmake_minimum_required (VERSION 2.8.8)
project(mazegen_2D)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
set(PROJECT_TITLE "MazeGen 2D")
set(PROJECT_AUTHOR "Matthew Chandler <tardarsauce@gmail.com>")
set(PROJECT_SUMMARY "2D maze generation")
set(PROJECT_WEBSITE "http://github.com/mattvchandler/mazerun")

#flags
set(CMAKE_CXX_FLAGS "-Wall -std=c++14")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(MSYS AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "-mwindows")
endif()

# libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-3.0 REQUIRED REQUIRED)

# configure variables
# set(bindir ${CMAKE_INSTALL_PREFIX}/bin)
# set(datadir ${CMAKE_INSTALL_PREFIX}/share/mazegen/)

# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in
#     ${PROJECT_BINARY_DIR}/src/config.hpp)
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mazegen.desktop.in
#     ${PROJECT_BINARY_DIR}/mazegen.desktop)
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mazegen.rc.in
#     ${PROJECT_BINARY_DIR}/mazegen.rc)

# directories
include_directories(
    ${PROJECT_BINARY_DIR}/src/
    ${PROJECT_BINARY_DIR}/mazegen_2D/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/
    ${GTKMM_INCLUDE_DIRS}
    )
link_directories(
    ${GTKMM_LIBRARY_DIRS}
    )

# main compilation
add_executable(${PROJECT_NAME}
    # ${PROJECT_BINARY_DIR}/mazegen.rc
    # src/config.cpp
    src/main.cpp
    src/maze.cpp
    $<TARGET_OBJECTS:mazegen>
    )

target_link_libraries(${PROJECT_NAME}
    ${GTKMM_LIBRARIES}
    )
