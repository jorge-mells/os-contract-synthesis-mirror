package contractgen.riscv.os.extractor;

import java.io.IOException;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.function.Function;
import java.util.function.Predicate;

import contractgen.Extractor;
import contractgen.TestResult;
import contractgen.riscv.isa.RISCVInstruction;
import contractgen.riscv.isa.RISCV_TYPE;
import contractgen.riscv.isa.contract.RISCVObservation;
import contractgen.riscv.isa.contract.RISCVTestResult;
import contractgen.riscv.isa.contract.RISCV_OBSERVATION_TYPE;
import contractgen.util.Pair;

public class RVFIOSExtractor implements Extractor {
  private List<TraceElement> traces1;
  private List<TraceElement> traces2;

  // Timeline Lookups for O(1) access
  private RegisterTimeline timeline1;
  private RegisterTimeline timeline2;

  private Set<RISCVObservation> obs1;
  private Set<RISCVObservation> obs2;

  @Override
  /**
   * Extracts test results from two files.
   * 
   * @param PATH                     A colon-separated string containing the paths
   *                                 to the primary and
   *                                 adversary log files (e.g.,
   *                                 "primary.log:adversary.log"). This is
   *                                 because the current implementation expects
   *                                 the two simulation
   *                                 traces to be generated independently.
   * @param adversaryDistinguishable Whether to factor in adversary distinctions.
   * @param index                    The test index to extract.
   * @return A Pair containing the primary and adversary TestResult objects.
   * @throws IllegalArgumentException if the PATH string does not contain a colon
   *                                  or if one of the file paths is empty.
   */
  public Pair<TestResult, TestResult> extractResults(String PATH, boolean adversaryDistinguishable, int index) {
    if (PATH == null || !PATH.contains(":")) {
      throw new IllegalArgumentException(
          "PATH must be a colon-separated string (e.g., 'primary.log:adversary.log')");
    }
    String[] parts = PATH.split(":", 2);
    return extractResults(parts[0], parts[1], adversaryDistinguishable, index, "isa");
  }

  private Pair<TestResult, TestResult> extractResults(String SIM1PATH, String SIM2PATH,
      boolean adversaryDistinguishable, int index, String type) {
    try {
      this.traces1 = new TraceParser(SIM1PATH.trim(), 0).parseUserSections();
      this.traces2 = new TraceParser(SIM2PATH.trim(), 0).parseUserSections();
      // if (type.equals("isa")) {
      // // System.out.println("[" + Thread.currentThread().getName() + "] " + " (" +
      // // SIM1PATH + ") " + traces1);
      // // System.out.println("[" + Thread.currentThread().getName() + "] " + " (" +
      // // SIM2PATH + ") " + traces2);
      // } else {
      // // WARN: fix later. the idea is to include syscall sections when doing
      // contract
      // // evaluation, or
      // // provide a switch to toggle this.
      // // this.traces1 = new TraceParser(SIM1PATH.trim()).parseSyscallSections();
      // // this.traces2 = new TraceParser(SIM2PATH.trim()).parseSyscallSections();
      // }
    } catch (IOException e) {
      throw new RuntimeException(e);
    }

    this.timeline1 = new RegisterTimeline(this.traces1);
    this.timeline2 = new RegisterTimeline(this.traces2);
    this.obs1 = new HashSet<>();
    this.obs2 = new HashSet<>();
    // WARN: both lists might be different lengths
    for (int i = 0; i < Math.min(this.traces1.size(), this.traces2.size()); i++) {
      compareInstructions(i);
      compareRegisters(i);
      compareMemory(i);
      compareBranch(i);
      for (int distance = 1; distance <= 4; distance++) {
        compareDependencies(i, distance);
      }
      compareSections(i);

    }
    return new Pair<>(new RISCVTestResult(obs1, adversaryDistinguishable, index * 2),
        new RISCVTestResult(obs2, adversaryDistinguishable, (index * 2) + 1));
  }

