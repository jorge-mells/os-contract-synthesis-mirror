import argparse
import sys
from dataclasses import dataclass
from operator import itemgetter
from typing import Optional, TypedDict, Union

import pylibfst


@dataclass
class CPUInfo:
    target_pid: int
    pid_address: int
    signal_map: dict


ibex_prefix = "TOP.ibex_simple_system.u_top.rvfi_"
ibex_info = CPUInfo(
    target_pid=1,
    pid_address=0x00003710,
    signal_map={
        "intr": f"{ibex_prefix}intr",
        "mem_addr": f"{ibex_prefix}mem_addr [31:0]",
        "mem_wdata": f"{ibex_prefix}mem_wdata [31:0]",
        "mem_wmask": f"{ibex_prefix}mem_wmask [3:0]",
        "insn": f"{ibex_prefix}insn [31:0]",
        "valid": f"{ibex_prefix}valid",
    },
)

cva6_prefix = "TOP.ariane_testharness."
cva6_info = CPUInfo(
    target_pid=1,
    pid_address=0x80003308,
    signal_map={
        "valid": (
            f"{cva6_prefix}rvfi_instr[0].valid [0:0]",
            f"{cva6_prefix}rvfi_instr[1].valid [0:0]",
        ),
        "insn": (
            f"{cva6_prefix}rvfi_instr[0].insn [31:0]",
            f"{cva6_prefix}rvfi_instr[1].insn [31:0]",
        ),
        "intr": (
            f"{cva6_prefix}rvfi_instr[0].intr [63:0]",
            f"{cva6_prefix}rvfi_instr[1].intr [63:0]",
        ),
        "mem_addr": (
            f"{cva6_prefix}rvfi_instr[0].mem_addr [63:0]",
            f"{cva6_prefix}rvfi_instr[1].mem_addr [63:0]",
        ),
        "mem_wdata": (
            f"{cva6_prefix}rvfi_instr[0].mem_wdata [63:0]",
            f"{cva6_prefix}rvfi_instr[1].mem_wdata [63:0]",
        ),
        "mem_wmask": (
            f"{cva6_prefix}rvfi_instr[0].mem_wmask [7:0]",
            f"{cva6_prefix}rvfi_instr[1].mem_wmask [7:0]",
        ),
    },
)


class Instruction(TypedDict):
    addr: int
    val: int
    write: bool
    mret: bool


class TraceAnalyzer:
    MRET_OPCODE = 0x30200073

    def __init__(
        self,
        target_pid: int,
        pid_address: int,
        signal_map: dict[str, Union[str, tuple[str, ...]]],
    ):
        self.target_pid = target_pid
        self.pid_address = pid_address
        self.sig_map = signal_map
        first_key = next(iter(signal_map))
        val = signal_map[first_key]
        self.num_ports = len(val) if isinstance(val, (tuple, list)) else 1

        self.intervals = []
        self.buffer = {"time": None, "signals": {}}
        self.state = "WAITING_FOR_PID"
        self.start_time = None
        self.last_intr_time = -1
        self.possible_end_time = -1

    def _get_signal(self, sigs: dict, key: str, port: int) -> str:
        mapping = self.sig_map[key]
        path = mapping[port] if isinstance(mapping, (tuple, list)) else mapping
        return "0" if path is None else sigs.get(path, "0")

    def _get_instr(self, sigs: dict, p: int) -> Optional[Instruction]:
        if self._get_signal(sigs, "valid", p) != "1":
            return None

        insn = int(self._get_signal(sigs, "insn", p), 2)
        is_intr = self._get_signal(sigs, "intr", p) != "0"

        if is_intr:
            self.possible_end_time = self.buffer["time"]
            if is_intr:
                self.last_intr_time = self.buffer["time"]

        return {
            "addr": int(self._get_signal(sigs, "mem_addr", p), 2),
            "val": int(self._get_signal(sigs, "mem_wdata", p), 2),
            "write": int(self._get_signal(sigs, "mem_wmask", p), 2) != 0,
            "mret": (insn == self.MRET_OPCODE),
        }

    def process_cycle(self, time: int, sigs: dict):
        instrs = [self._get_instr(sigs, p) for p in range(self.num_ports)]

        if self.state == "WAITING_FOR_PID":
            if any(
                i
                and i["write"]
                and i["addr"] == self.pid_address
                and i["val"] == self.target_pid
                for i in instrs
            ):
                self.state = "WAITING_FOR_MRET"

        if self.state == "WAITING_FOR_MRET":
            if any(i and i["mret"] for i in instrs):
                self.state = "RUNNING"
                self.start_time = time

        if self.state == "RUNNING":
            if any(
                i
                and i["write"]
                and i["addr"] == self.pid_address
                and i["val"] != self.target_pid
                for i in instrs
            ):
                self.intervals.append((self.start_time, self.last_intr_time))
                self.state = "WAITING_FOR_PID"

    def callback(self, time, facidx, value_str, signals_obj):
        if self.buffer["time"] is not None and time != self.buffer["time"]:
            self.process_cycle(self.buffer["time"], self.buffer["signals"])
        self.buffer["time"] = time
        name = signals_obj.by_handle[facidx].name
        for paths in self.sig_map.values():
            if name in paths:
                self.buffer["signals"][name] = value_str
                break


