# Lua_for_LinkIt_One

## Requirements
+ [gcc-arm-embedded](https://launchpad.net/gcc-arm-embedded)
+ make
+ PackTag & PushTool from LinkIt One SDK (available for Windows/Mac)

Add gcc-arm-embedded path to PATH variable, create `tools` direcotry and put PackTag & PushTool into it.

## Download

```
make flash PORT={linkit one debug port}
```

## Get started with Lua
Use serial port tool like miniterm.py to open linkit one modem port and enter:
```
print('hello, lua')
```

## API
+ audio
    - audio.play('music.mp3')
    - audio.pause()
    - audio.resume()
    - audio.stop()
    - audio.set_volome(n), n from 1 to 6
    - audio.get_volome()

+ gsm
    - gsm.call(phone_number)
    - gsm.hang()
    - gsm.accept()
    - gsm.on_incoming_call(function (phone_number) print('incoming call') end)
    - gsm.text(phone_number, message)
    - gsm.on_new_message(function (phone_number, message) print('got a message') end)

+ timer
    - id = timer.create(interval, repeat_function)
    - timer.delete(id)

+ gpio
    - gpio.mode(pin, mode) - mode: gpio.INPUT, gpio.OUTPUT, gpio.INPUT_PULLUP
    - gpio.read(pin)
    - gpio.write(pin, value)
