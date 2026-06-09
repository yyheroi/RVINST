#pragma once

#include <cstddef>

/** One sample assembly line per RV32I (+ Zifencei fence.i) mnemonic supported by INST. */
struct Rv32iInstructionCase {
    const char *assembly_;
    const char *expectedMnemonic_;
    const char *expectIsaSubstring_;
};

inline constexpr Rv32iInstructionCase G_kRv32iInstructionCases[] {
    { .assembly_= "add x3, x2, x1",    .expectedMnemonic_= "add",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "sub x3, x2, x1",    .expectedMnemonic_= "sub",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "sll x3, x2, x1",    .expectedMnemonic_= "sll",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "slt x3, x2, x1",    .expectedMnemonic_= "slt",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "sltu x3, x2, x1",   .expectedMnemonic_= "sltu",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "xor x3, x2, x1",    .expectedMnemonic_= "xor",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "srl x3, x2, x1",    .expectedMnemonic_= "srl",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "sra x3, x2, x1",    .expectedMnemonic_= "sra",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "or x3, x2, x1",     .expectedMnemonic_= "or",      .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "and x3, x2, x1",    .expectedMnemonic_= "and",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "lb x3, x2, 0",      .expectedMnemonic_= "lb",      .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "lh x3, x2, 0",      .expectedMnemonic_= "lh",      .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "lw x3, x2, 0",      .expectedMnemonic_= "lw",      .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "lbu x3, x2, 0",     .expectedMnemonic_= "lbu",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "lhu x3, x2, 0",     .expectedMnemonic_= "lhu",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "jalr x3, x2, 0",    .expectedMnemonic_= "jalr",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "addi x3, x2, 0",    .expectedMnemonic_= "addi",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "slli x3, x2, 1",    .expectedMnemonic_= "slli",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "slti x3, x2, 0",    .expectedMnemonic_= "slti",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "sltiu x3, x2, 0",   .expectedMnemonic_= "sltiu",   .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "xori x3, x2, 0",    .expectedMnemonic_= "xori",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "srli x3, x2, 1",    .expectedMnemonic_= "srli",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "srai x3, x2, 1",    .expectedMnemonic_= "srai",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "ori x3, x2, 0",     .expectedMnemonic_= "ori",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "andi x3, x2, 0",    .expectedMnemonic_= "andi",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "fence x0, x0, 0",   .expectedMnemonic_= "fence",   .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "fence.i x0, x0, 0", .expectedMnemonic_= "fence.i", .expectIsaSubstring_= "Zifencei" },
    { .assembly_= "ecall",             .expectedMnemonic_= "ecall",   .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "ebreak",            .expectedMnemonic_= "ebreak",  .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "beq x1, x2, 0",     .expectedMnemonic_= "beq",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "bne x1, x2, 0",     .expectedMnemonic_= "bne",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "blt x1, x2, 0",     .expectedMnemonic_= "blt",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "bge x1, x2, 0",     .expectedMnemonic_= "bge",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "bltu x1, x2, 0",    .expectedMnemonic_= "bltu",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "bgeu x1, x2, 0",    .expectedMnemonic_= "bgeu",    .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "sb x1, 0(x2)",      .expectedMnemonic_= "sb",      .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "sh x1, 0(x2)",      .expectedMnemonic_= "sh",      .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "sw x1, 0(x2)",      .expectedMnemonic_= "sw",      .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "lui x1, 0x12345",   .expectedMnemonic_= "lui",     .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "auipc x1, 0x12345", .expectedMnemonic_= "auipc",   .expectIsaSubstring_= "RV32I"    },
    { .assembly_= "jal x1, 4",         .expectedMnemonic_= "jal",     .expectIsaSubstring_= "RV32I"    },
};

inline constexpr std::size_t G_kRv32iInstructionCaseCount= sizeof(G_kRv32iInstructionCases) / sizeof(G_kRv32iInstructionCases[0]);
