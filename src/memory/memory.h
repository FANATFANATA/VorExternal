#pragma once
#include <windows.h>
#include <string>
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

class UniqueHandle
{
    HANDLE handle_{INVALID_HANDLE_VALUE};

public:
    UniqueHandle() noexcept = default;
    explicit UniqueHandle(HANDLE h) noexcept : handle_(h) {}

    ~UniqueHandle() noexcept
    {
        if (handle_ && handle_ != INVALID_HANDLE_VALUE)
            CloseHandle(handle_);
    }

    UniqueHandle(const UniqueHandle &) = delete;
    UniqueHandle &operator=(const UniqueHandle &) = delete;

    UniqueHandle(UniqueHandle &&other) noexcept : handle_(other.handle_)
    {
        other.handle_ = INVALID_HANDLE_VALUE;
    }

    UniqueHandle &operator=(UniqueHandle &&other) noexcept
    {
        if (this != &other)
        {
            if (handle_ && handle_ != INVALID_HANDLE_VALUE)
                CloseHandle(handle_);
            handle_ = other.handle_;
            other.handle_ = INVALID_HANDLE_VALUE;
        }
        return *this;
    }

    [[nodiscard]] HANDLE get() const noexcept { return handle_; }

    void reset(HANDLE h = INVALID_HANDLE_VALUE) noexcept
    {
        if (handle_ && handle_ != INVALID_HANDLE_VALUE)
            CloseHandle(handle_);
        handle_ = h;
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
        return handle_ && handle_ != INVALID_HANDLE_VALUE;
    }
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

    [[nodiscard]] bool read_string(std::uintptr_t address, char *buffer, std::size_t size) const noexcept;

private:
    std::wstring target_process_name_;
    std::uint32_t process_id_{0};
    UniqueHandle process_handle_;

    [[nodiscard]] std::uint32_t find_process_id() const noexcept;
};
