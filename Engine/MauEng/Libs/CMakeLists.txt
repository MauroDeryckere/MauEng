cmake_minimum_required(VERSION 3.10)

project(MauEngLibs)

# Core ImGui Library
add_library(MauENG_ImGUI STATIC
    ImGUI/imgui.cpp
    ImGUI/imgui_draw.cpp
    ImGUI/imgui_widgets.cpp
    ImGUI/imgui_tables.cpp
    ImGUI/imgui_demo.cpp
)

target_include_directories(MauENG_ImGUI PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ImGUI")

# ImGui Backend (SDL2 + Vulkan)

add_library(MauENG_ImGUI_Backends STATIC
   "${CMAKE_CURRENT_SOURCE_DIR}/ImGUI/backends/imgui_impl_vulkan.cpp"
   "${CMAKE_CURRENT_SOURCE_DIR}/ImGUI/backends/imgui_impl_sdl2.cpp"
)

target_include_directories(MauENG_ImGUI_Backends PUBLIC
     "${CMAKE_CURRENT_SOURCE_DIR}/ImGUI/imgui"
     "${CMAKE_CURRENT_SOURCE_DIR}/ImGUI/backends"
)

target_link_libraries(MauENG_ImGUI_Backends PUBLIC MauENG_ImGUI)

message(STATUS "ImGUI Linked! ")
