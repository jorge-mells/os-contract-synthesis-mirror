#! /usr/bin/env bash

# Define your functions (they can accept parameters using $1, $2, etc.)
compile() {
  echo 'hello' && exit 1
  local base_path="$1"

  cd "$base_path" || exit 1
  make clean
  make all PLATFORM=IBEX
  mkdir assets
  mv build/kmain assets
  make clean
  make all PRG_COUNT=1 PLATFORM=CVA6 INTERVAL=10000000 SET_XLEN=1 KERNEL_IMAGE_NAME=build/kmain_spike
  mv build/kmain assets/kmain_spike
} 2>&1

add_to_binary() {
  local base_path="$1"
  local id="$2"

  cd "$base_path" || exit 1
  cp assets/kmain "build/kmain1_${id}"
  make fast-build PLATFORM=IBEX \
    "SAMPLE_PROGS_DIR=test-programs1_$id" \
    "DISK_RAW=./disk1_$id.img" \
    "KERNEL_IMAGE=\$(BUILD)/kmain1_$id" \
    "PROGRAMS=vic atk"
  cp assets/kmain "build/kmain2_$id"
  make fast-build PLATFORM=IBEX \
    "SAMPLE_PROGS_DIR=test-programs2_$id" \
    "DISK_RAW=./disk2_$id.img" \
    "KERNEL_IMAGE=\$(BUILD)/kmain2_$id" \
    "PROGRAMS=vic atk"
  echo "##############"
  ls -la "build/kmain2_${id}.bin"
  echo "##############"
} 2>&1

simulate_ibex() {
  local base_path="$1"
  local id="$2"
  # local cpu="$3"
  # local pid="$4"

  cd "$base_path" || exit 1
  cat "test-programs2_$id/vic.s"
  "${base_path}"/simulators/Vibex1 "build/kmain1_$id.bin" "build/kmain2_$id.bin"
} 2>&1

run_spike() {
  local base_path="$1"
  local id="$2"

  cd "$base_path" || exit 1
  make clean
  cp assets/kmain_spike "build/kmain_spike1_${id}"
  make fast-build PLATFORM=CVA6 \
    "SAMPLE_PROGS_DIR=test-programs1_${id}" \
    "DISK_RAW=./disk_spike1_${id}.img" \
    "KERNEL_IMAGE=\$(BUILD)/kmain_spike1_$id" \
    "PROGRAMS=vic"
  cp assets/kmain_spike "build/kmain_spike2_${id}"
  make fast-build PLATFORM=CVA6 \
    "SAMPLE_PROGS_DIR=test-programs2_${id}" \
    "DISK_RAW=./disk_spike2_${id}.img" \
    "KERNEL_IMAGE=\$(BUILD)/kmain_spike2_$id" \
    "PROGRAMS=vic"
  mkdir -p "spike-outputs1_${id}"
  mkdir -p "spike-outputs2_${id}"
  spike --isa=rv32imafd -l --log="spike-outputs1_${id}/spike.log" --log-commits "build/kmain_spike1_${id}"
  spike --isa=rv32imafd -l --log="spike-outputs2_${id}/spike.log" --log-commits "build/kmain_spike2_${id}"
} 2>&1

cleanup() {
  local base_path="$1"
  local id="$2"

  cd "$base_path" || exit 1
  rm -rf debug/
  mkdir debug
  mv "spike-outputs1_${id}" debug/
  mv "spike-outputs2_${id}" debug/
  rm -rvf "spike-outputs1_${id}" "spike-outputs2_${id}"
  echo "######## ${id} ##########"
  cat "test-programs1_${id}/vic.s"
  cat "test-programs2_${id}/vic.s"
  echo "#########################"
  rm -rf prgs/
  mkdir prgs
  mv "build/kmain_spike1_${id}" prgs/
  mv "build/kmain_spike2_${id}" prgs/
  cp "assets/kmain_spike" prgs/
  rm -rvf "test-programs1_${id}" "test-programs2_${id}"
  rm -rvf "build/kmain1_${id}" "build/kmain2_${id}"
  rm -rvf "build/kmain1_${id}.bin" "build/kmain2_${id}.bin"
  rm -rvf "build/kmain_spike1_${id}" "build/kmain_spike2_${id}"
  rm -rvf "build/kmain_spike1_${id}.bin" "build/kmain_spike2_${id}.bin"
  rm -rvf "disk1_${id}.img" "disk2_${id}.img"
  rm -rvf "disk_spike1_${id}.img" "disk_spike2_${id}.img"
} 2>&1

debug() {
  local base_path="$1"
  local id="$2"

  cd "$base_path" || exit 1
  echo "=== Pair ${id} Disassembly ==="
  riscv64-unknown-elf-gcc -c "test-programs1_${id}/vic.s" -o code1.o
  riscv64-unknown-elf-gcc -c "test-programs2_${id}/vic.s" -o code2.o

  echo "--- Variant 1 ---"
  riscv64-unknown-elf-objdump -d code1.o

  echo "--- Variant 2 ---"
  riscv64-unknown-elf-objdump -d code2.o
  echo "=========================="
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
