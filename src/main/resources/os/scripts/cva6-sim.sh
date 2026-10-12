#! /usr/bin/env bash
set -euo pipefail
trap "echo 'error: Script failed: see failed command above'" ERR

SIM_BIN="./simulators/Variane"
ELF_FILE="build/kmain"
OUTPUT_DIR="${1:-simulators/outputs/cva6}"
LOG_FILE="$OUTPUT_DIR/simulation.log"

mkdir -p "$OUTPUT_DIR"

# run sim
TOHOST=$("$RISCV"/bin/riscv64-unknown-elf-nm "$ELF_FILE" | grep " tohost$" | cut -d ' ' -f 1)
$SIM_BIN -f verilator.fst "$ELF_FILE" +tohost_addr="$TOHOST" >"$LOG_FILE" 2>&1
cat "$LOG_FILE"

# outputs
rm -rf encaps.traces iti.traces
mv trace_hart_0.dasm "$OUTPUT_DIR/"
mv trace_rvfi_hart_00.dasm "$OUTPUT_DIR/"
[ -f verilator.fst ] && mv verilator.fst "$OUTPUT_DIR/"

SPIKE_DASM_INPUT="$OUTPUT_DIR/trace_rvfi_hart_00.dasm"
SPIKE_DASM_OUTPUT="$OUTPUT_DIR/trace_rvfi_hart_00_human_readable.log"
spike-dasm --isa=rv64gc_zba_zbb_zbs_zbc_zbkb_zbkx_zkne_zknd_zknh <"$SPIKE_DASM_INPUT" >"$SPIKE_DASM_OUTPUT"
