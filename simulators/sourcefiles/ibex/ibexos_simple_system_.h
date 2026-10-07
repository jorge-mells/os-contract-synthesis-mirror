// Copyright lowRISC contributors.
// SPDX-License-Identifier: Apache-2.0
// ibexos_simple_system_.h
#pragma once

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "verilated.h"
#if VM_TRACE_FST
#include "verilated_fst_c.h"
#else
#include "verilated_vcd_c.h"
#endif

// Include the full RVFI record structure and signal extraction definitions
#include "rvfi_signals.hpp"

// Forward declaration of Verilated top wrapper
class Vibex_simple_system;

// 1. Unbounded Thread-Safe Queue
template <typename T>
class UnboundedQueue {
 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable not_empty_;
  bool closed_ = false;

 public:
  void push(T item) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (closed_)
      return;
    queue_.push(std::move(item));
    not_empty_.notify_one();
  }

  bool pop(T &item) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, [this]() { return !queue_.empty() || closed_; });

    if (queue_.empty() && closed_)
      return false;

    item = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  void close() {
    std::lock_guard<std::mutex> lock(mutex_);
    closed_ = true;
    not_empty_.notify_all();
  }
};

// 2. Comparison State Container using RVFIRecord
struct ComparisonState {
  UnboundedQueue<RVFIRecord> queue1;
  UnboundedQueue<RVFIRecord> queue2;
  std::atomic<bool> attacker_distinguishable{false};
};

// 3. Helper Function Declarations
void usage(const char *name);

template <typename T>
void load_binary(const std::string &filename, T &ram);

// 4. Simulation Class Declaration
class Simulation {
 private:
  std::unique_ptr<VerilatedContext> context;
  std::unique_ptr<Vibex_simple_system> top;

#if VM_TRACE_FST
  std::unique_ptr<VerilatedFstC> trace;
#else
  std::unique_ptr<VerilatedVcdC> trace;
#endif

  std::string binary;
  std::string top_name;
  std::string trace_filename;
  bool trace_enabled;
  uint64_t max_cycles;

 public:
  Simulation(const std::string &binary, const std::string &top_name,
             const std::string &trace_filename, bool trace_enabled,
             uint64_t max_cycles);

  void construct(std::vector<char *> args);
  void setup_trace();
  void dump_trace();
  void close_trace();

  void reset_and_load();
  uint64_t run(ComparisonState &comparison, bool core1);
  void destroy();
};
