# Generating the sim binaries

## Ibex

clone the ibex repo and follow the steps to in ibex simple system to generate a verilator binary. However replace ibex_simple_system.h and rtl/ibex_simple_system.sv with the versions in sourcefiles/ibex. The binary will be in repo/build. Ensure PMPEnable=1

## cva6

clone the cva6 repo and follow the steps to setup. Then follow the steps to simulate a program. export traces_fast(see cva6 repo) to enable fst tracing. The binary will be in repo/work-ver
