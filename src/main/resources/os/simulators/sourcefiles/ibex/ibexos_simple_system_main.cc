// Copyright lowRISC contributors.
// SPDX-License-Identifier: Apache-2.0
// ibexos_simple_system_main.cpp

#include "Vibex_simple_system.h"
#include "ibexos_simple_system_.h"

#if (VERILATOR_VERSION_INTEGER >= 5000000)
#include "Vibex_simple_system___024root.h"
#include "Vibex_simple_system_ibex_core__pi3.h"
#include "Vibex_simple_system_ibex_simple_system.h"
#include "Vibex_simple_system_ibex_top__pi2.h"
#include "Vibex_simple_system_ibex_top_tracing__pi1.h"
#endif

#include <getopt.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

// Configuration constants
constexpr uint32_t TARGET_PID = 1;
constexpr uint32_t PID_ADDRESS = 0x00003710;
constexpr uint32_t MRET_OPCODE = 0x30200073;

// Stream analyzer tracking whether a core is currently inside the target
// execution window
class TraceAnalyzer {
 private:
  enum class State { WAITING_FOR_PID, WAITING_FOR_MRET, RUNNING };

  uint32_t target_pid;
  uint32_t pid_address;
  State state = State::WAITING_FOR_PID;

 public:
  TraceAnalyzer(uint32_t pid, uint32_t addr)
      : target_pid(pid), pid_address(addr) {}

  void process_record(const RVFIRecord &rec) {
    if (!rec.rvfi_valid)
      return;

    bool is_write = (rec.rvfi_mem_wmask != 0);
    bool is_mret = (rec.rvfi_insn == MRET_OPCODE);

    if (state == State::WAITING_FOR_PID) {
      // std::cout << "pid: here | is_write: " << is_write << " | mem_addr: 0x"
      //           << std::hex << rec.rvfi_mem_addr << " | pid_address: 0x"
      //           << pid_address << std::dec << " | pid:" << rec.rvfi_rd_wdata
      //           << "\n";
      if (is_write && rec.rvfi_mem_addr == pid_address &&
          rec.rvfi_mem_wdata == target_pid) {
        state = State::WAITING_FOR_MRET;
      }
    } else if (state == State::WAITING_FOR_MRET) {
      if (is_mret) {
        state = State::RUNNING;
      }
    } else if (state == State::RUNNING) {
      if (is_write && rec.rvfi_mem_addr == pid_address &&
          rec.rvfi_mem_wdata != target_pid) {
        state = State::WAITING_FOR_PID;
      }
    }
  }

  bool is_in_window() const { return state == State::RUNNING; }
};

void usage(const char *name) {
  std::cout << "Usage: " << name << " [OPTIONS] BINARY1 BINARY2\n\n"
            << "Options:\n"
            << "  -t, --trace            Enable waveform tracing\n"
            << "  -m, --max-cycles=N     Stop after N cycles\n"
            << "  -p, --pid=N            Target PID to analyze\n"
            << "  -h, --help             Show help\n";
}

#if (VERILATOR_VERSION_INTEGER >= 5000000)
#define RAM_STORAGE(top) \
  top->rootp->ibex_simple_system->__PVT__u_ram__DOT__u_ram__DOT__mem
#else
#define RAM_STORAGE(top) \
  top->ibex_simple_system->__PVT__u_ram__DOT__u_ram__DOT__mem
#endif

template <typename T>
void load_binary(const std::string &filename, T &ram) {
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open binary: " + filename);
  }

  const std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> data(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char *>(data.data()), size)) {
    throw std::runtime_error("Failed reading binary: " + filename);
  }

  constexpr size_t RAM_WORDS = 1024 * 1024;
  constexpr size_t RAM_BYTES = RAM_WORDS * sizeof(uint32_t);

  if (static_cast<size_t>(size) > RAM_BYTES) {
    throw std::runtime_error("Binary too large: " + filename);
  }

  for (size_t i = 0; i < RAM_WORDS; ++i) {
    ram[i] = 0;
  }

  for (size_t i = 0; i < data.size(); ++i) {
    const size_t word = i / 4;
    const size_t byte = i % 4;
    ram[word] |= static_cast<uint32_t>(data[i]) << (8 * byte);
  }

  std::cout << filename << ": loaded " << size << " bytes\n";
}

