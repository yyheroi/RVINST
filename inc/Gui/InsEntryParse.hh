#pragma once

#include <algorithm>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Core/Instruction.hh"

/** Same classification rules as the main-window entry field (hex vs binary vs assembly). */
inline bool insEntryLooksLikeHex(std::string_view s)
{
    if(s.empty()) {
        return false;
    }
    size_t start= 0;
    if(s.size() >= 2 && (s.substr(0, 2) == "0x" || s.substr(0, 2) == "0X")) {
        start= 2;
    }
    if(start >= s.size()) {
        return false;
    }
    const std::string_view tail= s.substr(start);
    return std::all_of(tail.begin(), tail.end(), [](unsigned char c) { return std::isxdigit(c) != 0; });
}

inline bool insEntryLooksLikeBinary(std::string_view s)
{
    if(s.empty()) {
        return false;
    }
    std::string_view digits;
    if(s.size() >= 2 && (s.substr(0, 2) == "0b" || s.substr(0, 2) == "0B")) {
        digits= s.substr(2);
    } else {
        digits= s;
        if(digits.size() < 2) {
            return false;
        }
    }
    if(digits.empty() || digits.size() > 32) {
        return false;
    }
    return std::all_of(digits.begin(), digits.end(), [](unsigned char c) { return c == '0' || c == '1'; });
}

inline std::string_view insEntryBinaryDigits(std::string_view s)
{
    if(s.size() >= 2 && (s.substr(0, 2) == "0b" || s.substr(0, 2) == "0B")) {
        return s.substr(2);
    }
    return s;
}

/**
 * Builds an Instruction the same way as typing into InsEntry_ and clicking Parse (no GTK).
 * Returns nullptr for empty input. May throw std::out_of_range / std::invalid_argument from parsing.
 */
inline std::unique_ptr<Instruction> instructionFromInsEntryLine(const std::string &inputStr, bool hasSetABI)
{
    if(inputStr.empty()) {
        return nullptr;
    }
    if(insEntryLooksLikeBinary(inputStr)) {
        const std::string cleanStr(insEntryBinaryDigits(inputStr));
        uint64_t value= std::stoull(cleanStr, nullptr, 2);
        if(value > 0xFFFFFFFFULL) {
            throw std::out_of_range("instruction value out of 32-bit range");
        }
        return std::make_unique<Instruction>(static_cast<uint32_t>(value), hasSetABI);
    }
    if(insEntryLooksLikeHex(inputStr)) {
        std::string cleanStr= inputStr;
        if(cleanStr.size() >= 2 && (cleanStr.substr(0, 2) == "0x" || cleanStr.substr(0, 2) == "0X")) {
            cleanStr= cleanStr.substr(2);
        }
        uint64_t value= std::stoull(cleanStr, nullptr, 16);
        if(value > 0xFFFFFFFFULL) {
            throw std::out_of_range("instruction value out of 32-bit range");
        }
        return std::make_unique<Instruction>(static_cast<uint32_t>(value), hasSetABI);
    }
    return std::make_unique<Instruction>(inputStr, hasSetABI);
}
