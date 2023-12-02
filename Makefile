# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc!
ENTRYPOINT	= 0x400400

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	=   0x400

# Programs, flags, etc.
ASM		= nasm
DASM	= ndisasm
CC		= gcc
CPP		= g++
LD		= ld
ASMBFLAGS	= -g -I boot/include/
ASMKFLAGS	= -g -I include/kernel/ -f elf
CFLAGS		= -g -Wall -fno-pie -masm=intel -std=c17 -m32 -fno-stack-protector -I include/ -c -fno-builtin
CPPFLAGS	= -g -Wall -fno-pie -masm=intel -std=c++20 -m32 -fno-stack-protector -I include/ -c -fno-builtin
LDFLAGS		= -melf_i386 -flto -Ttext $(ENTRYPOINT)
DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# This Program
BINBOOT	= boot/boot.bin boot/loader.bin
BINKERNEL	= kernel.bin
SYMKERNEL = kernel.dbg
OBJS		= 	kernel/kernel.o kernel/start.o kernel/i8259.o kernel/global.o kernel/protect.o kernel/proc.o kernel/clock.o \
				kernel/syscall.o kernel/keyboard.o kernel/tty.o kernel/console.o kernel/arith64.o kernel/sendrecv.o \
				kernel/systask.o \
				lib/asm.o lib/string.o lib/strings.o lib/display.o lib/syscall.o lib/clock.o lib/crc.o lib/ctype.o \
				lib/printf.o lib/stdlib.o lib/errno.o lib/puts.o lib/assert.o
DASMOUTPUT	= kernel.bin.asm

# All Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg split

# Default starting position
everything : $(BINBOOT) $(BINKERNEL)

split : everything
	objcopy --only-keep-debug $(BINKERNEL) $(SYMKERNEL)
	strip --strip-debug $(BINKERNEL)

all : realclean split

final : all clean

image : final buildimg

clean :
	rm -f $(OBJS)

realclean :
	rm -f $(OBJS) $(BINBOOT) $(BINKERNEL) $(SYMKERNEL)
	rm -rf ./floppy

disasm :
	$(DASM) $(DASMFLAGS) $(BINKERNEL) > $(DASMOUTPUT)

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

$(BINKERNEL) : $(OBJS)
	$(LD) $(LDFLAGS) -o $(BINKERNEL) $(OBJS)

kernel/%.o : kernel/%.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/%.o: kernel/%.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/%.o: kernel/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

lib/%.o : lib/%.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -o $@ $<

lib/%.o: lib/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<