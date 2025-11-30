#!/bin/bash

clang_bin="clang"

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
compile_command="${clang_bin} ${include_flags} ${src_files} ${link_flags} -o ./bin/pack"

echo "${compile_command}"

mkdir -p "./bin"
${compile_command}

