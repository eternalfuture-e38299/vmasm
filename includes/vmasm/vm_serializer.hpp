/*******************************************************************************
 * 文件名称: vm_serializer
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
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

namespace VMAsm {

    struct Value;
    struct Instruction;
    class VirtualMachine;

    class VMSerializer {
        public:
            static bool SaveToFile(const std::list<Instruction>& instructions, const std::unordered_map<std::string, long>& tables,
                                   const std::string& filename);
            static bool SaveToFile(VirtualMachine * vm, const std::string& filename);
            static bool LoadFromFile(VirtualMachine *vm, const std::string& filename);

        private:

            static void WriteSizedData(std::ofstream &file, const void *data, uint32_t size);
            static std::vector<uint8_t> ReadSizedData(std::ifstream &file);

            static void SerializeTables(const std::unordered_map<std::string, long> &tables, std::ofstream &file);
            static std::unordered_map<std::string, long> DeserializeTables(std::ifstream &file, uint32_t num_tables);

            static void SerializeValue(const Value& value, std::vector<uint8_t>& buffer);
            static void SerializeInstruction(const Instruction& instr, std::vector<uint8_t>& buffer);

            static Value DeserializeValue(const uint8_t*& data);
            static Instruction DeserializeInstruction(const uint8_t*& data);
    };

}
