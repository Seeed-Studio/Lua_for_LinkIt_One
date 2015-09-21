
#include "vmtimer.h"

#include "lua.h"
#include "lauxlib.h"

#define LUA_TIMER   "timer"
#define TIMER_ID_MAX 10

typedef struct {
    int cb_ref;
    lua_State *L;
} timer_info_t;

timer_info_t g_timer_info[TIMER_ID_MAX] = {0,};

static void __timer_callback(int timer_id)
{
    if (timer_id < TIMER_ID_MAX) {
        timer_info_t *p = g_timer_info + timer_id;
        lua_State *L = p->L;

        lua_rawgeti(L, LUA_REGISTRYINDEX, p->cb_ref);
        lua_call(L, 0, 0);
    }
}

int timer_create(lua_State *L)
{
    int id;
    int ref;
    unsigned interval = luaL_checkinteger(L, 1);

    lua_pushvalue(L, 2);

    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    id = vm_create_timer(interval, __timer_callback);
    if ((unsigned)id < TIMER_ID_MAX) {
        g_timer_info[id].L = L;
        g_timer_info[id].cb_ref = ref;
    }

    lua_pushnumber(L, id);

    return 1;
}

int timer_delete(lua_State *L)
{
    int id = luaL_checkinteger(L, 1);

    vm_delete_timer(id);

    return 0;
}



#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE timer_map[] =
{
    {LSTRKEY("create"), LFUNCVAL(timer_create)},
    {LSTRKEY("delete"), LFUNCVAL(timer_delete)},
    {LNILKEY, LNILVAL}
};


LUALIB_API int luaopen_timer(lua_State *L)
{
    luaL_register(L, "timer", timer_map);
    return 1;
}
