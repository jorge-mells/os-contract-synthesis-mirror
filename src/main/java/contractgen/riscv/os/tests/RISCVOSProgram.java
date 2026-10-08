package contractgen.riscv.os.tests;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Map;

import contractgen.riscv.isa.RISCVInstruction;
import contractgen.riscv.isa.RISCVProgram;

public class RISCVOSProgram extends RISCVProgram {

  public RISCVOSProgram(Map<Integer, Integer> registers, List<RISCVInstruction> program) {
    super(registers, program);
  }

  @Override
  public void printInstr(String path) {
    StringBuilder sb = new StringBuilder();
    for (RISCVInstruction instruction : this.program) {
      sb.append(".word ");
      sb.append("0x");
      sb.append(instruction.toHexEncoding());
      sb.append("\n");
    }
    try {
      String template = Files.readString(Path.of(path));
      String updated;

      if (template.contains("start") && template.contains("end")) {
        String replacement = "$1\n" + sb.toString() + "\n$2";
        updated = template.replaceFirst("(?s)(# start).*?(# end)", replacement);
      } else {
        // Fallback: if markers don't exist, just append or put the whole thing at the
        // end
        updated = template + "\n" + sb.toString();
      }

      Files.write(Path.of(path), updated.getBytes());
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
  }
}
