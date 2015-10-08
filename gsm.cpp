

#include "LGSM.h"

#include "lua.h"
#include "lauxlib.h"


int gsm_call(lua_State *L) {
  const char *phone_number = lua_tostring(L, -1);
  int result = LVoiceCall.voiceCall((char*)phone_number);
  lua_pushnumber(L, result);

  return 1;
}

int gsm_anwser(lua_State *L) {
  lua_pushnumber(L, LVoiceCall.answerCall());

  return 1;
}

int gsm_hang(lua_State *L) {
  lua_pushnumber(L, LVoiceCall.hangCall());

  return 1;
}

int gsm_on_incoming_call(lua_State *L)
{
    int ref;
    lua_pushvalue(L, 1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}


int gsm_text(lua_State *L)
{
    int result;
    const char *phone_number = luaL_checkstring(L, 1);
    const char *message = luaL_checkstring(L, 2);

    LSMS.beginSMS(phone_number);
    LSMS.print(message);
    result = LSMS.endSMS();

    lua_pushnumber(L, result);

    return 1;
}

int gsm_on_new_message(lua_State *L)
{
    int ref;
    lua_pushvalue(L, 1);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_pushnumber(L, 0);

    return 1;
}


static const luaL_Reg gsmlib[] = {
    {"call", gsm_call},
    {"answer", gsm_anwser},
    {"hang", gsm_hang},
    {"on_incoming_call", gsm_on_incoming_call},
    {"text", gsm_text},
    {"on_new_message", gsm_on_new_message},
    {NULL, NULL}
};

LUALIB_API int luaopen_gsm(lua_State *L) {
  luaL_register(L, "gsm", gsmlib);
  return 1;
}
