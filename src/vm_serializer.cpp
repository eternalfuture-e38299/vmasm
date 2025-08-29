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

#include "vmasm/vm_serializer.hpp"
#include "vmasm/vm.hpp"

#include <fstream>

void VMAsm::VMSerializer::WriteSizedData(std::ofstream& file, const void* data, const uint32_t size) {
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    if (size > 0) file.write(static_cast<const char*>(data), size);
}

std::vector<uint8_t> VMAsm::VMSerializer::ReadSizedData(std::ifstream& file) {
    uint32_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    std::vector<uint8_t> data(size);
    if (size > 0) file.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}

void VMAsm::VMSerializer::SerializeTables(const std::unordered_map<std::string, long>& tables, std::ofstream& file) {
    for (const auto& [key, value] : tables) {
        WriteSizedData(file, key.c_str(), static_cast<uint32_t>(key.size() + 1));
        file.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }
}

std::unordered_map<std::string, long> VMAsm::VMSerializer::DeserializeTables(std::ifstream& file, const uint32_t num_tables) {
    std::unordered_map<std::string, long> tables;
    for (uint32_t i = 0; i < num_tables; i++) {
        auto key_data = ReadSizedData(file);
        std::string key(reinterpret_cast<const char*>(key_data.data()));

        long value;
        file.read(reinterpret_cast<char*>(&value), sizeof(value));

        tables.emplace(std::move(key), value);
    }
    return tables;
}

void VMAsm::VMSerializer::SerializeValue(const Value& value, std::vector<uint8_t>& buffer) {
    buffer.push_back(value.is_reg ? 1 : 0);

    const auto data_size = static_cast<uint32_t>(value.data.size());
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&data_size),
                 reinterpret_cast<const uint8_t*>(&data_size) + sizeof(data_size));
    buffer.insert(buffer.end(), value.data.begin(), value.data.end());
}

void VMAsm::VMSerializer::SerializeInstruction(const Instruction& instr, std::vector<uint8_t>& buffer) {
    buffer.push_back(static_cast<uint8_t>(instr.code));

    buffer.push_back(static_cast<uint8_t>(instr.Args.size()));
    for (const auto& arg : instr.Args) {
        SerializeValue(arg, buffer);
    }
}

VMAsm::Value VMAsm::VMSerializer::DeserializeValue(const uint8_t*& data) {
    Value value;
    value.is_reg = (*data++ != 0);

    uint32_t data_size;
    memcpy(&data_size, data, sizeof(data_size));
    data += sizeof(data_size);

    value.data.assign(data, data + data_size);
    data += data_size;

    // ReSharper disable once CppSomeObjectMembersMightNotBeInitialized
    return value;
}

VMAsm::Instruction VMAsm::VMSerializer::DeserializeInstruction(const uint8_t*& data) {
    Instruction instr;
    instr.code = static_cast<OpCode>(*data++);

    const uint8_t num_args = *data++;
    for (uint8_t i = 0; i < num_args; i++) {
        instr.Args.push_back(DeserializeValue(data));
    }

    return instr;
}


bool VMAsm::VMSerializer::SaveToFile(
    const std::list<Instruction>& instructions,
    const std::unordered_map<std::string, long>& tables,
    const std::string& filename
) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    constexpr char header[] = {'V', 'M', 'C', 0x01};
    file.write(header, sizeof(header));

    const auto num_tables = static_cast<uint32_t>(tables.size());
    file.write(reinterpret_cast<const char*>(&num_tables), sizeof(num_tables));
    SerializeTables(tables, file);

    const auto num_instructions = static_cast<uint32_t>(instructions.size());
    file.write(reinterpret_cast<const char*>(&num_instructions), sizeof(num_instructions));

    std::vector<uint8_t> instr_buffer;
    for (const auto& instr : instructions) {
        instr_buffer.clear();
        SerializeInstruction(instr, instr_buffer);
        WriteSizedData(file, instr_buffer.data(), static_cast<uint32_t>(instr_buffer.size()));
    }

    return true;
}

bool VMAsm::VMSerializer::SaveToFile(VirtualMachine *vm, const std::string &filename) {
    return SaveToFile(vm->GetInstructions(), vm->GetTables(), filename);
}

bool VMAsm::VMSerializer::LoadFromFile(VirtualMachine* vm, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    char header[4];
    file.read(header, sizeof(header));
    if (memcmp(header, "VMC", 3) != 0 || header[3] != 0x01) return false;

    uint32_t num_tables;
    file.read(reinterpret_cast<char*>(&num_tables), sizeof(num_tables));
    auto tables = DeserializeTables(file, num_tables);
    vm->SetTables(tables);

    uint32_t num_instructions;
    file.read(reinterpret_cast<char*>(&num_instructions), sizeof(num_instructions));

    std::list<Instruction> instructions;
    for (uint32_t i = 0; i < num_instructions; i++) {
        auto instr_data = ReadSizedData(file);
        const uint8_t* ptr = instr_data.data();
        instructions.push_back(DeserializeInstruction(ptr));
    }
    vm->SetInstructions(instructions);

    return true;
}
