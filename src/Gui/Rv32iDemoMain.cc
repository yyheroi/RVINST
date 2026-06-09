#include <algorithm>
#include <cctype>
#include <cstdint>
#include <glibmm/main.h>
#include <gtkmm.h>
#include <memory>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <string_view>

#include "Logger.hh"
#include "Gui/RISCVInstructionWindow.hh"
#include "Gui/Rv32iInstructionCases.hh"

namespace {

// RVG major opcode buckets (inst[6:0] == 0b11 implied). Mirrors JS OPCODE + ISA_RV32I grouping.
enum class OpcodeCat : std::uint8_t {
    LOAD,     // 0000011  LOAD
    MISC_MEM, // 0001111  MISC_MEM
    OP_IMM,   // 0010011  OP_IMM
    AUIPC,    // 0010111  AUIPC
    STORE,    // 0100011  STORE
    OP,       // 0110011  OP
    LUI,      // 0110111  LUI
    BRANCH,   // 1100011  BRANCH
    JALR,     // 1100111  JALR
    JAL,      // 1101111  JAL
    SYSTEM,   // 1110011  SYSTEM
};

struct MnemonicOpcodeEntry {
    std::string_view name_;
    OpcodeCat cat_;
};

// Lexicographically sorted for std::lower_bound (O(log n), no allocations).
constexpr MnemonicOpcodeEntry G_kMnemonicOpcodeTable[] {
    { .name_= "add",     .cat_= OpcodeCat::OP       },
    { .name_= "addi",    .cat_= OpcodeCat::OP_IMM   },
    { .name_= "and",     .cat_= OpcodeCat::OP       },
    { .name_= "andi",    .cat_= OpcodeCat::OP_IMM   },
    { .name_= "auipc",   .cat_= OpcodeCat::AUIPC    },
    { .name_= "beq",     .cat_= OpcodeCat::BRANCH   },
    { .name_= "bge",     .cat_= OpcodeCat::BRANCH   },
    { .name_= "bgeu",    .cat_= OpcodeCat::BRANCH   },
    { .name_= "blt",     .cat_= OpcodeCat::BRANCH   },
    { .name_= "bltu",    .cat_= OpcodeCat::BRANCH   },
    { .name_= "bne",     .cat_= OpcodeCat::BRANCH   },
    { .name_= "ebreak",  .cat_= OpcodeCat::SYSTEM   },
    { .name_= "ecall",   .cat_= OpcodeCat::SYSTEM   },
    { .name_= "fence",   .cat_= OpcodeCat::MISC_MEM },
    { .name_= "fence.i", .cat_= OpcodeCat::MISC_MEM },
    { .name_= "jal",     .cat_= OpcodeCat::JAL      },
    { .name_= "jalr",    .cat_= OpcodeCat::JALR     },
    { .name_= "lb",      .cat_= OpcodeCat::LOAD     },
    { .name_= "lbu",     .cat_= OpcodeCat::LOAD     },
    { .name_= "lh",      .cat_= OpcodeCat::LOAD     },
    { .name_= "lhu",     .cat_= OpcodeCat::LOAD     },
    { .name_= "lui",     .cat_= OpcodeCat::LUI      },
    { .name_= "lw",      .cat_= OpcodeCat::LOAD     },
    { .name_= "or",      .cat_= OpcodeCat::OP       },
    { .name_= "ori",     .cat_= OpcodeCat::OP_IMM   },
    { .name_= "sb",      .cat_= OpcodeCat::STORE    },
    { .name_= "sh",      .cat_= OpcodeCat::STORE    },
    { .name_= "sll",     .cat_= OpcodeCat::OP       },
    { .name_= "slli",    .cat_= OpcodeCat::OP_IMM   },
    { .name_= "slt",     .cat_= OpcodeCat::OP       },
    { .name_= "slti",    .cat_= OpcodeCat::OP_IMM   },
    { .name_= "sltiu",   .cat_= OpcodeCat::OP_IMM   },
    { .name_= "sltu",    .cat_= OpcodeCat::OP       },
    { .name_= "sra",     .cat_= OpcodeCat::OP       },
    { .name_= "srai",    .cat_= OpcodeCat::OP_IMM   },
    { .name_= "srl",     .cat_= OpcodeCat::OP       },
    { .name_= "srli",    .cat_= OpcodeCat::OP_IMM   },
    { .name_= "sub",     .cat_= OpcodeCat::OP       },
    { .name_= "sw",      .cat_= OpcodeCat::STORE    },
    { .name_= "xor",     .cat_= OpcodeCat::OP       },
    { .name_= "xori",    .cat_= OpcodeCat::OP_IMM   },
};

static constexpr bool MNEMONIC_OPCODE_TABLE_IS_SORTED()
{
    constexpr std::size_t N= sizeof(G_kMnemonicOpcodeTable) / sizeof(G_kMnemonicOpcodeTable[0]);
    for(std::size_t i= 1; i < N; ++i) {
        if(!(G_kMnemonicOpcodeTable[i - 1].name_ < G_kMnemonicOpcodeTable[i].name_)) {
            return false;
        }
    }
    return true;
}

static_assert(MNEMONIC_OPCODE_TABLE_IS_SORTED(), "kMnemonicOpcodeTable must be sorted by name");

std::optional<OpcodeCat> opcodeCategoryForMnemonic(std::string_view m)
{
    const auto *const FIRST= std::begin(G_kMnemonicOpcodeTable);
    const auto *const LAST = std::end(G_kMnemonicOpcodeTable);
    const auto *const IT   = std::lower_bound(FIRST, LAST, m, [](const MnemonicOpcodeEntry &e, std::string_view key) {
        return e.name_ < key;
    });
    if(IT != LAST && IT->name_ == m) {
        return IT->cat_;
    }
    return std::nullopt;
}

std::optional<std::string_view> parseLeadingMnemonic(std::string_view s)
{
    std::size_t i= 0;
    while(i < s.size() && (std::isspace(static_cast<unsigned char>(s[i])) != 0)) {
        ++i;
    }
    if(i >= s.size()) {
        return std::nullopt;
    }
    const std::size_t START= i;
    while(i < s.size()) {
        const unsigned char C= static_cast<unsigned char>(s[i]);
        if((C >= 'a' && C <= 'z') || C == '.') {
            ++i;
        } else {
            break;
        }
    }
    if(i == START) {
        return std::nullopt;
    }
    return s.substr(START, i - START);
}

std::string randomizeRegisters(const std::string &assembly)
{
    static thread_local std::mt19937 rng(std::random_device {}());
    static thread_local std::uniform_int_distribution<int> s_regDist(0, 31);
    static thread_local std::uniform_int_distribution<int> s_iImmDist(-2048, 2047);
    static thread_local std::uniform_int_distribution<int> s_shiftImmDist(0, 31);
    static thread_local std::uniform_int_distribution<int> s_branchImmStepDist(-2048, 2047);
    static thread_local std::uniform_int_distribution<int> s_jalImmStepDist(-512, 511);
    static thread_local std::uniform_int_distribution<int> s_uImmDist(0, 0xFFFFF);
    static thread_local std::uniform_int_distribution<int> s_fenceImmDist(0, 255);

    const auto RAND_REG= [&]() { return "x" + std::to_string(s_regDist(rng)); };

    const auto MNEMONIC_SV= parseLeadingMnemonic(assembly);
    if(!MNEMONIC_SV) {
        return assembly;
    }
    const auto CAT_OPT= opcodeCategoryForMnemonic(*MNEMONIC_SV);
    if(!CAT_OPT) {
        return assembly;
    }
    const std::string MNEMONIC(*MNEMONIC_SV);
    const OpcodeCat cat= *CAT_OPT;

    switch(cat) {
    case OpcodeCat::SYSTEM:
        return MNEMONIC;

    case OpcodeCat::JAL: {
        const int IMM= s_jalImmStepDist(rng) * 2;
        return MNEMONIC + " " + RAND_REG() + ", " + std::to_string(IMM);
    }

    case OpcodeCat::MISC_MEM: {
        const int IMM= s_fenceImmDist(rng);
        return MNEMONIC + " " + RAND_REG() + ", " + RAND_REG() + ", " + std::to_string(IMM);
    }

    case OpcodeCat::LUI:
    case OpcodeCat::AUIPC: {
        const int IMM= s_uImmDist(rng);
        std::ostringstream immHex;
        immHex << std::hex << IMM;
        return MNEMONIC + " " + RAND_REG() + ", 0x" + immHex.str();
    }

    case OpcodeCat::STORE: {
        const int IMM= s_iImmDist(rng);
        return MNEMONIC + " " + RAND_REG() + ", " + std::to_string(IMM) + "(" + RAND_REG() + ")";
    }

    case OpcodeCat::BRANCH: {
        const int IMM= s_branchImmStepDist(rng) * 2;
        return MNEMONIC + " " + RAND_REG() + ", " + RAND_REG() + ", " + std::to_string(IMM);
    }

    case OpcodeCat::LOAD:
    case OpcodeCat::JALR: {
        const int IMM= s_iImmDist(rng);
        return MNEMONIC + " " + RAND_REG() + ", " + RAND_REG() + ", " + std::to_string(IMM);
    }

    case OpcodeCat::OP_IMM:
        if(MNEMONIC == "slli" || MNEMONIC == "srli" || MNEMONIC == "srai") {
            const int IMM= s_shiftImmDist(rng);
            return MNEMONIC + " " + RAND_REG() + ", " + RAND_REG() + ", " + std::to_string(IMM);
        }
        {
            const int IMM= s_iImmDist(rng);
            return MNEMONIC + " " + RAND_REG() + ", " + RAND_REG() + ", " + std::to_string(IMM);
        }

    case OpcodeCat::OP:
        return MNEMONIC + " " + RAND_REG() + ", " + RAND_REG() + ", " + RAND_REG();
    }

    return assembly;
}

class Rv32iDemoApplication: public Gtk::Application {
public:
    Rv32iDemoApplication()
        : Gtk::Application("org.gtkmm.riscv.rv32i.demo", Gio::Application::Flags::NONE)
    {
    }

