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
    Load,    // 0000011  LOAD
    MiscMem, // 0001111  MISC_MEM
    OpImm,   // 0010011  OP_IMM
    Auipc,   // 0010111  AUIPC
    Store,   // 0100011  STORE
    Op,      // 0110011  OP
    Lui,     // 0110111  LUI
    Branch,  // 1100011  BRANCH
    Jalr,    // 1100111  JALR
    Jal,     // 1101111  JAL
    System,  // 1110011  SYSTEM
};

struct MnemonicOpcodeEntry {
    std::string_view name;
    OpcodeCat cat;
};

// Lexicographically sorted for std::lower_bound (O(log n), no allocations).
static constexpr MnemonicOpcodeEntry kMnemonicOpcodeTable[] {
    { "add",     OpcodeCat::Op      },
    { "addi",    OpcodeCat::OpImm   },
    { "and",     OpcodeCat::Op      },
    { "andi",    OpcodeCat::OpImm   },
    { "auipc",   OpcodeCat::Auipc   },
    { "beq",     OpcodeCat::Branch  },
    { "bge",     OpcodeCat::Branch  },
    { "bgeu",    OpcodeCat::Branch  },
    { "blt",     OpcodeCat::Branch  },
    { "bltu",    OpcodeCat::Branch  },
    { "bne",     OpcodeCat::Branch  },
    { "ebreak",  OpcodeCat::System  },
    { "ecall",   OpcodeCat::System  },
    { "fence",   OpcodeCat::MiscMem },
    { "fence.i", OpcodeCat::MiscMem },
    { "jal",     OpcodeCat::Jal     },
    { "jalr",    OpcodeCat::Jalr    },
    { "lb",      OpcodeCat::Load    },
    { "lbu",     OpcodeCat::Load    },
    { "lh",      OpcodeCat::Load    },
    { "lhu",     OpcodeCat::Load    },
    { "lui",     OpcodeCat::Lui     },
    { "lw",      OpcodeCat::Load    },
    { "or",      OpcodeCat::Op      },
    { "ori",     OpcodeCat::OpImm   },
    { "sb",      OpcodeCat::Store   },
    { "sh",      OpcodeCat::Store   },
    { "sll",     OpcodeCat::Op      },
    { "slli",    OpcodeCat::OpImm   },
    { "slt",     OpcodeCat::Op      },
    { "slti",    OpcodeCat::OpImm   },
    { "sltiu",   OpcodeCat::OpImm   },
    { "sltu",    OpcodeCat::Op      },
    { "sra",     OpcodeCat::Op      },
    { "srai",    OpcodeCat::OpImm   },
    { "srl",     OpcodeCat::Op      },
    { "srli",    OpcodeCat::OpImm   },
    { "sub",     OpcodeCat::Op      },
    { "sw",      OpcodeCat::Store   },
    { "xor",     OpcodeCat::Op      },
    { "xori",    OpcodeCat::OpImm   },
};

static constexpr bool mnemonicOpcodeTableIsSorted()
{
    constexpr std::size_t n= sizeof(kMnemonicOpcodeTable) / sizeof(kMnemonicOpcodeTable[0]);
    for(std::size_t i= 1; i < n; ++i) {
        if(!(kMnemonicOpcodeTable[i - 1].name < kMnemonicOpcodeTable[i].name)) {
            return false;
        }
    }
    return true;
}

static_assert(mnemonicOpcodeTableIsSorted(), "kMnemonicOpcodeTable must be sorted by name");

std::optional<OpcodeCat> opcodeCategoryForMnemonic(std::string_view m)
{
    const auto *const first= std::begin(kMnemonicOpcodeTable);
    const auto *const last = std::end(kMnemonicOpcodeTable);
    const auto *const it   = std::lower_bound(first, last, m, [](const MnemonicOpcodeEntry &e, std::string_view key) {
        return e.name < key;
    });
    if(it != last && it->name == m) {
        return it->cat;
    }
    return std::nullopt;
}

std::optional<std::string_view> parseLeadingMnemonic(std::string_view s)
{
    std::size_t i= 0;
    while(i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
        ++i;
    }
    if(i >= s.size()) {
        return std::nullopt;
    }
    const std::size_t start= i;
    while(i < s.size()) {
        const unsigned char c= static_cast<unsigned char>(s[i]);
        if((c >= 'a' && c <= 'z') || c == '.') {
            ++i;
        } else {
            break;
        }
    }
    if(i == start) {
        return std::nullopt;
    }
    return s.substr(start, i - start);
}

