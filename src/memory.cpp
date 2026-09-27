#include "memory.h"

#include <TlHelp32.h>

MemoryReader::MemoryReader()
    : process_(nullptr),
      process_id_(0)
{
}

MemoryReader::~MemoryReader()
{
    detach();
}

bool MemoryReader::attach(
    const std::wstring& process_name
)
{
    detach();

    HANDLE snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS,
        0
    );

    if (snapshot == INVALID_HANDLE_VALUE)
        return false;

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(PROCESSENTRY32W);

    bool found = false;

    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            if (process_name == entry.szExeFile)
            {
                process_id_ = entry.th32ProcessID;
                found = true;
                break;
            }

        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);

    if (!found)
        return false;

    process_ = OpenProcess(
        PROCESS_VM_READ |
        PROCESS_QUERY_INFORMATION,
        FALSE,
        process_id_
    );

    if (!process_)
    {
        process_id_ = 0;
        return false;
    }

    return true;
}

void MemoryReader::detach()
{
    if (process_)
    {
        CloseHandle(process_);
        process_ = nullptr;
    }

    process_id_ = 0;
}

bool MemoryReader::is_attached() const
{
    return process_ != nullptr;
}

DWORD MemoryReader::process_id() const
{
    return process_id_;
}

bool MemoryReader::read(
    uintptr_t address,
    void* buffer,
    size_t size
) const
{
    if (!process_)
        return false;

    SIZE_T bytes_read = 0;

    BOOL result = ReadProcessMemory(
        process_,
        reinterpret_cast<LPCVOID>(address),
        buffer,
        size,
        &bytes_read
    );

    return result &&
           bytes_read == size;
}

uintptr_t MemoryReader::module_base(
    const std::wstring& module_name
) const
{
    if (!process_)
        return 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE |
        TH32CS_SNAPMODULE32,
        process_id_
    );

    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    MODULEENTRY32W module{};
    module.dwSize = sizeof(MODULEENTRY32W);

    uintptr_t result = 0;

    if (Module32FirstW(snapshot, &module))
    {
        do
        {
            if (module_name == module.szModule)
            {
                result =
                    reinterpret_cast<uintptr_t>(
                        module.modBaseAddr
                    );

                break;
            }

        } while (Module32NextW(snapshot, &module));
    }

    CloseHandle(snapshot);

    return result;
}