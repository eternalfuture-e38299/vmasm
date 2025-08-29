/*******************************************************************************
 * 文件名称: compiler
 * 项目名称: TEFModLoader
 * 创建时间: 25-8-28
 * 作者: EternalFuture゙
 * Github: https://github.com/eternalfuture-e38299
 * 版权声明: Copyright © 2025 EternalFuture゙
 * 
 * MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/

#include "vmasm/compiler.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "vmasm/vm.hpp"
#include "vmasm/vm_serializer.hpp"

bool VMAsm::Compiler::CompileString(const std::string &context, VirtualMachine *vm) {
    _labels.clear();
    _tables.clear();
    _instructions.clear();

    ParseString(context);

    ResolveReferences();
    GenerateTables();

    vm->SetInstructions(std::list(_instructions.begin(), _instructions.end()));
    vm->SetTables(_tables);

    return true;
}

bool VMAsm::Compiler::CompileString(const std::string &context, const std::string &outPath) {
    VirtualMachine vm;
    CompileString(context, &vm);
    return VMSerializer::SaveToFile(&vm, outPath);
}

bool VMAsm::Compiler::Compile(const std::vector<std::string>& sources, VirtualMachine* vm) {
    // 重置状态
    _labels.clear();
    _tables.clear();
    _instructions.clear();
    _source_files = sources;

    ParseFiles(sources);

    // 分析与生成阶段
    ResolveReferences();
    GenerateTables();

    // 输出到虚拟机
    vm->SetInstructions(std::list(_instructions.begin(), _instructions.end()));
    vm->SetTables(_tables);
    return true;
}

bool VMAsm::Compiler::Compile(const std::vector<std::string> &sources, const std::string &outPath) {
    VirtualMachine vm;
    Compile(sources, &vm);
    return VMSerializer::SaveToFile(&vm, outPath);
}

void VMAsm::Compiler::ProcessLine(std::string line, const size_t file_idx, const int line_num, bool &in_comment_block) {
    line = Trim(line);

    if (const size_t block_start = line.find("/*"); block_start != std::string::npos) {
        in_comment_block = true;
        line = line.substr(0, block_start);
    }

    if (in_comment_block) {
        if (const size_t block_end = line.find("*/"); block_end != std::string::npos) {
            in_comment_block = false;
            line = line.substr(block_end + 2);
        } else {
            return;
        }
    }

    if (const size_t comment_pos = line.find("//"); comment_pos != std::string::npos) {
        line = line.substr(0, comment_pos);
    }

    line = Trim(line);
    if (line.empty()) return;

    if (line.find("#table") == 0) {
        const auto tokens = Tokenize(line.substr(6));
        if (tokens.size() != 1) {
            throw std::runtime_error("Invalid table definition syntax");
        }
        _tables[ToLower(tokens[0])] = 0;
        return;
    }

    if (line.back() == ':') {
        const std::string label = ToLower(line.substr(0, line.size() - 1));
        _labels[label] = {_instructions.size(), file_idx, line_num};
        return;
    }

    try {
        _instructions.push_back(ParseInstruction(line, file_idx, line_num));
    } catch (const std::exception& e) {
        throw std::runtime_error(e.what());
    }
}

bool VMAsm::Compiler::ParseString(const std::string &source) {
    std::istringstream iss(source);
    std::string line;
    int line_num = 0;
    bool in_comment_block = false;

    while (std::getline(iss, line)) {
        ++line_num;
        ProcessLine(line, -1, line_num, in_comment_block);
    }

    return true;
}

bool VMAsm::Compiler::ParseFiles(const std::vector<std::string>& sources) {
    for (size_t file_idx = 0; file_idx < sources.size(); ++file_idx) {
        std::ifstream file(sources[file_idx]);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open a file: " + sources[file_idx]);
        }

        std::string line;
        int line_num = 0;
        bool in_comment_block = false;

        while (std::getline(file, line)) {
            ++line_num;
            ProcessLine(line, file_idx, line_num, in_comment_block);
        }
    }
    return true;
}

