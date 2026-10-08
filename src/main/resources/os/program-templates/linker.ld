SECTIONS
{
    /* Start at 0 so all internal offsets are relative to the file start */
    . = 0x0; 

    .text : { 
        *(.text.boot)
        *(.text) 
        *(.rodata*)
    }

    .data : { *(.data) }
    .bss  : { *(.bss) }

    /* Ensure the binary is exactly 4096 bytes */
    . = 0x1000; /* 4096 in hex */
    .fill : {
        BYTE(0x00);
    }
}