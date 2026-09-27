#include "inventory.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>
#include <vector>


static bool read_menu_manager_file(
    const std::string& file_path,
    uintptr_t& address
)
{
    std::ifstream file(
        file_path
    );

    if (!file.is_open())
        return false;

    std::string line;

    if (!std::getline(
        file,
        line
    ))
    {
        return false;
    }

    const std::string prefix =
        "userdata:";

    const size_t prefix_position =
        line.find(prefix);

    if (
        prefix_position ==
        std::string::npos
    )
    {
        return false;
    }

    std::string address_text =
        line.substr(
            prefix_position +
            prefix.length()
        );

    while (
        !address_text.empty() &&
        std::isspace(
            static_cast<unsigned char>(
                address_text.front()
            )
        )
    )
    {
        address_text.erase(
            address_text.begin()
        );
    }

    while (
        !address_text.empty() &&
        std::isspace(
            static_cast<unsigned char>(
                address_text.back()
            )
        )
    )
    {
        address_text.pop_back();
    }

    if (address_text.empty())
        return false;

    try
    {
        address =
            static_cast<uintptr_t>(
                std::stoull(
                    address_text,
                    nullptr,
                    16
                )
            );
    }
    catch (...)
    {
        address = 0;
        return false;
    }

    return address != 0;
}


InventoryReader::InventoryReader()
{
}


InventoryReader::~InventoryReader()
{
    disconnect();
}


bool InventoryReader::load_menu_manager_address()
{
    menu_manager_ = 0;

    std::vector<std::string> paths;

    /*
        1. Diretório atual.
    */

    paths.push_back(
        "re7_menu_manager.txt"
    );


    /*
        2. Diretório do executável do
           nosso programa.
    */

    char module_path[MAX_PATH]{};

    DWORD module_length =
        GetModuleFileNameA(
            nullptr,
            module_path,
            MAX_PATH
        );

    if (
        module_length > 0 &&
        module_length < MAX_PATH
    )
    {
        std::string exe_path(
            module_path,
            module_length
        );

        const size_t separator =
            exe_path.find_last_of(
                "\\/"
            );

        if (
            separator !=
            std::string::npos
        )
        {
            const std::string exe_directory =
                exe_path.substr(
                    0,
                    separator
                );

            paths.push_back(
                exe_directory +
                "\\re7_menu_manager.txt"
            );
        }
    }


    /*
        3. Diretório do RE7:

           re7.exe
             |
             +-- reframework
                   |
                   +-- data
                         |
                         +-- re7_menu_manager.txt
    */

    if (process_)
    {
        char game_path[MAX_PATH]{};

        DWORD game_path_length =
            MAX_PATH;

        if (
            QueryFullProcessImageNameA(
                process_,
                0,
                game_path,
                &game_path_length
            )
        )
        {
            std::string full_game_path(
                game_path,
                game_path_length
            );

            const size_t separator =
                full_game_path.find_last_of(
                    "\\/"
                );

            if (
                separator !=
                std::string::npos
            )
            {
                const std::string game_directory =
                    full_game_path.substr(
                        0,
                        separator
                    );

                paths.push_back(
                    game_directory +
                    "\\reframework\\data\\re7_menu_manager.txt"
                );
            }
        }
    }


    /*
        4. Diretório atual do processo.
    */

    char current_directory[MAX_PATH]{};

    DWORD current_length =
        GetCurrentDirectoryA(
            MAX_PATH,
            current_directory
        );

    if (
        current_length > 0 &&
        current_length < MAX_PATH
    )
    {
        paths.push_back(
            std::string(
                current_directory
            ) +
            "\\re7_menu_manager.txt"
        );
    }


    /*
        Tenta todas as localizações.
    */

    for (
        const std::string& path :
        paths
    )
    {
        uintptr_t address = 0;

        if (
            read_menu_manager_file(
                path,
                address
            )
        )
        {
            menu_manager_ =
                address;

            return true;
        }
    }

    return false;
}


