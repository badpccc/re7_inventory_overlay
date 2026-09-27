#pragma once

#include "memory.h"

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

struct InventoryItem
{
    int slot = -1;

    std::string id;
    std::string name;

    int quantity = 0;
    int max_stack = 0;

    int icon_frame = -1;
};

class InventoryReader
{
public:
    InventoryReader();
    ~InventoryReader();

    bool connect();
    void disconnect();

    bool is_connected() const;

    std::vector<InventoryItem> get_items();

private:
    bool read_pointer(
        uintptr_t address,
        uintptr_t& value
    );

    bool read_int32(
        uintptr_t address,
        int32_t& value
    );

    bool is_readable(
        uintptr_t address
    ) const;

    std::string read_string(
        uintptr_t string_object
    );

    bool read_item(
        uintptr_t item_param,
        InventoryItem& item
    );

    bool load_menu_manager_address();

private:
    MemoryReader memory_;

    HANDLE process_ = nullptr;

    uintptr_t menu_manager_ = 0;

    bool connected_ = false;

private:
    static constexpr uintptr_t PLAYER_STATUS_OFFSET = 208;
    static constexpr uintptr_t INVENTORY_OFFSET = 304;
    static constexpr uintptr_t ITEM_SLOT_MANAGER_OFFSET = 96;
    static constexpr uintptr_t SLOT_DATA_OFFSET = 40;
    static constexpr uintptr_t ITEM_PARAMS_OFFSET = 56;

    static constexpr uintptr_t LIST_ITEMS_OFFSET = 0x10;
    static constexpr uintptr_t LIST_COUNT_OFFSET = 0x18;
    static constexpr uintptr_t ARRAY_DATA_OFFSET = 0x20;

    static constexpr uintptr_t ITEM_BEHAVIOR_OFFSET = 0x10;
    static constexpr uintptr_t ITEM_DATA_OFFSET = 0x20;
    static constexpr uintptr_t SLOT_NO_OFFSET = 0x30;

    static constexpr uintptr_t ITEM_STACK_NUM_OFFSET = 0x88;

    static constexpr uintptr_t ITEM_DATA_ID_OFFSET = 0x18;
    static constexpr uintptr_t MAX_STACK_OFFSET = 0x50;
    static constexpr uintptr_t UI_SETTING_OFFSET = 0x68;

    static constexpr uintptr_t ICON_FRAME_OFFSET = 0x10;

    static constexpr uintptr_t STRING_LENGTH_OFFSET = 0x10;
    static constexpr uintptr_t STRING_CHARS_OFFSET = 0x14;
};