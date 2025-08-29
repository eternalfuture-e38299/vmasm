/*******************************************************************************
 * 文件名称: VmasmCompiler
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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "vmasm/compiler.hpp"

#include "vmasm/disassembler.hpp"
#include "vmasm/syscalls.hpp"
#include "vmasm/vm.hpp"
#include "vmasm/vm_serializer.hpp"

namespace fs = std::filesystem;

void printHelp() {
    std::cout << "VMAsm Tools - Official VMAsm utility toolkit\n\n"
              << "Usage: vmasm-tool <command> [options] [file...]\n\n"
              << "Commands:\n"
              << "  run     Execute a VMAsm program\n"
              << "  build   Compile VMAsm source to bytecode\n"
              << "  disasm  Disassemble bytecode to VMAsm\n\n"
              << "Options:\n"
              << "  -o, --output <file>  Specify output file\n"
              << "  -v, --verbose        Enable verbose output\n"
              << "  -h, --help           Show this help message\n";
}

int runCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Error: No input file specified for run command\n";
        return 1;
    }

    const std::string& inputFile = args[0];
    if (!fs::exists(inputFile)) {
        std::cerr << "Error: Input file not found: " << inputFile << "\n";
        return 1;
    }

    try {
        VMAsm::VirtualMachine vm;
        VMAsm::SysCallRegistry::Init(&vm);

        // Load and compile
        if (!VMAsm::VMSerializer::LoadFromFile(&vm, inputFile)) {
            std::cerr << "Compilation failed\n";
            return 1;
        }

        // Execute
        vm.Execute();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int buildCommand(const std::vector<std::string>& args, const std::string& outputFile, const bool verbose) {
    if (args.empty()) {
        std::cerr << "Error: No input files specified for build command\n";
        return 1;
    }

    // Verify input files exist
    for (const auto& file : args) {
        if (!fs::exists(file)) {
            std::cerr << "Error: Input file not found: " << file << "\n";
            return 1;
        }
    }

    try {
        const std::string outPath = outputFile.empty() ? "a.vmc" : outputFile;

        if (verbose) {
            std::cout << "Compiling " << args.size() << " file(s) to " << outPath << "...\n";
        }

        if (VMAsm::Compiler compiler; compiler.Compile(args, outPath)) {
            if (verbose) {
                std::cout << "Compilation successful. Output written to " << outPath << "\n";
            }
            return 0;
        } else {
            std::cerr << "Compilation failed\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int disasmCommand(const std::vector<std::string>& args, const std::string& outputFile, const bool verbose) {
    if (args.empty()) {
        std::cerr << "Error: No input file specified for disasm command\n";
        return 1;
    }

    const std::string& inputFile = args[0];
    if (!fs::exists(inputFile)) {
        std::cerr << "Error: Input file not found: " << inputFile << "\n";
        return 1;
    }

    try {

        std::ostream* out = &std::cout;
        std::ofstream outFile;

        if (!outputFile.empty()) {
            outFile.open(outputFile);
            if (!outFile) {
                std::cerr << "Error: Could not open output file " << outputFile << "\n";
                return 1;
            }
            out = &outFile;
        }

        if (verbose) {
            *out << "// Disassembly of " << inputFile << "\n";
            *out << "// Generated by VMAsm Tools\n\n";
        }

        *out << VMAsm::Disassembler().DisassembleFile(inputFile);

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int main(const int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    const std::string command = argv[1];
    std::vector<std::string> args;
    std::string outputFile;
    bool verbose = false;

    // Parse options
    for (int i = 2; i < argc; ++i) {
        if (std::string arg = argv[i]; arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputFile = argv[++i];
        } else {
            args.push_back(arg);
        }
    }

    if (command == "run") {
        return runCommand(args);
    }
    if (command == "build") {
        return buildCommand(args, outputFile, verbose);
    }
    if (command == "disasm") {
        return disasmCommand(args, outputFile, verbose);
    }
    std::cerr << "Error: Unknown command '" << command << "'\n";
    printHelp();
    return 1;
}
