package contractgen.riscv.isa.tests;

import contractgen.TestCase;
import contractgen.riscv.isa.*;
import contractgen.riscv.isa.contract.RISCVTestResult;
import contractgen.riscv.isa.contract.RISCVObservation;
import contractgen.riscv.isa.contract.RISCV_OBSERVATION_TYPE;
import contractgen.riscv.os.tests.RISCVOSProgram;
import contractgen.util.Pair;
import contractgen.util.StringUtils;

import java.util.*;
import java.util.stream.Stream;

/**
 * A generator for RISC-V test cases.
 */
public class RISCVTestGenerator {
    /**
     * The number of registers in the microarchitecture.
     */
    private static final int NUMBER_REGISTERS = 32;
    /**
     * The maximum value of a register.
     */
    private static final long MAX_REG = 4294967296L;
    /**
     * The maximum value of a I-type immediate.
     */
    private static final int MAX_IMM_I = 4096; // 2^12
    /**
     * The maximum value of a B-type immediate.
     */
    private static final long MAX_IMM_B = 8192; // 2^13
    /**
     * The maximum value of a U-type immediate.
     */
    private static final long MAX_IMM_U = 4294967296L; // 2^32
    /**
     * The maximum value of a J-type immediate.
     */
    private static final long MAX_IMM_J = 2097152; // 2^21

    /**
     * A pseudo-random number generator.
     */
    private final Random r;
    /**
     * The number of repetitions to be generated.
     */
    private final int repetitions;
    /**
     * The instruction types to be considered.
     */
    private final List<RISCV_TYPE> types;
    /**
     * The allowed observations.
     */
    private final List<RISCV_OBSERVATION_TYPE> allowed_observations;

    private boolean isOS = false;
    private int[] syscallSpecs = { /* 64 , */ 93, 124 }; // picked from the os code; will later be
    // updated to be dynamic(i.e: passed
    // through config or something)

    /**
     * @param subsets     the allowed ISA subsets.
     * @param seed        the random seed.
     * @param repetitions the number of repetitions to be generated.
     */
    RISCVTestGenerator(Set<RISCV_SUBSET> subsets, long seed, int repetitions) {
        if (subsets.contains(RISCV_SUBSET.ZICSR)) {
            throw new RuntimeException("invalid subset. Cannot construct tests from " + RISCV_SUBSET.ZICSR);
        }
        r = new Random(seed);
        this.repetitions = repetitions;
        this.types = Arrays.stream(RISCV_TYPE.values())
                .filter(t -> subsets.contains(t.getSubset())).toList();
        this.allowed_observations = Arrays.stream(RISCV_OBSERVATION_TYPE.values()).toList();
    }

    /**
     * @param subsets              the allowed ISA subsets.
     * @param allowed_observations the allowed observations.
     * @param seed                 the random seed.
     * @param repetitions          the number of repetitions to be generated.
     */
    RISCVTestGenerator(Set<RISCV_SUBSET> subsets, Set<RISCV_OBSERVATION_TYPE> allowed_observations, long seed,
            int repetitions) {
        if (subsets.contains(RISCV_SUBSET.ZICSR)) {
            throw new RuntimeException("invalid subset. Cannot construct tests from " + RISCV_SUBSET.ZICSR);
        }
        r = new Random(seed);
        this.repetitions = repetitions;
        this.types = Arrays.stream(RISCV_TYPE.values()).filter(t -> subsets.contains(t.getSubset())).toList();
        this.allowed_observations = allowed_observations.stream().toList();
    }

    RISCVTestGenerator(Set<RISCV_SUBSET> subsets, Set<RISCV_OBSERVATION_TYPE> allowed_observations, long seed,
            int repetitions, boolean isOS) {
        r = new Random(seed);
        if (subsets.contains(RISCV_SUBSET.ZICSR)) {
            throw new RuntimeException("invalid subset. Cannot construct tests from " + RISCV_SUBSET.ZICSR);
        }
        this.isOS = true;
        this.repetitions = repetitions;
        this.types = Arrays.stream(RISCV_TYPE.values()).filter(t -> subsets.contains(t.getSubset())).toList();
        this.allowed_observations = allowed_observations.stream().toList();
    }

