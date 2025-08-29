/*******************************************************************************
 * 文件名称: disassembler
 * 项目名称: TEFModLoader
 * 创建时间: 2025/8/29
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

#include "vmasm/disassembler.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>

#include "vmasm/vm.hpp"

#include <sstream>

#include "vmasm/vm_serializer.hpp"


std::string VMAsm::Disassembler::DisassembleFile(const std::string &src_path) {
    VirtualMachine vm;
    VMSerializer::LoadFromFile(&vm, src_path);
    return Disassemble(&vm);
}

std::string VMAsm::Disassembler::Disassemble(VirtualMachine* vm) {
    BuildReverseMaps(vm);

    std::stringstream output;
    const auto& instructions = vm->GetInstructions();

    // 反汇编所有指令
    for (const auto& instr : instructions) {
        output << DisassembleInstruction(instr) << "\n";
    }

    // 输出表定义
    for (const auto& [name, addr] : vm->GetTables()) {
        if (_labelMap.find(addr) == _labelMap.end()) {
            output << "#table " << name << "\n";
        }
    }

    return output.str();
}

std::string VMAsm::Disassembler::DisassembleInstruction(const Instruction& instr) {
    std::stringstream ss;

    // 检查当前地址是否是标签
    static long instrIndex = 0;
    if (_labelMap.count(instrIndex)) {
        ss << _labelMap[instrIndex] << ":\n";
    }
    instrIndex++;

    // 反汇编操作码
    switch (instr.code) {
        case OpCode::NOP: ss << "    nop"; break;
        case OpCode::JMP: ss << "    jmp"; break;
        case OpCode::MOV: ss << "    mov"; break;
        case OpCode::ADD: ss << "    add"; break;
        case OpCode::SUB: ss << "    sub"; break;
        case OpCode::NEG: ss << "    neg"; break;
        case OpCode::JZ: ss << "    jz"; break;
        case OpCode::JNZ: ss << "    jnz"; break;
        case OpCode::JG: ss << "    jg"; break;
        case OpCode::JL: ss << "    jl"; break;
        case OpCode::HALT: ss << "    halt"; break;
        case OpCode::SYS: ss << "    sys"; break;
        case OpCode::SNAP_SAVE: ss << "    snap_save"; break;
        case OpCode::SNAP_SWAP: ss << "    snap_swap"; break;
        case OpCode::SNAP_CLEAR: ss << "    snap_clear"; break;
        case OpCode::REGS_CLEAR: ss << "    regs_clear"; break;
    }

    // 反汇编参数
    for (size_t i = 0; i < instr.Args.size(); ++i) {
        if (i > 0) ss << ",";
        ss << " " << ValueToString(instr.Args[i]);
    }

    return ss.str();
}

std::string VMAsm::Disassembler::ValueToString(const Value& val) {
    if (val.is_reg) return "R" + std::to_string(val.to<uint8_t>());
    if (val.is_table) return "#" + val.to<std::string>();

    if (val.data.size() == sizeof(double)) {
        double d;
        memcpy(&d, val.data.data(), sizeof(double));
        if (IsValidDouble(d)) return FormatDouble(d);
    }

    if (val.data.size() == sizeof(long)) {
        const long num = val.to<long>();
        if (_labelMap.count(num)) return _labelMap[num];
        return std::to_string(num);
    }

    if (!val.data.empty() && val.data.back() == 0) return FormatString(val.to<std::string>());

    return FormatByteArray(val.data);
}

bool VMAsm::Disassembler::IsValidDouble(const double d) {
    if (std::isnan(d) || std::isinf(d)) return true;

    constexpr double min_sane_value = -1e300;
    if (constexpr double max_sane_value = 1e300; d > max_sane_value || d < min_sane_value) {
        return false;
    }

    uint64_t bits;
    memcpy(&bits, &d, sizeof(double));
    const uint64_t exponent = (bits >> 52) & 0x7FF;

    return exponent != 0 || bits == 0;
}

std::string VMAsm::Disassembler::FormatDouble(const double d) {
    std::stringstream ss;

    // 处理特殊值
    if (std::isnan(d)) {
        return "nan";
    }
    if (std::isinf(d)) {
        return d < 0 ? "-inf" : "inf";
    }

    if (d == 0.0) return "0";


    if (d == floor(d) && fabs(d) < 1e15) {
        ss << std::fixed << std::setprecision(0) << d;
        return ss.str();
    }

    ss << d;
    std::string s = ss.str();

    if (const size_t dot_pos = s.find('.'); dot_pos != std::string::npos) {
        if (const size_t last_non_zero = s.find_last_not_of('0');
            last_non_zero != std::string::npos && last_non_zero > dot_pos) {
            s.erase(last_non_zero + 1);
            if (s.back() == '.') {
                s += "0";
            }
            }
    }

    return s;
}

void VMAsm::Disassembler::BuildReverseMaps(VirtualMachine* vm) {
    // 构建标签反向映射
    for (const auto& [name, addr] : vm->GetTables()) {
        _labelMap[addr] = name;
    }

    // 构建表反向映射
    size_t instrIndex = 0;
    for (const auto&[code, Args] : vm->GetInstructions()) {
        for (const auto& arg : Args) {
            if (arg.is_table) {
                _tableMap[arg.to<long>()] = arg.to<std::string>();
            }
        }
        instrIndex++;
    }
}

std::string VMAsm::Disassembler::FormatHex(const uint8_t byte) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    return ss.str();
}

std::string VMAsm::Disassembler::FormatString(const std::string& str) {
    std::stringstream ss;
    ss << "\"";
    for (const char c : str) {
        switch (c) {
            case '\n': ss << "\\n"; break;
            case '\t': ss << "\\t"; break;
            case '\r': ss << "\\r"; break;
            case '\"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            default: ss << c;
        }
    }
    ss << "\"";
    return ss.str();
}

std::string VMAsm::Disassembler::FormatByteArray(const std::vector<uint8_t>& bytes) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << FormatHex(bytes[i]);
    }
    ss << "]";
    return ss.str();
}