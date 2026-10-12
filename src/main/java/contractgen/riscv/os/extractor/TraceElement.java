package contractgen.riscv.os.extractor;

import contractgen.riscv.isa.RISCVInstruction;

public record TraceElement(long time, long pc, RISCVInstruction instruction, State state) {

  // The polymorphic interface
  public interface State {
  }

  // Concrete state changes
  public record RegState(int reg, long val) implements State {
  }

  public record MemState(long addr, long val) implements State {

  }

  public record LoadState(long addr, int reg, long val) implements State {
  }

  public record CSRState(String csr, long val) implements State {
  }

  public record TrapState(String cause, long epc, Long tval) implements State {
  }

  public record NoState() implements State {
  }

  // Factory methods for cleaner construction
  public static TraceElement alu(long time, long pc, RISCVInstruction instr, RegState state) {
    return new TraceElement(time, pc, instr, state);
  }

  public static TraceElement store(long time, long pc, RISCVInstruction instr, MemState state) {
    return new TraceElement(time, pc, instr, state);
  }

  public static TraceElement load(long time, long pc, RISCVInstruction instr, LoadState state) {
    return new TraceElement(time, pc, instr, state);
  }

  public static TraceElement csr(long time, long pc, RISCVInstruction instr, CSRState state) {
    return new TraceElement(time, pc, instr, state);
  }

  public static TraceElement trap(long time, long pc, RISCVInstruction instr, TrapState state) {
    return new TraceElement(time, pc, instr, state);
  }

  public static TraceElement nostate(long time, long pc, RISCVInstruction instr, NoState state) {
    return new TraceElement(time, pc, instr, state);
  }

}
