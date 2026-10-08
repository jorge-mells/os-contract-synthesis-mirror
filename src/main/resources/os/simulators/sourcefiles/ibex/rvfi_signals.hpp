#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>

// Define all standard RVFI signals in one central macro list:
// Format: X(type, name, is_hex)
#define RVFI_SIGNAL_LIST(X)         \
  X(uint8_t, rvfi_valid, false)     \
  X(uint64_t, rvfi_order, true)     \
  X(uint32_t, rvfi_insn, true)      \
  X(uint8_t, rvfi_trap, false)      \
  X(uint8_t, rvfi_halt, false)      \
  X(uint8_t, rvfi_intr, false)      \
  X(uint8_t, rvfi_mode, false)      \
  X(uint8_t, rvfi_ixl, false)       \
  X(uint8_t, rvfi_rs1_addr, false)  \
  X(uint8_t, rvfi_rs2_addr, false)  \
  X(uint8_t, rvfi_rs3_addr, false)  \
  X(uint32_t, rvfi_rs1_rdata, true) \
  X(uint32_t, rvfi_rs2_rdata, true) \
  X(uint32_t, rvfi_rs3_rdata, true) \
  X(uint8_t, rvfi_rd_addr, false)   \
  X(uint32_t, rvfi_rd_wdata, true)  \
  X(uint32_t, rvfi_pc_rdata, true)  \
  X(uint32_t, rvfi_pc_wdata, true)  \
  X(uint32_t, rvfi_mem_addr, true)  \
  X(uint8_t, rvfi_mem_rmask, true)  \
  X(uint8_t, rvfi_mem_wmask, true)  \
  X(uint32_t, rvfi_mem_rdata, true) \
  X(uint32_t, rvfi_mem_wdata, true) \
  X(uint64_t, rvfi_ext_mcycle, true)

struct RVFIRecord {
  uint64_t cycle;
  bool finished;

  // Declare all standard signal fields using the X-macro
#define DECLARE_FIELD(type, name, is_hex) type name{};
  RVFI_SIGNAL_LIST(DECLARE_FIELD)
#undef DECLARE_FIELD

  // --- Inequality Operator with Macro-Driven Inline Printing (Excludes
  // cycle/finished from equality) ---
  bool operator!=(const RVFIRecord &o) const {
    bool differs = false;

    auto print_diff = [&](const char *name, auto expected, auto got,
                          bool is_hex) {
      if (expected != got) {
        differs = true;
        std::cout << "  " << name << ": expected ";
        if (is_hex)
          std::cout << "0x" << std::hex;
        std::cout << +expected;
        if (is_hex)
          std::cout << std::dec;
        std::cout << ", got ";
        if (is_hex)
          std::cout << "0x" << std::hex;
        std::cout << +got;
        if (is_hex)
          std::cout << std::dec;
        std::cout << "\n";
      }
    };

    // Expand macro to automatically check and print diffs for every RVFI signal
#define PRINT_DIFF_MACRO(type, name, is_hex) \
  print_diff(#name, o.name, this->name, is_hex);

    RVFI_SIGNAL_LIST(PRINT_DIFF_MACRO)
#undef PRINT_DIFF_MACRO

    // If there is an actual functional mismatch, print the cycle context as
    // well
    // if (differs && (this->cycle != o.cycle)) {
    //   std::cout << "  (Note - cycle mismatch: expected " << this->cycle
    //             << ", got " << o.cycle << ")\n";
    // }

    return differs;
  }

  // --- Core Equality Operator (Purely based on RVFI signals) ---
  bool operator==(const RVFIRecord &o) const { return !(*this != o); }

  // --- Specific Signal Equality Checker by Name using Macro Generation ---
  bool isEqualSignal(const std::string &name, const RVFIRecord &o) const {
    if (name == "cycle")
      return cycle == o.cycle;
    if (name == "finished")
      return finished == o.finished;

#define CHECK_EQUAL_MACRO(type, field_name, is_hex) \
  if (name == #field_name)                          \
    return field_name == o.field_name;
    RVFI_SIGNAL_LIST(CHECK_EQUAL_MACRO)
#undef CHECK_EQUAL_MACRO

    std::cerr << "Warning: Unknown signal name '" << name
              << "' passed to isEqualSignal().\n";
    return false;
  }
};

// --- Macro-driven Single Signal Extractor from Core ---
#if (VERILATOR_VERSION_INTEGER >= 5000000)
#define IBEX_CORE_PTR(top_ptr) \
  ((top_ptr)->rootp->ibex_simple_system->u_top->u_ibex_top->u_ibex_core)
#else
#define IBEX_CORE_PTR(top_ptr) \
  ((top_ptr)->ibex_simple_system->u_top->u_ibex_top->u_ibex_core)
#endif

template <typename TopModule>
inline uint64_t get_rvfi_signal_from_core(TopModule *top,
                                          const std::string &name) {
  auto *core = IBEX_CORE_PTR(top);

  if (name == "cycle")
    return 0;
  if (name == "finished")
    return 0;

#define EXTRACT_MACRO(type, field_name, is_hex) \
  if (name == #field_name)                      \
    return static_cast<uint64_t>(core->field_name);
  RVFI_SIGNAL_LIST(EXTRACT_MACRO)
#undef EXTRACT_MACRO

  std::cerr << "Error: Unknown RVFI signal name requested: " << name << "\n";
  return 0;
}
