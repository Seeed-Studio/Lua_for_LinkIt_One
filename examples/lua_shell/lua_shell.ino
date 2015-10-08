
#include "LAudio.h"
#include "LGSM.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


lua_State *L = NULL;

extern int luaopen_gpio(lua_State *L);
extern int luaopen_audio(lua_State *L);
extern int luaopen_gsm(lua_State *L);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(13, OUTPUT);

  for (int i = 0; i < 3; i++) {
    digitalWrite(13, HIGH);
    printf("hello, printf\n");
    Serial.print("hello, lua\n");
    delay(1000);
    digitalWrite(13, LOW);
    delay(1000);
  }

  L = lua_open();
  lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
  luaL_openlibs(L);  /* open libraries */

  luaopen_gpio(L);
  luaopen_audio(L);
  luaopen_gsm(L);

  lua_gc(L, LUA_GCRESTART, 0);

  luaL_dofile(L, "init.lua");
}

void loop() {
  dotty(L);
}
