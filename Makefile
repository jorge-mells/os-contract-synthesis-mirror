# Directories
BUILD=build
# Build
CC=riscv64-unknown-elf-gcc
CFLAGS=-ffreestanding -nostartfiles -nostdlib -nodefaultlibs 
CFLAGS+=-g -Wl,--gc-sections -mcmodel=medany
RUNTIME=src/asm/crt0.s
LINKER_SCRIPT=src/lds/riscv64-virt.ld
KERNEL_IMAGE=$(BUILD)/kmain

# QEMU
QEMU=qemu-system-riscv64
MACH=virt
MEM=128M
RUN=$(QEMU) -nographic -machine $(MACH) -m $(MEM)
RUN+=-bios none -kernel $(KERNEL_IMAGE)

# QEMU (debug)
GDB_PORT=1234

all: setup uart syscon common scheduler interrupts kmain
	$(CC) $(BUILD)/*.o $(RUNTIME) $(CFLAGS) -T $(LINKER_SCRIPT) -o $(KERNEL_IMAGE)

setup:
	mkdir -p $(BUILD)

uart:
	$(CC) -c src/uart/uart.c $(CFLAGS) -o $(BUILD)/uart.o

syscon:
	$(CC) -c src/syscon/syscon.c $(CFLAGS) -o $(BUILD)/syscon.o

common:
	$(CC) -c src/common/common.c $(CFLAGS) -o $(BUILD)/common.o

interrupts:
	$(CC) -c src/traps/interrupts.S $(CFLAGS) -o $(BUILD)/interrupts.o

scheduler:
	$(CC) -c src/traps/scheduler.c $(CFLAGS) -o $(BUILD)/scheduler.o

kmain:
	$(CC) -c src/kmain.c $(CFLAGS) -o $(BUILD)/kmain.o

run: all
	$(RUN)

debug: all
	$(RUN) -gdb tcp::$(GDB_PORT) -S

clean:
	rm -vf $(BUILD)/*.o
	rm -vf $(KERNEL_IMAGE)
