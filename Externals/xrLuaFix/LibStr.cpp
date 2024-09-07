#include "pch.hpp"

#include "LibStr.h"

#include <cctype>

static int str_trim(lua_State* L)
{
    size_t size;
    const char* front = luaL_checklstring(L, 1, &size);
    const char* end = &front[size - 1];
    for (; size && std::isspace(*front); size--, front++) {}
    for (; size && std::isspace(*end); size--, end--) {}
    lua_pushlstring(L, front, (size_t)(end - front) + 1);
    return 1;
}

static int str_trim_l(lua_State* L)
{
    size_t size;
    const char* front = luaL_checklstring(L, 1, &size);
    const char* end = &front[size - 1];
    for (; size && std::isspace(*front); size--, front++) {}
    lua_pushlstring(L, front, (size_t)(end - front) + 1);
    return 1;
}

static int str_trim_r(lua_State* L)
{
    size_t size;
    const char* front = luaL_checklstring(L, 1, &size);
    const char* end = &front[size - 1];
    for (; size && std::isspace(*end); size--, end--) {}
    lua_pushlstring(L, front, (size_t)(end - front) + 1);
    return 1;
}

static int str_trim_w(lua_State* L)
{
    int i = 0;
    const char* s = luaL_checkstring(L, 1);
    while (s[i] == ' ')
        i++;
    const int n = i;
    while (s[i] != ' ' && s[i])
        i++;
    const int d = i - n;
    lua_pushlstring(L, s + n, d);
    return 1;
}

int open_string(lua_State* L)
{
    constexpr luaL_Reg strlib[] =
    {
        { "trim",   str_trim },
        { "trim_l", str_trim_l },
        { "trim_r", str_trim_r },
        { "trim_w", str_trim_w },
        {nullptr, nullptr }
    };
    luaL_openlib(L, LUA_STRLIBNAME, strlib, 0);
    return 0;
}
