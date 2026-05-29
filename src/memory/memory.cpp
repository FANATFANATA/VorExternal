#include "memory.h"
#include <tlhelp32.h>

Memory::Memory(const std::wstring &process_name) noexcept
    : target_process_name_(process_name) {}

bool Memory::attach() noexcept
{
    process_id_ = find_process_id();
    if (process_id_ == 0)
        return false;

    constexpr std::uint32_t access_rights = PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION;
    HANDLE raw_handle = OpenProcess(access_rights, FALSE, process_id_);
    if (!raw_handle || raw_handle == INVALID_HANDLE_VALUE)
        return false;

    process_handle_.reset(raw_handle);
    return true;
}

std::uint32_t Memory::find_process_id() const noexcept
{
    std::uint32_t pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32W process_entry{};
    process_entry.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(snapshot, &process_entry))
    {
        do
        {
            if (target_process_name_ == process_entry.szExeFile)
            {
                pid = process_entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &process_entry));
    }
    CloseHandle(snapshot);
    return pid;
}

ModuleInfo Memory::get_module_info(const std::wstring &module_name) const noexcept
{
    ModuleInfo info{};
    if (process_id_ == 0)
        return info;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id_);
    if (snapshot == INVALID_HANDLE_VALUE)
        return info;

    MODULEENTRY32W module_entry{};
    module_entry.dwSize = sizeof(MODULEENTRY32W);

    if (Module32FirstW(snapshot, &module_entry))
    {
        do
        {
            if (module_name == module_entry.szModule)
            {
                info.base = reinterpret_cast<std::uintptr_t>(module_entry.modBaseAddr);
                info.size = module_entry.modBaseSize;
                break;
            }
        } while (Module32NextW(snapshot, &module_entry));
    }
    CloseHandle(snapshot);
    return info;
}

bool Memory::read_string(std::uintptr_t address, char *buffer, std::size_t size) const noexcept
{
    if (address == 0 || buffer == nullptr || size == 0)
        return false;

    const bool success = ReadProcessMemory(process_handle_.get(), reinterpret_cast<LPCVOID>(address), buffer, size, nullptr) != 0;
    if (success)
        buffer[size - 1] = '\0';

    return success;
}
