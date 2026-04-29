# List your programs here (without extensions)
PROGRAMS = prog1 prog2 prog3
DISK = ../disk.img

# Toolchain configuration
CC = riscv64-unknown-elf-gcc
OBJCOPY = riscv64-unknown-elf-objcopy

# Flags
# -march=rv64imafd: Forces 32-bit instructions (no 'c' extension)
# -mabi=lp64d: Matches the 64-bit hardware floating point ABI
CFLAGS = -T linker.ld -nostdlib -mcmodel=medany -march=rv64imafd -mabi=lp64d -mno-relax

# Default target: build all binaries
all: $(DISK) 

# Pattern rule to create .bin from .s
%.bin: %.s
	$(CC) $(CFLAGS) -o $*.elf $<
	$(OBJCOPY) -O binary $*.elf $@

# Clean up build artifacts
clean:
	rm -f *.elf *.bin

$(DISK): $(PROGRAMS:%=%.bin)
	@echo "Creating fixed-size disk image..."
	# 1. Create a 1MB empty disk
	dd if=/dev/zero of=$(DISK) bs=1M count=1 status=none
	
	# 2. Flash each program at fixed 8-sector (4KB) intervals
	# Sector 0: ToC
	# Sector 1-8: Prog1
	# Sector 9-16: Prog2...
	@offset=1; \
	for bin in $(PROGRAMS:%=%.bin); do \
		echo "Flashing $$bin to sector $$offset"; \
		dd if=$$bin of=$(DISK) bs=512 seek=$$offset conv=notrunc status=none; \
		offset=$$(($$offset + 8)); \
	done
	
	# 3. Generate ToC and write to Sector 0
	python3 generate_toc.py $(PROGRAMS:%=%.bin)
	dd if=toc.bin of=$(DISK) bs=512 count=1 conv=notrunc status=none

# DISK=../disk.img
# $(DISK): prog1.bin
# 	dd if=/dev/zero of=$(DISK) bs=1M count=1
# 	dd if=prog1.bin of=$(DISK) bs=512 seek=1 conv=notrunc
# 	python3 generate_toc.py
# 	dd if=toc.bin of=$(DISK) bs=512 count=1 conv=notrunc

# prog1.bin: prog1.s
# 	riscv64-unknown-elf-gcc -T linker.ld -nostdlib -mcmodel=medany -march=rv64g -mno-relax -o prog1.elf prog1.s
# 	riscv64-unknown-elf-gcc -nostdlib -march=rv64g -mabi=lp64 -T linker.ld -mcmodel=medany -mno-relax -o prog1.elf prog1.s
# 	riscv64-unknown-elf-objcopy -O binary prog1.elf prog1.bin