  // WARN: in the reference implementation, there's a check for invalid
  // instructions. check later!!
  private void compareInstructions(int idx) {
    TraceElement t1 = this.traces1.get(idx);
    TraceElement t2 = this.traces2.get(idx);

    RISCVInstruction i1 = t1.instruction();
    RISCVInstruction i2 = t2.instruction();

    // OPCODE Comparison
    if (!Objects.equals(i1.type(), i2.type())) {
      // System.out.println("[" + Thread.currentThread().getName() + "] " + " t1 " +
      // t1);
      // System.out.println("[" + Thread.currentThread().getName() + "] " + " t2 " +
      // t2);
      obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.OPCODE));
      obs2.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.OPCODE));
    }

    // Use a helper to avoid repeating the boilerplate
    checkField(i1, i2, RISCV_OBSERVATION_TYPE.RD, RISCVInstruction::hasRD, RISCVInstruction::rd, obs1, obs2);
    checkField(i1, i2, RISCV_OBSERVATION_TYPE.RS1, RISCVInstruction::hasRS1, RISCVInstruction::rs1, obs1, obs2);
    checkField(i1, i2, RISCV_OBSERVATION_TYPE.RS2, RISCVInstruction::hasRS2, RISCVInstruction::rs2, obs1, obs2);
    checkField(i1, i2, RISCV_OBSERVATION_TYPE.IMM, RISCVInstruction::hasIMM, RISCVInstruction::imm, obs1, obs2);
  }

  // Helper to keep the main logic clean
  private void checkField(RISCVInstruction i1, RISCVInstruction i2, RISCV_OBSERVATION_TYPE type,
      Predicate<RISCVInstruction> hasField, Function<RISCVInstruction, Object> getField,
      Set<RISCVObservation> obs1, Set<RISCVObservation> obs2) {
    if (hasField.test(i1) && hasField.test(i2) && !Objects.equals(getField.apply(i1), getField.apply(i2))) {
      obs1.add(new RISCVObservation(i1.type(), type));
      obs2.add(new RISCVObservation(i2.type(), type));
    }
  }

  private void compareRegisters(
      int idx) {
    // Extract instructions for cleaner syntax
    TraceElement t1 = this.traces1.get(idx);
    TraceElement t2 = this.traces2.get(idx);
    RISCVInstruction i1 = t1.instruction();
    RISCVInstruction i2 = t2.instruction();

    if (!(t1.state() instanceof TraceElement.RegState &&
        t2.state() instanceof TraceElement.RegState)) {
      return;
    }

    // RS1 Comparison
    if (i1.hasRS1() && i2.hasRS1()) {
      long t1_rs1_val = this.timeline1.get(t1.time(), i1.rs1());
      long t2_rs1_val = this.timeline2.get(t2.time(), i2.rs1());

      if (!Objects.equals(t1_rs1_val, t2_rs1_val)) {
        obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.REG_RS1));
        obs2.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.REG_RS1));
      }
    }

    // RS2 Comparison
    if (i1.hasRS2() && i2.hasRS2()) {
      long t1_rs2_val = this.timeline1.get(t1.time(), i1.rs2());
      long t2_rs2_val = this.timeline2.get(t2.time(), i2.rs2());

      if (!Objects.equals(t1_rs2_val, t2_rs2_val)) {
        obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.REG_RS2));
        obs2.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.REG_RS2));
      }
    }

    // RD Comparison
    if (i1.hasRD() && i2.hasRD()) {
      long t1_rd_val = this.timeline1.get(t1.time(), i1.rd());
      long t2_rd_val = this.timeline2.get(t2.time(), i2.rd());

      if (!Objects.equals(t1_rd_val, t2_rd_val)) {
        obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.REG_RD));
        obs2.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.REG_RD));
      }
    }
  }

  private void compareMemory(int idx) {
    TraceElement t1 = this.traces1.get(idx);
    TraceElement t2 = this.traces2.get(idx);
    RISCVInstruction i1 = t1.instruction();
    RISCVInstruction i2 = t2.instruction();

    boolean isMem1 = t1.state() instanceof TraceElement.MemState || t1.state() instanceof TraceElement.LoadState;
    boolean isMem2 = t2.state() instanceof TraceElement.MemState || t2.state() instanceof TraceElement.LoadState;

    if (!isMem1 || !isMem2)
      return;

    long addr1 = (t1.state() instanceof TraceElement.MemState m) ? m.addr()
        : ((TraceElement.LoadState) t1.state()).addr();
    long addr2 = (t2.state() instanceof TraceElement.MemState m) ? m.addr()
        : ((TraceElement.LoadState) t2.state()).addr();

    // 1. Compare Address
    if (addr1 != addr2) {
      obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.MEM_ADDR));
      obs2.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.MEM_ADDR));
    }

    // 2. Compare Alignment
    if (isWordAligned(addr1) != isWordAligned(addr2)) {
      obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.IS_ALIGNED));
      obs2.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.IS_ALIGNED));
    }

    if (isHalfAligned(addr1) != isHalfAligned(addr2)) {
      obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.IS_HALF_ALIGNED));
      obs2.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.IS_HALF_ALIGNED));
    }

    // 3. Compare Data
    long val1 = (t1.state() instanceof TraceElement.MemState m) ? m.val() : ((TraceElement.LoadState) t1.state()).val();
    long val2 = (t2.state() instanceof TraceElement.MemState m) ? m.val() : ((TraceElement.LoadState) t2.state()).val();

    if (val1 != val2) {
      RISCV_OBSERVATION_TYPE type = (t1.state() instanceof TraceElement.LoadState) ? RISCV_OBSERVATION_TYPE.MEM_R_DATA
          : RISCV_OBSERVATION_TYPE.MEM_W_DATA;
      obs1.add(new RISCVObservation(i1.type(), type));
      obs2.add(new RISCVObservation(i2.type(), type));
    }
  }

  private boolean isWordAligned(long addr) {
    return (addr & 0x3) == 0;
  }

  private boolean isHalfAligned(long addr) {
    return (addr & 0x1) == 0;
  }

  private static boolean isBranch(RISCV_TYPE type) {
    return switch (type) {
      case JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU -> true;
      default -> false;
    };
  }

  private static boolean isBranchTaken(RISCV_TYPE type, long rs1, long rs2) {
    if (type == RISCV_TYPE.JAL || type == RISCV_TYPE.JALR)
      return true;

    return switch (type) {
      case BEQ -> (rs1 == rs2);
      case BNE -> (rs1 != rs2);
      case BLT -> (rs1 < rs2);
      case BGE -> (rs1 >= rs2);
      case BLTU -> (Long.compareUnsigned(rs1, rs2) < 0);
      case BGEU -> (Long.compareUnsigned(rs1, rs2) >= 0);
      default -> false;
    };
  }

  private void compareBranch(int idx) {
    TraceElement t1 = this.traces1.get(idx);
    TraceElement t2 = this.traces2.get(idx);
    RISCVInstruction i1 = t1.instruction();
    RISCVInstruction i2 = t2.instruction();

    boolean isB1 = isBranch(i1.type());
    boolean isB2 = isBranch(i2.type());

    // 1. Compare branch identification
    if (isB1 != isB2) {
      obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.IS_BRANCH));
      obs2.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.IS_BRANCH));
    }

    // Guard clause: Only compare outcomes if both are actually branches
    if (!isB1 || !isB2) {
      return;
    }

    // 2. Compare branch outcome
    if (i1.hasRS1() && i2.hasRS1() && i1.hasRS2() && i2.hasRS2()) {
      long t1_rs1 = this.timeline1.get(t1.time(), i1.rs1());
      long t2_rs1 = this.timeline2.get(t2.time(), i2.rs1());
      long t1_rs2 = this.timeline1.get(t1.time(), i1.rs2());
      long t2_rs2 = this.timeline2.get(t2.time(), i2.rs2());

      boolean taken1 = isBranchTaken(i1.type(), t1_rs1, t1_rs2);
      boolean taken2 = isBranchTaken(i2.type(), t2_rs1, t2_rs2);

      if (taken1 != taken2) {
        obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.BRANCH_TAKEN));
        obs2.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.BRANCH_TAKEN));
      }
    }

    // TODO: there's some logic for new_pc, or rather no logic. Check it out later.
  }

  private void compareDependencies(int idx, int distance) {
    int prevIdx = idx - distance;
    if (prevIdx < 0)
      return;

    TraceElement current1 = this.traces1.get(idx);
    TraceElement current2 = this.traces2.get(idx);
    TraceElement prev1 = this.traces1.get(prevIdx);
    TraceElement prev2 = this.traces2.get(prevIdx);

    RISCVInstruction i1 = current1.instruction();
    RISCVInstruction i2 = current2.instruction();
    RISCVInstruction pi1 = prev1.instruction();
    RISCVInstruction pi2 = prev2.instruction();

    // Check RAW on RS1
    if (i1.hasRS1() && i2.hasRS1() && pi1.hasRD() && pi2.hasRD()) {
      boolean match1 = i1.rs1() == pi1.rd();
      boolean match2 = i2.rs1() == pi2.rd();

      if (match1 != match2) {
        RISCV_TYPE culprit = match1 ? pi1.type() : pi2.type();
        obs1.add(new RISCVObservation(culprit, getDependencyObservationType(DEPENDENCY.RAW_RS1, distance)));
        obs2.add(new RISCVObservation(culprit, getDependencyObservationType(DEPENDENCY.RAW_RS1, distance)));
      }
    }

    // Check RAW on RS2
    if (i1.hasRS2() && i2.hasRS2() && pi1.hasRD() && pi2.hasRD()) {
      boolean match1 = i1.rs2() == pi1.rd();
      boolean match2 = i2.rs2() == pi2.rd();

      if (match1 != match2) {
        RISCV_TYPE culprit = match1 ? pi1.type() : pi2.type();
        obs1.add(new RISCVObservation(culprit, getDependencyObservationType(DEPENDENCY.RAW_RS2, distance)));
        obs2.add(new RISCVObservation(culprit, getDependencyObservationType(DEPENDENCY.RAW_RS2, distance)));
      }
    }

    // Check WAW on RD
    if (i1.hasRD() && i2.hasRD() && pi1.hasRD() && pi2.hasRD()) {
      boolean match1 = i1.rd() == pi1.rd();
      boolean match2 = i2.rd() == pi2.rd();

      if (match1 != match2) {
        RISCV_TYPE culprit = match1 ? pi1.type() : pi2.type();
        obs1.add(new RISCVObservation(culprit, getDependencyObservationType(DEPENDENCY.WAW, distance)));
        obs2.add(new RISCVObservation(culprit, getDependencyObservationType(DEPENDENCY.WAW, distance)));
      }
    }
  }

  private enum DEPENDENCY {
    /**
     * Read-After-Write on RS1.
     */
    RAW_RS1,
    /**
     * Read-After-Write on RS2.
     */
    RAW_RS2,
    /**
     * Write-After-Write
     */
    WAW
  }

  /**
   * @param dep      the dependency
   * @param distance the distance
   * @return the respective observation.
   */
  private RISCV_OBSERVATION_TYPE getDependencyObservationType(DEPENDENCY dep, int distance) {
    switch (dep) {
      case RAW_RS1 -> {
        if (distance == 1)
          return RISCV_OBSERVATION_TYPE.RAW_RS1_1;
        if (distance == 2)
          return RISCV_OBSERVATION_TYPE.RAW_RS1_2;
        if (distance == 3)
          return RISCV_OBSERVATION_TYPE.RAW_RS1_3;
        if (distance == 4)
          return RISCV_OBSERVATION_TYPE.RAW_RS1_4;
      }
      case RAW_RS2 -> {
        if (distance == 1)
          return RISCV_OBSERVATION_TYPE.RAW_RS2_1;
        if (distance == 2)
          return RISCV_OBSERVATION_TYPE.RAW_RS2_2;
        if (distance == 3)
          return RISCV_OBSERVATION_TYPE.RAW_RS2_3;
        if (distance == 4)
          return RISCV_OBSERVATION_TYPE.RAW_RS2_4;
      }
      case WAW -> {
        if (distance == 1)
          return RISCV_OBSERVATION_TYPE.WAW_1;
        if (distance == 2)
          return RISCV_OBSERVATION_TYPE.WAW_2;
        if (distance == 3)
          return RISCV_OBSERVATION_TYPE.WAW_3;
        if (distance == 4)
          return RISCV_OBSERVATION_TYPE.WAW_4;
      }
    }
    return null;
  }

  /*
   * Checks sections of code for observation types which span more than one
   * instruction;
   * currently only for syscall observation types
   */
  private void compareSections(int idx) {
    if (idx < 8)
      return;
    TraceElement t1 = this.traces1.get(idx);
    TraceElement t2 = this.traces2.get(idx);

    RISCVInstruction i1 = t1.instruction();
    RISCVInstruction i2 = t2.instruction();
    if (!(Objects.equals(i1.type(), RISCV_TYPE.ECALL) || Objects.equals(i2.type(), RISCV_TYPE.ECALL)))
      return;

    // check the ecall args
    if (Objects.equals(i1.type(), RISCV_TYPE.ECALL) != Objects.equals(i2.type(), RISCV_TYPE.ECALL)) {
      obs1.add(new RISCVObservation(i1.type(), RISCV_OBSERVATION_TYPE.IS_SYSCALL));
      obs1.add(new RISCVObservation(i2.type(), RISCV_OBSERVATION_TYPE.IS_SYSCALL));
      return;
    }

    for (int i = 0; i < 6; i++) {
      // extract syscall arg i
      int instructionIdx = idx - 8 + i;
      TraceElement argt1 = this.traces1.get(instructionIdx);
      TraceElement argt2 = this.traces2.get(instructionIdx);
      RISCVInstruction argi1 = argt1.instruction();
      RISCVInstruction argi2 = argt2.instruction();

      if (argi1.imm() != argi2.imm()) {
        // Map i (0 to 5) to the corresponding syscall arg enum value (34 to 39)
        int enumValue = 34 + i;
        RISCV_OBSERVATION_TYPE argType = RISCV_OBSERVATION_TYPE.fromValue(enumValue); // or equivalent lookup method

        obs1.add(new RISCVObservation(RISCV_TYPE.ECALL, argType));
        obs2.add(new RISCVObservation(RISCV_TYPE.ECALL, argType));
      }
    }

    // extract syscall type observation
    int syscallTypeIdx = idx - 1;
    TraceElement typet1 = this.traces1.get(syscallTypeIdx);
    TraceElement typet2 = this.traces2.get(syscallTypeIdx);
    RISCVInstruction typei1 = typet1.instruction();
    RISCVInstruction typei2 = typet2.instruction();
    if (typei1.imm() != typei2.imm()) {
      obs1.add(new RISCVObservation(RISCV_TYPE.ECALL, RISCV_OBSERVATION_TYPE.SYSCALL_TYPE));
      obs1.add(new RISCVObservation(RISCV_TYPE.ECALL, RISCV_OBSERVATION_TYPE.SYSCALL_TYPE));
    }

  }

}
