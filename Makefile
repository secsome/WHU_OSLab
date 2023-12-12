# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc!
ENTRYPOINT	= 0x400000

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	=   0x400

# Programs, flags, etc.
ASM		= nasm
CC		= gcc
CPP		= g++
LD		= ld
ASMBFLAGS	= -g -I boot/include/
ASMKFLAGS	= -g -I include/ -f elf
CFLAGS		= -g -Wall -fno-pie -masm=intel -std=c17 -m32 -fno-stack-protector -I include/ -c -fno-builtin
CPPFLAGS	= -g -Wall -fno-pie -masm=intel -std=c++20 -m32 -fno-stack-protector -I include/ -c -fno-builtin
LDFLAGS		= -melf_i386 -flto -Ttext $(ENTRYPOINT)

# This Program
BINBOOT	= boot/boot.bin boot/loader.bin
BINKERNEL	= kernel.bin
SYMKERNEL = kernel.dbg
SRCDIRS = sys lib fs
SRCASM = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.asm))
SRCC = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
SRCCPP = $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.cpp))
OBJS = $(SRCASM:.asm=.o) $(SRCC:.c=.o) $(SRCCPP:.cpp=.o)
				
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

$(SRCASM:%.asm=%.o): %.o: %.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

$(SRCC:%.c=%.o): %.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

$(SRCCPP:%.cpp=%.o): %.o: %.cpp
	$(CPP) $(CPPFLAGS) -o $@ $<