package contractgen.riscv.isa;

/**
 * The RISC-V ISA subsets.
 */
public enum RISCV_SUBSET {
    /**
     * The base integer subset.
     */
    BASE,
    BASE64,
    BASE_SPECIAL,
    /**
     * The multiplication and division subset.
     */
    M,
    M64,
    /**
     * The Control and Status Register (CSR) Instructions subset.
     */
    ZICSR
}
