#pragma once

#include <sstream>
#include <string>

#include "Core/Instruction.hh"

/** First two lines written to InsTextView_ on successful parse (see RISCVInstructionWindow::showInsResult). */
inline std::string instructionSummaryTextForTextView(const Instruction &inst)
{
    std::ostringstream oss;
    oss << "Format          = " << std::hex << inst.GetFormat() << '\n';
    oss << "Instruction set = " << inst.GetXLEN() << "\n";
    return oss.str();
}
