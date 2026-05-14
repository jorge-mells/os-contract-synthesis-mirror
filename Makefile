# Directories
BUILD=build
# Build
CC = riscv64-unknown-elf-gcc
# CC=/home/jorge/thesis/riscv_tools/bin/riscv-none-elf-gcc
CFLAGS=-ffreestanding -nostartfiles -nostdlib -nodefaultlibs -static
CFLAGS+=-g -Wl,-N,--gc-sections -mcmodel=medany -march=rv64imafd
RUNTIME=src/asm/crt0.s
LINKER_SCRIPT=src/lds/riscv64-virt.ld
KERNEL_IMAGE=$(BUILD)/kmain

# QEMU
QEMU=qemu-system-riscv64
MACH=virt
MEM=128M
RUN=$(QEMU) -nographic -machine $(MACH) -m $(MEM)
RUN+=-bios none -kernel $(KERNEL_IMAGE) 
# uncomment below for an actual virtio disk abstraction
# RUN+=-drive file=disk.img,if=none,format=raw,id=hd0
# RUN+=-device virtio-blk-device,drive=hd0 -global virtio-mmio.force-legacy=false
# RUN+=-device loader,file=disk.img,addr=0x80100000,force-raw=on

# QEMU (debug)
GDB_PORT=1234

all: setup uart syscon common kmalloc storage-driver loader scheduler traps syscalls handler sample-programs asm kmain ## build the kernel and all programs
	$(CC) $(BUILD)/*.o $(RUNTIME) $(CFLAGS) -T $(LINKER_SCRIPT) -o $(KERNEL_IMAGE)

setup:
	mkdir -p $(BUILD)

uart:
	$(CC) -c src/uart/uart.c $(CFLAGS) -o $(BUILD)/uart.o

syscon:
	$(CC) -c src/syscon/syscon.c $(CFLAGS) -o $(BUILD)/syscon.o

common:
	$(CC) -c src/common/common.c $(CFLAGS) -o $(BUILD)/common.o

traps:
	$(CC) -c src/traps/trap-handler.S $(CFLAGS) -o $(BUILD)/traps.o
	$(CC) -c src/traps/traps.c $(CFLAGS) -o $(BUILD)/traps-c.o

scheduler:
	$(CC) -c src/process-mgmt/scheduler.c $(CFLAGS) -o $(BUILD)/scheduler.o

loader:
	$(CC) -c src/process-mgmt/loader.c $(CFLAGS) -o $(BUILD)/loader.o

storage-driver:
	$(CC) -c src/process-mgmt/storage-driver-ram.c $(CFLAGS) -o $(BUILD)/storage-driver.o

kmalloc:
	$(CC) -c src/heap-mgmt/kmalloc.c $(CFLAGS) -o $(BUILD)/kmalloc.o

handler:
	$(CC) -c src/syscalls/handler.c $(CFLAGS) -o $(BUILD)/handler.o

syscalls:
	$(CC) -c src/syscalls/syscalls.c $(CFLAGS) -o $(BUILD)/syscalls.o

asm:
	$(CC) -c src/asm/disk.S $(CLFAGS) -o $(BUILD)/disk.o
	$(CC) -c src/asm/sim.S $(CLFAGS) -o $(BUILD)/sim.o

sample-programs:
	$(MAKE) all -C ./sample-programs

kmain:
	$(CC) -c src/kmain.c $(CFLAGS) -o $(BUILD)/kmain.o

run: all ## run the kernel with programs
	$(RUN) -no-reboot -d int,guest_errors,in_asm,int -no-reboot -D qemu.log

debug: all
	$(RUN) -gdb tcp::$(GDB_PORT) -S

clean: ## remove the build directory
	$(MAKE) clean -C ./sample-programs
	rm -vf disk.img
	rm -vf qemu.log
	rm -vf $(BUILD)/*.o
	rm -vf $(KERNEL_IMAGE)

.PHONY: help sample-programs
help: ## Show this help message
	@echo "Usage: make run [PROGRAMS=\"prog1 prog2\"]"
	@echo "Usage: make [other-targets]"
	@echo ""
	@echo "Targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "Hint: To create new programs, add them to the sample-programs directory. Only supported ecalls are write and exit"
	@echo "Hint: Call make clean before each different invocation of make run"