class SequenceExtractor:
    def __init__(self, cpuinfo):
        self.cpuinfo = cpuinfo

    def find_sequences(self, filename):
        fst = pylibfst.lib.fstReaderOpen(filename.encode("UTF-8"))
        _, signals = pylibfst.get_scopes_signals2(fst)

        # Mask only required signals
        for item in self.cpuinfo.signal_map.values():
            names = item if isinstance(item, (tuple, list)) else (item,)
            for name in names:
                sig = signals.by_name.get(name)
                if sig:
                    pylibfst.lib.fstReaderSetFacProcessMask(fst, sig.handle)
                else:
                    sys.exit(f"Error: Could not mask signal: {name}")

        analyzer = TraceAnalyzer(
            self.cpuinfo.target_pid, self.cpuinfo.pid_address, self.cpuinfo.signal_map
        )

        # Helper callbacks
        def cb(analyzer, time, facidx, val):
            analyzer.callback(time, facidx, pylibfst.helpers.string(val), signals)

        pylibfst.helpers.fstReaderIterBlocks2(fst, cb, None, analyzer)

        pylibfst.lib.fstReaderClose(fst)
        return analyzer.intervals


class SignalComparator:
    def __init__(self, filename1, filename2, signal_names):
        self.file1 = filename1
        self.file2 = filename2
        self.signal_names = signal_names
        pass

    def get_signals(self, filename, start_time, end_time):
        fst = pylibfst.lib.fstReaderOpen(filename.encode("UTF-8"))
        _, signals = pylibfst.get_scopes_signals2(fst)

        # Mask only required signals
        for name in self.signal_names:
            sig = signals.by_name.get(name)
            if sig:
                pylibfst.lib.fstReaderSetFacProcessMask(fst, sig.handle)
            else:
                sys.exit(f"Error: Could not mask signal: {name}")

        signal_values = []

        def cb(signal_values, time, facidx, val):
            name = signals.by_handle[facidx].name
            if start_time <= time <= end_time:
                signal_values.append((time, name, pylibfst.helpers.string(val)))

        pylibfst.helpers.fstReaderIterBlocks2(fst, cb, None, signal_values)

        pylibfst.lib.fstReaderClose(fst)

        return signal_values

    def compare_sequences(self, seq1, seq2):
        assert len(seq1) == len(seq2)
        for interval1, interval2 in zip(seq1, seq2):
            signals1 = self.get_signals(self.file1, interval1[0], interval1[1])
            signals1.sort(key=itemgetter(0, 1))
            start = signals1[0][0]
            signals1 = [
                (actual_start - start, sig_name, sig_val)
                for actual_start, sig_name, sig_val in signals1
            ]
            signals2 = self.get_signals(self.file2, interval2[0], interval2[1])
            # must be stable sort to preserve order in case signal 'changes' more than once
            signals2.sort(key=itemgetter(0, 1))
            start = signals2[0][0]
            signals2 = [
                (actual_start - start, sig_name, sig_val)
                for actual_start, sig_name, sig_val in signals2
            ]
            if len(signals1) != len(signals2):
                print("more signals detected in one trace than the other.")
                return False
            for signal1, signal2 in zip(signals1, signals2):
                if signal1 != signal2:
                    print("Different signals detected")
                    print(f"trace1: {signal1}")
                    print(f"trace2: {signal2}")
                    return False
        return True


def get_args():
    parser = argparse.ArgumentParser(description="Sim trace comparator")

    # Optional arguments with defaults
    parser.add_argument(
        "--cpu",
        default="ibex",
        choices=["cva6", "ibex"],
        help="Name of the CPU core (default: ibex)",
    )
    parser.add_argument("--pid", type=int, default=0, help="Target PID (default: 0)")

    # Required positional arguments
    parser.add_argument("simfile1", help="Path to the first simulation trace file")
    parser.add_argument("simfile2", help="Path to the second simulation trace file")

    return parser.parse_args()


def main():
    args = get_args()
    info = ibex_info if args.cpu == "ibex" else cva6_info
    info.target_pid = args.pid
    simfile1 = args.simfile1
    simfile2 = args.simfile2
    extractor = SequenceExtractor(info)
    intervals1 = extractor.find_sequences(simfile1)
    print(f"intervals1: {intervals1}")
    intervals2 = extractor.find_sequences(simfile2)
    print(f"intervals2: {intervals2}")
    fst = pylibfst.lib.fstReaderOpen(simfile1.encode("UTF-8"))
    _, signals = pylibfst.get_scopes_signals2(fst)
    signals_to_check = [name for name in signals.by_name if "rvfi" in name]
    comparator = SignalComparator(
        filename1=simfile1, filename2=simfile2, signal_names=signals_to_check
    )
    if comparator.compare_sequences(intervals1, intervals2):
        print("attacker indistinguishable")
        sys.exit(1)
    else:
        print("attacker distinguishable")
        sys.exit(0)


if __name__ == "__main__":
    main()
