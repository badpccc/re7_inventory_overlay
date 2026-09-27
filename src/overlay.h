#pragma once

#include "inventory.h"

#include <windows.h>

#include <string>
#include <vector>

class Overlay
{
public:
    bool initialize(
        HINSTANCE instance,
        InventoryReader* inventory
    );

    void run();

    void shutdown();

private:
    static LRESULT CALLBACK window_proc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    bool create_window(
        HINSTANCE instance
    );

    void update_inventory();

    void draw(
        HDC hdc
    );

private:
    HWND hwnd_ = nullptr;

    HINSTANCE instance_ = nullptr;

    InventoryReader* inventory_ = nullptr;

    std::vector<InventoryItem> items_;

    bool running_ = true;
};