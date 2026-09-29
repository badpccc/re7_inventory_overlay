#include "inventory.h"
#include "overlay.h"

#include <windows.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")


int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE,
    LPSTR,
    int
)
{
    Gdiplus::GdiplusStartupInput gdiplus_input{};

    ULONG_PTR gdiplus_token = 0;

    if (
        Gdiplus::GdiplusStartup(
            &gdiplus_token,
            &gdiplus_input,
            nullptr
        ) != Gdiplus::Ok
    )
    {
        MessageBoxA(
            nullptr,
            "Nao foi possivel inicializar o GDI+.",
            "RE7 Inventory",
            MB_OK |
            MB_ICONERROR
        );

        return 1;
    }


    InventoryReader inventory;


    if (!inventory.connect())
    {
        const char* message =
            "Nao foi possivel conectar ao RE7.\n\n"
            "Verifique se:\n"
            "- RE7 esta aberto\n"
            "- REFramework esta aberto\n"
            "- re7_menu_manager.txt existe";


        MessageBoxA(
            nullptr,
            message,
            "RE7 Inventory",
            MB_OK |
            MB_ICONERROR
        );


        Gdiplus::GdiplusShutdown(
            gdiplus_token
        );


        return 1;
    }


    Overlay overlay;


    if (
        !overlay.initialize(
            instance,
            &inventory
        )
    )
    {
        MessageBoxA(
            nullptr,
            "Nao foi possivel criar a janela do overlay.",
            "RE7 Inventory",
            MB_OK |
            MB_ICONERROR
        );


        Gdiplus::GdiplusShutdown(
            gdiplus_token
        );


        return 1;
    }


    overlay.run();


    overlay.shutdown();


    Gdiplus::GdiplusShutdown(
        gdiplus_token
    );


    return 0;
}