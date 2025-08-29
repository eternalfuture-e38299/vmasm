/*******************************************************************************
 * 文件名称: syscalls
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

#include "vmasm/syscalls.hpp"

#include <iostream>

#include "vmasm/vm.hpp"

#include <sstream>
#include <stdexcept>

template<typename T>
T getValueAs(const VMAsm::Value& arg, VMAsm::VirtualMachine* vm) {
    if (arg.is_reg) {
        return vm->GetRegisterValue(arg.to<uint8_t>()).to<T>();
    }
    return arg.to<T>();
}

void VMAsm::SysCallRegistry::SysPrint(VirtualMachine *vm, const std::vector<Value> &args) {
    if (args.empty()) {
        throw std::runtime_error("printf requires format string");
    }

    std::string fmt;
    if (args[0].is_reg) {
        fmt = vm->GetRegisterValue(args[0].to<uint8_t>()).to<std::string>();
    } else {
        fmt = args[0].to<std::string>();
    }

    std::ostringstream output;
    size_t arg_index = 1;

    for (size_t i = 0; i < fmt.size(); ++i) {
        if (fmt[i] == '%' && i + 1 < fmt.size()) {
            const char spec = fmt[++i];
            if (arg_index >= args.size()) {
                throw std::runtime_error("Not enough arguments for format string");
            }

            const Value& arg = args[arg_index++];
            switch (spec) {
                case 'd': // 整数
                    output << getValueAs<long>(arg, vm);
                    break;
                case 'f': // 浮点数
                    output << getValueAs<double>(arg, vm);
                    break;
                case 's': // 字符串
                    output << getValueAs<std::string>(arg, vm);
                    break;
                case 'c': // 字符
                    output << static_cast<char>(getValueAs<int>(arg, vm));
                    break;
                case 'x': // 十六进制
                    output << std::hex << getValueAs<long>(arg, vm) << std::dec;
                    break;
                case '%':
                    output << '%';
                    arg_index--;
                    break;
                default:
                    throw std::runtime_error("Invalid format specifier: %" + std::string(1, spec));
            }
        } else {
            output << fmt[i];
        }
    }

    std::cout << output.str() << std::flush;
}

void VMAsm::SysCallRegistry::SysExit(VirtualMachine *vm, const std::vector<Value> &args) {
    auto& arg = args.at(0);
    exit(arg.is_reg ? vm->GetRegisterValue(arg.to<uint8_t>()).to<int>() : arg.to<int>());
}

void VMAsm::SysCallRegistry::SysRand(VirtualMachine *vm, const std::vector<Value> &args) {
    Value rand;
}

void VMAsm::SysCallRegistry::Init(VirtualMachine *vm) {
    vm->RegisterSyscall(1, SysPrint);
    vm->RegisterSyscall(2, SysExit);
}
