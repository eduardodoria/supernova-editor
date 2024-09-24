cmake_minimum_required(VERSION 3.20)
project(supernova-editor)

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW) # NEW for GLVND, OLD for legacy OpenGL
endif()

if(WIN32)
    set(PLATFORM_EXEC_FLAGS WIN32)
    if(MSVC)
        set(PLATFORM_PROPERTIES_FLAGS LINK_FLAGS "/ENTRY:mainCRTStartup")
    endif()

    if(MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
        include(InstallRequiredSystemLibraries)
        file(COPY ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ${CMAKE_BINARY_DIR})
    endif()

    set(CMAKE_PREFIX_PATH "$ENV{ProgramFiles}/GLFW" ${CMAKE_PREFIX_PATH})
    set(CMAKE_PREFIX_PATH "$ENV{ProgramFiles\(X86\)}/GLFW" ${CMAKE_PREFIX_PATH})
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(glfw3 REQUIRED)

if(NOT DEFINED GLFW_LIBRARIES)
    set(GLFW_LIBRARIES glfw)
endif()

set(SUPERNOVA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/supernova)

add_definitions("-DNO_LUA_INIT")
add_definitions("-DNO_CPP_INIT")
add_definitions("-DSOKOL_GLCORE")
add_definitions("-DWITH_MINIAUDIO") # For SoLoud
add_definitions("-DSUPERNOVA_EDITOR")

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

# Add ImGui source files
set(IMGUI_DIR libs/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)


# Create executable
set(EDITOR_DIR editor)
add_executable(
    supernova-editor

    ${PLATFORM_EXEC_FLAGS}

    ${EDITOR_DIR}/main.cpp
    ${EDITOR_DIR}/App.cpp
    ${EDITOR_DIR}/Platform.cpp
    ${EDITOR_DIR}/window/Objects.cpp
    ${EDITOR_DIR}/window/Properties.cpp
    ${EDITOR_DIR}/window/Console.cpp

    ${IMGUI_SOURCES}
)

include_directories (editor)

# Include directories
target_include_directories(supernova-editor PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${OPENGL_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(
    supernova-editor
    
    PRIVATE
    
    ${GLFW_LIBRARIES}
    ${OPENGL_LIBRARIES}
    Threads::Threads
    supernova
    $<$<CXX_COMPILER_ID:MSVC>:ucrt>
)

set_target_properties(
    supernova-editor

    PROPERTIES

    ${PLATFORM_PROPERTIES_FLAGS}
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

install(TARGETS supernova-editor 
        CONFIGURATIONS Release MinSizeRel 
        RUNTIME DESTINATION bin)
