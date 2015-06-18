extern "C" {
#include "lua.h"
}


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
}

void loop() {
  // put your main code here, to run repeatedly:
  {
    char *argv[] = {"lua", 0};
    int argc = 1;
    lua_main(argc, argv);
  }
}
