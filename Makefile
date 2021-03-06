

# GCC_BIN = ../LinkIt-1.1-1.60/hardware/tools/gcc-arm-none-eabi-4.8.3-2014q1/bin/
PROJECT = lua
OBJECTS = main.o syscalls_mtk.o console.o shell.o laudiolib.o lgsmlib.o timer.o variant.o wiring_digital.o gpio.o
OBJECTS += lua/linenoise.o lua/lapi.o lua/lcode.o lua/ldebug.o lua/ldo.o lua/ldump.o lua/lfunc.o lua/lgc.o lua/llex.o lua/lmem.o \
	lua/lobject.o lua/lopcodes.o lua/lparser.o lua/lstate.o lua/lstring.o lua/ltable.o lua/ltm.o  \
	lua/lundump.o lua/lvm.o lua/lzio.o lua/lrotable.o \
	lua/lauxlib.o lua/lbaselib.o lua/ldblib.o lua/liolib.o lua/lmathlib.o lua/loslib.o lua/ltablib.o \
	lua/lstrlib.o lua/loadlib.o lua/linit.o
SYS_OBJECTS =
INCLUDE_PATHS = -I. -I./libmtk -I./libmtk/include -I./lua
LIBRARY_PATHS = -L./libmtk
LIBRARIES = -lmtk
LINKER_SCRIPT = ./link.ld

###############################################################################
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-g++
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
OBJDUMP = $(GCC_BIN)arm-none-eabi-objdump
SIZE 	= $(GCC_BIN)arm-none-eabi-size
PACK    = ./tools/PackTag
PUSH    = ./tools/PushTool

CPU = -mcpu=arm7tdmi-s -mthumb -mlittle-endian
CC_FLAGS = $(CPU) -c -fvisibility=hidden -fpic -O2
CC_SYMBOLS = -D__LINKIT_V1__

LD_FLAGS = $(CPU) -O2 -Wl,--gc-sections -Wl,-Map=$(PROJECT).map -lm -fpic -pie -Wl,--entry=gcc_entry -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-unresolved-symbols
LD_SYS_LIBS =

all: $(PROJECT).vxp size

clean:
	rm -f $(PROJECT).vxp $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst $(OBJECTS) $(DEPS)

.s.o:
	$(AS) $(CPU) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 $(INCLUDE_PATHS) -o $@ $<


$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ -Wl,--start-group $^ $(LIBRARIES) $(LD_SYS_LIBS) -Wl,--end-group

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	@$(OBJCOPY) -O ihex $< $@

$(PROJECT).vxp: $(PROJECT).elf
	@$(OBJCOPY) --strip-debug $<
	@$(PACK) $< $@

$(PROJECT).lst: $(PROJECT).elf
	@$(OBJDUMP) -Sdh $< > $@

lst: $(PROJECT).lst

size:
	$(SIZE) $(PROJECT).elf

flash: $(PROJECT).vxp
	$(PUSH) -v -v -v -v -t arduino -clear -port $(PORT) -app $(PROJECT).vxp

DEPS = $(OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
