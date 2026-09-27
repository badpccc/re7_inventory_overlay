#include "overlay.h"

#include <windows.h>

#include <sstream>
#include <string>


bool Overlay::initialize(
    HINSTANCE instance,
    InventoryReader* inventory
)
{
    instance_ = instance;

    inventory_ = inventory;

    if (!inventory_)
        return false;

    return create_window(
        instance
    );
}


bool Overlay::create_window(
    HINSTANCE instance
)
{
    const wchar_t* class_name =
        L"RE7InventoryOverlay";


    WNDCLASSEXW wc{};

    wc.cbSize =
        sizeof(WNDCLASSEXW);

    wc.lpfnWndProc =
        Overlay::window_proc;

    wc.hInstance =
        instance;

    wc.hCursor =
        LoadCursor(
            nullptr,
            IDC_ARROW
        );

    wc.hbrBackground =
        CreateSolidBrush(
            RGB(
                0,
                0,
                0
            )
        );

    wc.lpszClassName =
        class_name;


    if (!RegisterClassExW(&wc))
    {
        if (GetLastError() !=
            ERROR_CLASS_ALREADY_EXISTS)
        {
            return false;
        }
    }


    hwnd_ =
        CreateWindowExW(
            WS_EX_TOPMOST |
            WS_EX_TOOLWINDOW,

            class_name,

            L"RE7 Inventory",

            WS_OVERLAPPED |
            WS_CAPTION |
            WS_SYSMENU |
            WS_MINIMIZEBOX,

            20,
            20,

            620,
            500,

            nullptr,
            nullptr,
            instance,
            this
        );


    if (!hwnd_)
        return false;


    ShowWindow(
        hwnd_,
        SW_SHOW
    );

    UpdateWindow(
        hwnd_
    );


    SetTimer(
        hwnd_,
        1,
        100,
        nullptr
    );


    return true;
}


void Overlay::run()
{
    MSG message{};


    while (running_)
    {
        BOOL result =
            GetMessageW(
                &message,
                nullptr,
                0,
                0
            );


        if (result <= 0)
            break;


        TranslateMessage(
            &message
        );

        DispatchMessageW(
            &message
        );
    }
}


void Overlay::shutdown()
{
    running_ = false;


    if (hwnd_)
    {
        KillTimer(
            hwnd_,
            1
        );


        DestroyWindow(
            hwnd_
        );


        hwnd_ = nullptr;
    }
}


void Overlay::update_inventory()
{
    if (!inventory_)
        return;


    std::vector<InventoryItem> new_items =
        inventory_->get_items();


    items_ =
        std::move(
            new_items
        );


    InvalidateRect(
        hwnd_,
        nullptr,
        FALSE
    );
}


void Overlay::draw(
    HDC hdc
)
{
    RECT rect{};

    GetClientRect(
        hwnd_,
        &rect
    );


    HBRUSH background =
        CreateSolidBrush(
            RGB(
                0,
                0,
                0
            )
        );


    FillRect(
        hdc,
        &rect,
        background
    );


    DeleteObject(
        background
    );


    SetBkMode(
        hdc,
        TRANSPARENT
    );


    SetTextColor(
        hdc,
        RGB(
            255,
            255,
            255
        )
    );


    HFONT font =
        CreateFontW(
            18,
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH |
            FF_DONTCARE,
            L"Consolas"
        );


    HFONT old_font =
        reinterpret_cast<HFONT>(
            SelectObject(
                hdc,
                font
            )
        );


    int y = 15;


    TextOutW(
        hdc,
        15,
        y,
        L"RE7 INVENTORY",
        13
    );


    y += 30;


    std::wstringstream header;

    header
        << L"Total: "
        << items_.size();


    const std::wstring header_text =
        header.str();


    TextOutW(
        hdc,
        15,
        y,
        header_text.c_str(),
        static_cast<int>(
            header_text.size()
        )
    );


    y += 35;


    for (
        const InventoryItem& item :
        items_
    )
    {
        std::wstringstream line;


        line
            << L"Slot "
            << item.slot
            << L" | "
            << std::wstring(
                item.id.begin(),
                item.id.end()
            )
            << L" | Qty "
            << item.quantity
            << L" | Max "
            << item.max_stack
            << L" | Icon "
            << item.icon_frame;


        const std::wstring text =
            line.str();


        TextOutW(
            hdc,
            15,
            y,
            text.c_str(),
            static_cast<int>(
                text.size()
            )
        );


        y += 25;


        if (y > rect.bottom - 25)
            break;
    }


    SelectObject(
        hdc,
        old_font
    );


    DeleteObject(
        font
    );
}


LRESULT CALLBACK Overlay::window_proc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    Overlay* overlay =
        reinterpret_cast<Overlay*>(
            GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )
        );


    switch (message)
    {
        case WM_NCCREATE:
        {
            CREATESTRUCTW* create =
                reinterpret_cast<CREATESTRUCTW*>(
                    lParam
                );


            overlay =
                reinterpret_cast<Overlay*>(
                    create->lpCreateParams
                );


            SetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(
                    overlay
                )
            );


            return TRUE;
        }


        case WM_TIMER:
        {
            if (
                overlay &&
                wParam == 1
            )
            {
                overlay->update_inventory();
            }


            return 0;
        }


        case WM_PAINT:
        {
            PAINTSTRUCT ps{};


            HDC hdc =
                BeginPaint(
                    hwnd,
                    &ps
                );


            if (overlay)
            {
                overlay->draw(
                    hdc
                );
            }


            EndPaint(
                hwnd,
                &ps
            );


            return 0;
        }


        case WM_CLOSE:
        {
            DestroyWindow(
                hwnd
            );


            return 0;
        }


        case WM_DESTROY:
        {
            if (overlay)
            {
                overlay->running_ =
                    false;
            }


            KillTimer(
                hwnd,
                1
            );


            PostQuitMessage(
                0
            );


            return 0;
        }
    }


    return DefWindowProcW(
        hwnd,
        message,
        wParam,
        lParam
    );
}