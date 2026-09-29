#define NOMINMAX

#include "overlay.h"

#include <windows.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

using namespace Gdiplus;

constexpr wchar_t Overlay::WINDOW_CLASS_NAME[];

Overlay::Overlay()
{
}

Overlay::~Overlay()
{
    shutdown();
}

bool Overlay::initialize(
    HINSTANCE instance,
    InventoryReader* inventory
)
{
    instance_ = instance;
    inventory_ = inventory;

    if (!inventory_)
        return false;

    wchar_t module_path[MAX_PATH]{};

    DWORD length =
        GetModuleFileNameW(
            nullptr,
            module_path,
            MAX_PATH
        );

    if (length > 0)
    {
        std::filesystem::path path(
            module_path
        );

        executable_directory_ =
            path.parent_path().wstring();
    }

    return create_window(
        instance_
    );
}

bool Overlay::create_window(
    HINSTANCE instance
)
{
    WNDCLASSEXW wc{};

    wc.cbSize =
        sizeof(WNDCLASSEXW);

    wc.style =
        CS_HREDRAW |
        CS_VREDRAW;

    wc.lpfnWndProc =
        &Overlay::window_proc;

    wc.hInstance =
        instance;

    wc.hCursor =
        LoadCursorW(
            nullptr,
            MAKEINTRESOURCEW(32512)
        );

    wc.hbrBackground =
        CreateSolidBrush(
            RGB(0, 0, 0)
        );

    wc.lpszClassName =
        WINDOW_CLASS_NAME;

    if (!RegisterClassExW(&wc))
    {
        DWORD error =
            GetLastError();

        if (error != ERROR_CLASS_ALREADY_EXISTS)
            return false;
    }

    /*
        Janela opaca.

        Não usamos:
        WS_EX_LAYERED
        WS_EX_TRANSPARENT
    */

    DWORD extended_style =
        WS_EX_TOPMOST |
        WS_EX_TOOLWINDOW;

    hwnd_ =
        CreateWindowExW(
            extended_style,

            WINDOW_CLASS_NAME,

            L"RE7 Inventory Overlay",

            WS_POPUP,

            20,
            20,

            window_width_,
            window_height_,

            nullptr,
            nullptr,
            instance,
            this
        );

    if (!hwnd_)
        return false;

    SetWindowPos(
        hwnd_,
        HWND_TOPMOST,

        20,
        20,

        window_width_,
        window_height_,

        SWP_SHOWWINDOW
    );

    ShowWindow(
        hwnd_,
        SW_SHOW
    );

    UpdateWindow(
        hwnd_
    );

    /*
        Primeira leitura do inventário.

        A partir daqui a overlay só será
        redesenhada quando o estado mudar.
    */

    cached_items_ =
        inventory_->get_items();

    inventory_initialized_ = true;

    /*
        Primeiro desenho.
    */

    InvalidateRect(
        hwnd_,
        nullptr,
        FALSE
    );

    running_ = true;

    return true;
}

void Overlay::run()
{
    if (!hwnd_)
        return;

    MSG message{};

    /*
        Esse intervalo NÃO controla o redesenho.

        Ele somente serve para verificar se
        o inventário mudou.

        O desenho só acontece quando detectamos
        uma mudança.
    */

    DWORD last_inventory_check =
        GetTickCount();

    constexpr DWORD INVENTORY_CHECK_INTERVAL =
        100;

    while (running_)
    {
        while (
            PeekMessageW(
                &message,
                nullptr,
                0,
                0,
                PM_REMOVE
            )
        )
        {
            if (
                message.message ==
                WM_QUIT
            )
            {
                running_ = false;
                break;
            }

            TranslateMessage(
                &message
            );

            DispatchMessageW(
                &message
            );
        }

        if (!running_)
            break;

        DWORD now =
            GetTickCount();

        /*
            =====================================
            VERIFICAÇÃO DO INVENTÁRIO
            =====================================

            A leitura acontece periodicamente,
            mas NÃO redesenhamos a janela se
            nada mudou.
        */

        if (
            now -
            last_inventory_check >=
            INVENTORY_CHECK_INTERVAL
        )
        {
            last_inventory_check =
                now;

            if (inventory_)
            {
                std::vector<InventoryItem>
                    current_items =
                    inventory_->get_items();

                if (
                    inventory_changed(
                        current_items
                    )
                )
                {
                    /*
                        O inventário mudou.

                        Atualiza o cache.
                    */

                    cached_items_ =
                        std::move(
                            current_items
                        );

                    /*
                        Agora sim solicita
                        um novo desenho.
                    */

                    InvalidateRect(
                        hwnd_,
                        nullptr,
                        FALSE
                    );
                }
            }
        }

        /*
            Mantém a janela acima do jogo.

            Isso não redesenha o conteúdo.
        */

        SetWindowPos(
            hwnd_,
            HWND_TOPMOST,

            20,
            20,

            window_width_,
            window_height_,

            SWP_NOMOVE |
            SWP_NOSIZE |
            SWP_NOACTIVATE |
            SWP_NOOWNERZORDER
        );

        Sleep(10);
    }
}

