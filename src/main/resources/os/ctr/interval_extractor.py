import sys


# WARN: this assumes that the spike trace files will always have the same format.
class TraceAnalyzer:
    def __init__(self, target_pid: int, pid_address: int):
        self.target_pid = target_pid
        self.pid_address = pid_address

        self.state = "WAITING_FOR_PID"
        self.intervals = []
        self.start_line = None
        self.last_sync_line = None
        self.cycle = 0

    def _process_state_change(self, line_num, event_type, mem_addr=None, mem_val=None):
        """Pure state transition logic."""
        if self.state == "WAITING_FOR_PID":
            if (
                event_type == "store"
                and mem_addr == self.pid_address
                and mem_val == self.target_pid
            ):
                self.state = "WAITING_FOR_MRET"

        elif self.state == "WAITING_FOR_MRET":
            if event_type == "mret":
                self.state = "RUNNING"
                self.start_line = (line_num, self.cycle)

        elif self.state == "RUNNING":
            if event_type in ("trap_or_exception", "mret"):
                self.last_sync_line = (line_num, self.cycle)
            elif (
                event_type == "store"
                and mem_addr == self.pid_address
                and mem_val != self.target_pid
            ):
                self.intervals.append([self.start_line, self.last_sync_line])
                self.state = "WAITING_FOR_PID"

    def analyze(self, file_path):
        with open(file_path, "r") as f:
            for line_num, line in enumerate(f, 1):
                # 1. Trigger checks
                # OPTIMIZE: you may actually use state checks to reduce number of function calls
                # at expense of readability

                if line.split()[2].startswith("0x"):
                    self.cycle += 1

                # WARN: note the spaces in the strings; they are important for exact matching of
                # the trace file
                if " mret" in line:
                    self._process_state_change(line_num, "mret")
                    continue

                if " ecall" in line:
                    self._process_state_change(line_num, "trap_or_exception")
                    continue

                if " epc " in line:
                    self._process_state_change(line_num - 1, "trap_or_exception")
                    continue

                # 2. Memory line check
                if " mem " in line:
                    parts = line.split()
                    mem_idx = parts.index("mem")

                    # Ensure indices exist before accessing
                    if len(parts) > mem_idx + 2:
                        addr = int(parts[mem_idx + 1], 16)
                        val = int(parts[mem_idx + 2], 16)
                        # Process using the instruction line number
                        self._process_state_change(line_num - 1, "store", addr, val)


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python script.py <file_path> <target_pid> <pid_address_hex>")
        sys.exit(1)

    path = sys.argv[1]
    pid = int(sys.argv[2])
    addr = int(sys.argv[3], 16)

    analyzer = TraceAnalyzer(pid, addr)
    analyzer.analyze(path)

    print(f"Analysis Complete. Found {len(analyzer.intervals)} intervals:")
    for start, end in analyzer.intervals:
        print(f"Interval: Lines {start} to {end}")
