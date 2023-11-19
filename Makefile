# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc!
ENTRYPOINT	= 0x30400

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	=   0x400

# Programs, flags, etc.
ASM		= nasm
DASM	= ndisasm
CC		= gcc
LD		= ld
ASMBFLAGS	= -I boot/include/
ASMKFLAGS	= -I include/ -f elf
CFLAGS		= -g -masm=intel -m32 -fno-stack-protector -I include/ -c -fno-builtin
LDFLAGS		= -melf_i386 -Ttext $(ENTRYPOINT)
DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# This Program
ORANGESBOOT	= boot/boot.bin boot/loader.bin
ORANGESKERNEL	= kernel.bin
OBJS		= kernel/kernel.o kernel/start.o kernel/i8259.o kernel/global.o kernel/protect.o lib/asm.o lib/string.o lib/strings.o lib/display.o
DASMOUTPUT	= kernel.bin.asm

# All Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg

# Default starting position
everything : $(ORANGESBOOT) $(ORANGESKERNEL)

all : realclean everything

final : all clean

image : final buildimg

clean :
	rm -f $(OBJS)

realclean :
	rm -f $(OBJS) $(ORANGESBOOT) $(ORANGESKERNEL)
	rm -rf ./floppy

disasm :
	$(DASM) $(DASMFLAGS) $(ORANGESKERNEL) > $(DASMOUTPUT)

# We assume that "a.img" exists in current folder
buildimg :
	mkdir ./floppy
	dd if=boot/boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img ./floppy/
	sudo cp -fv boot/loader.bin ./floppy/
	sudo cp -fv kernel.bin ./floppy
	sudo umount ./floppy

boot/boot.bin : boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin : boot/loader.asm boot/include/load.inc \
			boot/include/fat12hdr.inc boot/include/pm.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(ORANGESKERNEL) : $(OBJS)
	$(LD) $(LDFLAGS) -o $(ORANGESKERNEL) $(OBJS)

kernel/kernel.o : kernel/kernel.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/start.o: kernel/start.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o : kernel/i8259.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/global.o : kernel/global.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o : kernel/protect.c
	$(CC) $(CFLAGS) -o $@ $<

lib/asm.o : lib/asm.c
	$(CC) $(CFLAGS) -o $@ $<

lib/string.o : lib/string.c
	$(CC) $(CFLAGS) -o $@ $<

lib/strings.o : lib/strings.c
	$(CC) $(CFLAGS) -o $@ $<

lib/display.o : lib/display.c
	$(CC) $(CFLAGS) -o $@ $<