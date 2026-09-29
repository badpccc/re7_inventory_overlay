#pragma once

#include "inventory.h"

#include <windows.h>
#include <gdiplus.h>

#include <string>
#include <vector>

class Overlay
{
public:
    Overlay();
    ~Overlay();

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
        WPARAM wparam,
        LPARAM lparam
    );

    bool create_window(
        HINSTANCE instance
    );

    void paint();

    void draw_inventory(
        Gdiplus::Graphics& graphics
    );

    Gdiplus::Image* load_icon(
        int icon_frame
    );

    std::wstring get_icon_path(
        int icon_frame
    ) const;

    bool inventory_changed(
        const std::vector<InventoryItem>& current_items
    ) const;

private:
    HWND hwnd_ = nullptr;

    HINSTANCE instance_ = nullptr;

    InventoryReader* inventory_ = nullptr;

    bool running_ = false;

    /*
        Cache do último estado conhecido
        do inventário.
    */

    std::vector<InventoryItem> cached_items_;

    bool inventory_initialized_ = false;

    int window_width_ = 500;
    int window_height_ = 360;

    int origin_x_ = 20;
    int origin_y_ = 20;

    int slot_width_ = 112;
    int slot_height_ = 88;

    int slot_gap_ = 5;

    int icon_padding_ = 8;

    std::wstring executable_directory_;

private:
    static constexpr wchar_t WINDOW_CLASS_NAME[] =
        L"RE7InventoryOverlayWindow";
};