#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "StringUtils.h"
using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: preprocess <file.s>\n";
        return 1;
    }
    const string filename = argv[1];
    ifstream infile(filename);
    if (!infile) {
        cerr << "preprocess: cannot open '" << filename << "'\n";
        return 1;
    }

    vector<string> raw;
    for (string line; getline(infile, line);)
    	raw.push_back(line);
    infile.close();

    struct Entry {
        string text;
        bool is_mem_decl;
    };

    vector<Entry> entries;
    map<string, int> label_pc;
    vector<string> pending_labels; // labels seen before the next instruction
    int pc = 0;
    for (const auto &raw_line : raw) {
        string line = stripComment(raw_line);
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

    const set<string> branch_ops = {"beq", "bne", "blt", "ble"};

    vector<string> output;
    output.reserve(entries.size());

    for (auto &entry : entries) {
        if (entry.is_mem_decl) {
            output.push_back(entry.text);
            continue;
        }
        const string &text = entry.text;
        istringstream iss(text);
        string op;
        iss >> op;
        if (op == "j") { // Syntax: j label
            string target;
            iss >> target;
			output.push_back("j " + to_string(label_pc[target]));
        } else if (branch_ops.count(op)) { // Syntax:  op rs1, rs2, label
            size_t last_comma = text.rfind(',');
            string target = trim(text.substr(last_comma + 1));
			output.push_back(text.substr(0, last_comma + 1) + " " + to_string(label_pc[target])); 
        } else {
            output.push_back(text);
        }
    }

    ofstream outfile(filename);
    if (!outfile) {
        cerr << "preprocess: cannot write to '" << filename << "'\n";
        return 1;
    }

    for (const auto &l : output)
        outfile << l << endl;

    return 0;
}

