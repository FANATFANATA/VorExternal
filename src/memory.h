#pragma once

#include <windows.h>
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <type_traits>

struct ModuleInfo
{
    std::uintptr_t base{0};
    std::uint32_t size{0};
};

class Memory
{
public:
    explicit Memory(const std::wstring &process_name) noexcept;
    ~Memory() = default;

    Memory(const Memory &) = delete;
    Memory &operator=(const Memory &) = delete;
    Memory(Memory &&) noexcept = default;
    Memory &operator=(Memory &&) noexcept = default;

    [[nodiscard]] bool attach() noexcept;
    [[nodiscard]] ModuleInfo get_module_info(const std::wstring &module_name) const noexcept;
    [[nodiscard]] std::uintptr_t find_pattern(const std::wstring &module_name, const std::string &pattern) const noexcept;

    template <typename T>
        requires std::is_trivially_copyable_v<T>
    [[nodiscard]] T read(std::uintptr_t address) const noexcept
    {
        T value{};
        if (address == 0)
        {
            return value;
        }
        ReadProcessMemory(process_handle_.get(), reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
        return value;
    }

    template <typename T>
        requires std::is_trivially_copyable_v<T>
    bool write(std::uintptr_t address, const T &value) const noexcept
    {
        if (address == 0)
        {
            return false;
        }
        return WriteProcessMemory(process_handle_.get(), reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr) != 0;
    }

    bool read_string(std::uintptr_t address, char *buffer, std::size_t size) const noexcept;

private:
    struct HandleDeleter
    {
        using pointer = HANDLE;
        void operator()(HANDLE handle) const noexcept
        {
            if (handle && handle != INVALID_HANDLE_VALUE)
            {
                CloseHandle(handle);
            }
        }
    };
    using UniqueHandle = std::unique_ptr<HANDLE, HandleDeleter>;

    std::wstring target_process_name_;
    std::uint32_t process_id_{0};
    UniqueHandle process_handle_;

    [[nodiscard]] std::uint32_t find_process_id() const noexcept;
};
