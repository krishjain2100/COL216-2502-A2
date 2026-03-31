#pragma once
#include <string>
#include <utility>
#include <vector>
#include <sstream>

inline std::string trim(const std::string &s) {
    int start = 0, end = (int)s.size();
    for (; start < end; start++)
        if (s[start] != ' ' && s[start] != '\t' && s[start] != '\r' && s[start] != '\n') break;
    for (; end > start; end--)
        if (s[end-1] != ' ' && s[end-1] != '\t' && s[end-1] != '\r' && s[end-1] != '\n') break;
    return s.substr(start, end - start);
}

inline std::string stripComment(const std::string &line) {
    size_t pos = line.find('#');
    return trim(pos == std::string::npos ? line : line.substr(0, pos));
}

inline std::pair<std::string, std::string> extractLabel(const std::string &line) {
    size_t colon = line.find(':');
    if (colon == std::string::npos) return {"", line};
    std::string label = trim(line.substr(0, colon));
    if (label.empty()) return {"", line};
    return {label, trim(line.substr(colon + 1))};
}

inline std::vector<std::string> splitComma(const std::string &s) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string tok;
    while (getline(ss, tok, ',')) out.push_back(trim(tok));
    return out;
}

inline int parseReg(const std::string &s) { return stoi(s.substr(1)); }

inline bool isMemoryDecl(const std::string &line) { return !line.empty() && line[0] == '.'; }