bool InventoryReader::connect()
{
    disconnect();

    /*
        Conecta ao RE7.
    */

    if (
        !memory_.attach(
            L"re7.exe"
        )
    )
    {
        return false;
    }


    /*
        Abre o processo para podermos
        descobrir o caminho do re7.exe.
    */

    process_ =
        OpenProcess(
            PROCESS_QUERY_INFORMATION,
            FALSE,
            memory_.process_id()
        );

    if (!process_)
    {
        memory_.detach();

        return false;
    }


    /*
        Carrega o endereço dinâmico
        do MenuManager.
    */

    if (
        !load_menu_manager_address()
    )
    {
        CloseHandle(
            process_
        );

        process_ = nullptr;

        memory_.detach();

        return false;
    }


    connected_ = true;

    return true;
}


void InventoryReader::disconnect()
{
    connected_ = false;

    menu_manager_ = 0;

    if (process_)
    {
        CloseHandle(
            process_
        );

        process_ = nullptr;
    }

    memory_.detach();
}


bool InventoryReader::is_connected() const
{
    return connected_;
}


bool InventoryReader::read_pointer(
    uintptr_t address,
    uintptr_t& value
)
{
    value = 0;

    return memory_.read(
        address,
        value
    );
}


bool InventoryReader::read_int32(
    uintptr_t address,
    int32_t& value
)
{
    value = 0;

    return memory_.read(
        address,
        value
    );
}


bool InventoryReader::is_readable(
    uintptr_t address
) const
{
    if (!process_)
        return false;

    MEMORY_BASIC_INFORMATION mbi{};

    const SIZE_T result =
        VirtualQueryEx(
            process_,
            reinterpret_cast<LPCVOID>(
                address
            ),
            &mbi,
            sizeof(mbi)
        );

    if (result == 0)
        return false;

    if (
        mbi.State !=
        MEM_COMMIT
    )
    {
        return false;
    }

    if (
        mbi.Protect ==
        PAGE_NOACCESS ||
        mbi.Protect ==
        PAGE_GUARD
    )
    {
        return false;
    }

    return true;
}


std::string InventoryReader::read_string(
    uintptr_t string_object
)
{
    if (string_object == 0)
        return {};

    if (
        !is_readable(
            string_object
        )
    )
    {
        return {};
    }

    int32_t length = 0;

    if (
        !memory_.read(
            string_object +
            STRING_LENGTH_OFFSET,
            length
        )
    )
    {
        return {};
    }

    if (
        length <= 0 ||
        length > 256
    )
    {
        return {};
    }

    std::vector<wchar_t> characters(
        static_cast<size_t>(
            length
        )
    );

    if (
        !memory_.read(
            string_object +
            STRING_CHARS_OFFSET,
            characters.data(),
            characters.size() *
            sizeof(wchar_t)
        )
    )
    {
        return {};
    }

    std::string result;

    result.reserve(
        static_cast<size_t>(
            length
        )
    );

    for (
        wchar_t character :
        characters
    )
    {
        if (
            character >= 0 &&
            character <= 127
        )
        {
            result.push_back(
                static_cast<char>(
                    character
                )
            );
        }
        else
        {
            result.push_back('?');
        }
    }

    return result;
}


