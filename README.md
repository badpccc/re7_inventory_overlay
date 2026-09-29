# RE7 Inventory Overlay

A Resident Evil 7 inventory overlay project built with **REFramework + Lua + C++**, focused on reading the game's internal inventory structures externally and displaying their contents through a real-time overlay.

The project is currently in the **prototype / reverse-engineering stage**. The game's inventory structure has been identified and successfully accessed at runtime. The current implementation can read the player's inventory externally and display its contents in a standalone overlay window.

---

## Overview

The main goal of this project is to create a real-time inventory overlay for **Resident Evil 7**.

The project uses **REFramework** as a bridge to dynamically obtain the address of `app.MenuManager`. An external C++ application then uses `ReadProcessMemory` to access the game's inventory structures.

The current implementation can access:

* Inventory
* Item slots
* Individual `ItemParam` objects
* Item IDs
* Stack quantities
* Maximum stack sizes
* Slot numbers
* Icon frame IDs
* Item data
* Item behavior references

The inventory is continuously monitored and the overlay updates automatically when the inventory changes.

---

## Requirements

* Resident Evil 7
* [REFramework](https://github.com/praydog/REFramework)
* C++17 compatible compiler
* MSVC
* CMake 3.20+
* Windows

---

## REFramework Setup

Start **Resident Evil 7** with **REFramework** installed.

The `RE7 Inventory Bridge` Lua script runs automatically when REFramework loads.

The bridge automatically searches for the current `app.MenuManager` address and creates:

`re7_menu_manager.txt`

Example output:

```text
RE7 INVENTORY BRIDGE
====================

MenuManager encontrado
Address: userdata: 000001643D4D6FB0

Arquivo: SALVO
Nome: re7_menu_manager.txt

====================
END
```

---

## Building

### Requirements

Make sure you have the following installed:

* **Visual Studio Build Tools** with MSVC
* **CMake 3.20+**
* **Windows SDK**
* **Git**

### Create the Build Directory

Open **PowerShell** in the directory where you cloned or extracted the project.

For example, if you selected:

```text
C:\Projects\re7_inventory_overlay
```

open PowerShell in that directory.

The project can be located anywhere you choose. The exact path will depend on where you cloned or extracted the project.

Once PowerShell is open in the project directory, create the `build` directory:

```powershell
mkdir build
cd build
```

### Configure the Project

Run CMake to configure the project:

```powershell
cmake ..
```

### Build the Project

Build the project in **Release** mode:

```powershell
cmake --build . --config Release
```

After a successful build, the executable will be located inside the project's `build` directory:

```text
re7_inventory_overlay/
└── build/
    └── Release/
        └── RE7InventoryOverlay.exe
```

The exact location of `re7_inventory_overlay` depends on where you selected to clone or extract the project.

---

## Clean Build

If you want to perform a completely clean build, open PowerShell in the project's directory and run:

```powershell
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

mkdir build
cd build

cmake ..
cmake --build . --config Release
```

---

## Running

Before running the executable, make sure:

1. **Resident Evil 7** is running.
2. **REFramework** is loaded.
3. The `RE7 Inventory Bridge` Lua script is installed and loaded by REFramework.
4. `re7_menu_manager.txt` has been generated automatically.

The generated file should be located inside the game's REFramework data directory:

```text
reframework/
└── data/
    └── re7_menu_manager.txt
```

After building the project, return to the project's `build` directory:

```powershell
cd build
```

Then run the executable:

```powershell
.\Release\RE7InventoryOverlay.exe
```

The inventory overlay window should appear.

The C++ application reads the `MenuManager` address generated automatically by the REFramework bridge and connects to the running **Resident Evil 7** process.

The inventory is then read continuously and the overlay updates automatically when the inventory changes.

---

## Credits

### RE7 3D Item Icons

Special thanks to **Ryan155194** for the collaboration and for providing access to the 3D item icons used by this project.

**Original work:**  
[RE7 3D Icons Overhaul](https://www.nexusmods.com/residentevil7/mods/264)

**Creator:**  
[Ryan155194](https://forums.nexusmods.com/profile/194413794-ryan155194/)

The 3D item icon assets are credited to their original creator and are not claimed as original assets of this project.
