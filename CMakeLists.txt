cmake_minimum_required(VERSION 3.25)
project(supernova-editor)

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW) # NEW for GLVND, OLD for legacy OpenGL
endif()

# Required CMake 3.25+
# https://github.com/wxWidgets/wxWidgets/issues/22590
# https://docs.wxwidgets.org/trunk/overview_cmake.html#cmake_find_package


#for using static libraries
set(wxWidgets_USE_STATIC ON)

find_package(wxWidgets REQUIRED COMPONENTS gl core base propgrid)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

include(${wxWidgets_USE_FILE})

set(SUPERNOVA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/supernova)

add_definitions("-DNO_LUA_INIT")
add_definitions("-DNO_CPP_INIT")
add_definitions("-DSOKOL_GLCORE")
add_definitions("-DWITH_MINIAUDIO") # For SoLoud
add_definitions("-DSUPERNOVA_EDITOR")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_EXEC_FLAGS WIN32)
endif()

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

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

    ${PLATFORM_EXEC_FLAGS}

    editor/main.cpp
    editor/Frame.cpp
    editor/Canvas.cpp
    editor/App.cpp
    editor/Platform.cpp
)

set_target_properties(
    supernova-editor

    PROPERTIES

    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(supernova-editor ${wxWidgets_LIBRARIES} ${OPENGL_LIBRARIES} Threads::Threads supernova)