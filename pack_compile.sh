#!/bin/bash

clang_bin="clang"

# Compiler flags
compiler_flags=""
#compiler_flags="${compiler_flags} -DP_WINDOWS_X86_64"
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

