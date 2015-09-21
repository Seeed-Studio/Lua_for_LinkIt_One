
#include <stdio.h>
#include <string.h>
#include "vmsys.h"
#include "vmthread.h"
#include "vmpromng.h"
#include "vmlog.h"
#include "vmtel.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"

#include "console.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "shell.h"


lua_State *L = NULL;

extern int luaopen_audio(lua_State *L);
extern int luaopen_gsm(lua_State *L);

static int msleep_c(lua_State *L)
{
    long ms = lua_tointeger(L, -1);
    vm_thread_sleep(ms);
    return 0;
}

void setup_lua()
{
    VM_THREAD_HANDLE handle;

    L = lua_open();
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
    luaL_openlibs(L);  /* open libraries */

    luaopen_audio(L);
    luaopen_gsm(L);

    lua_register(L, "msleep", msleep_c);

    lua_gc(L, LUA_GCRESTART, 0);

    luaL_dofile(L, "init.lua");

    handle = vm_thread_create(shell_thread, L, 0);
	vm_thread_change_priority(handle, 245);

}


VMUINT32 blink_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
    VM_DCL_HANDLE gpio_handle;

    gpio_handle = vm_dcl_open(VM_DCL_GPIO, 27);
    vm_dcl_control(gpio_handle,VM_GPIO_CMD_SET_MODE_0,NULL);
    vm_dcl_control(gpio_handle,VM_GPIO_CMD_SET_DIR_OUT, NULL);

    while (1) {
        vm_dcl_control(gpio_handle,VM_GPIO_CMD_WRITE_HIGH, NULL);
        vm_thread_sleep(1000);
        vm_dcl_control(gpio_handle,VM_GPIO_CMD_WRITE_LOW, NULL);
        vm_thread_sleep(1000);
    }
}

void __handle_sysevt(VMINT message, VMINT param)
{
    switch (message)
    {
    case VM_MSG_CREATE:
        setup_lua();
        break;

    case SHELL_MESSAGE_ID:
        shell_docall(L);
        break;

    case VM_MSG_QUIT:
        break;
    }
}

/*
 * \brief Main entry point of the application
 */
void vm_main( void )
{
    VM_THREAD_HANDLE handle;
    int i;

    console_init();
    console_puts("wait 10 seconds\n");
    for (i = 0; i < 10; i++) {
        vm_thread_sleep(1000);
        console_putc('.');
    }
    console_puts("system is running\n");

	vm_reg_sysevt_callback(__handle_sysevt);


    vm_thread_create(blink_thread, NULL, 0);
}
