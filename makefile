############################################
# Global Defines
ifeq ($(OS),Windows_NT)
	MONITOR ?= $(wildcard C:/intelFPGA_lite/*/University_Program/Monitor_Program)
else
	MONITOR ?= $(wildcard ~/intelFPGA_lite/*/University_Program/Monitor_Program)
endif
ARM_TOOLS := $(MONITOR)/arm_tools/baremetal/bin
DEFINE_COMMA := ,

############################################
# Compilation Targets

# Programs
AS		:= $(ARM_TOOLS)/arm-altera-eabi-as
CC		:= $(ARM_TOOLS)/arm-altera-eabi-gcc
LD		:= $(ARM_TOOLS)/arm-altera-eabi-ld
OC		:= $(ARM_TOOLS)/arm-altera-eabi-objcopy
RM		:= rm -f

# Flags
USERCCFLAGS	:= -g -O2 -std=gnu11
ARCHASFLAGS	:= -mfloat-abi=soft -march=armv7-a -mcpu=cortex-a9 --gstabs -I "$(ARM_TOOLS)/../arm-altera-eabi/include/"
ARCHCCFLAGS	:= -mfloat-abi=soft -march=armv7-a -mtune=cortex-a9 -mcpu=cortex-a9
ARCHLDFLAGS	:= --defsym arm_program_mem=0x40 --defsym arm_available_mem_size=0x3fffffb8 --defsym __cs3_stack=0x3ffffff8 --section-start .vectors=0x0
ARCHLDSCRIPT	:= -T"$(MONITOR)/build/altera-socfpga-hosted-with-vectors.ld"
ASFLAGS		:= $(ARCHASFLAGS)
CCFLAGS		:= -Wall -c $(USERCCFLAGS) $(ARCHCCFLAGS) -I.
LDFLAGS		:= $(patsubst %, -Wl$(DEFINE_COMMA)%, $(ARCHLDFLAGS)) $(ARCHLDSCRIPT)
OCFLAGS		:= -O srec

# Files
HDRS		:= $(wildcard */*.h)
SRCS		:= $(wildcard */*.c)
OBJS		:= $(patsubst %, %.o, $(SRCS))

# Targets
.PHONY: compile
compile: main.srec

main.srec: main.axf
	@$(RM) $@
	$(OC) $(OCFLAGS) $< $@

main.axf: $(OBJS)
	@$(RM) $@
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.c.o: %.c $(HDRS)
	@$(RM) $@
	$(CC) $(CCFLAGS) $< -o $@

%.s.o: %.s $(HDRS)
	@$(RM) $@
	$(AS) $(ASFLAGS) $< -o $@

.PHONY: cc
cc: compile_commands.json
compile_commands.json: make_cc_json.py makefile
	python3 $< $(CC) --target=armv7-altera-eabi $(CCFLAGS) -isystem"$(ARM_TOOLS)/../arm-altera-eabi/include/" > $@

.PHONY: clean
clean:
	$(RM) main.srec main.axf $(OBJS)