bool Overlay::inventory_changed(
    const std::vector<InventoryItem>& current_items
) const
{
    /*
        Se a quantidade de itens mudou,
        alguma coisa entrou ou saiu.
    */

    if (
        current_items.size() !=
        cached_items_.size()
    )
    {
        return true;
    }

    /*
        Compara os itens por slot.

        Como o inventário é ordenado pelo
        InventoryReader por SlotNo, podemos
        comparar a posição correspondente.
    */

    for (
        size_t i = 0;
        i < current_items.size();
        ++i
    )
    {
        const InventoryItem& old_item =
            cached_items_[i];

        const InventoryItem& new_item =
            current_items[i];

        /*
            Slot diferente
            = mudança de inventário.
        */

        if (
            old_item.slot !=
            new_item.slot
        )
        {
            return true;
        }

        /*
            Item diferente no mesmo slot
            = item entrou/saiu/trocou.
        */

        if (
            old_item.id !=
            new_item.id
        )
        {
            return true;
        }

        /*
            Quantidade diferente também
            atualiza a overlay.
        */

        if (
            old_item.quantity !=
            new_item.quantity
        )
        {
            return true;
        }

        /*
            Caso o frame do ícone mude.
        */

        if (
            old_item.icon_frame !=
            new_item.icon_frame
        )
        {
            return true;
        }
    }

    return false;
}

void Overlay::shutdown()
{
    running_ = false;

    if (hwnd_)
    {
        DestroyWindow(
            hwnd_
        );

        hwnd_ = nullptr;
    }
}

LRESULT CALLBACK Overlay::window_proc(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam
)
{
    Overlay* overlay =
        reinterpret_cast<Overlay*>(
            GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )
        );

    if (
        message ==
        WM_NCCREATE
    )
    {
        CREATESTRUCTW* create =
            reinterpret_cast<CREATESTRUCTW*>(
                lparam
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
    }

    if (!overlay)
    {
        return DefWindowProcW(
            hwnd,
            message,
            wparam,
            lparam
        );
    }

    switch (message)
    {
        case WM_ERASEBKGND:
        {
            return 1;
        }

        case WM_PAINT:
        {
            overlay->paint();

            return 0;
        }

        case WM_CLOSE:
        {
            overlay->running_ =
                false;

            DestroyWindow(
                hwnd
            );

            return 0;
        }

        case WM_DESTROY:
        {
            overlay->running_ =
                false;

            PostQuitMessage(
                0
            );

            return 0;
        }

        default:
            break;
    }

    return DefWindowProcW(
        hwnd,
        message,
        wparam,
        lparam
    );
}

void Overlay::paint()
{
    if (!hwnd_)
        return;

    PAINTSTRUCT ps{};

    HDC hdc =
        BeginPaint(
            hwnd_,
            &ps
        );

    if (!hdc)
        return;

    RECT client_rect{};

    GetClientRect(
        hwnd_,
        &client_rect
    );

    Graphics graphics(
        hdc
    );

    graphics.SetSmoothingMode(
        SmoothingModeAntiAlias
    );

    graphics.SetInterpolationMode(
        InterpolationModeHighQualityBicubic
    );

    graphics.SetPixelOffsetMode(
        PixelOffsetModeHighQuality
    );

    /*
        =====================================
        FUNDO PRETO
        =====================================
    */

    SolidBrush background(
        Color(
            255,
            0,
            0,
            0
        )
    );

    graphics.FillRectangle(
        &background,

        0,
        0,

        client_rect.right,
        client_rect.bottom
    );

    draw_inventory(
        graphics
    );

    EndPaint(
        hwnd_,
        &ps
    );
}

