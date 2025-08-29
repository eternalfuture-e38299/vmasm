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

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace VMAsm {
    struct Value;
    struct Instruction;
    class VirtualMachine;

    class Disassembler {
        public:
            std::string DisassembleFile(const std::string& src_path);
            std::string Disassemble(VirtualMachine* vm);

        private:
            // 反汇编工具方法
            std::string DisassembleInstruction(const Instruction& instr);
            std::string ValueToString(const Value& val);

            static bool IsValidDouble(double d);
            static std::string FormatDouble(double d);

            // 格式化工具
            static std::string FormatHex(uint8_t byte);
            static std::string FormatString(const std::string& str);
            static std::string FormatByteArray(const std::vector<uint8_t>& bytes);

            // 反向查找表
            std::unordered_map<long, std::string> _labelMap;
            std::unordered_map<long, std::string> _tableMap;


            void BuildReverseMaps(VirtualMachine* vm);
    };
}
