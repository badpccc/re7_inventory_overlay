local log_text = ""

local output_path =
    "re7_menu_manager.txt"


local menu_manager_found = false

local retry_timer = 0

local retry_interval = 1.0


local function ptr_text(obj)

    if obj == nil then
        return "nil"
    end


    local ok, p =
        pcall(function()
            return sdk.to_ptr(obj)
        end)


    if not ok or p == nil then
        return "invalid"
    end


    return tostring(p)
end


local function save_address(address)

    local file =
        io.open(
            output_path,
            "w"
        )


    if not file then
        return false
    end


    file:write(
        address
    )


    file:close()


    return true
end


local function copy_log()

    pcall(function()

        sdk.copy_to_clipboard(
            log_text
        )

    end)
end


local function run_test()

    local lines = {}


    table.insert(
        lines,
        "RE7 INVENTORY BRIDGE"
    )


    table.insert(
        lines,
        "===================="
    )


    table.insert(
        lines,
        ""
    )


    local menu =
        sdk.get_managed_singleton(
            "app.MenuManager"
        )


    if menu == nil then

        table.insert(
            lines,
            "MenuManager: NAO ENCONTRADO"
        )


        table.insert(
            lines,
            ""
        )


        table.insert(
            lines,
            "Aguardando MenuManager..."
        )


        log_text =
            table.concat(
                lines,
                "\n"
            )


        return false
    end


    local address =
        ptr_text(menu)


    table.insert(
        lines,
        "MenuManager encontrado"
    )


    table.insert(
        lines,
        "Address: " .. address
    )


    table.insert(
        lines,
        ""
    )


    local saved =
        save_address(
            address
        )


    if saved then

        table.insert(
            lines,
            "Arquivo: SALVO"
        )

    else

        table.insert(
            lines,
            "Arquivo: ERRO AO SALVAR"
        )

    end


    table.insert(
        lines,
        "Nome: " .. output_path
    )


    table.insert(
        lines,
        ""
    )


    table.insert(
        lines,
        "===================="
    )


    table.insert(
        lines,
        "END"
    )


    log_text =
        table.concat(
            lines,
            "\n"
        )


    copy_log()


    log.info(
        "[RE7 Bridge] " ..
        log_text
    )


    menu_manager_found = true


    return true
end


re.on_frame(
    function()

        if menu_manager_found then
            return
        end


        retry_timer =
            retry_timer +
            0.016


        if retry_timer <
            retry_interval
        then
            return
        end


        retry_timer = 0


        run_test()

    end
)


re.on_draw_ui(
    function()

        if imgui.tree_node(
            "RE7 Inventory Bridge"
        ) then


            imgui.text(
                "Ponte dinamica do MenuManager."
            )


            imgui.separator()


            if menu_manager_found then

                imgui.text(
                    "Status: MenuManager encontrado."
                )

            else

                imgui.text(
                    "Status: procurando MenuManager..."
                )

            end


            imgui.separator()


            if imgui.button(
                "RUN TEST"
            ) then

                menu_manager_found =
                    run_test()

            end


            imgui.same_line()


            if imgui.button(
                "COPY LOG"
            ) then

                copy_log()

            end


            imgui.separator()


            imgui.text(
                log_text
            )


            imgui.tree_pop()

        end

    end
)


run_test()