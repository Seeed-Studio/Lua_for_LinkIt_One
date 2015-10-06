
#include "Arduino.h"
#include "wiring_digital.h"

#include "lua.h"
#include "lauxlib.h"


int gpio_mode(lua_State *L)
{
    int pin = luaL_checkinteger(L, 1);
    int mode = luaL_checkinteger(L, 2);

    pinMode(pin, mode);

    return 0;
}

int gpio_read(lua_State *L)
{
    int pin = luaL_checkinteger(L, 1);

    lua_pushnumber(L, digitalRead(pin));

    return 1;
}

int gpio_write(lua_State *L)
{
    int pin = luaL_checkinteger(L, 1);
    int value = luaL_checkinteger(L, 2);

    digitalWrite(pin, value);

    return 0;
}


#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

#define MOD_REG_NUMBER(L, name, value)      lua_pushnumber(L, value); \
                                            lua_setfield(L, -2, name);

const LUA_REG_TYPE gpio_map[] =
{
    {LSTRKEY("mode"), LFUNCVAL(gpio_mode)},
    {LSTRKEY("read"), LFUNCVAL(gpio_read)},
    {LSTRKEY("write"), LFUNCVAL(gpio_write)},
#if LUA_OPTIMIZE_MEMORY > 0
  { LSTRKEY( "OUTPUT" ), LNUMVAL( OUTPUT ) },
  { LSTRKEY( "INPUT" ), LNUMVAL( INPUT ) },
  { LSTRKEY( "HIGH" ), LNUMVAL( HIGH ) },
  { LSTRKEY( "LOW" ), LNUMVAL( LOW ) },
  { LSTRKEY( "INPUT_PULLUP" ), LNUMVAL( INPUT_PULLUP ) },
#endif
    {LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_gpio(lua_State *L)
{
    lua_register(L, "pinMode", gpio_mode);
    lua_register(L, "digitalRead", gpio_read);
    lua_register(L, "digitalWrite", gpio_write);

#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0

  luaL_register(L, "gpio", gpio_map);
  // Add constants
  MOD_REG_NUMBER( L, "OUTPUT", OUTPUT );
  MOD_REG_NUMBER( L, "INPUT", INPUT );
  MOD_REG_NUMBER( L, "HIGH", HIGH );
  MOD_REG_NUMBER( L, "LOW", LOW );
  MOD_REG_NUMBER( L, "INPUT_PULLUP", INPUT_PULLUP );
  return 1;
#endif // #if LUA_OPTIMIZE_MEMORY > 0
}
