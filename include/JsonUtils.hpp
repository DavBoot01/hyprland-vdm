#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace VDM::Json {

inline std::string escape(std::string_view input) {
    std::string out;
    out.reserve(input.size() + 8);

    auto hexDigit = [](uint8_t v) -> char {
        return v < 10 ? static_cast<char>('0' + v) : static_cast<char>('A' + (v - 10));
    };

    for (unsigned char c : input) {
        switch (c) {
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (c < 0x20) {
                out += "\\u00";
                out += hexDigit(static_cast<uint8_t>((c >> 4) & 0xF));
                out += hexDigit(static_cast<uint8_t>(c & 0xF));
            } else {
                out.push_back(static_cast<char>(c));
            }
            break;
        }
    }

    return out;
}

inline std::string quote(std::string_view input) {
    std::string out;
    out.reserve(input.size() + 2);
    out.push_back('"');
    out += escape(input);
    out.push_back('"');
    return out;
}

} // namespace VDM::Json
