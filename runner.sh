#! /usr/bin/env bash

# Define your functions (they can accept parameters using $1, $2, etc.)
compile() {
  local base_path="$1"

  cd "$base_path" || exit 1
  make clean
  make all PLATFORM=IBEX
} 2>&1

add_to_binary() {
  local base_path="$1"
  local id="$2"

  cd "$base_path" || exit 1
  cp build/kmain "build/kmain1_${id}"
  make fast-build PLATFORM=IBEX \
    "SAMPLE_PROGS_DIR=test-programs1_$id" \
    "DISK_RAW=./disk1_$id.img" \
    "KERNEL_IMAGE=\$(BUILD)/kmain1_$id" \
    "PROGRAMS=vic atk"
  cp build/kmain "build/kmain2_$id"
  make fast-build PLATFORM=IBEX \
    "SAMPLE_PROGS_DIR=test-programs2_$id" \
    "DISK_RAW=./disk2_$id.img" \
    "KERNEL_IMAGE=\$(BUILD)/kmain2_$id" \
    "PROGRAMS=vic atk"
} 2>&1

simulate_ibex() {
  local base_path="$1"
  local id="$2"

  cd "$base_path" || exit 1

  "${base_path}/scripts/ibex-sim.sh" "${base_path}/ibex/outputs1_${id}" 1 "$id"
  "${base_path}/scripts/ibex-sim.sh" "${base_path}/ibex/outputs2_${id}" 2 "$id"
} 2>&1

get_adv_distinguishability() {
  local base_path="$1"
  local id="$2"
  local cpu="$3"
  local pid="$4"

  cd "$base_path" || exit 1
  python3 adv/analyze.py --cpu "$cpu" --pid "$pid" "ibex/outputs1_${id}/sim.fst" "ibex/outputs2_${id}/sim.fst"

} 2>&1

run_spike() {
  local base_path="$1"
  local id="$2"

  cd "$base_path" || exit 1
  make clean
  make all PLATFORM=CVA6 INTERVAL=10000000
  cp build/kmain "build/kmain1_${id}"
  make fast-build PLATFORM=CVA6 \
    "SAMPLE_PROGS_DIR=test-programs1_${id}" \
    "DISK_RAW=./disk1_${id}.img" \
    "KERNEL_IMAGE=\$(BUILD)/kmain1_$id" \
    "PROGRAMS=vic"
  cp build/kmain "build/kmain2_${id}"
  make fast-build PLATFORM=CVA6 \
    "SAMPLE_PROGS_DIR=test-programs2_${id}" \
    "DISK_RAW=./disk2_${id}.img" \
    "KERNEL_IMAGE=\$(BUILD)/kmain2_$id" \
    "PROGRAMS=vic"
  mkdir -p "spike-outputs1_${id}"
  mkdir -p "spike-outputs2_${id}"
  spike -l --log="spike-outputs1_${id}/spike.log" --log-commits "build/kmain1_${id}"
  spike -l --log="spike-outputs2_${id}/spike.log" --log-commits "build/kmain2_${id}"

} 2>&1

cleanup() {
  local base_path="$1"
  local id="$2"

  cd "$base_path" || exit 1
  rm -rvf "spike-outputs1_${id}" "spike-outputs2_${id}"
  rm -rvf "ibex/outputs1_${id}" "ibex/outputs2_${id}"
  rm -rvf "test-programs1_${id}" "test-programs2_${id}"
  rm -rvf "build/kmain1_${id}" "build/kmain2_${id}"
  rm -rvf "disk1_${id}.img" "disk2_${id}.img"
} 2>&1

# Check if at least a function name was provided
if [ -z "$1" ]; then
  echo "Error: No function name provided."
  echo "Usage: $0 <function_name> [arg1 arg2 ...]"
  exit 1
fi

# Capture the function name
FUNC_NAME="$1"

# Shift the arguments so that $1 becomes the first extra parameter, $2 becomes the second, etc.
shift

# Check if the function exists
if declare -f "$FUNC_NAME" >/dev/null; then
  # Call the function and pass all remaining arguments to it
  "$FUNC_NAME" "$@"
else
  echo "Error: Function '$FUNC_NAME' does not exist."
  exit 1
fi