void VMAsm::Compiler::ResolveReferences() {
    for (auto&[code, Args] : _instructions) {
        for (auto arg : Args) {
            if (!arg.is_reg && !arg.is_table) {
                if (auto ref = arg.to<std::string>(); _labels.count(ToLower(ref))) arg.write(static_cast<long>(_labels[ToLower(ref)].instruction_index));
                else if (_tables.count(ToLower(ref))) {
                    arg.is_table = true;
                    arg.write("#" + ref);
                }
            }
        }
    }
}

void VMAsm::Compiler::GenerateTables() {
    for (const auto& [label, info] : _labels) {
        _tables[label] = static_cast<long>(info.instruction_index);
    }
}

VMAsm::Instruction VMAsm::Compiler::ParseInstruction(const std::string &line, size_t file_idx, int line_num) {
    const auto tokens = Tokenize(line);
    if (tokens.empty()) {
        throw std::runtime_error("Null instructions");
    }

    Instruction instr;

    if (const std::string opcode = ToLower(tokens[0]); opcode == "nop") instr.code = OpCode::NOP;
    else if (opcode == "jmp") instr.code = OpCode::JMP;
    else if (opcode == "mov") instr.code = OpCode::MOV;
    else if (opcode == "add") instr.code = OpCode::ADD;
    else if (opcode == "sub") instr.code = OpCode::SUB;
    else if (opcode == "neg") instr.code = OpCode::NEG;
    else if (opcode == "snap_save") instr.code = OpCode::SNAP_SAVE;
    else if (opcode == "snap_swap") instr.code = OpCode::SNAP_SWAP;
    else if (opcode == "snap_clear") instr.code = OpCode::SNAP_CLEAR;
    else if (opcode == "regs_clear") instr.code = OpCode::REGS_CLEAR;
    else if (opcode == "jz") instr.code = OpCode::JZ;
    else if (opcode == "jnz") instr.code = OpCode::JNZ;
    else if (opcode == "jg") instr.code = OpCode::JG;
    else if (opcode == "jl") instr.code = OpCode::JL;
    else if (opcode == "halt") instr.code = OpCode::HALT;
    else if (opcode == "sys") instr.code = OpCode::SYS;
    else {
        throw std::runtime_error("Unknown opcodes:" + tokens[0]);
    }

    for (size_t i = 1; i < tokens.size(); ++i) {
        if (tokens[i] != ",") {
            instr.Args.push_back(ParseValue(tokens[i]));
        }
    }

    return instr;
}

VMAsm::Value VMAsm::Compiler::ParseValue(const std::string& token) {
    Value val { false, false };

    if (IsRegister(token)) {
        const std::string reg_num_str = ToLower(token).substr(1);
        const auto reg_index = static_cast<uint8_t>(std::stoi(reg_num_str));
        if (reg_index >= 64) {
            throw std::runtime_error("Register index out of range (0-63): " + token);
        }
        val.is_reg = true;
        val.write(reg_index);
        return val;
    }

    if (IsTableRef(token)) {
        val.is_table = true;
        val.write(token.substr(1));
        return val;
    }

    if (IsByteArray(token)) {
        const auto bytes = ParseByteArray(token);
        val.write_container(bytes);
        return val;
    }

    if (IsStringLiteral(token)) {
        const std::string str = UnescapeString(token.substr(1, token.size() - 2));
        val.write(str);
        return val;
    }

    if (IsFloat(token)) {
        const double num = std::stod(token);
        val.write(num);
        return val;
    }

    if (IsInteger(token)) {
        const long num = std::stol(token);
        val.write(num);
        return val;
    }

    val.write(token);
    return val;
}


