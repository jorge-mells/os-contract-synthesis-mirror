import sys

from instruction_parser import TraceElement, parse_spike_log
from interval_extractor import TraceAnalyzer


class FileParser:
    def __init__(self, file_path: str):
        self.file_path = file_path

    def get_parsed_sequences(
        self, intervals: list[tuple[int, int]]
    ) -> list[list[TraceElement]]:
        """Parses the identified intervals into a list of sequences of TraceElements."""
        all_sequences = []

        with open(self.file_path, "r") as f:
            all_lines = f.readlines()

        for [(start, start_cycle), (end, _end_cycle)] in intervals:
            sequence = []
            idx = start - 1
            limit = end - 1
            time = start_cycle

            while idx <= limit:
                line1 = all_lines[idx].strip()
                line2 = all_lines[idx + 1].strip() if (idx + 1) < len(all_lines) else ""

                line3 = ""
                if " epc " in line2 and (idx + 2) < len(all_lines):
                    line3 = all_lines[idx + 2].strip()

                sequence.append(parse_spike_log(time, line1, line2, line3))

                idx += 3 if line3 else 2
                time += 1

            all_sequences.append(sequence)

        return all_sequences


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python file_parser.py <file_path> <target_pid> <pid_address_hex>")
        sys.exit(1)

    path = sys.argv[1]
    pid = int(sys.argv[2])
    addr = int(sys.argv[3], 16)

    analyzer = TraceAnalyzer(pid, addr)
    analyzer.analyze(path)
    print(f"Found {len(analyzer.intervals)} intervals.")

    parser = FileParser(path)
    parsed_sequences = parser.get_parsed_sequences(analyzer.intervals)

    print(f"Parsed {len(parsed_sequences)} sequences.")
    for i, seq in enumerate(parsed_sequences):
        print(f"Sequence {i} has {len(seq)} elements.")
        print(seq)
