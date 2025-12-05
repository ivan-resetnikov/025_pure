#!/bin/bash

build_windows_x86_64() {
    clang_bin="clang"

    # Compiler flags
    compiler_flags=""
    compiler_flags="${compiler_flags} -g"
    compiler_flags="${compiler_flags} -DP_WINDOWS_X86_64"
    compiler_flags="${compiler_flags} -D_CRT_SECURE_NO_WARNINGS"

    # Source files
    src_path="./src"

    src_files=""
    src_files="${src_files} ${src_path}/pack.c"

    # Include
    include_flags=""
    include_flags="${include_flags}"

    # Linking
    link_flags=""
    link_flags="${link_flags} -lshlwapi"

    # Compile
    compile_command="${clang_bin} ${compiler_flags} ${include_flags} ${src_files} ${link_flags} -o ./bin/pack"

    echo "${compile_command}"

    mkdir -p "./bin"
    ${compile_command}
}

build_linux_x86_64() {
    clang_bin="clang"

    # Compiler flags
    compiler_flags=""
    compiler_flags="${compiler_flags} -DP_LINUX_X86_64"

    # Source files
    src_path="./src"

    src_files=""
    src_files="${src_files} ${src_path}/pack.c"

    # Include
    include_flags=""
    include_flags="${include_flags}"

    # Linking
    link_flags=""

    # Compile
    compile_command="${clang_bin} ${compiler_flags} ${include_flags} ${src_files} ${link_flags} -o ./bin/pack"

    echo "${compile_command}"

    mkdir -p "./bin"
    ${compile_command}
}


if [[ "$1" == "linux_x86_64" ]]; then
    build_linux_x86_64
    exit 0
fi
if [[ "$1" == "windows_x86_64" ]]; then
    build_windows_x86_64
    exit 0
fi

echo "No valid platform target specified!"
echo "Automatically detecting platform target (To run on current OS)"

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    build_linux_x86_64
    exit 0
fi
if [[ "$OSTYPE" == "msys" ]]; then
    build_windows_x86_64
    exit 0
fi
