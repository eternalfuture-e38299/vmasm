/*******************************************************************************
 * 文件名称: test
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

#include <iostream>

#include <filesystem>
#include "vmasm/compiler.hpp"
#include "vmasm/disassembler.hpp"
#include "vmasm/syscalls.hpp"
#include "vmasm/vm.hpp"

int main(int argc, char *argv[]) {
    std::filesystem::path path = std::filesystem::path(".").parent_path();

    std::cout << path;

    VMAsm::VirtualMachine vm;
    VMAsm::Compiler compiler;

    VMAsm::SysCallRegistry::Init(&vm);

    if (compiler.Compile({
        "/home/yuwu/CLionProjects/Vmasm/test/test.vmasm",
    }, &vm)) {
        std::cout << "编译成功!" << std::endl;
        vm.Execute(); // 从main标签开始执行
    } else {
        std::cerr << "编译失败!" << std::endl;
    }

    VMAsm::Disassembler disassembler;
    std::string asmCode = disassembler.Disassemble(&vm);

    std::cout << "Disassembled code:\n";
    std::cout << asmCode << std::endl;


    return 0;
}