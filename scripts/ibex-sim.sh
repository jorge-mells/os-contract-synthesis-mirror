OUTPUT_DIR="simulators/outputs/ibex"
LOG_FILE="$OUTPUT_DIR/simulation.log"

mkdir -p "$OUTPUT_DIR"

# run sim
./simulators/Vibex --meminit=ram,build/kmain  -t > "$LOG_FILE"

mv trace_core_00000000.log "$OUTPUT_DIR/"
mv ibex_simple_system.log "$OUTPUT_DIR/"
mv ibex_simple_system_pcount.csv "$OUTPUT_DIR/"
[ -f sim.fst ] && mv sim.fst "$OUTPUT_DIR/"