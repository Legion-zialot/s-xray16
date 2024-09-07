#include "pch.hpp"

#include "xrLuaFix.h"

#include "LibStr.h"
#include "LibMath.h"
#include "LibTab.h"
#include "Logger.h"

#include "lfs.h"

extern "C" int luaopen_lua_pack(lua_State* L);
extern "C" int luaopen_marshal(lua_State* L);

// Override math.random and math.randomseed
// only when RvP() called
static int open_rvp(lua_State* L)
{
    open_math(L);
    return 0;
}

int luaopen_xrluafix(lua_State* L)
{
    lua_register(L, "RvP", open_rvp);

    // Anomaly compatibility
    luaL_register(L, "lua_extensions", nullptr);

    open_string(L);
    open_table(L);
    open_log(L);

    luaopen_lua_pack(L);
    luaopen_marshal(L);
    luaopen_lfs(L);

    return 0;
}
