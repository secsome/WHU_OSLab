# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc!
ENTRYPOINT = 0x400000

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	= 0x400

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
.PHONY : everything final image clean realclean disasm all buildimg split compress decompress check

# Default starting position
everything : check $(BINBOOT) $(BINKERNEL)

# verify the tool chain version
check :
	@nasm_version=$$(nasm -v 2>&1 | awk '/version/ {print $$3}'); \
	if [ "$$nasm_version" \< "2.15.05" ]; then \
		echo "Error: NASM version must be greater or equal than 2.15.05"; \
		exit 1; \
	fi

	@if ! echo "int main() { return 0; }" | $(CC) -std=c17 -xc - -o test.o >/dev/null 2>&1; then \
		echo "Error: $(CC) does not support the stdc17 standard"; \
		exit 1; \
	fi; \
	rm -f test.c test.o

	@if ! echo "int main() { return 0; }" | $(CPP) -std=c++20 -xc++ - -o test.o >/dev/null 2>&1; then \
		echo "Error: $(CPP) does not support the c++20 standard"; \
		exit 1; \
	fi; \
	rm -f test.c test.o

	@bochs_features=$$(bochs --help features 2>&1); \
    if ! echo "$$bochs_features" | grep -q "gdbstub"; then \
        echo "Error: Bochs does not support gdbstub"; \
        exit 1; \
    fi

	@if ! command -v gdb >/dev/null; then \
		echo "Error: gdb is not installed"; \
		exit 1; \
	fi

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