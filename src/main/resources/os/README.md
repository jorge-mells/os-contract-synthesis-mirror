# Thesis OS
A minimalist program scheduler designed for testing **Hardware-Software contracts** on the RISC-V architecture.

---

## Quickstart
Run the default build with:
`make run`

*For a full list of available build targets, see:* `make help`

## Prerequisites
Before running the OS, ensure you have the necessary cross-compilation toolchain and emulators installed.

> See the **Dependencies** section in [orig_README.md](orig_README.md) for installation instructions.

## Usage & Customization
*   **Test Sample Programs:** Place your source files in [sample-programs/](sample-programs/) and execute:
    `make run PLATFORM=CVA6|IBEX|QEMU`
> See the makefile for more customisations. Find the outputs for cva6 and ibex in simulators/output
*   **Syscall Support:** Programs using unsupported features or complex syscalls may exit prematurely or fail to execute.
*   **Modify Scheduling:** You can experiment with different scheduling algorithms by editing [src/process-mgmt/scheduler.c](src/process-mgmt/scheduler.c).

## Credits
*   Technical reference and architecture guidance from the [OSDev Wiki](https://wiki.osdev.org).
*   Development assisted by AI tools for code optimization and debugging.