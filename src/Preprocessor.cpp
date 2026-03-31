#include "../include/Preprocessor.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <vector>

bool preprocess(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "preprocess: cannot open '" << filename << "'\n";
        return false;
    }

    std::vector<std::string> raw;
    for (std::string line; getline(infile, line);)
    	raw.push_back(line);
    infile.close();

    std::vector<Entry> entries;
    std::map<std::string, int> label_pc;
    std::vector<std::string> pending_labels; // labels seen before the next instruction
    int pc = 0;
    for (const auto &raw_line : raw) {
        std::string line = stripComment(raw_line);
        if (line.empty()) continue;
        if (isMemoryDecl(line)) {
            entries.push_back({line, true});
            continue;
        }
        auto [label, rest] = extractLabel(line);
        if (!label.empty()) {
            pending_labels.push_back(label);
            line = rest;
        }
        if (line.empty()) {
            continue;
        }
        for (const auto &label : pending_labels) label_pc[label] = pc;
        pending_labels.clear();
        entries.push_back({line, false});
        pc++;
    }

    const std::set<std::string> branch_ops = {"beq", "bne", "blt", "ble"};

    std::vector<std::string> output;
    output.reserve(entries.size());

    for (auto &entry : entries) {
        if (entry.is_mem_decl) {
            output.push_back(entry.text);
            continue;
        }
        const std::string &text = entry.text;
        std::istringstream iss(text);
        std::string op;
        iss >> op;
        if (op == "j") { // Syntax: j label
            std::string target;
            iss >> target;
			output.push_back("j " + std::to_string(label_pc[target]));
        } else if (branch_ops.count(op)) { // Syntax:  op rs1, rs2, label
            size_t last_comma = text.rfind(',');
            std::string target = trim(text.substr(last_comma + 1));
			output.push_back(text.substr(0, last_comma + 1) + " " + std::to_string(label_pc[target])); 
        } else {
            output.push_back(text);
        }
    }

    std::ofstream outfile(filename);
    if (!outfile) {
        std::cerr << "preprocess: cannot write to '" << filename << "'\n";
        return false;
    }

    for (const auto &l : output)
        outfile << l << std::endl;

    return true;
}

