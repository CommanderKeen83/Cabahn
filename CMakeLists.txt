cmake_minimum_required(VERSION 3.31.5)

if (NOT CMAKE_VERSION VERSION_EQUAL "3.31.5")
    message(FATAL_ERROR "CMake version must be 3.31.5. You have ${CMAKE_VERSION}.")
endif()

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "")

# Must come BEFORE project() — this is the gate that unlocks import std
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

# Also needs to be set before project() when using Clang/libc++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

project(modulnorlokakis CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Known CMake quirk: import std currently requires extensions ON, not OFF
# https://gitlab.kitware.com/cmake/cmake/-/issues/25916
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_CXX_MODULE_STD ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(app
    src/main.cpp
    src/MenuState.cpp
    src/GameState.cpp
    src/PauseState.cpp
    src/OptionState.cpp
)
target_sources(app
    PUBLIC
    FILE_SET CXX_MODULES FILES
        src/SharedContext.cppm
        src/EventManager.cppm
        src/Window.cppm
        src/Application.cppm
        src/core.cppm
        src/StateManager.cppm
        src/State.cppm
        src/MenuState.cppm
        src/GameState.cppm
        src/PauseState.cppm
        src/OptionState.cppm
)
target_link_libraries(app PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)