bool InventoryReader::read_item(
    uintptr_t item_param,
    InventoryItem& item
)
{
    if (item_param == 0)
        return false;


    /*
        ItemBehavior
    */

    uintptr_t item_behavior = 0;

    if (
        !read_pointer(
            item_param +
            ITEM_BEHAVIOR_OFFSET,
            item_behavior
        )
    )
    {
        return false;
    }

    if (item_behavior == 0)
        return false;


    /*
        ItemData
    */

    uintptr_t item_data = 0;

    if (
        !read_pointer(
            item_param +
            ITEM_DATA_OFFSET,
            item_data
        )
    )
    {
        return false;
    }

    if (item_data == 0)
        return false;


    /*
        SlotNo
    */

    int32_t slot = -1;

    if (
        !read_int32(
            item_param +
            SLOT_NO_OFFSET,
            slot
        )
    )
    {
        return false;
    }


    /*
        ItemStackNum
    */

    int32_t quantity = 0;

    if (
        !read_int32(
            item_behavior +
            ITEM_STACK_NUM_OFFSET,
            quantity
        )
    )
    {
        return false;
    }


    /*
        MaxStackNum
    */

    int32_t max_stack = 0;

    if (
        !read_int32(
            item_data +
            MAX_STACK_OFFSET,
            max_stack
        )
    )
    {
        return false;
    }


    /*
        ItemDataID
    */

    uintptr_t string_object = 0;

    if (
        !read_pointer(
            item_data +
            ITEM_DATA_ID_OFFSET,
            string_object
        )
    )
    {
        return false;
    }

    const std::string id =
        read_string(
            string_object
        );

    if (id.empty())
        return false;


    /*
        UISetting
    */

    uintptr_t ui_setting = 0;

    if (
        !read_pointer(
            item_data +
            UI_SETTING_OFFSET,
            ui_setting
        )
    )
    {
        return false;
    }


    /*
        IconFrameNo
    */

    int32_t icon_frame = -1;

    if (ui_setting != 0)
    {
        read_int32(
            ui_setting +
            ICON_FRAME_OFFSET,
            icon_frame
        );
    }


    /*
        Monta o item.
    */

    item.slot =
        static_cast<int>(
            slot
        );

    item.id =
        id;

    item.name =
        id;

    item.quantity =
        static_cast<int>(
            quantity
        );

    item.max_stack =
        static_cast<int>(
            max_stack
        );

    item.icon_frame =
        static_cast<int>(
            icon_frame
        );


    return true;
}


std::vector<InventoryItem>
InventoryReader::get_items()
{
    std::vector<InventoryItem> result;

    if (!connected_)
        return result;

    if (menu_manager_ == 0)
        return result;


    /*
        MenuManager
        +208
        PlayerStatus
    */

    uintptr_t player_status = 0;

    if (
        !read_pointer(
            menu_manager_ +
            PLAYER_STATUS_OFFSET,
            player_status
        )
    )
    {
        return result;
    }


    /*
        PlayerStatus
        +304
        Inventory
    */

    uintptr_t inventory = 0;

    if (
        !read_pointer(
            player_status +
            INVENTORY_OFFSET,
            inventory
        )
    )
    {
        return result;
    }


    /*
        Inventory
        +96
        ItemSlotManager
    */

    uintptr_t item_slot_manager = 0;

    if (
        !read_pointer(
            inventory +
            ITEM_SLOT_MANAGER_OFFSET,
            item_slot_manager
        )
    )
    {
        return result;
    }


    /*
        ItemSlotManager
        +40
        SlotData
    */

    uintptr_t slot_data = 0;

    if (
        !read_pointer(
            item_slot_manager +
            SLOT_DATA_OFFSET,
            slot_data
        )
    )
    {
        return result;
    }


    /*
        SlotData
        +56
        ItemParams
    */

    uintptr_t item_params = 0;

    if (
        !read_pointer(
            slot_data +
            ITEM_PARAMS_OFFSET,
            item_params
        )
    )
    {
        return result;
    }

    if (item_params == 0)
        return result;


    /*
        List._items
    */

    uintptr_t items_array = 0;

    if (
        !read_pointer(
            item_params +
            LIST_ITEMS_OFFSET,
            items_array
        )
    )
    {
        return result;
    }


    /*
        List.Count
    */

    int32_t count = 0;

    if (
        !read_int32(
            item_params +
            LIST_COUNT_OFFSET,
            count
        )
    )
    {
        return result;
    }

    if (
        count <= 0 ||
        count > 256
    )
    {
        return result;
    }


    /*
        Percorre os ItemParam*.
    */

    for (
        int32_t index = 0;
        index < count;
        ++index
    )
    {
        uintptr_t item_param = 0;


        const uintptr_t element_address =
            items_array +
            ARRAY_DATA_OFFSET +
            (
                static_cast<uintptr_t>(
                    index
                ) * 8
            );


        if (
            !read_pointer(
                element_address,
                item_param
            )
        )
        {
            continue;
        }


        if (item_param == 0)
            continue;


        InventoryItem item;


        if (
            read_item(
                item_param,
                item
            )
        )
        {
            result.push_back(
                item
            );
        }
    }


    /*
        Ordena pelo número do slot.
    */

    std::sort(
        result.begin(),
        result.end(),
        [](const InventoryItem& a,
           const InventoryItem& b)
        {
            return a.slot < b.slot;
        }
    );


    return result;
}