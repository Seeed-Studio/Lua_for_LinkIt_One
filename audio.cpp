

#include "LAudio.h"

#include "lua.h"
#include "lauxlib.h"


int audio_play(lua_State *L)
{
    char *name = (char *)luaL_checkstring(L, 1);
    int sd = lua_toboolean(L, 2);

    if (sd) {
        LAudio.playFile(storageSD, name);
    } else {
        LAudio.playFile(storageFlash, name);
    }

    return 0;
}

int audio_stop(lua_State *L)
{
    LAudio.stop();

    return 0;
}

int audio_pause(lua_State *L)
{
    LAudio.pause();

    return 0;
}

int audio_resume(lua_State *L)
{
    LAudio.resume();
    return 0;
}

int audio_get_status(lua_State *L)
{
    lua_pushnumber(L, LAudio.getStatus());
    return 1;
}


int audio_set_volume(lua_State *L)
{
    int volume = luaL_checkinteger(L, 1);

	LAudio.setVolume(volume);

	return 0;
}

int audio_get_volume(lua_State *L)
{
    lua_pushnumber(L, LAudio.getVolume());

	return 1;
}

static const luaL_Reg audiolib[] =
{
    {"play", audio_play},
    {"stop", audio_stop},
    {"pause", audio_pause},
    {"resume", audio_resume},
    {"get_status", audio_get_status},
    {"set_volume", audio_set_volume},
    {"get_volume", audio_get_volume},
    {NULL, NULL}
};

LUALIB_API int luaopen_audio(lua_State *L)
{
    luaL_register(L, "audio", audiolib);
    return 1;
}
