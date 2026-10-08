package contractgen.riscv.isa;

/**
 * The types of RISC-V instructions.
 */
@SuppressWarnings("MissingJavadoc")
public enum RISCV_TYPE {
    LUI("lui", RISCV_SUBSET.BASE, RISCV_FORMAT.UTYPE, "0110111"),
    AUIPC("aupc", RISCV_SUBSET.BASE, RISCV_FORMAT.UTYPE, "0010111"),
    JAL("jal", RISCV_SUBSET.BASE, RISCV_FORMAT.JTYPE, "1101111"),
    JALR("jalr", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "1100111", "000"),
    BEQ("beq", RISCV_SUBSET.BASE, RISCV_FORMAT.BTYPE, "1100011", "000"),
    BNE("bne", RISCV_SUBSET.BASE, RISCV_FORMAT.BTYPE, "1100011", "001"),
    BLT("blt", RISCV_SUBSET.BASE, RISCV_FORMAT.BTYPE, "1100011", "100"),
    BGE("bge", RISCV_SUBSET.BASE, RISCV_FORMAT.BTYPE, "1100011", "101"),
    BLTU("bltu", RISCV_SUBSET.BASE, RISCV_FORMAT.BTYPE, "1100011", "110"),
    BGEU("bgeu", RISCV_SUBSET.BASE, RISCV_FORMAT.BTYPE, "1100011", "111"),
    LB("lb", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0000011", "000"),
    LH("lh", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0000011", "001"),
    LW("lw", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0000011", "010"),
    LBU("lbu", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0000011", "100"),
    LHU("lhu", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0000011", "101"),
    SB("sb", RISCV_SUBSET.BASE, RISCV_FORMAT.STYPE, "0100011", "000"),
    SH("sh", RISCV_SUBSET.BASE, RISCV_FORMAT.STYPE, "0100011", "001"),
    SW("sw", RISCV_SUBSET.BASE, RISCV_FORMAT.STYPE, "0100011", "010"),
    ADDI("addi", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0010011", "000"),
    SLTI("slti", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0010011", "010"),
    SLTIU("sltui", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0010011", "011"),
    XORI("xorii", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0010011", "100"),
    ORI("ori", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0010011", "110"),
    ANDI("andi", RISCV_SUBSET.BASE, RISCV_FORMAT.ITYPE, "0010011", "111"),
    SLLI("slli", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0010011", "001", "0000000"),
    SRLI("srli", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0010011", "101", "0000000"),
    SRAI("srai", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0010011", "101", "0100000"),
    ADD("add", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "000", "0000000"),
    SUB("sub", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "000", "0100000"),
    SLL("sll", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "001", "0000000"),
    SLT("slt", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "010", "0000000"),
    SLTU("sltu", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "011", "0000000"),
    XOR("xor", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "100", "0000000"),
    SRL("srl", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "101", "0000000"),
    SRA("sra", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "101", "0100000"),
    OR("or", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "110", "0000000"),
    AND("and", RISCV_SUBSET.BASE, RISCV_FORMAT.RTYPE, "0110011", "111", "0000000"),

    MUL("mul", RISCV_SUBSET.M, RISCV_FORMAT.RTYPE, "0110011", "000", "0000001"),
    MULH("mulh", RISCV_SUBSET.M, RISCV_FORMAT.RTYPE, "0110011", "001", "0000001"),
    MULHSU("mulhsu", RISCV_SUBSET.M, RISCV_FORMAT.RTYPE, "0110011", "010", "0000001"),
    MULHU("mulhu", RISCV_SUBSET.M, RISCV_FORMAT.RTYPE, "0110011", "011", "0000001"),
    DIV("div", RISCV_SUBSET.M, RISCV_FORMAT.RTYPE, "0110011", "100", "0000001"),
    DIVU("divu", RISCV_SUBSET.M, RISCV_FORMAT.RTYPE, "0110011", "101", "0000001"),
    REM("rem", RISCV_SUBSET.M, RISCV_FORMAT.RTYPE, "0110011", "110", "0000001"),
    REMU("remu", RISCV_SUBSET.M, RISCV_FORMAT.RTYPE, "0110011", "111", "0000001"),

    // ########### RV64 ###########
    // RV64I Load/Store Extensions
    LD("ld", RISCV_SUBSET.BASE64, RISCV_FORMAT.ITYPE, "0000011", "011", "0000000"),
    SD("sd", RISCV_SUBSET.BASE64, RISCV_FORMAT.STYPE, "0100011", "011", "0000000"),
    LWU("lwu", RISCV_SUBSET.BASE64, RISCV_FORMAT.ITYPE, "0000011", "110", "0000000"),

    // RV64I Immediate Word Operations (OP-IMM-32)
    ADDIW("addiw", RISCV_SUBSET.BASE64, RISCV_FORMAT.ITYPE, "0011011", "000", "0000000"),
    SLLIW("slliw", RISCV_SUBSET.BASE64, RISCV_FORMAT.ITYPE, "0011011", "001", "0000000"),
    SRLIW("srliw", RISCV_SUBSET.BASE64, RISCV_FORMAT.ITYPE, "0011011", "101", "0000000"),
    SRAIW("sraiw", RISCV_SUBSET.BASE64, RISCV_FORMAT.ITYPE, "0011011", "101", "0100000"),

    // RV64I Register Word Operations (OP-32)
    ADDW("addw", RISCV_SUBSET.BASE64, RISCV_FORMAT.RTYPE, "0111011", "000", "0000000"),
    SUBW("subw", RISCV_SUBSET.BASE64, RISCV_FORMAT.RTYPE, "0111011", "000", "0100000"),
    SLLW("sllw", RISCV_SUBSET.BASE64, RISCV_FORMAT.RTYPE, "0111011", "001", "0000000"),
    SRLW("srlw", RISCV_SUBSET.BASE64, RISCV_FORMAT.RTYPE, "0111011", "101", "0000000"),
    SRAW("sraw", RISCV_SUBSET.BASE64, RISCV_FORMAT.RTYPE, "0111011", "101", "0100000"),

    // RV64M Multiply Word Extension
    MULW("mulw", RISCV_SUBSET.M64, RISCV_FORMAT.RTYPE, "0111011", "000", "0000001"),
    DIVW("divw", RISCV_SUBSET.M64, RISCV_FORMAT.RTYPE, "0111011", "100", "0000001"),
    DIVUW("divuw", RISCV_SUBSET.M64, RISCV_FORMAT.RTYPE, "0111011", "101", "0000001"),
    REMW("remw", RISCV_SUBSET.M64, RISCV_FORMAT.RTYPE, "0111011", "110", "0000001"),
    REMUW("remuw", RISCV_SUBSET.M64, RISCV_FORMAT.RTYPE, "0111011", "111", "0000001"),

    // Priviledged Instructions
    ECALL("ecall", RISCV_SUBSET.BASE_SPECIAL, RISCV_FORMAT.SYSTEM, "1110011", "000", "0000000"),
    EBREAK("ebreak", RISCV_SUBSET.BASE_SPECIAL, RISCV_FORMAT.SYSTEM, "1110011", "000", "0000001"),
    MRET("mret", RISCV_SUBSET.BASE_SPECIAL, RISCV_FORMAT.SYSTEM, "1110011", "000", "0011000"),
    // CSRRW("csrrw", RISCV_SUBSET.ZICSR, RISCV_FORMAT.ITYPE, "1110011", "001"),
    // CSRRS("csrrs", RISCV_SUBSET.ZICSR, RISCV_FORMAT.ITYPE, "1110011", "010"),
    // CSRRC("csrrc", RISCV_SUBSET.ZICSR, RISCV_FORMAT.ITYPE, "1110011", "011"),
    // CSRRWI("csrrwi", RISCV_SUBSET.ZICSR, RISCV_FORMAT.ITYPE, "1110011", "101"),
    // CSRRSI("csrrsi", RISCV_SUBSET.ZICSR, RISCV_FORMAT.ITYPE, "1110011", "110"),
    // CSRRCI("csrrci", RISCV_SUBSET.ZICSR, RISCV_FORMAT.ITYPE, "1110011", "111"),
    WFI("wfi", RISCV_SUBSET.BASE_SPECIAL, RISCV_FORMAT.SYSTEM, "1110011", "000", "0001000"),
    C_UNIMP("c.unimp", RISCV_SUBSET.BASE_SPECIAL, RISCV_FORMAT.ITYPE, "0000000", "000", "0000000");

    private final String name;

    private final RISCV_SUBSET subset;
    private final RISCV_FORMAT format;
    private final String opcode;

    private final String funct3;
    private final String funct7;

    RISCV_TYPE(String name, RISCV_SUBSET subset, RISCV_FORMAT format, String opcode) {
        this.name = name;
        this.subset = subset;
        this.format = format;
        this.opcode = opcode;
        this.funct3 = null;
        this.funct7 = null;
    }

    RISCV_TYPE(String name, RISCV_SUBSET subset, RISCV_FORMAT format, String opcode, String funct3) {
        this.name = name;
        this.subset = subset;
        this.format = format;
        this.opcode = opcode;
        this.funct3 = funct3;
        this.funct7 = null;
    }

    RISCV_TYPE(String name, RISCV_SUBSET subset, RISCV_FORMAT format, String opcode, String funct3, String funct7) {
        this.name = name;
        this.subset = subset;
        this.format = format;
        this.opcode = opcode;
        this.funct3 = funct3;
        this.funct7 = funct7;
    }

    public String getOpcode() {
        return opcode;
    }

    public String getFunct3() {
        return funct3;
    }

    public String getFunct7() {
        return funct7;
    }

    public String getName() {
        return name;
    }

    public RISCV_FORMAT getFormat() {
        return format;
    }

    /**
     * @param suffix The index of the core that should be used.
     * @return The first line of a contract for this instruction in Verilog.
     */
    public String generateContract(String suffix) {
        if (funct3 == null)
            return "if (op_" + suffix + " == 'b" + opcode + ") begin\n";
        if (funct7 == null)
            return "if (op_" + suffix + " == 'b" + opcode + " && funct_3_" + suffix + " == 'b" + funct3 + ") begin\n";
        return "if (op_" + suffix + " == 'b" + opcode + " && funct_3_" + suffix + " == 'b" + funct3 + " && funct_7_"
                + suffix + " == 'b" + funct7 + ") begin\n";
    }

    public RISCV_SUBSET getSubset() {
        return subset;
    }
}
