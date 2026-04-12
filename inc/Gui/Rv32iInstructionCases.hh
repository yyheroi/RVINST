#pragma once

#include <cstddef>

/** One sample assembly line per supported mnemonic (RV32I / RV64I / Zifencei). */
struct Rv32iInstructionCase {
    const char *assembly;
    const char *expectedMnemonic;
    const char *expectIsaSubstring;
};

inline constexpr Rv32iInstructionCase kRv32iInstructionCases[] {
    { "add x3, x2, x1",    "add",     "RV32I"    },
    { "sub x3, x2, x1",    "sub",     "RV32I"    },
    { "sll x3, x2, x1",    "sll",     "RV32I"    },
    { "slt x3, x2, x1",    "slt",     "RV32I"    },
    { "sltu x3, x2, x1",   "sltu",    "RV32I"    },
    { "xor x3, x2, x1",    "xor",     "RV32I"    },
    { "srl x3, x2, x1",    "srl",     "RV32I"    },
    { "sra x3, x2, x1",    "sra",     "RV32I"    },
    { "or x3, x2, x1",     "or",      "RV32I"    },
    { "and x3, x2, x1",    "and",     "RV32I"    },
    { "lb x3, x2, 0",      "lb",      "RV32I"    },
    { "lh x3, x2, 0",      "lh",      "RV32I"    },
    { "lw x3, x2, 0",      "lw",      "RV32I"    },
    { "lbu x3, x2, 0",     "lbu",     "RV32I"    },
    { "lhu x3, x2, 0",     "lhu",     "RV32I"    },
    { "jalr x3, x2, 0",    "jalr",    "RV32I"    },
    { "addi x3, x2, 0",    "addi",    "RV32I"    },
    { "slli x3, x2, 1",    "slli",    "RV32I"    },
    { "slti x3, x2, 0",    "slti",    "RV32I"    },
    { "sltiu x3, x2, 0",   "sltiu",   "RV32I"    },
    { "xori x3, x2, 0",    "xori",    "RV32I"    },
    { "srli x3, x2, 1",    "srli",    "RV32I"    },
    { "srai x3, x2, 1",    "srai",    "RV32I"    },
    { "ori x3, x2, 0",     "ori",     "RV32I"    },
    { "andi x3, x2, 0",    "andi",    "RV32I"    },
    { "fence x0, x0, 0",   "fence",   "RV32I"    },
    { "fence.i x0, x0, 0", "fence.i", "Zifencei" },
    { "ecall",             "ecall",   "RV32I"    },
    { "ebreak",            "ebreak",  "RV32I"    },
    { "beq x1, x2, 0",     "beq",     "RV32I"    },
    { "bne x1, x2, 0",     "bne",     "RV32I"    },
    { "blt x1, x2, 0",     "blt",     "RV32I"    },
    { "bge x1, x2, 0",     "bge",     "RV32I"    },
    { "bltu x1, x2, 0",    "bltu",    "RV32I"    },
    { "bgeu x1, x2, 0",    "bgeu",    "RV32I"    },
    { "sb x1, 0(x2)",      "sb",      "RV32I"    },
    { "sh x1, 0(x2)",      "sh",      "RV32I"    },
    { "sw x1, 0(x2)",      "sw",      "RV32I"    },
    { "lui x1, 0x12345",   "lui",     "RV32I"    },
    { "auipc x1, 0x12345", "auipc",   "RV32I"    },
    { "jal x1, 4",         "jal",     "RV32I"    },
};

inline constexpr std::size_t kRv32iInstructionCaseCount= sizeof(kRv32iInstructionCases) / sizeof(kRv32iInstructionCases[0]);