    /**
     * Generates a list of test cases by generating a random initial valuation for
     * each register,
     * a random instruction of a certain type and suffix to be appended to the
     * program.
     *
     * @return The list of test cases.
     */
    public List<TestCase> generate() {
        List<TestCase> cases = new ArrayList<>();
        for (int i = 0; i < repetitions; i++) {
            cases.addAll(nextRepetition(cases.size()));
            // if (i % 3 == 0)
            // cases.add(new RISCVTestCase(new RISCVProgram(randomRegisters(),
            // randomSequence(r.nextInt(5, 50))), new RISCVProgram(randomRegisters(),
            // randomSequence(r.nextInt(5, 50))), 50, index++));
        }
        return cases;

    }

    /**
     * @param index starting index
     * @return a list of test cases.
     */
    public List<TestCase> nextRepetition(int index) {
        List<TestCase> cases = new ArrayList<>();
        for (RISCV_TYPE type : types) {
            Map<Integer, Integer> registers = randomRegisters();
            List<RISCVInstruction> suffix = randomSequence(r.nextInt(5, 25));
            List<RISCVInstruction> instructionBlock = randomInstructionBlockFromType(type);
            for (RISCV_OBSERVATION_TYPE observation : allowed_observations) {
                Pair<List<RISCVInstruction>, List<RISCVInstruction>> prefix = alterObservation(observation,
                        instructionBlock);
                if (prefix != null) {
                    // cases.add(new RISCVTestCase(
                    // new RISCVProgram(registers, Stream.concat(prefix.left().stream(),
                    // suffix.stream()).toList()),
                    // new RISCVProgram(registers, Stream.concat(prefix.right().stream(),
                    // suffix.stream()).toList()),
                    // Integer.max(prefix.left().size() + suffix.size(), prefix.right().size() +
                    // suffix.size()), new RISCVTestResult(Set.of(new RISCVObservation(type,
                    // observation)), true, index), index++));

                    cases.add(new RISCVTestCase(
                            this.isOS
                                    ? new RISCVOSProgram(registers,
                                            Stream.concat(prefix.left().stream(), suffix.stream()).toList())
                                    : new RISCVProgram(registers,
                                            Stream.concat(prefix.left().stream(), suffix.stream()).toList()),
                            this.isOS
                                    ? new RISCVOSProgram(registers,
                                            Stream.concat(prefix.right().stream(), suffix.stream()).toList())
                                    : new RISCVProgram(registers,
                                            Stream.concat(prefix.right().stream(), suffix.stream()).toList()),
                            Integer.max(prefix.left().size() + suffix.size(), prefix.right().size() + suffix.size()),
                            new RISCVTestResult(Set.of(new RISCVObservation(type, observation)), true, index),
                            index++));
                }
            }
        }
        return cases;
    }

    /**
     * @return A random initialization for each register.
     */
    private Map<Integer, Integer> randomRegisters() {
        Map<Integer, Integer> result = new HashMap<>(NUMBER_REGISTERS - 1);
        for (int i = 1; i < NUMBER_REGISTERS; i++) {
            if (r.nextBoolean()) {
                result.put(i, r.nextInt(MAX_IMM_I));
            } else {
                if (r.nextBoolean()) {
                    result.put(i, null);
                }
            }
        }
        return result;
    }

