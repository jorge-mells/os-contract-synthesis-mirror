package contractgen.riscv.os.extractor;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;

public class TraceParser {

  private final String filePath;
  private long time;

  public TraceParser(String filePath, long initialTime) {
    this.filePath = filePath;
    this.time = initialTime;
  }

  public List<TraceElement> parseUserSections() throws IOException {
    List<TraceElement> allElements = new ArrayList<>();

    int startLine = -1;
    int currentLineNumber = 0;

    try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
      String line;

      while ((line = reader.readLine()) != null) {
        currentLineNumber++;
        String trimmed = line.trim();

        // Start of a user section.
        if (startLine == -1
            && trimmed.contains("mret")
            && !trimmed.contains(">>>")) {

          startLine = currentLineNumber + 2; // mret is not a user instruction
        }

        // End of a user section.
        else if (startLine != -1
            && trimmed.contains("exception")
            && !trimmed.contains(">>>")) {

          // The exception itself is not part of the section.
          // The two lines immediately before it are also excluded,
          // matching the original section extraction logic.
          int endLine = currentLineNumber;

          if (endLine >= startLine) {
            allElements.addAll(parseSection(startLine, endLine));
          }

          startLine = -1;
        }
      }
    }

    // Handle a section which reaches the end of the file.
    if (startLine != -1) {
      allElements.addAll(parseSection(startLine, Integer.MAX_VALUE));
    }

    return allElements;
  }

  private List<TraceElement> parseSection(
      int start,
      int end) throws IOException {

    List<TraceElement> elements = new ArrayList<>();

    try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
      String line;
      int currentLineNumber = 0;

      while ((line = reader.readLine()) != null) {
        currentLineNumber++;

        if (currentLineNumber < start) {
          continue;
        }

        if (currentLineNumber > end) {
          break;
        }

        TraceElement element = InstructionParser.parseSpikeLog(0, line);

        if (element != null) {
          elements.add(element);
        }
        if (InstructionParser.error) {
          List<String> lines = Files.readAllLines(Path.of(filePath));
          System.out.println(lines.subList(start - 1, end));
          System.exit(1);
        }
      }
    }

    return consolidate(elements);
  }

  private List<TraceElement> consolidate(
      List<TraceElement> elements) {

    List<TraceElement> result = new ArrayList<>();

    TraceElement currentInstruction = null;
    TraceElement.State currentState = null;

    for (TraceElement element : elements) {

      // Every group starts with an instruction.
      if (element.instruction() != null) {

        // Finish the previous instruction.
        if (currentInstruction != null) {
          result.add(withState(
              currentInstruction,
              currentState,
              time));

          time++;
        }

        currentInstruction = element;
        currentState = null;
        continue;
      }

      // Every non-instruction element belongs to the
      // closest preceding instruction.
      if (currentInstruction == null) {
        throw new IllegalStateException(
            "Found trace element without a preceding instruction");
      }

      TraceElement.State state = element.state();

      // No state means there is nothing to add.
      if (state == null) {
        continue;
      }

      // All state-bearing elements belonging to one instruction
      // must have the same state type.
      // if (currentState != null
      // && currentState.getClass() != state.getClass()) {
      //
      // throw new IllegalStateException(
      // "Instruction" + currentInstruction.instruction().toString() + "at 0x"
      // + Long.toHexString(currentInstruction.pc())
      // + " has incompatible state types: "
      // + currentState.getClass().getSimpleName()
      // + " followed by "
      // + state.getClass().getSimpleName());
      // }

      // ignore all subsequent states
      if (currentState == null) {
        currentState = state;
      }
    }

    // Flush the final instruction.
    if (currentInstruction != null) {
      result.add(withState(
          currentInstruction,
          currentState,
          time));

      time++;
    }

    return result;
  }

  private static TraceElement withState(
      TraceElement instruction,
      TraceElement.State state,
      long time) {

    if (state == null) {
      return TraceElement.nostate(
          time,
          instruction.pc(),
          instruction.instruction(),
          new TraceElement.NoState());
    }

    return new TraceElement(
        time,
        instruction.pc(),
        instruction.instruction(),
        state);
  }
}
