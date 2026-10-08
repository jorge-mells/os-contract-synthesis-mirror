from dataclasses import dataclass
from typing import Optional, Union

import tinyrv


@dataclass(frozen=True)
class RegWrite:
    reg: int
    val: int


@dataclass(frozen=True)
class MemWrite:
    addr: int
    val: int


@dataclass(frozen=True)
class RegWriteFromMem:
    addr: int
    reg: int
    val: int


@dataclass(frozen=True)
class CSRWrite:
    csr: str
    val: int


@dataclass(frozen=True)
class TrapEffect:
    cause: str
    epc: int
    tval: Optional[int] = None


@dataclass(frozen=True)
class TraceElement:
    time: int
    instruction: tinyrv.rvop
    state: Union[RegWrite, MemWrite, RegWriteFromMem, CSRWrite, TrapEffect]

    @classmethod
    def alu(cls, time: int, instr: tinyrv.rvop, effect: RegWrite):
        return cls(time, instr, effect)

    @classmethod
    def store(cls, time: int, instr: tinyrv.rvop, effect: MemWrite):
        return cls(time, instr, effect)

    @classmethod
    def load(cls, time: int, instr: tinyrv.rvop, effect: RegWriteFromMem):
        return cls(time, instr, effect)

    @classmethod
    def csr(cls, time: int, instr: tinyrv.rvop, effect: CSRWrite):
        return cls(time, instr, effect)

    @classmethod
    def trap(cls, time: int, instr: tinyrv.rvop, effect: TrapEffect):
        return cls(time, instr, effect)


def parse_spike_log(time: int, line1: str, line2: str, line3: str = "") -> TraceElement:
    w1 = line1.split()
    w2 = line2.split()

    pc = int(w1[2], 16)
    instr = tinyrv.decode(int(w1[3].strip("()"), 16), pc)

    if "exception" in line2:
        cause = w2[3].strip(",")
        epc = int(w2[5], 16)
        tval = int(line3.split()[3], 16) if "tval" in line3 else None
        return TraceElement.trap(time, instr, TrapEffect(cause, epc, tval))

    if w2[5].startswith("c"):
        return TraceElement.csr(time, instr, CSRWrite(w2[5], int(w2[6], 16)))

    if w2[5] == "mem":
        return TraceElement.store(time, instr, MemWrite(int(w2[6], 16), int(w2[7], 16)))

    if "mem" in w2:
        reg = int(w2[5].replace("x", ""))
        val = int(w2[6], 16)
        addr = int(w2[8], 16)
        return TraceElement.load(time, instr, RegWriteFromMem(addr, reg, val))

    reg = int(w2[5].replace("x", ""))
    val = int(w2[6], 16)
    return TraceElement.alu(time, instr, RegWrite(reg, val))


if __name__ == "__main__":
    line1 = "core   0: 0x00000000800016f4 (0x00001517) auipc   a0, 0x1"
    line2 = "core   0: 3 0x00000000800016f4 (0x00001517) x10 0x00000000800026f4"
    # line1 = "core   0: 0x000000008000155c (0x30200073) mret"
    # line2 = (
    #     "core   0: 3 0x000000008000155c (0x30200073) c768_mstatus 0x0000000a00000088"
    # )

    result = parse_spike_log(1, line1, line2)
    print("Instruction Info")
    # Iterates through all attributes
    for attr, value in vars(result.instruction).items():
        print(f"{attr}: {value}")
    print(f"State: {result.state}")
