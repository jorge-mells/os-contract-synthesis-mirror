package contractgen.riscv.os.tests;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import contractgen.TestCase;
import contractgen.TestCases;
import contractgen.riscv.isa.RISCVInstruction;
import contractgen.riscv.isa.RISCVProgram;
import contractgen.riscv.isa.RISCVTestCase;

public class RISCVOSSimpleTestCases extends TestCases {

  public RISCVOSSimpleTestCases(boolean parallelize) {
    super(createTestCases(parallelize), 2);
  }

  private static List<Iterator<TestCase>> createTestCases(boolean parallelize) {
    RISCVProgram p1 = new RISCVOSProgram(new HashMap<>(),
        List.of(RISCVInstruction.ADD(5, 6, 7), RISCVInstruction.ADD(5, 6, 7)));
    RISCVProgram p2 = new RISCVOSProgram(new HashMap<>(),
        List.of(RISCVInstruction.MUL(5, 6, 7), RISCVInstruction.MUL(5, 6, 7)));
    TestCase tc1 = new RISCVTestCase(p1, p2, 3, 0);

    RISCVProgram p3 = new RISCVOSProgram(new HashMap<>(),
        List.of(RISCVInstruction.ADD(5, 6, 7), RISCVInstruction.MUL(5, 6, 7)));
    RISCVProgram p4 = new RISCVOSProgram(new HashMap<>(),
        List.of(RISCVInstruction.MUL(5, 6, 7), RISCVInstruction.ADD(5, 6, 7)));
    TestCase tc2 = new RISCVTestCase(p3, p4, 3, 0);

    if (parallelize) {
      return List.of(
          List.of(tc1).iterator(),
          List.of(tc2).iterator());
    } else {
      return List.of(
          List.of(tc2, tc1).iterator());
    }
  }

}
