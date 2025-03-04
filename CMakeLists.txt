cmake_minimum_required(VERSION 3.20)
project(supernova-editor)

set(SUPERNOVA_SHARED ON)
set(GRAPHIC_BACKEND "glcore")

if(NOT API_BACKEND)
    set(API_BACKEND "glfw")
endif()
message(STATUS "API backend is set to ${API_BACKEND}")

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW) # NEW for GLVND, OLD for legacy OpenGL
endif()

if(WIN32)
    set(PLATFORM_EXEC_FLAGS WIN32)
 
    if(MSVC)
        set(PLATFORM_PROPERTIES_FLAGS LINK_FLAGS "/ENTRY:mainCRTStartup")
        if(SUPERNOVA_SHARED)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        endif()
    endif()

    if(MINGW)
        set(MINGW_GCC_LIBS "-static -static-libgcc -static-libstdc++")
    endif()

    set(CMAKE_PREFIX_PATH "$ENV{ProgramFiles}/GLFW" ${CMAKE_PREFIX_PATH})
    set(CMAKE_PREFIX_PATH "$ENV{ProgramFiles\(X86\)}/GLFW" ${CMAKE_PREFIX_PATH})
endif()

set(IMGUI_DIR libs/imgui)
set(IMGUICOLORTEXTEDIT_DIR libs/ImGuiColorTextEdit)
set(EDITOR_DIR editor)

# Find OpenGL
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

set(BACKEND_SOURCES)
set(BACKEND_LIBS)

if(API_BACKEND STREQUAL "glfw")
    find_package(glfw3 REQUIRED)

    if(NOT DEFINED GLFW_LIBRARIES)
        set(GLFW_LIBRARIES glfw)
    endif()

    list(APPEND BACKEND_SOURCES
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${EDITOR_DIR}/backend/BackendGLFW.cpp
    )

    list(APPEND BACKEND_LIBS
        ${GLFW_LIBRARIES}
    )
endif()

if(API_BACKEND STREQUAL "sdl")
    find_package(SDL2 REQUIRED)

    list(APPEND BACKEND_SOURCES
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
        ${EDITOR_DIR}/backend/BackendSDL.cpp
    )

    list(APPEND BACKEND_LIBS
        ${SDL2_LIBRARIES}
    )
endif()

set(SUPERNOVA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/supernova)

add_definitions("-DNO_LUA_INIT")
add_definitions("-DNO_CPP_INIT")
add_definitions("-DSOKOL_GLCORE")
add_definitions("-DWITH_MINIAUDIO") # For SoLoud
set(NFD_PORTAL ON CACHE BOOL "Use xdg-desktop-portal instead of GTK") # For NFD

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

add_subdirectory(libs/nfd)
add_subdirectory(libs/yaml-cpp)

# Add ImGui source files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(IMGUICOLORTEXTEDIT_SOURCES
    ${IMGUICOLORTEXTEDIT_DIR}/TextEditor.cpp
    ${IMGUICOLORTEXTEDIT_DIR}/LanguageDefinitions.cpp
    #${IMGUICOLORTEXTEDIT_DIR}/ImGuiDebugPanel.cpp
    #${IMGUICOLORTEXTEDIT_DIR}/UnitTests.cpp
)

include_directories (${EDITOR_DIR})

if(UNIX AND NOT APPLE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

add_executable(
    supernova-editor

    ${PLATFORM_EXEC_FLAGS}

    ${EDITOR_DIR}/main.cpp
    ${EDITOR_DIR}/App.cpp
    ${EDITOR_DIR}/AppSettings.cpp
    ${EDITOR_DIR}/Factory.cpp
    ${EDITOR_DIR}/Out.cpp
    ${EDITOR_DIR}/Platform.cpp
    ${EDITOR_DIR}/Project.cpp
    ${EDITOR_DIR}/Catalog.cpp
    ${EDITOR_DIR}/Conector.cpp
    ${EDITOR_DIR}/Generator.cpp
    ${EDITOR_DIR}/Stream.cpp

    ${EDITOR_DIR}/window/Properties.cpp
    ${EDITOR_DIR}/window/OutputWindow.cpp
    ${EDITOR_DIR}/window/SceneWindow.cpp
    ${EDITOR_DIR}/window/Structure.cpp
    ${EDITOR_DIR}/window/ResourcesWindow.cpp
    ${EDITOR_DIR}/window/CodeEditor.cpp

    ${EDITOR_DIR}/render/ViewportGizmo.cpp
    ${EDITOR_DIR}/render/ToolsLayer.cpp
    ${EDITOR_DIR}/render/UILayer.cpp
    ${EDITOR_DIR}/render/SceneRender.cpp
    ${EDITOR_DIR}/render/gizmo/TranslateGizmo.cpp
    ${EDITOR_DIR}/render/gizmo/RotateGizmo.cpp
    ${EDITOR_DIR}/render/gizmo/ScaleGizmo.cpp

    ${EDITOR_DIR}/command/CommandHandle.cpp
    ${EDITOR_DIR}/command/CommandHistory.cpp
    ${EDITOR_DIR}/command/type/CreateEntityCmd.cpp
    ${EDITOR_DIR}/command/type/DeleteEntityCmd.cpp
    ${EDITOR_DIR}/command/type/MoveEntityOrderCmd.cpp
    ${EDITOR_DIR}/command/type/ObjectTransformCmd.cpp
    ${EDITOR_DIR}/command/type/EntityNameCmd.cpp
    ${EDITOR_DIR}/command/type/SceneNameCmd.cpp
    ${EDITOR_DIR}/command/type/CopyFileCmd.cpp
    ${EDITOR_DIR}/command/type/RenameFileCmd.cpp
    ${EDITOR_DIR}/command/type/CreateDirCmd.cpp
    ${EDITOR_DIR}/command/type/DeleteFileCmd.cpp

    ${IMGUI_SOURCES}
    ${IMGUICOLORTEXTEDIT_SOURCES}
    ${BACKEND_SOURCES}
)

# Include directories
target_include_directories(supernova-editor PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUICOLORTEXTEDIT_DIR}
    ${IMGUICOLORTEXTEDIT_DIR}/vendor/regex/include
    ${OPENGL_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(
    supernova-editor
    
    PRIVATE
    
    ${BACKEND_LIBS}
    ${OPENGL_LIBRARIES}
    ${MINGW_GCC_LIBS}
    Threads::Threads
    supernova
    nfd
    yaml-cpp
)

set_target_properties(
    supernova-editor

    PROPERTIES

    ${PLATFORM_PROPERTIES_FLAGS}
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

install(TARGETS supernova-editor supernova
        CONFIGURATIONS Release MinSizeRel 
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION bin
)
