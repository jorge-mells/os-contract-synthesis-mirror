# cpu/riscv emulator being targeted
PLATFORM ?= QEMU
XLEN?=64
LD_ADDR = 0x80000000
# Map names to numbers
ifeq ($(PLATFORM),IBEX)
    PLATFORM_ID = 1
	XLEN=32
	LD_ADDR = 0x00000000
else ifeq ($(PLATFORM),CVA6)
    PLATFORM_ID = 2
	XLEN=64
else
    PLATFORM_ID = 3
endif

# Directories
ifeq ($(XLEN), 64)
    MABI = lp64d
else
    MABI = ilp32d
endif
ifeq ($(XLEN), 64)
    TARGET_FORMAT = elf64-littleriscv
else
    TARGET_FORMAT = elf32-littleriscv
endif
BUILD=build
# Build
# CC = riscv64-unknown-elf-gcc
COMPILER?=gcc
PREFIX?=riscv64-unknown-elf-
RISCV?=/usr
CC = $(RISCV)/bin/$(PREFIX)$(COMPILER)
CFLAGS=-ffreestanding -nostartfiles -nostdlib -nodefaultlibs -static
CFLAGS+=-mno-relax -mcmodel=medany -march=rv$(XLEN)imafd -mabi=$(MABI) -DPLATFORM_ID=$(PLATFORM_ID)
# note that you might have to specify endianness in certain cpu's. note this for debugging purposes
LDFLAGS=-g -Wl,-N,--gc-sections,--oformat,$(TARGET_FORMAT),--defsym=BASE_ADDR=$(LD_ADDR)
RUNTIME=src/asm/crt0.s
LINKER_SCRIPT=src/lds/riscv-virt.ld
KERNEL_IMAGE=$(BUILD)/kmain

ifeq ($(PLATFORM), CVA6)
	RUN=./scripts/cva6-sim.sh
else ifeq ($(PLATFORM), IBEX)
	RUN=./scripts/ibex-sim.sh
else
	# QEMU
	QEMU=qemu-system-riscv$(XLEN)
	MACH=virt
	MEM=128M
	RUN=$(QEMU) -nographic -machine $(MACH) -m $(MEM)
	RUN+=-bios none -kernel $(KERNEL_IMAGE) 
	RUN+=-no-reboot -d int,guest_errors,in_asm,int -no-reboot -D qemu.log
	# uncomment below for an actual virtio disk abstraction
	# RUN+=-drive file=disk.img,if=none,format=raw,id=hd0
	# RUN+=-device virtio-blk-device,drive=hd0 -global virtio-mmio.force-legacy=false
	# RUN+=-device loader,file=disk.img,addr=0x00100000,force-raw=on
endif


# QEMU (debug)
GDB_PORT=1234

all: setup uart syscon common kmalloc storage-driver loader scheduler traps syscalls handler sample-programs asm kmain ## build the kernel and all programs
	$(CC) $(BUILD)/*.o $(RUNTIME) $(CFLAGS) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $(KERNEL_IMAGE)

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
	$(CC) -c src/asm/disk.S $(CFLAGS) -o $(BUILD)/disk.o
	$(CC) -c src/asm/sim.S $(CFLAGS) -o $(BUILD)/sim.o

sample-programs:
	$(MAKE) all -C ./sample-programs XLEN=$(XLEN) MABI=$(MABI) COMPILER=$(COMPILER) PREFIX=$(PREFIX) RISCV=$(RISCV)

kmain:
	$(CC) -c src/kmain.c $(CFLAGS) -o $(BUILD)/kmain.o

run: all ## run the kernel with programs
	$(RUN)

debug: all
	$(RUN) -gdb tcp::$(GDB_PORT) -S

clean: ## remove the build directory
	$(MAKE) clean -C ./sample-programs
	rm -vf disk.img
	rm -vf qemu.log
	rm -vf $(BUILD)/*.o
	rm -vf $(KERNEL_IMAGE)
	rm -rvf simulators/outputs

clean-sim:
	rm -rvf simulators/output/$(PLATFORM)

.PHONY: help sample-programs
help: ## Show this help message
	@echo "Ensure that the following variables are set: RISCV(e.g: /opt/riscv), PREFIX(e.g: riscv32-unknown-elf-), COMPILER(optional)(clang or gcc)"
	@echo "Usage: make run [PROGRAMS=\"prog1 prog2\"] [XLEN=xlen]"
	@echo "Usage: make [other-targets]"
	@echo ""
	@echo "Targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-15s\033[0m %s\n", $$1, $$2}'
	@echo ""
	@echo "Hint: To create new programs, add them to the sample-programs directory. Only supported ecalls are write and exit"
	@echo "Hint: Call make clean before each different invocation of make run"
