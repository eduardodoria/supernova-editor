cmake_minimum_required(VERSION 3.10)
project(supernova-editor)

# Specify C++ Standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(wxWidgets REQUIRED COMPONENTS gl core base propgrid)

include(${wxWidgets_USE_FILE})

set(SUPERNOVA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/supernova)

add_definitions("-DSOKOL_GLCORE")
add_definitions("-DWITH_MINIAUDIO") # For SoLoud

include_directories (${SUPERNOVA_ROOT}/engine/libs/sokol)
include_directories (${SUPERNOVA_ROOT}/engine/libs/lua)
include_directories (${SUPERNOVA_ROOT}/engine/libs/box2d/include)
include_directories (${SUPERNOVA_ROOT}/engine/libs/joltphysics)

include_directories (${SUPERNOVA_ROOT}/engine/core)
include_directories (${SUPERNOVA_ROOT}/engine/core/action)
include_directories (${SUPERNOVA_ROOT}/engine/core/buffer)
include_directories (${SUPERNOVA_ROOT}/engine/core/component)
include_directories (${SUPERNOVA_ROOT}/engine/core/ecs)
include_directories (${SUPERNOVA_ROOT}/engine/core/io)
include_directories (${SUPERNOVA_ROOT}/engine/core/math)
include_directories (${SUPERNOVA_ROOT}/engine/core/object)
include_directories (${SUPERNOVA_ROOT}/engine/core/object/audio)
include_directories (${SUPERNOVA_ROOT}/engine/core/object/ui)
include_directories (${SUPERNOVA_ROOT}/engine/core/object/environment)
include_directories (${SUPERNOVA_ROOT}/engine/core/object/physics)
include_directories (${SUPERNOVA_ROOT}/engine/core/script)
include_directories (${SUPERNOVA_ROOT}/engine/core/subsystem)
include_directories (${SUPERNOVA_ROOT}/engine/core/texture)
include_directories (${SUPERNOVA_ROOT}/engine/core/util)
include_directories (${SUPERNOVA_ROOT}/engine/renders)

add_subdirectory(${SUPERNOVA_ROOT}/engine)

# Add the executable
add_executable(
    supernova-editor 

    editor/main.cpp
    editor/EditorFrame.cpp
    editor/EngineCanvas.cpp
    )

target_link_libraries(supernova-editor ${wxWidgets_LIBRARIES} GL glfw)