// Simulation Method Implementations
Simulation::Simulation(const std::string &binary, const std::string &top_name,
                       const std::string &trace_filename, bool trace_enabled,
                       uint64_t max_cycles)
    : context(std::make_unique<VerilatedContext>()),
      top(nullptr),
      binary(binary),
      top_name(top_name),
      trace_filename(trace_filename),
      trace_enabled(trace_enabled),
      max_cycles(max_cycles) {
  if (trace_enabled) {
    context->traceEverOn(true);
  }
}

void Simulation::construct(std::vector<char *> args) {
  Verilated::threadContextp(context.get());
  context->commandArgs(static_cast<int>(args.size()), args.data());
  top = std::make_unique<Vibex_simple_system>(context.get(), top_name.c_str());
}

void Simulation::setup_trace() {
  if (!trace_enabled)
    return;
#if VM_TRACE_FST
  trace = std::make_unique<VerilatedFstC>();
#else
  trace = std::make_unique<VerilatedVcdC>();
#endif
  top->trace(trace.get(), 99);
  trace->open(trace_filename.c_str());
}

void Simulation::dump_trace() {
  if (trace)
    trace->dump(context->time());
}

void Simulation::close_trace() {
  if (trace) {
    trace->close();
    trace.reset();
  }
}

void Simulation::reset_and_load() {
  Verilated::threadContextp(context.get());
  top->IO_RST_N = 0;
  top->IO_CLK = 0;
  top->eval();

  load_binary(binary, RAM_STORAGE(top));

  for (int i = 0; i < 5; ++i) {
    top->IO_CLK = 1;
    top->eval();
    dump_trace();
    context->timeInc(1);

    top->IO_CLK = 0;
    top->eval();
    dump_trace();
    context->timeInc(1);
  }
  top->IO_RST_N = 1;
}

