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
ASMKFLAGS	= -g -I include/ -f elf
CFLAGS		= -g -Wall -fno-pie -masm=intel -std=c17 -m32 -fno-stack-protector -I include/ -c -fno-builtin
CPPFLAGS	= -g -Wall -fno-pie -masm=intel -std=c++20 -m32 -fno-stack-protector -I include/ -c -fno-builtin
LDFLAGS		= -melf_i386 -flto -Ttext $(ENTRYPOINT)
DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)

# This Program
BINBOOT	= boot/boot.bin boot/loader.bin
BINKERNEL	= kernel.bin
SYMKERNEL = kernel.dbg
OBJS		= 	sys/kernel.o sys/start.o sys/i8259.o sys/global.o sys/protect.o sys/proc.o sys/clock.o \
				sys/syscall.o sys/keyboard.o sys/tty.o sys/console.o sys/arith64.o sys/sendrecv.o \
				sys/systask.o sys/debug.o sys/harddisk.o \
				lib/asm.o lib/string.o lib/strings.o lib/display.o lib/syscall.o lib/clock.o lib/crc.o lib/ctype.o \
				lib/printf.o lib/stdlib.o lib/errno.o lib/puts.o lib/assert.o \
				fs/main.o fs/device.o fs/core.o
DASMOUTPUT	= kernel.bin.asm

# All Phony Targets
.PHONY : everything final image clean realclean disasm all buildimg split compress decompress

# Default starting position
everything : $(BINBOOT) $(BINKERNEL)

compress : harddisk.img
	gzip harddisk.img

decompress : harddisk.img.gz
	gunzip harddisk.img.gz

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

sys/%.o : sys/%.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

sys/%.o: sys/%.c
	$(CC) $(CFLAGS) -o $@ $<

sys/%.o: sys/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

lib/%.o : lib/%.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/%.o: lib/%.c
	$(CC) $(CFLAGS) -o $@ $<

lib/%.o: lib/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<

fs/%.o : fs/%.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

fs/%.o: fs/%.c
	$(CC) $(CFLAGS) -o $@ $<

fs/%.o: fs/%.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<