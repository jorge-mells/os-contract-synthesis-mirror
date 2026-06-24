import sys
import time
import pylibfst
from dataclasses import dataclass
from typing import List, Optional, Dict

@dataclass
class Sequence:
    mret_time: int
    post_mret_pc_rdata: int
    post_mret_pc_wdata: int
    intr_time: int
    intr_pc_rdata: int
    intr_pc_wdata: int
    delta: int

    def __repr__(self):
        return (f"Sequence(MRET@{self.mret_time}, "
                f"PC_R:{hex(self.post_mret_pc_rdata)}, "
                f"Intr@{self.intr_time}, "
                f"Delta:{self.delta})")

class SequenceExtractor:
    def __init__(self, mem_start=0x100000, mem_end=0x101400):
        self.mem_start = mem_start
        self.mem_end = mem_end
        self.mret_opcode = 0x30200073
        self.prefix = "TOP.ibex_simple_system.u_top.u_ibex_top.u_ibex_core.rvfi_"
        self.signal_map = {
            "valid": "valid", "insn": "insn [31:0]", "intr": "intr",
            "pc_rdata": "pc_rdata [31:0]", "pc_wdata": "pc_wdata [31:0]"
        }

    def get_sequences(self, fst_path: str) -> List[Sequence]:
        fst = pylibfst.lib.fstReaderOpen(fst_path.encode("UTF-8"))
        (_, signals) = pylibfst.get_scopes_signals2(fst)
        h = {k: signals.by_name[self.prefix + v].handle for k, v in self.signal_map.items()}

        # The state persists across the entire iteration
        state = {
            'h': h,
            'vals': {},
            'last_time': -1,
            'pending_mret': None,
            'capture_next': False,
            'sequences': []
        }

        def update(d, time, handle, value):
            if time != d['last_time'] and d['last_time'] != -1:
                self._process_snapshot(d, d['last_time'])
            d['last_time'] = time
            d['vals'][handle] = int(pylibfst.helpers.string(value), 2)

        pylibfst.lib.fstReaderClrFacProcessMaskAll(fst)
        for val in h.values(): pylibfst.lib.fstReaderSetFacProcessMask(fst, val)

        pylibfst.helpers.fstReaderIterBlocks(fst, update, state)
        
        # Flush the final time-step
        if state['last_time'] != -1:
            self._process_snapshot(state, state['last_time'])
            
        pylibfst.lib.fstReaderClose(fst)
        return state['sequences']

    def _process_snapshot(self, d, time):
        h = d['h']
        v = d['vals']
        if v.get(h['valid']) != 1: return

        insn = v.get(h['insn'], 0)
        intr = v.get(h['intr'], 0)
        rdata = v.get(h['pc_rdata'], 0)
        wdata = v.get(h['pc_wdata'], 0)

        if d['capture_next']:
            d['pending_mret']['post_r'] = rdata
            d['pending_mret']['post_w'] = wdata
            d['capture_next'] = False

        if insn == self.mret_opcode:
            d['pending_mret'] = {'time': time}
            d['capture_next'] = True
        elif intr == 1 and d['pending_mret'] and 'post_r' in d['pending_mret']:
            if self.mem_start <= d['pending_mret']['post_r'] < self.mem_end:
                delta = time - d['pending_mret']['time']
                seq = Sequence(d['pending_mret']['time'], d['pending_mret']['post_r'], 
                               d['pending_mret']['post_w'], time, rdata, wdata, delta)
                d['sequences'].append(seq)
                # print(f"Sequence Found: MRET@{seq.mret_time} -> Post-MRET PC_R:{hex(seq.post_mret_pc_rdata)} -> "
                #       f"Intr@{seq.intr_time} (PC:{hex(seq.intr_pc_rdata)}) | Delta: {delta}")
            d['pending_mret'] = None

