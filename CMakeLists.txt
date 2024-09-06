cmake_minimum_required(VERSION 3.10)
project(supernova-editor)

# Specify C++ Standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(wxWidgets REQUIRED COMPONENTS core base propgrid)

include(${wxWidgets_USE_FILE})

# Add the executable
add_executable(
    supernova-editor 

    editor/main.cpp
    editor/EditorFrame.cpp
    )

target_link_libraries(supernova-editor ${wxWidgets_LIBRARIES})