    static Glib::RefPtr<Rv32iDemoApplication> create()
    {
        return Glib::RefPtr<Rv32iDemoApplication>(new Rv32iDemoApplication());
    }

protected:
    void on_activate() override
    {
        auto *pWin= new RISCVInstructionWindow();
        pWin->set_title("RV32I demo — one instruction every 0.5 s");
        add_window(*pWin);

        auto idx      = std::make_shared<std::size_t>(0);
        auto timerConn= std::make_shared<sigc::connection>();

        *timerConn= Glib::signal_timeout().connect(
            [pWin, idx] {
                if(!pWin->InsEntry_) {
                    return false;
                }
                const auto &c      = G_kRv32iInstructionCases[*idx % G_kRv32iInstructionCaseCount];
                std::string randStr= randomizeRegisters(c.assembly_);
                LOG_DEBUG("c.assembly: ", randStr);
                pWin->InsEntry_->set_text(randStr);
                pWin->parseCurrentEntry();
                ++(*idx);
                return true;
            },
            500);

        pWin->signal_close_request().connect(
            [pWin, timerConn] {
                timerConn->disconnect();
                delete pWin;
                return true;
            },
            false);

        pWin->show();

        if(G_kRv32iInstructionCaseCount == 0) {
            return;
        }
        pWin->InsEntry_->set_text(randomizeRegisters(G_kRv32iInstructionCases[0].assembly));
        pWin->parseCurrentEntry();
        *idx= 1;
    }
};

} // namespace

int main(int argc, char *argv[])
{
    return Rv32iDemoApplication::create()->run(argc, argv);
}
