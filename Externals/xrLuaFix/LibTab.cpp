#include "pch.hpp"

#include "LibTab.h"
#include "LibMath.h"

static int tab_keys(lua_State* L)
{
    int i = 1;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_newtable(L);
    lua_pushnil(L);
    while (lua_next(L, 1) != 0)
    {
        lua_pushinteger(L, i);
        ++i;
        lua_pushvalue(L, -3);
        lua_settable(L, 2);
        lua_pop(L, 1);
    }
    return 1;
}

static int tab_values(lua_State* L)
{
    int i = 1;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_newtable(L);
    lua_pushnil(L);
    while (lua_next(L, 1) != 0)
    {
        lua_pushinteger(L, i);
        ++i;
        lua_pushvalue(L, -2);
        lua_settable(L, 2);
        lua_pop(L, 1);
    }
    return 1;
}

static int get_size(lua_State* L)
{
    int i = 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 2);
    while (lua_next(L, 1))
    {
        ++i;
        lua_pop(L, 1);
    }
    lua_pushinteger(L, i);
    return 1;
}

static int C_get_size(lua_State* L)
{
    int i = 0;
    lua_settop(L, 2);
    while (lua_next(L, 1))
    {
        ++i;
        lua_pop(L, 1);
    }
    return i;
}

static int get_random(lua_State* L)
{
    int i = C_get_size(L);
    const int j = gen_random_in_range(1, i);
    i = 0;
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 2);
    while (lua_next(L, 1))
    {
        ++i;
        if (i == j)
        {
            lua_pushvalue(L, -2);
            lua_pushvalue(L, -2);
            return 2;
        }
        lua_pop(L, 1);
    }
    return 0;
}

int open_table(lua_State* L)
{
    constexpr luaL_Reg tab_funcs[] =
    {
        { "keys",   tab_keys },
        { "values", tab_values },
        { "size",   get_size },
        { "random", get_random },
        { nullptr,  nullptr }
    };

    luaL_openlib(L, LUA_TABLIBNAME, tab_funcs, 0);
    return 0;
}
