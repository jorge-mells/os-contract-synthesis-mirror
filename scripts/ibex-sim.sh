#! /usr/bin/env bash
set -euo pipefail
trap "echo 'error: Script failed: see failed command above'" ERR
OUTPUT_DIR="simulators/outputs/ibex"
LOG_FILE="$OUTPUT_DIR/simulation.log"

mkdir -p "$OUTPUT_DIR"

# run sim
./simulators/Vibex --meminit=ram,build/kmain -t > "$LOG_FILE" 2>&1

mv trace_core_00000000.log "$OUTPUT_DIR/"
cat ibex_simple_system.log
mv ibex_simple_system.log "$OUTPUT_DIR/"
mv ibex_simple_system_pcount.csv "$OUTPUT_DIR/"
[ -f sim.fst ] && mv sim.fst "$OUTPUT_DIR/"