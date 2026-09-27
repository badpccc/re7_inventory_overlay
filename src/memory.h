#pragma once

#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <string>

class MemoryReader
{
public:
    MemoryReader();
    ~MemoryReader();

    bool attach(const std::wstring& process_name);
    void detach();

    bool is_attached() const;

    DWORD process_id() const;

    bool read(
        uintptr_t address,
        void* buffer,
        size_t size
    ) const;

    template <typename T>
    bool read(
        uintptr_t address,
        T& value
    ) const
    {
        return read(
            address,
            &value,
            sizeof(T)
        );
    }

    uintptr_t module_base(
        const std::wstring& module_name
    ) const;

private:
    HANDLE process_;
    DWORD process_id_;
};