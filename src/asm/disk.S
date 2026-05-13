.section .disk_data, "a", @progbits
.balign 4096
/* We removed the .global _disk_start here because we are 
   defining it in the Linker Script instead. */
// be super aware of the path here and later make it global
.incbin "disk.img"