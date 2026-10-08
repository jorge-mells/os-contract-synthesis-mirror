package contractgen.riscv.os.extractor;

import java.util.List;
import java.util.HashMap;
import java.util.Map;

// NOTE: this makes the assumption that the regfile starts with everything initialised to 0!!!!
// OPTIMIZE: space complexity here is pretty bad.
public class RegisterTimeline {
  // Maps time -> copy of the register file at that time
  private final Map<Long, long[]> timeline = new HashMap<>();
  private final long[] currentRegs = new long[32];

  public RegisterTimeline(List<TraceElement> trace) {
    for (TraceElement te : trace) {
      // Apply change to the current state
      if (te.state() instanceof TraceElement.RegState rs) {
        if (rs.reg() > 0 && rs.reg() < 32) {
          currentRegs[rs.reg()] = rs.val();
        }
      }
      // Snapshot the state at this time
      timeline.put(te.time(), currentRegs.clone());
    }
  }

  public long get(long time, int reg) {
    long[] regsAtTime = timeline.get(time);
    if (regsAtTime == null)
      throw new IllegalArgumentException("No state at time " + time);
    return regsAtTime[reg];
  }
}
