cmake_minimum_required(VERSION 3.20)
project(VorExternal LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(VorExternal
    src/main.cpp
    src/config/config.cpp
    src/memory/memory.cpp
    src/overlay/overlay.cpp
    src/cheat/cheat.cpp
    src/render/render.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/backends/imgui_impl_win32.cpp
    src/imgui/backends/imgui_impl_dx11.cpp
)

target_include_directories(VorExternal PRIVATE
    src
    src/imgui
    src/imgui/backends
)

target_link_libraries(VorExternal PRIVATE
    d3d11
    dxgi
    dwmapi
)
