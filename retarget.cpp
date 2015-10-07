#include "Arduino.h"

extern "C" {
void retarget_putc(char c)
{
  Serial.print(c);
}

int retarget_getc(void)
{
  while (!Serial.available());
  
  return Serial.read();
}

void retarget_puts(char *str)
{
    Serial.print(str);
}

} // extern "C"

