#include "cheat.h"
#include "../offsets/offsets.h"
#include <thread>
#include <chrono>

namespace consts
{
constexpr int MAX_PLAYERS = 64;
constexpr std::uint32_t HANDLE_MASK = 0x7FFF;
constexpr std::uint32_t INVALID_HANDLE = 0xFFFFFFFF;
constexpr int ENTITY_LIST_STRIDE = 112;
constexpr int ENTITY_LIST_PAGE_SHIFT = 9;
constexpr int ENTITY_LIST_PAGE_MASK = 0x1FF;
constexpr int MAX_NAME_LENGTH = 128;
constexpr int ENTITY_LIST_POINTER_SIZE = 8;
constexpr int ENTITY_LIST_PAGE_OFFSET = 16;
}

std::uintptr_t get_ent(const Memory &m, std::uintptr_t base, int i)
{
auto list_opt = m.read<std::uintptr_t>(base + game::offsets::dwEntityList);
if (!list_opt)
return 0;
auto entry_opt = m.read<std::uintptr_t>(*list_opt + (consts::ENTITY_LIST_POINTER_SIZE * (i >> consts::ENTITY_LIST_PAGE_SHIFT)) + consts::ENTITY_LIST_PAGE_OFFSET);
if (!entry_opt)
return 0;
auto ent_opt = m.read<std::uintptr_t>(*entry_opt + consts::ENTITY_LIST_STRIDE * (i & consts::ENTITY_LIST_PAGE_MASK));
return ent_opt.value_or(0);
}

std::uintptr_t get_pawn(const Memory &m, std::uintptr_t base, std::uint32_t handle)
{
if (handle == 0 || handle == consts::INVALID_HANDLE)
return 0;
return get_ent(m, base, static_cast<int>(handle & consts::HANDLE_MASK));
}

void cheat::worker(const Memory &m, std::uintptr_t c_base, SharedState &s)
{
std::vector<PlayerData> tmp;
tmp.reserve(consts::MAX_PLAYERS);
while (s.is_running)
{
auto lp_opt = m.read<std::uintptr_t>(c_base + game::offsets::dwLocalPlayerPawn);
auto vm_opt = m.read<ViewMatrix>(c_base + game::offsets::dwViewMatrix);
if (!lp_opt || !vm_opt)
{
std::this_thread::sleep_for(std::chrono::milliseconds(10));
continue;
}
std::uintptr_t lp = *lp_opt;
ViewMatrix vm = *vm_opt;
std::uint8_t local_team = 0;
if (lp)
{
auto lt_opt = m.read<std::uint8_t>(lp + game::fields::m_iTeamNum);
if (lt_opt)
local_team = *lt_opt;
}
tmp.clear();
for (int i = 1; i <= consts::MAX_PLAYERS; ++i)
{
std::uintptr_t ctrl = get_ent(m, c_base, i);
if (!ctrl)
continue;
auto handle_opt = m.read<std::uint32_t>(ctrl + game::fields::m_hPlayerPawn);
if (!handle_opt)
continue;
std::uintptr_t p = get_pawn(m, c_base, *handle_opt);
if (!p || p == lp)
continue;
auto alive_opt = m.read<bool>(ctrl + game::fields::m_bPawnIsAlive);
if (!alive_opt || !*alive_opt)
continue;
PlayerData d;
auto health_opt = m.read<int>(p + game::fields::m_iHealth);
auto team_opt = m.read<std::uint8_t>(p + game::fields::m_iTeamNum);
auto pos_opt = m.read<Vector3>(p + game::fields::m_vOldOrigin);
if (!health_opt || !team_opt || !pos_opt)
continue;
d.health = *health_opt;
d.team = *team_opt;
d.position = *pos_opt;
d.is_alive = true;
char buf[consts::MAX_NAME_LENGTH]{};
if (m.read_string(ctrl + game::fields::m_iszPlayerName, buf, consts::MAX_NAME_LENGTH))
{
d.name = buf;
}
tmp.push_back(std::move(d));
}
{
std::lock_guard<std::mutex> lock(s.write_mutex);
int w_idx = s.write_index.load();
s.players_buffer[w_idx] = std::move(tmp);
s.vm_buffer[w_idx] = vm;
s.local_team_buffer[w_idx] = local_team;
s.read_index.store(w_idx);
s.write_index.store(w_idx == 0 ? 1 : 0);
}
std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
}
