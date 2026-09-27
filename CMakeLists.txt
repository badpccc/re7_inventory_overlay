cmake_minimum_required(VERSION 3.20)

project(RE7InventoryOverlay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    RE7InventoryOverlay
    WIN32

    src/main.cpp

    src/overlay.cpp
    src/overlay.h

    src/inventory.cpp
    src/inventory.h

    src/memory.cpp
    src/memory.h
)

target_link_libraries(
    RE7InventoryOverlay

    user32
    gdi32
)