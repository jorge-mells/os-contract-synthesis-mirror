import sys
import pylibfst

# Configuration Profiles
CORE_PROFILES = {
    "cva6": {
        "fst_path": "simulators/outputs/cva6/verilator.fst",
        "prefix": "TOP.ariane_testharness.rvfi_instr[0].",
        "map": {
            "valid": "valid",
            "insn": "insn[31:0]",
        }
    },
    "ibex": {
        "fst_path": "simulators/outputs/ibex/sim.fst",
        "prefix": "TOP.ibex_simple_system.u_top.u_ibex_top.u_ibex_core.rvfi_",
        "map": {
            "valid": "valid",
            "insn": "insn [31:0]",
        }
    }
}

# Select your target
target_core = "cva6"  # Or "ibex"
profile = CORE_PROFILES[target_core]
RVFI_CPU_PREFIX = profile["prefix"]
FST_PATH = profile["fst_path"]
SIGNAL_MAP = profile["map"]



class VCDWriter:
    def __init__(self, filename):
        self.f = open(filename, "w")
        self.f.write("$timescale 1ns $end\n$scope module top $end\n$scope module ctr $end\n")
        self.vars = {}
        self.next_char = 33 
    def add_var(self, name):
        uid = chr(self.next_char)
        self.vars[name] = uid
        self.f.write(f"$var wire 1 {uid} {name} $end\n")
        self.next_char += 1
        return uid
    def finalize_header(self):
        self.f.write("$upscope $end\n$upscope $end\n$enddefinitions $end\n")
    def write_change(self, time, name, val):
        self.f.write(f"#{time}\n{val}{self.vars[name]}\n")
    def close(self):
        self.f.close()

class CoreMonitor:
    def __init__(self, vcd, h_insn, h_retire):
        self.vcd = vcd
        self.h_insn = h_insn
        self.h_retire = h_retire
        self.pending_insn = 0 
        self.last_branch_val = -1

    def update_state(self, handle, val, time):
        if handle == self.h_insn: self.pending_insn = val
        if handle == self.h_retire and val == 1:
            self.on_retire(time)

    def on_retire(self, time):
        insn = self.pending_insn & 0xFFFFFFFF
        opcode = insn & 0x7F
        # 0x6F = JAL, 0x67 = JALR, 0x63 = Branch
        new_branch_val = 1 if opcode in [0x6F, 0x67, 0x63] else 0
        
        # --- Only write if the state has changed ---
        if new_branch_val != self.last_branch_val:
            self.vcd.write_change(time, "is_branch_1", new_branch_val)
            self.last_branch_val = new_branch_val

# 1. Open FST
fst_ptr = pylibfst.lib.fstReaderOpen(FST_PATH.encode("UTF-8"))
if fst_ptr == pylibfst.ffi.NULL:
    print(f"Unable to open {FST_PATH}")
    sys.exit(1)

# 2. Get scopes and signals
(scopes, signals) = pylibfst.get_scopes_signals2(fst_ptr)

def get_h(name):
    full_path = RVFI_CPU_PREFIX + name
    if full_path not in signals.by_name:
        raise KeyError(f"Signal {full_path} not found.")
    return signals.by_name[full_path].handle

h_retire = get_h(SIGNAL_MAP['valid'])
h_insn = get_h(SIGNAL_MAP['insn'])

# 3. Setup VCD and Monitor
vcd = VCDWriter("sim_processed.vcd")
vcd.add_var("is_branch_1")
vcd.finalize_header()
monitor = CoreMonitor(vcd, h_insn, h_retire)

# 4. Define Pythonic Callback (No @def_extern needed with helpers)
def value_change_callback(data, time, handle, value):
    int_val = int(pylibfst.helpers.string(value), 2)
    data.update_state(handle, int_val, time)

def varlen_callback(data, time, handle, value, length):
    pass

# 5. Configure Mask and Iterate
# 1. Clear everything so we start with a clean slate
pylibfst.lib.fstReaderClrFacProcessMaskAll(fst_ptr)

# 2. Add only the signals we actually care about
target_handles = [h_retire, h_insn]
for h in target_handles:
    pylibfst.lib.fstReaderSetFacProcessMask(fst_ptr, h)

print("Starting iteration (masked)...")
# Pass 'monitor' (the object) instead of the string "monitor_data" 
# to keep your state clean and accessible in the callback
pylibfst.helpers.fstReaderIterBlocks2(
    fst_ptr,
    value_change_callback,
    varlen_callback,
    monitor
)

vcd.close()
pylibfst.lib.fstReaderClose(fst_ptr)
print("Done.")