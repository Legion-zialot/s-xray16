#include "pch.hpp"

#include "LibMath.h"

#include <random>

static std::random_device ndrng;
static std::mt19937 intgen;
static std::uniform_real_distribution<float> float_random;

int gen_random_in_range(int a1, int a2)
{
    //unsigned?
    std::uniform_int_distribution dist(a1, a2);
    return dist(intgen);
}

static int math_randomseed(lua_State* L)
{
    switch (lua_gettop(L))
    {
    case 0:
    {
        intgen.seed(ndrng());
        break;
    }
    case 1:
    {
        const int seed_value = luaL_checkint(L, 1);
        intgen.seed(seed_value);
        break;
    }
    default: return luaL_error(L, "math_randomseed: wrong number of arguments");
    }
    return 0;
}

static int math_random(lua_State* L)
{
    switch (lua_gettop(L))
    {
    case 0:
    {
        lua_pushnumber(L, (lua_Number)float_random(intgen));
        break;
    }
    case 1:
    {
        const int u = luaL_checkint(L, 1);
        luaL_argcheck(L, 1<=u, 1, "interval is empty");
        lua_pushinteger(L, gen_random_in_range(1, u));
        break;
    }
    case 2:
    {
        const int l = luaL_checkint(L, 1);
        const int u = luaL_checkint(L, 2);
        luaL_argcheck(L, l<=u, 2, "interval is empty");
        lua_pushinteger(L, gen_random_in_range(l, u));
        break;
    }
    default:
        return luaL_error(L, "wrong number of arguments");
    }
    return 1;
}

int open_math(lua_State* L)
{
    constexpr luaL_Reg mathlib[] =
    {
        { "random",     math_random },
        { "randomseed", math_randomseed },
        { nullptr,      nullptr }
    };
    luaL_openlib(L, LUA_MATHLIBNAME, mathlib, 0);
    return 0;
}
