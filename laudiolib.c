
#include <string.h>

#include "vmsys.h"
#include "vmlog.h"
#include "vmchset.h"
#include "vmmm.h" // "vmaudio_play.h"

#include "lua.h"
#include "lauxlib.h"

#define MAX_NAME_LEN 32  /* Max length of file name */

static VMINT g_audio_handle = -1;  /* The handle of play */
static VMINT g_audio_interrupt_handle = 0; /* The handle of interrupt */

/* The callback function when playing. */
static void audio_play_callback(VMINT result)
{

}

/* Play the audio file. */
static void _audio_play(char *name)
{
  VMINT drv ;
  VMWCHAR w_file_name[MAX_NAME_LEN] = {0};
  VMCHAR file_name[MAX_NAME_LEN];

  drv = vm_get_system_driver();

  sprintf(file_name, (VMSTR)"%c:\\%s", drv, name);
  vm_ascii_to_ucs2(w_file_name, MAX_NAME_LEN, file_name);

  /* start to play */
  vm_audio_play_file(w_file_name, audio_play_callback);

}


int audio_play(lua_State *L)
{
    char *name = luaL_checkstring(L, 1);

    _audio_play(name);

    return 0;
}

int audio_stop(lua_State *L)
{
    vm_audio_stop(audio_play_callback);

    return 0;
}

int audio_pause(lua_State *L)
{
    vm_audio_pause(audio_play_callback);

    return 0;
}

int audio_resume(lua_State *L)
{
    vm_audio_resume(audio_play_callback);
    return 0;
}


int audio_set_volume(lua_State *L)
{
    int volume = luaL_checkinteger(L, 1);

	vm_set_volume(volume);

	return 0;
}

int audio_get_volume(lua_State *L)
{
    int volume = vm_get_volume();

    lua_pushnumber(L, volume);

	return 1;
}

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE audio_map[] =
{
    {LSTRKEY("play"), LFUNCVAL(audio_play)},
    {LSTRKEY("stop"), LFUNCVAL(audio_stop)},
    {LSTRKEY("pause"), LFUNCVAL(audio_pause)},
    {LSTRKEY("resume"), LFUNCVAL(audio_resume)},
    {LSTRKEY("set_volume"), LFUNCVAL(audio_set_volume)},
    {LSTRKEY("get_volume"), LFUNCVAL(audio_get_volume)},
    {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_audio(lua_State *L)
{
    luaL_register(L, "audio", audio_map);
    return 1;
}
