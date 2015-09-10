
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




lua_State *L = NULL;
vm_call_listener_func g_call_status_callback = NULL;

void vm_thread_change_priority(VM_THREAD_HANDLE thread_handle, VMUINT32 new_priority);
void dotty (lua_State *L);

void __handle_sysevt(VMINT message, VMINT param)
{
    switch (message)
    {
    case VM_MSG_CREATE:
        break;

    case VM_MSG_QUIT:
        break;
    }
}

void __call_listener_func(vm_call_listener_data* data)
{
	if(g_call_status_callback)
	{
		g_call_status_callback(data);
	}
}

VMINT32 __main_thread(VM_THREAD_HANDLE thread_handle, void* user_data)
{
    if (1)
    {
        const char *script = "print('Lua on Linkit\n')";
        int error;
        error = luaL_loadstring(L, script);
        printf("luaL_loadstring(): %d\n", error);
        if (!error) {
            lua_pcall(L,0,0,0);
        }
    }

    dotty(L);

	for (;;)
	{
        // console_puts("hello, linkit\n");


        console_putc(console_getc());
	}
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

    printf("heap size: %d\n", vm_get_total_heap_size());

    L = lua_open();
    lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */

    luaL_openlibs(L);  /* open libraries */

    lua_pushcfunction(L, luaopen_io);
    lua_pushstring(L, "io");
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_table);
    lua_pushstring(L, "table");
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_string);
    lua_pushstring(L, "string");
    lua_call(L, 1, 0);

    lua_gc(L, LUA_GCRESTART, 0);

    if (1)
    {
        char buf[64];
        FILE *fd = fopen("init.lua", "r");
        if (fd != NULL)
        {
            fread(buf, 1, sizeof(buf), fd);
            console_puts(buf);
            fclose(fd);
        } else {
            printf("failed to open file.txt\n");
        }
    }

    luaL_dofile(L, "init.lua");

	vm_reg_sysevt_callback(__handle_sysevt);
	vm_call_reg_listener(__call_listener_func);
	handle = vm_thread_create(__main_thread, NULL, 0);
	vm_thread_change_priority(handle, 245);
    vm_thread_create(blink_thread, NULL, 0);
}
