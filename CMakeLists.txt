cmake_minimum_required (VERSION 2.8.8)
project(mazerun)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
set(PROJECT_TITLE "MazeRun")
set(PROJECT_AUTHOR "Matthew Chandler <tardarsauce@gmail.com>")
set(PROJECT_SUMMARY "A 3D maze adventure game")
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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ASSIMP assimp REQUIRED)
pkg_check_modules(SIGC sigc++-2.0 REQUIRED)
pkg_check_modules(FONTCONFIG fontconfig REQUIRED)

find_package(SFML 2 COMPONENTS audio graphics system window REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(Freetype REQUIRED)
find_package(Threads REQUIRED)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(X11 REQUIRED)
endif()

# directories
include_directories(
    ${PROJECT_BINARY_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${ASSIMP_INCLUDE_DIRS}
    ${SIGC_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${SFML_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
    )
link_directories(
    ${ASSIMP_LIBRARIES_DIRS}
    ${SIGC_LIBRARIES_DIRS}
    ${FREETYPE_LIBRARIES_DIRS}
    ${SFML_LIBRARIES_DIRS}
    ${GLEW_LIBRARIES_DIRS}
    ${FREETYPE_LIBRARIES_DIRS}
    ${X11_LIBRARIES_DIRS}
    )

# configure variables
set(bindir ${CMAKE_INSTALL_PREFIX}/bin)
set(datadir ${CMAKE_INSTALL_PREFIX}/share/mazerun)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in
    ${PROJECT_BINARY_DIR}/src/config.hpp)
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mazerun.desktop.in
#     ${PROJECT_BINARY_DIR}/mazerun.desktop)
# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mazerun.rc.in
#     ${PROJECT_BINARY_DIR}/mazerun.rc)

# main compilation
add_library(mazegen OBJECT
    src/mazegen/gen_rooms.cpp
    src/mazegen/grid.cpp
    src/mazegen/mazegen.cpp
    src/util/logger.cpp
    )

set(MAZERUN_BUILD_MAZEGEN_2D 1 CACHE STRING "Build 2D maze generator")

if(MAZERUN_BUILD_MAZEGEN_2D)
    add_subdirectory(mazegen_2D)
endif()

add_executable(${PROJECT_NAME}
    # ${PROJECT_BINARY_DIR}/mazerun.rc
    src/main.cpp
    src/components/audio.cpp
    src/components/light.cpp
    src/components/model.cpp
    src/config.cpp
    src/entities/player.cpp
    src/entities/testmdl.cpp
    src/entities/walls.cpp
    src/opengl/framebuffer.cpp
    src/opengl/gl_helpers.cpp
    src/opengl/gl_wrappers.cpp
    src/opengl/renderbuffer.cpp
    src/opengl/shader_prog.cpp
    src/opengl/texture.cpp
    src/util/font.cpp
    src/util/font_libs.cpp
    src/util/message.cpp
    src/util/static_text.cpp
    src/world/entity.cpp
    src/world/quad.cpp
    src/world/skybox.cpp
    src/world/world.cpp
    $<TARGET_OBJECTS:mazegen>
    )
target_link_libraries(${PROJECT_NAME}
    ${ASSIMP_LIBRARIES}
    ${SIGC_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    ${SFML_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${X11_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

# install targets
# install(TARGETS "${PROJECT_NAME}" DESTINATION "bin")
# install(FILES "img/cursor.png" DESTINATION "share/mazerun/img")
# install(DIRECTORY "shaders" "examples" DESTINATION "share/mazerun")
# install(FILES "icons/icon.svg" DESTINATION "share/mazerun/icons")

# if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#     install(FILES "icons/icon.svg" DESTINATION "share/icons/hicolor/scalable/apps" RENAME "mazerun.svg")
#     install(FILES "icons/icon_128.png" DESTINATION "share/icons/hicolor/128x128/apps" RENAME "mazerun.png")
#     install(FILES "${PROJECT_BINARY_DIR}/mazerun.desktop" DESTINATION "share/applications")
# endif()

# CPack vars
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_CONTACT "${PROJECT_AUTHOR}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY)
set(CPACK_INSTALL_CMAKE_PROJECTS
    "${CMAKE_BINARY_DIR};${PROJECT_NAME};ALL;/")
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_TITLE}: ${PROJECT_SUMMARY}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_FILE_README "${CMAKE_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README")
set(CPACK_PACKAGE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}")

# Debian-specific settings
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_program(DPKG "dpkg")
    if(DPKG)
        set(CPACK_GENERATOR DEB;TGZ)
        execute_process(COMMAND ${DPKG} --print-architecture
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
        set(CPACK_DEBIAN_PACKAGE_SECTION "games")
        string(TOLOWER "${PROJECT_NAME}" PACKAGE_NAME_LOWER)
        set(CPACK_PACKAGE_FILE_NAME
            "${PACKAGE_NAME_LOWER}_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
    endif()
endif()

# Windows NSIS specifics:
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CPACK_GENERATOR NSIS;TGZ)
    set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME};${PROJECT_TILE}")
    set(CPACK_NSIS_DISPLAY_NAME "${PROJECT_TILE}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
    set(CPACK_NSIS_CONTACT "${PROJECT_AUTHOR}")
    set(CPACK_NSIS_HELP_LINK "${PROJECT_WEBSITE}")
    # set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/icons/icon_128.ico")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
endif()

include(CPack)