    /**
     * Alters a given instruction to introduce a specific possible leakage. In most
     * cases this requires executing
     * different instructions before the relevant instruction to alter the
     * architectural state accordingly.
     *
     * @param type        The type of observation that should be triggered.
     * @param instruction The instruction to be altered.
     * @return A pair of sequences of instructions.
     */
    private Pair<List<RISCVInstruction>, List<RISCVInstruction>> alterObservation(RISCV_OBSERVATION_TYPE type,
            List<RISCVInstruction> instructionBlock) {
        var instruction = instructionBlock.getLast(); // alter last instruction in most cases
        return switch (type) {
            case TYPE, OPCODE, FUNCT5, FUNCT3 -> null; // new Pair<>(List.of(instruction), randomSequence(1));
            case RD -> {
                if (instruction.rd() == null)
                    yield null;
                RISCVInstruction ins1 = instruction.cloneAlteringRD(r.nextInt(1, NUMBER_REGISTERS));
                RISCVInstruction ins2 = instruction.cloneAlteringRD(r.nextInt(1, NUMBER_REGISTERS));
                yield new Pair<>(List.of(ins1), List.of(ins2));
            }
            case RS1 -> {
                if (instruction.rs1() == null)
                    yield null;
                RISCVInstruction ins1 = instruction.cloneAlteringRS1(r.nextInt(NUMBER_REGISTERS));
                RISCVInstruction ins2 = instruction.cloneAlteringRS1(r.nextInt(NUMBER_REGISTERS));
                yield new Pair<>(List.of(ins1), List.of(ins2));
            }
            case RS2 -> {
                if (instruction.rs2() == null)
                    yield null;
                RISCVInstruction ins1 = instruction.cloneAlteringRS2(r.nextInt(NUMBER_REGISTERS));
                RISCVInstruction ins2 = instruction.cloneAlteringRS2(r.nextInt(NUMBER_REGISTERS));
                yield new Pair<>(List.of(ins1), List.of(ins2));
            }
            case IMM -> {
                if (instruction.imm() == null || instruction.type().getFormat() == RISCV_FORMAT.SYSTEM)
                    yield null;
                RISCVInstruction ins1 = instruction.cloneAlteringIMM(r.nextLong(getBound(instruction)));
                RISCVInstruction ins2 = instruction.cloneAlteringIMM(r.nextLong(getBound(instruction)));
                yield new Pair<>(List.of(ins1), List.of(ins2));
            }
            case REG_RS1 -> {
                if (instruction.rs1() == null)
                    yield null;
                RISCVInstruction ins1 = RISCVInstruction.ADDI(instruction.rs1(), 0, r.nextLong(MAX_IMM_I));
                RISCVInstruction ins2 = RISCVInstruction.ADDI(instruction.rs1(), 0, r.nextLong(MAX_IMM_I));
                yield new Pair<>(List.of(ins1, instruction), List.of(ins2, instruction));
            }
            case REG_RS2 -> {
                if (instruction.rs2() == null)
                    yield null;
                RISCVInstruction ins1 = RISCVInstruction.ADDI(instruction.rs2(), 0, r.nextLong(MAX_IMM_I));
                RISCVInstruction ins2 = RISCVInstruction.ADDI(instruction.rs2(), 0, r.nextLong(MAX_IMM_I));
                yield new Pair<>(List.of(ins1, instruction), List.of(ins2, instruction));
            }
            case MEM_ADDR -> {
                if (instruction.rs1() == null)
                    yield null;
                long address = r.nextLong(MAX_IMM_I);
                RISCVInstruction val1 = RISCVInstruction.ADDI(31, 0, r.nextLong(MAX_IMM_I));
                RISCVInstruction val2 = RISCVInstruction.ADDI(30, 0, r.nextLong(MAX_IMM_I));
                RISCVInstruction instr_addr = RISCVInstruction.ADDI(instruction.rs1(), 0, address);
                RISCVInstruction ins1 = RISCVInstruction.SW(instruction.rs1(), 31, 0);
                RISCVInstruction ins2 = RISCVInstruction.SW(instruction.rs1(), 30, 0);
                yield new Pair<>(List.of(val1, val2, instr_addr, ins1, instruction),
                        List.of(val1, val2, instr_addr, ins2, instruction));
            }
            case MEM_W_DATA -> {
                if (instruction.rs2() == null)
                    yield null;
                long address = r.nextLong(MAX_IMM_I);
                RISCVInstruction val1 = RISCVInstruction.ADDI(31, 0, r.nextLong(MAX_IMM_I));
                RISCVInstruction val2 = RISCVInstruction.ADDI(30, 0, r.nextLong(MAX_IMM_I));
                RISCVInstruction instr_addr = RISCVInstruction.ADDI(instruction.rs2(), 0, address);
                RISCVInstruction ins1 = RISCVInstruction.SW(instruction.rs2(), 31, 0);
                RISCVInstruction ins2 = RISCVInstruction.SW(instruction.rs2(), 30, 0);
                yield new Pair<>(List.of(val1, val2, instr_addr, ins1, instruction),
                        List.of(val1, val2, instr_addr, ins2, instruction));
            }
            case REG_RD -> {
                if (instruction.rd() == null)
                    yield null;
                RISCVInstruction ins1 = instruction.cloneAlteringRD(r.nextInt(NUMBER_REGISTERS));
                RISCVInstruction ins2 = instruction.cloneAlteringRD(r.nextInt(NUMBER_REGISTERS));
                yield new Pair<>(List.of(ins1), List.of(ins2));
            }
            case MEM_R_DATA -> {
                if (instruction.imm() == null || instruction.rs1() == null)
                    yield null;
                long address = r.nextLong(MAX_IMM_I);
                RISCVInstruction val1 = RISCVInstruction.ADDI(31, 0, r.nextLong(MAX_IMM_I));
                RISCVInstruction val2 = RISCVInstruction.ADDI(30, 0, r.nextLong(MAX_IMM_I));
                RISCVInstruction instr_addr_1 = RISCVInstruction.ADDI(instruction.rs1(), 0, address);
                RISCVInstruction ins1 = RISCVInstruction.SW(instruction.rs1(), 31, 0);
                RISCVInstruction ins2 = RISCVInstruction.SW(instruction.rs1(), 30, 0);
                RISCVInstruction instr_addr_2 = RISCVInstruction.ADDI(instruction.rs1(), instruction.rs1(),
                        instruction.imm());
                yield new Pair<>(List.of(val1, val2, instr_addr_1, ins1, instr_addr_2, instruction),
                        List.of(val1, val2, instr_addr_1, ins2, instr_addr_2, instruction));
            }
            case IS_BRANCH -> null;
            case BRANCH_TAKEN -> null;
            case IS_ALIGNED -> null;
            case IS_HALF_ALIGNED -> null;
            case NEW_PC -> null;
            case RAW_RS1_2, RAW_RS1_3, RAW_RS1_4, RAW_RS2_2, RAW_RS2_3, RAW_RS2_4, WAW_2, WAW_3, WAW_4 -> null;
            case RAW_RS1_1 -> {
                if (instruction.rs1() == null)
                    yield null;
                RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rs1(), 0, 0);
                RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
                yield new Pair<>(List.of(add1, instruction), List.of(add2, instruction));
            }
            /*
             * case RAW_RS1_2 -> {
             * if (instruction.rs1() == null) yield null;
             * RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rs1(), 0, 0);
             * RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
             * RISCVInstruction nop = RISCVInstruction.NOP();
             * yield new Pair<>(List.of(add1, nop, instruction), List.of(add2, nop,
             * instruction));
             * }
             * case RAW_RS1_3 -> {
             * if (instruction.rs1() == null) yield null;
             * RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rs1(), 0, 0);
             * RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
             * RISCVInstruction nop = RISCVInstruction.NOP();
             * yield new Pair<>(List.of(add1, nop, nop, instruction), List.of(add2, nop,
             * nop, instruction));
             * }
             * case RAW_RS1_4 -> {
             * if (instruction.rs1() == null) yield null;
             * RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rs1(), 0, 0);
             * RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
             * RISCVInstruction nop = RISCVInstruction.NOP();
             * yield new Pair<>(List.of(add1, nop, nop, nop, instruction), List.of(add2,
             * nop, nop, nop, instruction));
             * }
             * 
             */
            case RAW_RS2_1 -> {
                if (instruction.rs2() == null)
                    yield null;
                RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rs2(), 0, 0);
                RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
                yield new Pair<>(List.of(add1, instruction), List.of(add2, instruction));
            }
            /*
             * case RAW_RS2_2 -> {
             * if (instruction.rs2() == null) yield null;
             * RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rs2(), 0, 0);
             * RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
             * RISCVInstruction nop = RISCVInstruction.NOP();
             * yield new Pair<>(List.of(add1, nop, instruction), List.of(add2, nop,
             * instruction));
             * }
             * case RAW_RS2_3 -> {
             * if (instruction.rs2() == null) yield null;
             * RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rs2(), 0, 0);
             * RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
             * RISCVInstruction nop = RISCVInstruction.NOP();
             * yield new Pair<>(List.of(add1, nop, nop, instruction), List.of(add2, nop,
             * nop, instruction));
             * }
             * case RAW_RS2_4 -> {
             * if (instruction.rs2() == null) yield null;
             * RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rs2(), 0, 0);
             * RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
             * RISCVInstruction nop = RISCVInstruction.NOP();
             * yield new Pair<>(List.of(add1, nop, nop, nop, instruction), List.of(add2,
             * nop, nop, nop, instruction));
             * }
             */
            case WAW_1 -> {
                if (instruction.rd() == null)
                    yield null;
                RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rd(), 0, 0);
                RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
                yield new Pair<>(List.of(add1, instruction), List.of(add2, instruction));
            }
            /*
             * case WAW_2 -> {
             * if (instruction.rd() == null) yield null;
             * RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rd(), 0, 0);
             * RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
             * RISCVInstruction nop = RISCVInstruction.NOP();
             * yield new Pair<>(List.of(add1, nop, instruction), List.of(add2, nop,
             * instruction));
             * }
             * case WAW_3 -> {
             * if (instruction.rd() == null) yield null;
             * RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rd(), 0, 0);
             * RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
             * RISCVInstruction nop = RISCVInstruction.NOP();
             * yield new Pair<>(List.of(add1, nop, nop, instruction), List.of(add2, nop,
             * nop, instruction));
             * }
             * case WAW_4 -> {
             * if (instruction.rd() == null) yield null;
             * RISCVInstruction add1 = RISCVInstruction.ADD(instruction.rd(), 0, 0);
             * RISCVInstruction add2 = RISCVInstruction.ADD(0, 0, 0);
             * RISCVInstruction nop = RISCVInstruction.NOP();
             * yield new Pair<>(List.of(add1, nop, nop, nop, instruction), List.of(add2,
             * nop, nop, nop, instruction));
             * }
             * 
             */
            case IS_SYSCALL -> null;
            case SYSCALL_TYPE -> {
                if (instruction.type() != RISCV_TYPE.ECALL) {
                    yield null;
                }
                var randomIdx = r.nextInt(syscallSpecs.length);
                while (randomIdx == instructionBlock.get(7).imm()) {
                    randomIdx = r.nextInt(syscallSpecs.length);
                }
                var randomSyscall = syscallSpecs[randomIdx];
                List<RISCVInstruction> newBlock = new ArrayList<>(instructionBlock);
                newBlock.set(7, RISCVInstruction.ADDI(17, 0, randomSyscall));
                var block = new Pair<>(instructionBlock, newBlock);
                yield block;
            }
            case SYSCALL_ARG_1 -> {
                if (instruction.type() != RISCV_TYPE.ECALL) {
                    yield null;
                }
                var syscall1 = instructionBlock;
                List<RISCVInstruction> syscall2 = new ArrayList<>(instructionBlock);
                syscall1.set(0, RISCVInstruction.ADDI(10, 0, r.nextInt(MAX_IMM_I)));
                syscall2.set(0, RISCVInstruction.ADDI(10, 0, r.nextInt(MAX_IMM_I)));
                var block = new Pair<>(syscall1, syscall2);
                yield block;
            }
            case SYSCALL_ARG_2 -> {
                if (instruction.type() != RISCV_TYPE.ECALL) {
                    yield null;
                }
                var syscall1 = instructionBlock;
                List<RISCVInstruction> syscall2 = new ArrayList<>(instructionBlock);
                syscall1.set(1, RISCVInstruction.ADDI(11, 0, r.nextInt(MAX_IMM_I)));
                syscall2.set(1, RISCVInstruction.ADDI(11, 0, r.nextInt(MAX_IMM_I)));
                var block = new Pair<>(syscall1, syscall2);
                yield block;
            }
            case SYSCALL_ARG_3 -> {
                if (instruction.type() != RISCV_TYPE.ECALL) {
                    yield null;
                }
                var syscall1 = instructionBlock;
                List<RISCVInstruction> syscall2 = new ArrayList<>(instructionBlock);
                syscall1.set(2, RISCVInstruction.ADDI(12, 0, r.nextInt(MAX_IMM_I)));
                syscall2.set(2, RISCVInstruction.ADDI(12, 0, r.nextInt(MAX_IMM_I)));
                var block = new Pair<>(syscall1, syscall2);
                yield block;
            }
            case SYSCALL_ARG_4 -> {
                if (instruction.type() != RISCV_TYPE.ECALL) {
                    yield null;
                }
                var syscall1 = instructionBlock;
                List<RISCVInstruction> syscall2 = new ArrayList<>(instructionBlock);
                syscall1.set(3, RISCVInstruction.ADDI(13, 0, r.nextInt(MAX_IMM_I)));
                syscall2.set(3, RISCVInstruction.ADDI(13, 0, r.nextInt(MAX_IMM_I)));
                var block = new Pair<>(syscall1, syscall2);
                yield block;
            }
            case SYSCALL_ARG_5 -> {
                if (instruction.type() != RISCV_TYPE.ECALL) {
                    yield null;
                }
                var syscall1 = instructionBlock;
                List<RISCVInstruction> syscall2 = new ArrayList<>(instructionBlock);
                syscall1.set(4, RISCVInstruction.ADDI(14, 0, r.nextInt(MAX_IMM_I)));
                syscall2.set(4, RISCVInstruction.ADDI(14, 0, r.nextInt(MAX_IMM_I)));
                var block = new Pair<>(syscall1, syscall2);
                yield block;
            }
            case SYSCALL_ARG_6 -> {
                if (instruction.type() != RISCV_TYPE.ECALL) {
                    yield null;
                }
                var syscall1 = instructionBlock;
                List<RISCVInstruction> syscall2 = new ArrayList<>(instructionBlock);
                syscall1.set(5, RISCVInstruction.ADDI(15, 0, r.nextInt(MAX_IMM_I)));
                syscall2.set(5, RISCVInstruction.ADDI(15, 0, r.nextInt(MAX_IMM_I)));
                var block = new Pair<>(syscall1, syscall2);
                yield block;
            }
            case SYSCALL_ARG_7 -> {
                if (instruction.type() != RISCV_TYPE.ECALL) {
                    yield null;
                }
                var syscall1 = instructionBlock;
                List<RISCVInstruction> syscall2 = new ArrayList<>(instructionBlock);
                syscall1.set(6, RISCVInstruction.ADDI(16, 0, r.nextInt(MAX_IMM_I)));
                syscall2.set(6, RISCVInstruction.ADDI(16, 0, r.nextInt(MAX_IMM_I)));
                var block = new Pair<>(syscall1, syscall2);
                yield block;
            }
        };
    }

    private List<RISCVInstruction> makeSyscall(int syscallNum, int[] args) {
        if (args.length != 7) {
            throw new AssertionError("too many arguments(linux riscv abi requires 7 regs)");
        }
        List<RISCVInstruction> results = new ArrayList<>();
        for (int regNum = 10; regNum <= 16; regNum++) {
            results.add(RISCVInstruction.ADDI(regNum, 0, args[regNum - 10]));
        }
        results.add(RISCVInstruction.ADDI(17, 0, syscallNum));
        results.add(RISCVInstruction.ECALL());
        return results;
    }

    /**
     * @param instruction an instruction
     * @return the maximum immediate value.
     */
    private static long getBound(RISCVInstruction instruction) {
        return switch (instruction.type().getFormat()) {
            case RTYPE -> 0L;
            case ITYPE, STYPE, SYSTEM -> MAX_IMM_I;
            case BTYPE -> MAX_IMM_B;
            case UTYPE -> MAX_IMM_U;
            case JTYPE -> MAX_IMM_J;
        };
    }

    /**
     * @param size The length of the sequence
     * @return A sequence of given length of random instructions.
     */
    private List<RISCVInstruction> randomSequence(int size) {
        List<RISCVInstruction> result = new ArrayList<>(size);
        for (int i = 0; i < size; i++) {
            RISCV_TYPE type = types.get(r.nextInt(types.size() - 1));
            result.addAll(randomInstructionBlockFromType(type));
        }
        return result;
    }

    /**
     * @param type the ytype of the desired instruction
     * @return a random instance of an instruction of the given type.
     */
    private List<RISCVInstruction> randomInstructionBlockFromType(RISCV_TYPE type) {
        RISCVInstruction instr = switch (type.getFormat()) {
            case RTYPE ->
                RISCVInstruction.RTYPE(type, r.nextInt(1, NUMBER_REGISTERS), r.nextInt(NUMBER_REGISTERS),
                        r.nextInt(NUMBER_REGISTERS));
            case ITYPE ->
                RISCVInstruction.ITYPE(type, r.nextInt(1, NUMBER_REGISTERS), r.nextInt(NUMBER_REGISTERS),
                        r.nextLong(MAX_IMM_I));
            case STYPE ->
                RISCVInstruction.STYPE(type, r.nextInt(NUMBER_REGISTERS), r.nextInt(NUMBER_REGISTERS),
                        r.nextLong(MAX_IMM_I));
            case BTYPE ->
                RISCVInstruction.BTYPE(type, r.nextInt(NUMBER_REGISTERS), r.nextInt(NUMBER_REGISTERS),
                        r.nextLong(MAX_IMM_B));
            case UTYPE ->
                RISCVInstruction.UTYPE(type, r.nextInt(1, NUMBER_REGISTERS), r.nextLong(MAX_IMM_I - 1, MAX_IMM_U));
            case JTYPE -> RISCVInstruction.JTYPE(type, r.nextInt(1, NUMBER_REGISTERS), r.nextLong(MAX_IMM_J));
            case SYSTEM -> RISCVInstruction.ECALL(); // always generate ecall for system instructions

        };
        //
        if (instr.type().getSubset() == RISCV_SUBSET.BASE_SPECIAL && instr.type() != RISCV_TYPE.ECALL) {
            instr = RISCVInstruction.NOP();
        }
        List<RISCVInstruction> result = new ArrayList<>();
        if (instr.type() == RISCV_TYPE.ECALL) {
            var randomSyscall = syscallSpecs[r.nextInt(syscallSpecs.length)];
            var randomArgs = new int[7];
            for (int i = 0; i < randomArgs.length; i++) {
                randomArgs[i] = r.nextInt(MAX_IMM_I);
            }
            result.addAll(makeSyscall(randomSyscall, randomArgs));
        } else {
            result.add(instr);
        }
        return result;
    }
}
