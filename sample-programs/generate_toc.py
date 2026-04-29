import struct
import sys

# Fixed constants based on your linker script
PROG_SIZE_BYTES = 4096
SECTORS_PER_PROG = 8
START_SECTOR = 1

def generate():
    bin_files = sys.argv[1:]
    
    with open("toc.bin", "wb") as f:
        # 1. Number of programs (64-bit)
        f.write(struct.pack("<Q", len(bin_files)))
        
        current_sector = START_SECTOR
        for bin_file in bin_files:
            name = bin_file.replace(".bin", "")
            
            # Write Name (32 bytes), Start Sector (64-bit), Size (64-bit)
            f.write(name.encode('utf-8').ljust(32, b'\0'))
            f.write(struct.pack("<QQ", current_sector, PROG_SIZE_BYTES))
            
            current_sector += SECTORS_PER_PROG
        
        # 2. Pad ToC to 512 bytes
        f.write(b'\0' * (512 - f.tell()))

if __name__ == "__main__":
    generate()