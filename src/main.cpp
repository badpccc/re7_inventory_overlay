#include "inventory.h"
#include "overlay.h"

#include <windows.h>


int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE,
    LPSTR,
    int
)
{
    InventoryReader inventory;


    if (!inventory.connect())
    {
        const char* message =
            "Nao foi possivel conectar ao RE7.\n\n"
            "Verifique se:\n"
            "- RE7 esta aberto\n"
            "- REFramework esta aberto\n"
            "- RUN TEST foi executado\n"
            "- re7_menu_manager.txt existe";


        MessageBoxA(
            nullptr,
            message,
            "RE7 Inventory",
            MB_OK |
            MB_ICONERROR
        );


        return 1;
    }


    Overlay overlay;


    if (!overlay.initialize(
        instance,
        &inventory
    ))
    {
        MessageBoxA(
            nullptr,
            "Nao foi possivel criar a janela do overlay.",
            "RE7 Inventory",
            MB_OK |
            MB_ICONERROR
        );


        return 1;
    }


    overlay.run();


    overlay.shutdown();


    return 0;
}