CC = gcc
LD = ld
AS = as
CFLAGS = -m64 -g -c -ffreestanding -Wall -Werror -fcommon -Iapi/ -Iisched/ -Isyscalls/ -Ikernel/ -fPIE \
	-nostdlib \
	-nostartfiles
	
ASFLAGS = -f elf64

LDFLAGS = -m elf_x86_64 \
	-nostdlib \
	-static \
	-pie \
	--no-dynamic-linker \
	-z text \
	-T linker.ld

OUTDIR = ../build

OUTPUT = $(TARGET)/boot/kernel.bin

C_SOURCES := $(shell find . -name '*.c' ! -name "printf.c" -printf "%P\n")
ASM_SOURCE := $(shell find . -name '*.asm' -printf "%P\n")

C_OBJS = $(patsubst %.c, %.o, $(C_SOURCES))
C_EXTRA_OBJS = idrivers/printf.o

ASM_OBJS = $(patsubst %.asm, %.asm.o, $(ASM_SOURCE))

all: $(C_OBJS) $(ASM_OBJS) $(C_EXTRA_OBJS)
	mkdir $(TARGET)/ -p
	mkdir $(TARGET)/boot/ -p
	$(LD) $(LDFLAGS) -o $(OUTPUT) $(C_OBJS) $(C_EXTRA_OBJS) $(ASM_OBJS)	

idrivers/printf.o: idrivers/printf.c
	$(CC) $(CFLAGS) idrivers/printf.c -o drivers/printf.o -DPRINTF_INCLUDE_CONFIG_H=1

%.o: %.c
	$(CC) $(CFLAGS) $(subst .o,.c,$@) -o $@

%.asm.o: %.asm
	$(AS) $(ASFLAGS) -o $@ $(subst .asm.o,.asm,$@)

clean:
	find . -name '*.o' -delete
	rm -r -f $(TARGET)/kernel.bin