#include "memory.h"
#include <tlhelp32.h>
#include <sstream>

Memory::Memory(const std::wstring &process_name) noexcept
    : target_process_name_(process_name) {}

bool Memory::attach() noexcept
{
    process_id_ = find_process_id();
    if (process_id_ == 0)
    {
        return false;
    }

    constexpr std::uint32_t access_rights = PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION;
    HANDLE raw_handle = OpenProcess(access_rights, FALSE, process_id_);
    if (!raw_handle || raw_handle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    process_handle_.reset(raw_handle);
    return true;
}

std::uint32_t Memory::find_process_id() const noexcept
{
    std::uint32_t pid = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

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
    {
        return info;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id_);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return info;
    }

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
    {
        return false;
    }
    const bool success = ReadProcessMemory(process_handle_.get(), reinterpret_cast<LPCVOID>(address), buffer, size, nullptr) != 0;
    if (success)
    {
        buffer[size - 1] = '\0';
    }
    return success;
}

std::uintptr_t Memory::find_pattern(const std::wstring &module_name, const std::string &pattern) const noexcept
{
    if (process_id_ == 0)
    {
        return 0;
    }

    const ModuleInfo mod = get_module_info(module_name);
    if (mod.base == 0 || mod.size == 0)
    {
        return 0;
    }

    std::vector<std::uint8_t> module_bytes(mod.size);
    if (!ReadProcessMemory(process_handle_.get(), reinterpret_cast<LPCVOID>(mod.base), module_bytes.data(), mod.size, nullptr))
    {
        return 0;
    }

    std::vector<int> pattern_bytes;
    std::stringstream ss(pattern);
    std::string temp;

    while (ss >> temp)
    {
        if (temp == "?" || temp == "??")
        {
            pattern_bytes.push_back(-1);
        }
        else
        {
            pattern_bytes.push_back(std::stoi(temp, nullptr, 16));
        }
    }

    const std::size_t pattern_length = pattern_bytes.size();
    if (mod.size < pattern_length)
    {
        return 0;
    }

    const std::size_t search_limit = mod.size - pattern_length;
    for (std::size_t i = 0; i <= search_limit; ++i)
    {
        bool match = true;
        for (std::size_t j = 0; j < pattern_length; ++j)
        {
            if (pattern_bytes[j] != -1 && module_bytes[i + j] != pattern_bytes[j])
            {
                match = false;
                break;
            }
        }
        if (match)
        {
            return mod.base + i;
        }
    }

    return 0;
}
