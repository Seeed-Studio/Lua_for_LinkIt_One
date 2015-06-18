#include "Arduino.h"

extern "C" {
void uart_putchar(char c)
{
  Serial.print(c) ;
}

int uart_getchar(void)
{
  while (!Serial.available());
  
  return Serial.read();
}

void uart_puts(char *str)
{
    while (*str) {
        uart_putchar(*str);
        str++;
    }
}

} // extern "C"

