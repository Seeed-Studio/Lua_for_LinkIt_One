
#include <string.h>
#include "vmdcl.h"
#include "vmdcl_sio.h"
#include "vmthread.h"
#include "vmlog.h"

#define SERIAL_BUFFER_SIZE      64

static VM_DCL_HANDLE usb_device_handle = -1;
VM_SIGNAL_ID console_rx_signal_id;
static char console_rx_buffer[SERIAL_BUFFER_SIZE];
unsigned console_rx_buffer_head = 0;
unsigned console_rx_buffer_tail = 0;

void __console_irq_handler(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
    if(event == VM_UART_READY_TO_READ)
    {
        char data[SERIAL_BUFFER_SIZE];
        int i;
        VM_DCL_STATUS status;
        VM_DCL_BUFF_LEN returned_len = 0;

        status = vm_dcl_read(device_handle,(VM_DCL_BUFF*)data,SERIAL_BUFFER_SIZE,&returned_len,vm_dcl_get_ownerid());
        if(status<VM_DCL_STATUS_OK)
        {
            vm_log_info((char*)"read failed");
        }
        else if (returned_len)
        {
            if (console_rx_buffer_head == console_rx_buffer_tail) {
                vm_signal_post(console_rx_signal_id);
            }

            for (i = 0; i < returned_len; i++)
            {
                console_rx_buffer[console_rx_buffer_head % SERIAL_BUFFER_SIZE] = data[i];
                console_rx_buffer_head++;
                if ((unsigned)(console_rx_buffer_head - console_rx_buffer_tail) > SERIAL_BUFFER_SIZE) {
                    console_rx_buffer_tail = console_rx_buffer_head - SERIAL_BUFFER_SIZE;
                }
            }
        }

    }
    else
    {
    }
}

void console_init(void)
{
    VM_DCL_HANDLE uart_handle;
    vm_sio_ctrl_dcb_t data;

    if (usb_device_handle != -1)
    {
        return;
    }
    uart_handle = vm_dcl_open(vm_usb_port1,vm_dcl_get_ownerid());
    data.u4OwenrId = vm_dcl_get_ownerid();
    data.rUARTConfig.u4Baud = 115200;
    data.rUARTConfig.u1DataBits = 8;
    data.rUARTConfig.u1StopBits = 1;
    data.rUARTConfig.u1Parity = 0;
    data.rUARTConfig.u1FlowControl = 0;
    data.rUARTConfig.ucXonChar = 0x11;
    data.rUARTConfig.ucXoffChar = 0x13;
    data.rUARTConfig.fgDSRCheck = 0;
    vm_dcl_control(uart_handle,VM_SIO_CMD_SET_DCB_CONFIG,(void *)&data);

    console_rx_signal_id = vm_signal_init();

    vm_dcl_registercallback(uart_handle,VM_UART_READY_TO_READ,(VM_DCL_CALLBACK)__console_irq_handler,(void*)NULL);

    usb_device_handle = uart_handle;
}

void console_putc(char ch)
{
    VM_DCL_BUFF_LEN writen_len = 0;
    vm_dcl_write(usb_device_handle, (VM_DCL_BUFF*)&ch, 1, &writen_len, vm_dcl_get_ownerid());
    if (writen_len == 0) {
        vm_dcl_write(usb_device_handle, (VM_DCL_BUFF*)&ch, 1, &writen_len, vm_dcl_get_ownerid());
    }
}

void console_puts(const char *str)
{
    VM_DCL_BUFF_LEN writen_len = 0;
    VM_DCL_BUFF_LEN len = strlen(str);

    vm_dcl_write(usb_device_handle, (VM_DCL_BUFF*)str, len, &writen_len, vm_dcl_get_ownerid());
}

int console_getc(void)
{
    char ch;
    if (console_rx_buffer_head == console_rx_buffer_tail)
    {
        vm_signal_wait(console_rx_signal_id);
    }

    ch = console_rx_buffer[console_rx_buffer_tail % SERIAL_BUFFER_SIZE];
    console_rx_buffer_tail++;

    return ch;
}