class TraceComparator:
    def __init__(self, fst_path_a: str, fst_path_b: str):
        self.paths = [fst_path_a, fst_path_b]
        self.prefix = "TOP.ibex_simple_system.u_top.u_ibex_top.u_ibex_core.rvfi_"

    def get_all_core_handles(self, signals):
        """Find all handles in the core, excluding massive memory arrays."""
        handles = {}
        for name, sig in signals.by_name.items():
            # Adjust this filter to match your exact hierarchy
            # We skip signals that are clearly memory arrays or too deep
            if "u_ibex_core" in name and not "unused" in name:
                short_name = name.split('.')[-1]
                handles[short_name] = sig.handle
        return handles

    def get_signal_window(self, fst_path: str, start_t: int, end_t: int) -> Dict[int, Dict[str, int]]:
        fst = pylibfst.lib.fstReaderOpen(fst_path.encode("UTF-8"))
        (_, signals) = pylibfst.get_scopes_signals2(fst)
        
        # Capture all core signals
        handles = self.get_all_core_handles(signals)
        handle_to_name = {h: n for n, h in handles.items()}

        
        # Initialize a list instead of a dict to collect raw events
        # 1. Pre-allocate based on expected max events
        # (If you don't know the exact count, use a safe upper bound)
        length = (end_t - start_t + 1) * len(handles)
        raw_events = [None] * length
        curr_idx = 0
        
        def callback(data, time, handle, value):
            nonlocal curr_idx # Essential: lets you modify the outer variable
            if start_t <= time <= end_t:
                name = handle_to_name[handle]
                if name:
                    # 2. Direct index assignment (blazing fast)
                    raw_events[curr_idx] = (time, name, int(pylibfst.string(value), 2))
                    curr_idx += 1
        
        pylibfst.lib.fstReaderClrFacProcessMaskAll(fst)
        # Enable all discovered handles
        for h in handles.values(): 
            pylibfst.lib.fstReaderSetFacProcessMask(fst, h)
            
        pylibfst.helpers.fstReaderIterBlocks(fst, callback, None)
        pylibfst.lib.fstReaderClose(fst)

        return raw_events[:curr_idx]

    def compare_windows(self, seq_a: Sequence, seq_b: Sequence) -> bool:
        events_a = self.get_signal_window(self.paths[0], seq_a.mret_time, seq_a.intr_time)
        events_b = self.get_signal_window(self.paths[1], seq_b.mret_time, seq_b.intr_time)
        
        # Determine the length of the shorter list to avoid index errors
        len_a, len_b = len(events_a), len(events_b)
        min_len = min(len_a, len_b)
        
        # Compare element by element
        for i in range(min_len):
            if events_a[i] != events_b[i]:
                print(f"Mismatch at index {i}:")
                print(f"Trace A: {events_a[i]}")
                print(f"Trace B: {events_b[i]}")
                return True
        
        # If one list is longer than the other, that's also a mismatch
        if len_a != len_b:
            print(f"Length mismatch: Trace A has {len_a} events, Trace B has {len_b}")
            return True
            
        return False

# Usage
if __name__ == "__main__":
    ext = SequenceExtractor()
    seqs_a = ext.get_sequences("sim1.fst")
    seqs_b = ext.get_sequences("sim2.fst") # Or your second file
    print(seqs_a)
    print(seqs_b)
    # idea: once there's a timing difference it is exploitable. no need to check full sequence.
    # if len(seqs_a) != len(seqs_b):
    #     print("attacker distinguishable")
    #     sys.exit(0)
    # for seq_a, seq_b in zip(seqs_a, seqs_b):
    #     if seq_a.mret_time != seq_b.mret_time or seq_a.intr_time != seq_b.intr_time:
    #         print("attacker distinguishable(timing difference found)")
    #         sys.exit(0)

    comp = TraceComparator("sim1.fst", "sim2.fst")
    
    for i in range(min(len(seqs_a), len(seqs_b))):
        if comp.compare_windows(seqs_a[i], seqs_b[i]):
            print("attacker distinguishable(deeper microarchitectural difference)")
            sys.exit(0)
    print("attacker indistinguishable")