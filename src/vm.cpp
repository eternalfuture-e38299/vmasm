/*******************************************************************************
 * 文件名称: vm
 * 项目名称: TEFModLoader
 * 创建时间: 25-8-27
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

#include "vmasm/vm.hpp"

#include <stdexcept>

int VMAsm::VirtualMachine::Interpreter(const Instruction &instruction) {
    switch (instruction.code) {
        // 基础指令
        case OpCode::NOP:
            break;

        case OpCode::JMP: {
            const auto target  = instruction.Args.at(0);
            _program_counter =  target.is_reg ? _regs[target.to<uint8_t>()].to<long>() :
            target.is_table ? _tables[target.to<std::string>()] : target.to<long>();
        } break;

        case OpCode::MOV: {
            Value table;
            const Value& src = instruction.Args.at(0);
            const auto dst_reg = instruction.Args.at(1).to<uint8_t>();
            _regs[dst_reg] =
                src.is_reg ? _regs[src.to<uint8_t>()] :
            src.is_table ? *table.write<long>(_tables[src.to<std::string>()]) : src;
        } break;

        case OpCode::ADD: {
            const auto dst_reg = instruction.Args.at(2).to<uint8_t>();
            const long val1 = instruction.Args.at(0).is_reg ?
                _regs[instruction.Args.at(0).to<uint8_t>()].to<long>() :
                instruction.Args.at(0).to<long>();
            const long val2 = instruction.Args.at(1).is_reg ?
                _regs[instruction.Args.at(1).to<uint8_t>()].to<long>() :
                instruction.Args.at(1).to<long>();
            _regs[dst_reg].write(val1 + val2);
        } break;

        case OpCode::SUB: {
            const auto dst_reg = instruction.Args.at(2).to<uint8_t>();
            const long val1 = instruction.Args.at(0).is_reg ?
                _regs[instruction.Args.at(0).to<uint8_t>()].to<long>() :
                instruction.Args.at(0).to<long>();
            const long val2 = instruction.Args.at(1).is_reg ?
                _regs[instruction.Args.at(1).to<uint8_t>()].to<long>() :
                instruction.Args.at(1).to<long>();
            _regs[dst_reg].write(val1 - val2);
        } break;

        case OpCode::NEG: {
            const auto src_reg = instruction.Args.at(0).to<uint8_t>();
            const auto dst_reg = instruction.Args.at(1).to<uint8_t>();
            _regs[dst_reg].write(-_regs[src_reg].to<long>());
        } break;

        // 快照指令
        case OpCode::SNAP_SAVE: {
            _regs_snap = _regs;
        } break;

        case OpCode::SNAP_SWAP: {
            std::swap(_regs, _regs_snap);
        } break;

        case OpCode::SNAP_CLEAR: {
            _regs_snap.clear();
            _regs_snap.resize(64);
        } break;

        case OpCode::REGS_CLEAR: {
            _regs.clear();
            _regs.resize(64);
        } break;

        // 控制指令
        case OpCode::JZ: {
            const auto& src = instruction.Args.at(0);
            const long val = src.is_reg ?
                _regs[src.to<uint8_t>()].to<long>() :
                src.to<long>();
            if (val == 0) {
                const auto& target = instruction.Args.at(1);
                _program_counter = target.is_reg ? _regs[target.to<uint8_t>()].to<long>() :
                target.is_table ? _tables[target.to<std::string>()] : target.to<long>();
            }
        } break;

        case OpCode::JNZ: {
            const auto& src = instruction.Args.at(0);
            const long val = src.is_reg ?
                _regs[src.to<uint8_t>()].to<long>() :
                src.to<long>();

            if (val != 0) {
                const auto& target = instruction.Args.at(1);
                _program_counter = target.is_reg ? _regs[target.to<uint8_t>()].to<long>() :
                target.is_table ? _tables[target.to<std::string>()] : target.to<long>();
            }
        } break;

        case OpCode::JG: {
            const auto& src = instruction.Args.at(0);
            const long val = src.is_reg ?
                _regs[src.to<uint8_t>()].to<long>() :
                src.to<long>();

            if (val > 0) {
                const auto& target = instruction.Args.at(1);
                _program_counter = target.is_reg ? _regs[target.to<uint8_t>()].to<long>() :
                target.is_table ? _tables[target.to<std::string>()] : target.to<long>();
            }
        } break;

        case OpCode::JL: {
            const auto& src = instruction.Args.at(0);
            const long val = src.is_reg ?
                _regs[src.to<uint8_t>()].to<long>() :
                src.to<long>();

            if (val < 0) {
                const auto& target = instruction.Args.at(1);
                _program_counter = target.is_reg ? _regs[target.to<uint8_t>()].to<long>() :
                target.is_table ? _tables[target.to<std::string>()] : target.to<long>();
            }
        } break;

        // 系统指令
        case OpCode::HALT: {
            return 1; // 停止执行
        }

        case OpCode::SYS: {
            if (instruction.Args.empty()) {
                throw std::runtime_error("SYS call requires at least call ID");
            }

            const auto syscall_id = instruction.Args[0].to<uint8_t>();
            std::vector<Value> args;

            for (size_t i = 1; i < instruction.Args.size(); ++i) args.push_back(instruction.Args[i]);

            if (const auto it = SyscallTable.find(syscall_id); it != SyscallTable.end()) {
                try {
                    it->second(this, args);
                } catch (const std::exception& e) {
                    throw std::runtime_error(std::string("Syscall ") +
                                           std::to_string(syscall_id) +
                                           " failed: " + e.what());
                }
            } else {
                throw std::runtime_error("Undefined syscall: " + std::to_string(syscall_id));
            }
        } break;

        default:
            throw std::runtime_error("Unknown instruction");
    }
    return 0;
}

int VMAsm::VirtualMachine::Run(const long start) {
    _program_counter = start;
    auto it = _instructions.begin();
    while (it != _instructions.end()) {
        if (const int result = Interpreter(*it); result != 0) return result;


        // ReSharper disable once CppDFAConstantConditions
        if (_program_counter != start) {
            it = std::next(_instructions.begin(), _program_counter);
            _program_counter = start;
        } else {
            // ReSharper disable once CppDFAUnreachableCode
            ++it;
        }
    }
    return 0;
}

bool VMAsm::VirtualMachine::RegisterSyscall(const int id, const VirtualMethod &method) {
    if (id == 0) return false;
    SyscallTable[id] = method;
    return true;
}

int VMAsm::VirtualMachine::Execute(const std::string &table) {
    return Run(_tables[table]);
}

void VMAsm::VirtualMachine::AddInstruction(const Instruction& instruction) {
    _instructions.push_back(instruction);
}

void VMAsm::VirtualMachine::SetRegisterValue(const uint8_t register_index, const Value &value) {
    if (register_index >= _regs.size()) {
        throw std::out_of_range("Register index out of range");
    }
    _regs[register_index] = value;
}

VMAsm::Value VMAsm::VirtualMachine::GetRegisterValue(const uint8_t register_index) {
    if (register_index >= _regs.size()) {
        throw std::out_of_range("Register index out of range");
    }
    return _regs[register_index];
}