package contractgen.riscv.ibexos;

import static contractgen.util.FileUtils.copyFileOrFolder;
import static contractgen.util.ScriptUtils.runScript;
import static java.nio.file.StandardCopyOption.REPLACE_EXISTING;

import java.io.IOException;
import java.nio.file.Path;

import contractgen.MARCH;
import contractgen.SIMULATION_RESULT;
import contractgen.TestCase;
import contractgen.TestCases;
import contractgen.TestResult;
import contractgen.Updater;
import contractgen.riscv.isa.RISCV;
import contractgen.riscv.os.extractor.RVFIOSExtractor;
import contractgen.util.Pair;

public class IBEXOS extends MARCH {

  private static final String BASE_PATH = System.getProperty("user.dir") + "/src/main/resources/os/";

  public IBEXOS(Updater updater, TestCases testCases) {
    super(new RISCV(updater, testCases), new RVFIOSExtractor());
  }

  @Override
  public void generateSources(TestCase testCase, Integer max_count) {
    throw new UnsupportedOperationException("Unimplemented method 'generateSources'");
  }

  @Override
  public String runCover(int steps) {
    throw new UnsupportedOperationException("Unimplemented method 'runCover'");
  }

  @Override
  public int extractSteps(String coverTrace) {
    throw new UnsupportedOperationException("Unimplemented method 'extractSteps'");
  }

  @Override
  public boolean run(int steps) {
    throw new UnsupportedOperationException("Unimplemented method 'run'");
  }

  @Override
  public Pair<TestResult, TestResult> extractCTX(TestCase testCase) {
    return extractCTX(0, testCase);
  }

  @Override
  public Pair<TestResult, TestResult> extractCTX(int id, TestCase testCase) {
    // WARN: I am assuming each test case precisely targets a contract atom
    // otherwise, I'm not sure how it would work out
    // WARN: Actually, we just assume our contracts are imprecise, because it is
    // not so obvious how we'd actually clean them up.

    // run spike simulations
    String command = "%s/runner.sh run_spike %s %s 2>&1".formatted(BASE_PATH, BASE_PATH, id);
    runScript(command, true, 100);
    String output1 = BASE_PATH + "spike-outputs1_" + id + "/spike.log";
    String output2 = BASE_PATH + "spike-outputs2_" + id + "/spike.log";
    String paths = output1 + ":" + output2;
    // String debugcmd = "%s/runner.sh print_spike %s %s 2>&1".formatted(BASE_PATH,
    // BASE_PATH, id);

    var results = getExtractor().extractResults(paths, true, testCase.getIndex());
    // System.out.println("[" + Thread.currentThread().getName() + "] " + results);
    String command2 = "%s/runner.sh cleanup %s %s 2>&1".formatted(BASE_PATH, BASE_PATH, id);
    runScript(command2, true, 100);
    return results;

  }

  @Override
  public Pair<TestResult, TestResult> extractDifferences(int index) {
    return extractDifferences(0, index);
  }

  @Override
  public Pair<TestResult, TestResult> extractDifferences(int id, int index) {
    // run spike simulations
    String command = "%s/runner.sh run_spike %s %s 2>&1".formatted(BASE_PATH, BASE_PATH, id);
    runScript(command, true, 100);
    String output1 = BASE_PATH + "spike-outputs1_" + id + "/spike.log";
    String output2 = BASE_PATH + "spike-outputs2_" + id + "/spike.log";
    String paths = output1 + ":" + output2;

    var results = getExtractor().extractResults(paths, false, index);
    // System.out.println("[" + Thread.currentThread().getName() + "] " + results);
    String command2 = "%s/runner.sh cleanup %s %s 2>&1".formatted(BASE_PATH, BASE_PATH, id);
    runScript(command2, true, 100);
    return results;
  }

  @Override
  public void compile() {
    String command = "%s/runner.sh compile %s 2>&1".formatted(BASE_PATH, BASE_PATH);
    runScript(command, true, 100);
  }

  @Override
  public void writeTestCase(TestCase testCase) {
    writeTestCase(0, testCase);
  }

  @Override
  public void writeTestCase(int id, TestCase testCase) {
    try {
      copyFileOrFolder(Path.of(BASE_PATH + "program-templates/").toFile(),
          Path.of(BASE_PATH + "test-programs1_" + id + "/").toFile(), REPLACE_EXISTING);
      copyFileOrFolder(Path.of(BASE_PATH + "program-templates/").toFile(),
          Path.of(BASE_PATH + "test-programs2_" + id + "/").toFile(), REPLACE_EXISTING);
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
    testCase.getProgram1().printInstr(BASE_PATH + "test-programs1_" + id + "/vic.s");
    testCase.getProgram2().printInstr(BASE_PATH + "test-programs2_" + id + "/vic.s");
    // add testcases to final binary
    String command = "%s/runner.sh add_to_binary %s %s".formatted(BASE_PATH, BASE_PATH, id);
    runScript(command, true, 100);
  }

  @Override
  public SIMULATION_RESULT simulate() {
    return simulate(0);
  }

  @Override
  public SIMULATION_RESULT simulate(int id) {
    // NOTE: try fast-run-no-check and see if it makes a difference later
    String command1 = "%s/runner.sh simulate_ibex %s %s".formatted(BASE_PATH,
        BASE_PATH, id);
    String result = runScript(command1, true, 100);
    // command1 = "%s/runner.sh debug %s %s".formatted(BASE_PATH,
    // BASE_PATH, id);
    // runScript(command1, false, 100);

    // FIX: be sure to cleanup afterwards so as to not clutter up space.
    if (result.contains("attacker distinguishable")) {
      return SIMULATION_RESULT.FAIL;
    } else {
      return SIMULATION_RESULT.SUCCESS;
    }
  }

  @Override
  public String getName() {
    return "ibexos";
  }

}