void Overlay::draw_inventory(
    Graphics& graphics
)
{
    /*
        IMPORTANTE:

        Aqui NÃO chamamos:

        inventory_->get_items()

        novamente.

        Usamos somente cached_items_.

        Portanto o desenho usa exatamente
        o último estado detectado.
    */

    for (
        const InventoryItem& item :
        cached_items_
    )
    {
        if (item.slot < 0)
            continue;

        /*
            =================================
            POSIÇÃO
            =================================
        */

        int column =
            item.slot % 4;

        int row =
            item.slot / 4;

        int x =
            origin_x_ +
            column *
            (
                slot_width_ +
                slot_gap_
            );

        int y =
            origin_y_ +
            row *
            (
                slot_height_ +
                slot_gap_
            );

        /*
            =================================
            FUNDO DO SLOT
            =================================
        */

        SolidBrush slot_background(
            Color(
                255,
                0,
                0,
                0
            )
        );

        graphics.FillRectangle(
            &slot_background,

            x,
            y,

            slot_width_,
            slot_height_
        );

        /*
            =================================
            BORDA
            =================================
        */

        Pen border(
            Color(
                255,
                255,
                255,
                255
            ),
            1.0f
        );

        graphics.DrawRectangle(
            &border,

            x,
            y,

            slot_width_ - 1,
            slot_height_ - 1
        );

        /*
            =================================
            ÍCONE
            =================================
        */

        if (item.icon_frame >= 0)
        {
            std::wstring icon_path =
                get_icon_path(
                    item.icon_frame
                );

            if (
                !icon_path.empty() &&
                std::filesystem::exists(
                    icon_path
                )
            )
            {
                Image* icon =
                    load_icon(
                        item.icon_frame
                    );

                if (
                    icon &&
                    icon->GetLastStatus() ==
                    Ok
                )
                {
                    UINT icon_width =
                        icon->GetWidth();

                    UINT icon_height =
                        icon->GetHeight();

                    if (
                        icon_width > 0 &&
                        icon_height > 0
                    )
                    {
                        int available_width =
                            slot_width_ -
                            icon_padding_ * 2;

                        int available_height =
                            slot_height_ -
                            icon_padding_ * 2;

                        double scale_x =
                            static_cast<double>(
                                available_width
                            ) /
                            static_cast<double>(
                                icon_width
                            );

                        double scale_y =
                            static_cast<double>(
                                available_height
                            ) /
                            static_cast<double>(
                                icon_height
                            );

                        double scale =
                            std::min(
                                scale_x,
                                scale_y
                            );

                        /*
                            Nunca aumenta o PNG.
                        */

                        scale =
                            std::min(
                                scale,
                                1.0
                            );

                        int draw_width =
                            static_cast<int>(
                                icon_width *
                                scale
                            );

                        int draw_height =
                            static_cast<int>(
                                icon_height *
                                scale
                            );

                        int draw_x =
                            x +
                            (
                                slot_width_ -
                                draw_width
                            ) / 2;

                        int draw_y =
                            y +
                            (
                                slot_height_ -
                                draw_height
                            ) / 2;

                        graphics.DrawImage(
                            icon,

                            draw_x,
                            draw_y,

                            draw_width,
                            draw_height
                        );
                    }
                }

                delete icon;
            }
        }

        /*
            =================================
            QUANTIDADE
            =================================
        */

        if (item.quantity > 0)
        {
            std::wstring quantity_text =
                std::to_wstring(
                    item.quantity
                );

            FontFamily font_family(
                L"Arial"
            );

            Font font(
                &font_family,
                16.0f,
                FontStyleBold,
                UnitPixel
            );

            SolidBrush quantity_brush(
                Color(
                    255,
                    255,
                    255,
                    255
                )
            );

            StringFormat format;

            format.SetAlignment(
                StringAlignmentFar
            );

            format.SetLineAlignment(
                StringAlignmentFar
            );

            RectF quantity_rect(
                static_cast<REAL>(
                    x + 4
                ),

                static_cast<REAL>(
                    y + 4
                ),

                static_cast<REAL>(
                    slot_width_ - 8
                ),

                static_cast<REAL>(
                    slot_height_ - 8
                )
            );

            graphics.DrawString(
                quantity_text.c_str(),

                -1,

                &font,

                quantity_rect,

                &format,

                &quantity_brush
            );
        }
    }
}

std::wstring Overlay::get_icon_path(
    int icon_frame
) const
{
    if (icon_frame < 0)
        return L"";

    /*
        ==========================================
        CAMINHO 1
        ==========================================

        build\Release\assets\icons\
    */

    std::filesystem::path exe_path =
        executable_directory_;

    std::filesystem::path path_near_exe =
        exe_path /
        L"assets" /
        L"icons" /
        (
            std::to_wstring(
                icon_frame
            ) +
            L".png"
        );

    if (
        std::filesystem::exists(
            path_near_exe
        )
    )
    {
        return path_near_exe.wstring();
    }

    /*
        ==========================================
        CAMINHO 2
        ==========================================

        projeto\assets\icons\
    */

    std::filesystem::path project_path =
        exe_path.parent_path().parent_path();

    std::filesystem::path project_icon_path =
        project_path /
        L"assets" /
        L"icons" /
        (
            std::to_wstring(
                icon_frame
            ) +
            L".png"
        );

    if (
        std::filesystem::exists(
            project_icon_path
        )
    )
    {
        return project_icon_path.wstring();
    }

    return L"";
}

Gdiplus::Image* Overlay::load_icon(
    int icon_frame
)
{
    std::wstring path =
        get_icon_path(
            icon_frame
        );

    if (path.empty())
        return nullptr;

    return new Image(
        path.c_str()
    );
}