std::vector<std::string> VMAsm::Compiler::Tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_string = false;
    bool in_array = false;

    for (const char c : line) {
        if (in_string) {
            if (c == '"') {
                token += c;
                tokens.push_back(token);
                token.clear();
                in_string = false;
            } else {
                token += c;
            }
        } else if (in_array) {
            if (c == ']') {
                token += c;
                tokens.push_back(token);
                token.clear();
                in_array = false;
            } else {
                token += c;
            }
        } else {
            if (isspace(c) || c == ',') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                if (c == ',') tokens.emplace_back(",");
            } else if (c == '"') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                token += c;
                in_string = true;
            } else if (c == '[') {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
                token += c;
                in_array = true;
            } else {
                token += c;
            }
        }
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}

bool VMAsm::Compiler::IsRegister(const std::string& token) {
    if (token.size() < 2) return false;
    if (const auto first = tolower(token[0]); first != 'r') return false;
    return IsInteger(token.substr(1));
}

bool VMAsm::Compiler::IsTableRef(const std::string& token) {
    return !token.empty() && token[0] == '#';
}

bool VMAsm::Compiler::IsByteArray(const std::string& token) {
    return token.size() >= 2 && token.front() == '[' && token.back() == ']';
}

bool VMAsm::Compiler::IsStringLiteral(const std::string& token) {
    return token.size() >= 2 && token.front() == '"' && token.back() == '"';
}

bool VMAsm::Compiler::IsFloat(const std::string &token) {
    if (token.empty()) return false;

    size_t start = 0;
    bool hasDecimal = false;
    bool hasExponent = false;
    bool digitSeen = false;

    if (token[0] == '+' || token[0] == '-') {
        start = 1;
        if (token.size() == 1) return false;
    }

    for (size_t i = start; i < token.size(); ++i) {
        const char c = token[i];

        if (isdigit(c)) {
            digitSeen = true;
            continue;
        }

        if (c == '.') {
            if (hasDecimal || hasExponent) return false;
            hasDecimal = true;
            continue;
        }

        if (c == 'e' || c == 'E') {
            if (hasExponent || !digitSeen) return false;
            hasExponent = true;
            digitSeen = false;

            if (i + 1 < token.size() && (token[i+1] == '+' || token[i+1] == '-')) ++i;
            continue;
        }

        return false;
    }

    return digitSeen && (hasDecimal || hasExponent);
}

bool VMAsm::Compiler::IsInteger(const std::string& token) {
    if (token.empty()) return false;

    long start = 0;
    if (token[0] == '+' || token[0] == '-') {
        start = 1;
        if (token.size() == 1) return false;
    }

    return std::all_of(token.begin() + start, token.end(), ::isdigit);
}

std::vector<uint8_t> VMAsm::Compiler::ParseByteArray(const std::string& token) {
    std::vector<uint8_t> bytes;
    const std::string content = token.substr(1, token.size() - 2);
    std::istringstream iss(content);
    std::string byte_str;

    while (std::getline(iss, byte_str, ',')) {
        byte_str = Trim(byte_str);
        if (byte_str.empty()) continue;

        if (byte_str.find("0x") == 0 || byte_str.find("0X") == 0) byte_str = byte_str.substr(2);

        char* end;
        const long byte = strtol(byte_str.c_str(), &end, 16);
        if (*end != '\0') throw std::runtime_error("Invalid byte format: " + byte_str);
        if (byte < 0 || byte > 255) throw std::runtime_error("Byte value out of range (0-255): " + byte_str);

        bytes.push_back(static_cast<uint8_t>(byte));
    }

    return bytes;
}

std::string VMAsm::Compiler::ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](const unsigned char c) {
        return std::tolower(c);
    });
    return s;
}

std::string VMAsm::Compiler::Trim(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](const int ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](const int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

std::string VMAsm::Compiler::UnescapeString(const std::string &s) {
    std::string result;
    bool escape = false;

    for (const char c : s) {
        if (escape) {
            switch (c) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '0': result += '\0'; break;
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                default: throw std::runtime_error("Invalid escape sequence: \\" + std::string(1, c));
            }
            escape = false;
        } else if (c == '\\') {
            escape = true;
        } else {
            result += c;
        }
    }

    if (escape) {
        throw std::runtime_error("Unfinished escape sequences");
    }

    return result;
}