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
 
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace VMAsm {

    struct Instruction;
    struct Value;
    class VirtualMachine;

    class Compiler {
        public:
            bool CompileString(const std::string& context, VirtualMachine* vm);
            bool CompileString(const std::string& context, const std::string& outPath);
            bool Compile(const std::vector<std::string>& sources, VirtualMachine* vm);
            bool Compile(const std::vector<std::string>& sources, const std::string& outPath);

        private:
            struct LabelInfo {
                size_t instruction_index;
                size_t file_index;
                int line_number;
            };

            // 编译状态
            std::unordered_map<std::string, LabelInfo> _labels;
            std::unordered_map<std::string, long> _tables;
            std::vector<Instruction> _instructions;
            std::vector<std::string> _source_files;

            // 核心方法
            void ProcessLine(std::string line, size_t file_idx, int line_num, bool& in_comment_block);
            bool ParseString(const std::string& source);
            bool ParseFiles(const std::vector<std::string>& sources);
            void ResolveReferences();
            void GenerateTables();

            // 工具方法
            static Instruction ParseInstruction(const std::string &line, size_t file_idx, int line_num);

            static Value ParseValue(const std::string& token);

            static std::vector<std::string> Tokenize(const std::string& line);

            // 类型判断
            static bool IsRegister(const std::string& token);
            static bool IsTableRef(const std::string& token);
            static bool IsByteArray(const std::string& token);
            static bool IsStringLiteral(const std::string& token);
            static bool IsFloat(const std::string& token);
            static bool IsInteger(const std::string& token);

            // 转换方法
            static std::string ToLower(std::string s);
            static std::string Trim(std::string s);
            static std::vector<uint8_t> ParseByteArray(const std::string& token);
            static std::string UnescapeString(const std::string &s);
    };

}