uint64_t Simulation::run(ComparisonState &comparison, bool core1) {
  Verilated::threadContextp(context.get());
  uint64_t cycles = 0;

  std::cout << "\nRunning simulation for " << binary << "...\n";

  while (true) {
    top->IO_CLK = 1;
    top->eval();
    dump_trace();
    context->timeInc(1);

    const bool finished_after_rising = context->gotFinish();

    if (!finished_after_rising) {
      top->IO_CLK = 0;
      top->eval();
      dump_trace();
      context->timeInc(1);
    }

    ++cycles;

    bool is_finished =
        finished_after_rising || context->gotFinish() || cycles >= max_cycles;

    // Construct full RVFIRecord using macro list
    RVFIRecord rec;
    rec.cycle = cycles;
    rec.finished = is_finished;

#define POPULATE_FIELD(type, name, is_hex) \
  rec.name = static_cast<type>(get_rvfi_signal_from_core(top.get(), #name));
    RVFI_SIGNAL_LIST(POPULATE_FIELD)
#undef POPULATE_FIELD

    if (rec.rvfi_valid) {
      if (core1) {
        comparison.queue1.push(rec);
      } else {
        comparison.queue2.push(rec);
      }
    }

    if (is_finished) {
      RVFIRecord fin_rec = rec;
      fin_rec.finished = true;
      if (core1) {
        comparison.queue1.push(fin_rec);
      } else {
        comparison.queue2.push(fin_rec);
      }
      break;
    }
  }

  return cycles;
}

void Simulation::destroy() {
  close_trace();
  top.reset();
  context.reset();
}

int main(int argc, char **argv) {
  const auto wall_start = std::chrono::high_resolution_clock::now();
  const auto cpu_start = std::clock();

  bool trace_enabled = false;
  uint64_t max_cycles = UINT64_MAX;
  uint32_t target_pid = TARGET_PID;

  while (true) {
    static struct option long_options[] = {
        {"trace", no_argument, nullptr, 't'},
        {"max-cycles", required_argument, nullptr, 'm'},
        {"pid", required_argument, nullptr, 'p'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}};

    int option_index = 0;
    const int c =
        getopt_long(argc, argv, "tm:p:h", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 't':
        trace_enabled = true;
        break;
      case 'm':
        max_cycles = std::stoull(optarg);
        break;
      case 'p':
        target_pid = static_cast<uint32_t>(std::stoul(optarg));
        break;
      case 'h':
        usage(argv[0]);
        return 0;
      default:
        usage(argv[0]);
        return 1;
    }
  }

  if (optind + 1 >= argc) {
    usage(argv[0]);
    return 1;
  }

  const std::string binary1 = argv[optind];
  const std::string binary2 = argv[optind + 1];

  std::vector<std::string> base_args;
  for (int i = 0; i < argc; ++i) {
    base_args.emplace_back(argv[i]);
  }

  std::vector<std::string> args1_strings = base_args;
  args1_strings.emplace_back("+ibex_tracer_file_base=ibex_top1");
  std::vector<std::string> args2_strings = base_args;
  args2_strings.emplace_back("+ibex_tracer_file_base=ibex_top2");

  std::vector<char *> args1, args2;
  for (auto &arg : args1_strings)
    args1.push_back(arg.data());
  for (auto &arg : args2_strings)
    args2.push_back(arg.data());

  ComparisonState comparison;
  uint64_t cycles1 = 0, cycles2 = 0;

  // Asynchronous Inline Stream Comparison Thread
  std::thread comparison_thread([&comparison, target_pid]() {
    TraceAnalyzer analyzer1(target_pid, PID_ADDRESS);
    TraceAnalyzer analyzer2(target_pid, PID_ADDRESS);

    RVFIRecord r1, r2;
    bool has_r1 = false;
    bool has_r2 = false;

    while (true) {
      if (!has_r1) {
        if (!comparison.queue1.pop(r1))
          break;
        if (r1.finished)
          break;
        analyzer1.process_record(r1);
        has_r1 = true;
      }

      if (!has_r2) {
        if (!comparison.queue2.pop(r2))
          break;
        if (r2.finished)
          break;
        analyzer2.process_record(r2);
        has_r2 = true;
      }

      bool in1 = analyzer1.is_in_window();
      bool in2 = analyzer2.is_in_window();

      if (in1 && in2) {
        // std::cout << "insn1: 0x" << std::hex << r1.rvfi_insn << std::dec
        //           << " (cycle: " << r1.cycle << ")\n"
        //           << "insn2: 0x" << std::hex << r2.rvfi_insn << std::dec
        //           << " (cycle: " << r2.cycle << ")\n";
        //
        // std::cout << " Core 1 Cycle: " << r1.cycle
        //           << " / Core 2 Cycle: " << r2.cycle << "\n";
        // Both cores are inside the target execution window: compare signals
        // instruction-by-instruction
        if (r1 != r2) {
          comparison.attacker_distinguishable.store(true);
          std::cout << "\n========================================\n";
          std::cout << "        RVFI SIGNAL MISMATCH DETECTED     \n";
          std::cout << "========================================\n";
          std::cout << " Core 1 Cycle: " << r1.cycle
                    << " / Core 2 Cycle: " << r2.cycle << "\n";
          break;
        }
        // Match passed: consume both records to advance lockstep
        has_r1 = false;
        has_r2 = false;
      } else if (in1 && !in2) {
        // Core 1 is at/in the window, wait for Core 2 to reach the window
        has_r2 = false;
      } else if (!in1 && in2) {
        // Core 2 is at/in the window, wait for Core 1 to reach the window
        has_r1 = false;
      } else {
        // Neither core is in the window yet: consume both records to advance
        // streams
        has_r1 = false;
        has_r2 = false;
      }
    }

    comparison.queue1.close();
    comparison.queue2.close();
  });

  std::thread thread1([&]() {
    Simulation sim1(binary1, "top1", "sim_core1.fst", trace_enabled,
                    max_cycles);
    sim1.construct(std::move(args1));
    sim1.setup_trace();
    sim1.reset_and_load();
    cycles1 = sim1.run(comparison, true);
    sim1.destroy();
  });

  std::thread thread2([&]() {
    Simulation sim2(binary2, "top2", "sim_core2.fst", trace_enabled,
                    max_cycles);
    sim2.construct(std::move(args2));
    sim2.setup_trace();
    sim2.reset_and_load();
    cycles2 = sim2.run(comparison, false);
    sim2.destroy();
  });

  thread1.join();
  thread2.join();
  comparison_thread.join();

  const auto wall_end = std::chrono::high_resolution_clock::now();
  const double cpu_ms =
      1000.0 * static_cast<double>(std::clock() - cpu_start) / CLOCKS_PER_SEC;
  const double wall_ms =
      std::chrono::duration<double, std::milli>(wall_end - wall_start).count();

  std::cout << "\nSimulation finished\n"
            << "Core 1 cycles: " << cycles1 << "\n"
            << "Core 2 cycles: " << cycles2 << "\n"
            << "CPU time: " << cpu_ms << " ms\n"
            << "Wall time: " << wall_ms << " ms\n";

  if (comparison.attacker_distinguishable.load()) {
    std::cout << "attacker distinguishable\n";
    return 0;
  } else {
    std::cout << "attacker indistinguishable\n";
    return 1;
  }
}
