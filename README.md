- This is simply a skeleton adapted from wiki.osdev.org
- planned features: interrupts(including scheduling), keyboard(input), basic filesystem support, system calls, basic program loading
- not planned: everything else
- ideally we should be able to run most assembly fairly smoothly on the os.
- for now the os is non-parallel

- next steps: fix program loading, fix kernel vs user stack issues, then maybe start thinking about syscalls. filesystem is unecessary