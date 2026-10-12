package contractgen.riscv.os.extractor;

import java.util.Arrays;

import contractgen.riscv.isa.RISCVInstruction;

public class InstructionParser {

  public static boolean error = false;

  public static TraceElement parseSpikeLog(long time, String line) {
    if (line == null || line.isBlank()) {
      return null;
    }

    String[] words = line.trim().split("\\s+");

    // Spike instruction lines have:
    // core 0: 0xPC (0xINSTRUCTION) ...
    //
    // Commit/event lines have:
    // core 0: 3 0xPC (0xINSTRUCTION) ...
    //
    // We only care about the first four/five fields needed to distinguish them.
    if (isInstructionLine(words)) {
      long pc = parseHex(words[2]);
      try {

        RISCVInstruction instruction = parseInstruction(words[3]);
        return new TraceElement(time, pc, instruction, null);
      } catch (Exception e) {
        error = true;
        return null;
      }

    }

    // Exception lines do not contain an instruction encoding.
    //
    // Example:
    // core 0: exception trap_user_ecall, epc 0x80100010
    if (isExceptionLine(words)) {
      String cause = words[3].replace(",", "");
      long epc = parseHex(words[5]);

      return TraceElement.trap(
          time,
          epc,
          null,
          new TraceElement.TrapState(cause, epc, null));
    }

    // Some exceptions spill information such as tval onto another line.
    // We intentionally ignore those continuation lines for now.
    // TODO: preserve tval when multi-line exception parsing is implemented.
    if (containsTval(words)) {
      return null;
    }

    // Spike emits marker lines such as:
    // core 0: >>>> main_trap_handler.
    //
    // These do not represent an instruction or state change, so ignore them.
    if (containsMarker(words)) {
      return null;
    }

    // Everything remaining is treated as a commit/event line.
    if (isCommitLine(words)) {
      return parseCommitLine(time, words);
    }

    // Unknown/uninteresting Spike output.
    return null;
  }

  private static TraceElement parseCommitLine(long time, String[] words) {
    long pc = parseHex(words[3]);
    RISCVInstruction instruction = null;

    // No state change.
    //
    // Example:
    // core 0: 3 0x00001010 (0x00028067)
    if (words.length < 6) {
      return TraceElement.nostate(
          time,
          pc,
          instruction,
          new TraceElement.NoState());
    }

    String update = words[5];

    // CSR update.
    //
    // Example:
    // core 0: 3 0x80001ac8 (0x30200073)
    // c768_mstatus 0x00000088 c784_mstatush 0x00000000
    if (update.startsWith("c")) {
      long value = parseHex(words[6]);

      return TraceElement.csr(
          time,
          pc,
          instruction,
          new TraceElement.CSRState(update, value));
    }

    // Store update.
    //
    // Example:
    // ... mem 0x00001018 0x00000001
    if ("mem".equals(update)) {
      long address = parseHex(words[6]);

      long value = words.length > 7
          ? parseHex(words[7])
          : 0;

      return TraceElement.store(
          time,
          pc,
          instruction,
          new TraceElement.MemState(address, value));
    }

    // Load update.
    //
    // Example:
    // ... x5 0x80000000 mem 0x00001018
    if (containsMem(words)) {
      int reg = parseRegister(update);
      long value = parseHex(words[6]);
      long address = parseHex(words[8]);

      return TraceElement.load(
          time,
          pc,
          instruction,
          new TraceElement.LoadState(address, reg, value));
    }

    // Register update.
    //
    // Example:
    // ... x5 0x00001000
    int reg = parseRegister(update);
    long value = parseHex(words[6]);

    return TraceElement.alu(
        time,
        pc,
        instruction,
        new TraceElement.RegState(reg, value));
  }

  private static boolean isInstructionLine(String[] words) {
    return words.length >= 4
        && words[2].startsWith("0x")
        && words[3].startsWith("(0x");
  }

  private static boolean isCommitLine(String[] words) {
    return words.length >= 5
        && words[3].startsWith("0x")
        && words[4].startsWith("(0x");
  }

  private static boolean isExceptionLine(String[] words) {
    return words.length >= 6
        && "exception".equals(words[2]);
  }

  private static boolean containsTval(String[] words) {
    for (String word : words) {
      if ("tval".equals(word)) {
        return true;
      }
    }

    return false;
  }

  private static boolean containsMarker(String[] words) {
    for (String word : words) {
      if (word.startsWith(">>>>")) {
        return true;
      }
    }

    return false;
  }

  private static boolean containsMem(String[] words) {
    for (String word : words) {
      if ("mem".equals(word)) {
        return true;
      }
    }

    return false;
  }

  private static RISCVInstruction parseInstruction(String word) {
    return RISCVInstruction.parseHexString(
        word.replace("(", "").replace(")", ""));
  }

  private static long parseHex(String value) {
    return Long.parseUnsignedLong(
        value.replace("0x", ""),
        16);
  }

  private static int parseRegister(String value) {
    return Integer.parseInt(value.replace("x", ""));
  }
}