std::string randomizeRegisters(const std::string &assembly)
{
    static thread_local std::mt19937 rng(std::random_device {}());
    static thread_local std::uniform_int_distribution<int> regDist(0, 31);
    static thread_local std::uniform_int_distribution<int> iImmDist(-2048, 2047);
    static thread_local std::uniform_int_distribution<int> shiftImmDist(0, 31);
    static thread_local std::uniform_int_distribution<int> branchImmStepDist(-2048, 2047);
    static thread_local std::uniform_int_distribution<int> jalImmStepDist(-512, 511);
    static thread_local std::uniform_int_distribution<int> uImmDist(0, 0xFFFFF);
    static thread_local std::uniform_int_distribution<int> fenceImmDist(0, 255);

    const auto randReg= [&]() { return "x" + std::to_string(regDist(rng)); };

    const auto mnemonicSv= parseLeadingMnemonic(assembly);
    if(!mnemonicSv) {
        return assembly;
    }
    const auto catOpt= opcodeCategoryForMnemonic(*mnemonicSv);
    if(!catOpt) {
        return assembly;
    }
    const std::string mnemonic(*mnemonicSv);
    const OpcodeCat cat= *catOpt;

    switch(cat) {
    case OpcodeCat::System:
        return mnemonic;

    case OpcodeCat::Jal: {
        const int imm= jalImmStepDist(rng) * 2;
        return mnemonic + " " + randReg() + ", " + std::to_string(imm);
    }

    case OpcodeCat::MiscMem: {
        const int imm= fenceImmDist(rng);
        return mnemonic + " " + randReg() + ", " + randReg() + ", " + std::to_string(imm);
    }

    case OpcodeCat::Lui:
    case OpcodeCat::Auipc: {
        const int imm= uImmDist(rng);
        std::ostringstream immHex;
        immHex << std::hex << imm;
        return mnemonic + " " + randReg() + ", 0x" + immHex.str();
    }

    case OpcodeCat::Store: {
        const int imm= iImmDist(rng);
        return mnemonic + " " + randReg() + ", " + std::to_string(imm) + "(" + randReg() + ")";
    }

    case OpcodeCat::Branch: {
        const int imm= branchImmStepDist(rng) * 2;
        return mnemonic + " " + randReg() + ", " + randReg() + ", " + std::to_string(imm);
    }

    case OpcodeCat::Load:
    case OpcodeCat::Jalr: {
        const int imm= iImmDist(rng);
        return mnemonic + " " + randReg() + ", " + randReg() + ", " + std::to_string(imm);
    }

    case OpcodeCat::OpImm:
        if(mnemonic == "slli" || mnemonic == "srli" || mnemonic == "srai") {
            const int imm= shiftImmDist(rng);
            return mnemonic + " " + randReg() + ", " + randReg() + ", " + std::to_string(imm);
        }
        {
            const int imm= iImmDist(rng);
            return mnemonic + " " + randReg() + ", " + randReg() + ", " + std::to_string(imm);
        }

    case OpcodeCat::Op:
        return mnemonic + " " + randReg() + ", " + randReg() + ", " + randReg();
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
        auto *win= new RISCVInstructionWindow();
        win->set_title("RV32I demo — one instruction every 0.5 s");
        add_window(*win);

        auto idx      = std::make_shared<std::size_t>(0);
        auto timerConn= std::make_shared<sigc::connection>();

        *timerConn= Glib::signal_timeout().connect(
            [win, idx] {
                if(!win->InsEntry_) {
                    return false;
                }
                const auto &c      = kRv32iInstructionCases[*idx % kRv32iInstructionCaseCount];
                std::string randStr= randomizeRegisters(c.assembly);
                LOG_DEBUG("c.assembly: ", randStr);
                win->InsEntry_->set_text(randStr);
                win->parseCurrentEntry();
                ++(*idx);
                return true;
            },
            500);

        win->signal_close_request().connect(
            [win, timerConn] {
                timerConn->disconnect();
                delete win;
                return true;
            },
            false);

        win->show();

        if(kRv32iInstructionCaseCount == 0) {
            return;
        }
        win->InsEntry_->set_text(randomizeRegisters(kRv32iInstructionCases[0].assembly));
        win->parseCurrentEntry();
        *idx= 1;
    }
};

} // namespace

int main(int argc, char *argv[])
{
    return Rv32iDemoApplication::create()->run(argc, argv);
}
