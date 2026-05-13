# List your programs here (without extensions)
# to change program order or add more programs change the PROGRAMS variable.
PROGRAMS ?= prog1 prog2 prog3 prog4
DISK=../disk.img
BUILD=build

# Toolchain configuration
CC = riscv64-unknown-elf-gcc
# CC=/home/jorge/thesis/riscv_tools/bin/riscv-none-elf-gcc
OBJCOPY = riscv64-unknown-elf-objcopy
# OBJCOPY=/home/jorge/thesis/riscv_tools/bin/riscv-none-elf-objcopy

# Flags
# -march=rv64imafd: Forces 32-bit instructions (no 'c' extension)
# -mabi=lp64d: Matches the 64-bit hardware floating point ABI
CFLAGS = -T linker.ld -nostdlib -mcmodel=medany -march=rv64imafd -mabi=lp64d -mno-relax

# Default target: build all binaries
all: $(DISK) 

# Pattern rule to create .bin from .s
$(BUILD)/%.bin: %.s
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $(BUILD)/$*.elf $<
	$(OBJCOPY) -O binary $(BUILD)/$*.elf $@

# Clean up build artifacts
clean:
	rm -f $(BUILD)/*.elf $(BUILD)/*.bin

$(DISK): $(PROGRAMS:%=$(BUILD)/%.bin)
	@echo "Creating fixed-size disk image..."
	# 1. Create a 1MB empty disk
	dd if=/dev/zero of=$(DISK) bs=1M count=1 status=none
	
	# 2. Flash each program at fixed 8-sector (4KB) intervals
	# Sector 0: ToC
	# Sector 1-8: Prog1
	# Sector 9-16: Prog2...
	@offset=1; \
	for bin in $(PROGRAMS:%=$(BUILD)/%.bin); do \
		echo "Flashing $$bin to sector $$offset"; \
		dd if=$$bin of=$(DISK) bs=512 seek=$$offset conv=notrunc status=none; \
		offset=$$(($$offset + 8)); \
	done
	
	# 3. Generate ToC and write to Sector 0
	python3 generate_toc.py $(PROGRAMS:%=%.bin)
	dd if=$(BUILD)/toc.bin of=$(DISK) bs=512 count=1 conv=notrunc status=none