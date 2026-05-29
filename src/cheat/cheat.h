#pragma once
#include "../types.h"
#include "../memory/memory.h"
#include <cstdint>

namespace cheat
{
    void worker(const Memory &m, std::uintptr_t c_base, SharedState &s);
}
