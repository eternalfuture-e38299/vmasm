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
 
#pragma once

#include <cstdint>
#include <cstring>
#include <functional>
#include <list>
#include <string>
#include <vector>

namespace VMAsm {

    enum class OpCode : uint8_t {
        // 基础指令
        NOP = 0,    // 空指令
        JMP,        // 无条件跳转
        MOV,        // 写入寄存器
        ADD,        // 整数加法
        SUB,        // 整数减法
        NEG,        // 取相反数
        SNAP_SAVE,  // 将寄存器写入快照
        SNAP_SWAP,  // 交换快照与寄存器
        SNAP_CLEAR, // 清空快照
        REGS_CLEAR, // 清空寄存器

        // 控制指令
        JZ,         // 等于零跳转
        JNZ,        // 不等于零跳转
        JG,        // 大于零跳转
        JL,        // 小于零跳转

        // 系统指令
        HALT,       // 停机
        SYS         // 系统调用
    };

    struct Value {
        bool is_reg{};
        bool is_table;
        std::vector<uint8_t> data{};

        template<typename T>
        T* to_ptr() {
            return reinterpret_cast<T*>(data.data());
        }

        template<typename T>
        T to() const {
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
            static_assert(alignof(T) <= alignof(std::max_align_t), "T's alignment is too strict");

            if (data.empty()) return T{};

            alignas(T) uint8_t buffer[sizeof(T)];

            const size_t bytes_to_copy = std::min(data.size(), sizeof(T));
            std::memcpy(buffer, data.data(), bytes_to_copy);

            if (bytes_to_copy < sizeof(T)) std::memset(buffer + bytes_to_copy, 0, sizeof(T) - bytes_to_copy);

            return *reinterpret_cast<T*>(buffer);
        }

        void clear() {
            is_reg = false;
            data.clear();
        }

        template<typename T>
        Value *write(const T &value) {
            static_assert(std::is_fundamental_v<T>, "Only fundamental types are supported");
            data.resize(sizeof(T));
            std::memcpy(data.data(), &value, sizeof(T));

            return this;
        }

        Value * write(const char* str) {
            if (str == nullptr) {
                data.clear();
                return this;
            }
            const size_t len = std::strlen(str) + 1;
            data.resize(len);
            std::memcpy(data.data(), str, len);

            return this;
        }

        Value * write(const std::string& str) {
            data.resize(str.size() + 1);
            std::memcpy(data.data(), str.c_str(), str.size() + 1);

            return this;
        }

        template<typename Container>
        Value* write_container(const Container& container) {
            using ValueType = typename Container::value_type;
            static_assert(std::is_fundamental_v<ValueType>, "Only containers of fundamental types are supported");

            data.resize(container.size() * sizeof(ValueType));
            if constexpr (std::is_same_v<ValueType, uint8_t>) std::copy(container.begin(), container.end(), data.begin());
            else std::memcpy(data.data(), container.data(), data.size());

            return this;
        }

        std::string to();
    };

    struct Instruction {
        OpCode code{};
        std::vector<Value> Args{};
    };

    class VirtualMachine {
        long _program_counter{};

        std::unordered_map<std::string, long> _tables;
        std::vector<Value> _regs {64};
        std::vector<Value> _regs_snap {64};

        std::list<Instruction> _instructions{0};

        int Interpreter(const Instruction &instruction);

        int Run(long start);

        public:
            typedef std::function<void(VirtualMachine* vm, std::vector<Value>& args)> VirtualMethod;
            std::unordered_map<int, VirtualMethod> SyscallTable;

            bool RegisterSyscall(int id, const VirtualMethod &method);
            int Execute(const std::string& table = "main");
            void AddInstruction(const Instruction& instruction);
            void SetRegisterValue(uint8_t register_index, const Value& value);
            Value GetRegisterValue(uint8_t register_index);

            void SetInstructions(const std::list<Instruction> &instructions) { _instructions = instructions; }
            std::list<Instruction>& GetInstructions() { return _instructions; }

            void SetTables(const std::unordered_map<std::string, long>& tables) { _tables = tables; }
            std::unordered_map<std::string, long>& GetTables() { return _tables; }
    };
}

template<>
inline std::string VMAsm::Value::to<std::string>() const {
    if (data.empty()) return "";
    return data.back() == '\0'
        ? std::string(reinterpret_cast<const char*>(data.data()))
        : std::string(reinterpret_cast<const char*>(data.data()), data.size());
}
