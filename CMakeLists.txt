cmake_minimum_required(VERSION 3.31)
project(vmasm)

set(CMAKE_CXX_STANDARD 17)

add_library(vmasm STATIC
        src/vm.cpp
        src/vm_serializer.cpp
        src/compiler.cpp
        src/disassembler.cpp
        src/syscalls.cpp
)

target_include_directories(vmasm
        PUBLIC
        includes
)

set(VMASM_TEST OFF)
if (${VMASM_TEST})
    add_executable(test
            test/test.cpp
    )

    target_link_libraries(test
            vmasm
    )
endif ()

set(EXAMPLE ON)
if (${EXAMPLE})
    add_executable(VMAsmCLI
            example/VMAsmCLI.cpp
    )

    target_link_libraries(VMAsmCLI
            vmasm
    )
endif ()