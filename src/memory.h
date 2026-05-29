#pragma once
#include <windows.h>
#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <optional>

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
    [[nodiscard]] std::optional<T> read(std::uintptr_t address) const noexcept
    {
        if (address == 0)
            return std::nullopt;

        T value{};
        SIZE_T bytes_read = 0;
        if (ReadProcessMemory(process_handle_.get(), reinterpret_cast<LPCVOID>(address), &value, sizeof(T), &bytes_read) && bytes_read == sizeof(T))
        {
            return value;
        }
        return std::nullopt;
    }

    template <typename T>
        requires std::is_trivially_copyable_v<T>
    bool write(std::uintptr_t address, const T &value) const noexcept
    {
        if (address == 0)
            return false;

        SIZE_T bytes_written = 0;
        return WriteProcessMemory(process_handle_.get(), reinterpret_cast<LPVOID>(address), &value, sizeof(T), &bytes_written) && bytes_written == sizeof(T);
    }

    bool read_string(std::uintptr_t address, char *buffer, std::size_t size) const noexcept;

private:
    struct HandleDeleter
    {
        using pointer = HANDLE;
        void operator()(HANDLE handle) const noexcept
        {
            if (handle && handle != INVALID_HANDLE_VALUE)
                CloseHandle(handle);
        }
    };

    using UniqueHandle = std::unique_ptr<HANDLE, HandleDeleter>;
    std::wstring target_process_name_;
    std::uint32_t process_id_{0};
    UniqueHandle process_handle_;

    [[nodiscard]] std::uint32_t find_process_id() const noexcept;
};
