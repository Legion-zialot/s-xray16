#include "pch.hpp"

#include "Logger.h"

#include "Common/Common.hpp"
#include "xrCore/xrCore.h"

#include <fstream>
#include <ctime>

static int SetLog(lua_State* L)
{
    constexpr const char* fmt_default = "[%x %X]\t";
    static char fmt[64];

    static std::ofstream file;

    if (!file.is_open())
    {
        const char* filename{};
        const char* format{};

        switch (lua_gettop(L))
        {
        case 1:
            filename = luaL_checkstring(L, 1);
            format   = fmt_default;
            break;
        case 2:
            filename = luaL_checkstring(L, 1);
            format   = luaL_checkstring(L, 2);
            break;
        default:
            return luaL_error(L, "SetLog: wrong number of arguments");
        }
        file.open(filename);
        xr_strcpy(fmt, format);

        if (!file.is_open())
        {
            Msg("! [xrLuaFix]: Cannot open log file [%s]", filename);
        }
    }

    SetLogCB({ [](void* /*ctx*/, const char* s)
    {
        const time_t t = time(nullptr);
        const tm* ti = std::localtime(&t);
        char buf[64];
        std::ignore = std::strftime(buf, 64, fmt, ti);

        file << buf << s << std::endl;
        file.flush();
    }, nullptr });

    return 0;
}

static int log123(lua_State* L)
{
    const int n = lua_gettop(L);
    for (int i = 0; i < n; ++i)
    {
        Log(luaL_checkstring(L, i+1));
    }
    return 0;
}


int open_log(lua_State* L)
{
    lua_register(L, "log123", log123);
    lua_register(L, "SetLog", SetLog);
    return 0;
}
