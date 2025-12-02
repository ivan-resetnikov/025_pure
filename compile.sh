#!/bin/bash

build_linux_x86_64() {
    clang_bin="clang"
    pack_bin="./bin/pack"

    # Compile flags
    compile_flags=""
    compile_flags="${compile_flags} -DP_LINUX_X86_64"
    compile_flags="${compile_flags} -Wall"
    # compile_flags="${compile_flags} -Werror"
    compile_flags="${compile_flags} -g"

    # Source files
    src_path="./src"

    src_files="${src_path}/main_linux_x86_64.c"

    # Linking
    link_flags=""
    link_flags="${link_flags} -lm"
    link_flags="${link_flags} -lX11"
    link_flags="${link_flags} -lXrandr"

    # Compile
    mkdir -p "./bin"

    compile_command="${clang_bin} ${compile_flags} ${include_flags} ${src_files} ${link_flags} -o ./bin/main"

    echo "${clang_bin}:"
    echo "${compile_command}"
    ${compile_command}

    # Pack assets
    exclusion_flags=""

    pack_command="${pack_bin} -i:./assets -o:./bin/assets.bin ${exclusion_flags}"

    echo "${pack_bin}:"
    echo "${pack_command}"

    # TODO: Need to port the packer to Windows and Linux
    ${pack_command}
}

build_windows_x86_64() {
    clang_bin="clang"
    pack_bin="./bin/pack"

    # Compile flags
    compile_flags=""
    compile_flags="${compile_flags} -DP_WINDOWS_X86_64"
    compile_flags="${compile_flags} -Wall"
    # compile_flags="${compile_flags} -Werror"
    compile_flags="${compile_flags} -g"

    # Source files
    src_path="./src"

    src_files="${src_path}/main_windows_x86_64.c"

    # Linking
    link_flags=""
    link_flags="${link_flags} -luser32"
    link_flags="${link_flags} -lgdi32"
    link_flags="${link_flags} -lkernel32"

    # Compile
    mkdir -p "./bin"

    compile_command="${clang_bin} ${compile_flags} ${include_flags} ${src_files} ${link_flags} -o ./bin/main.exe"

    echo "${clang_bin}:"
    echo "${compile_command}"
    ${compile_command}

    # Pack assets
    exclusion_flags=""

    pack_command="${pack_bin} -i:./assets -o:./bin/assets.bin ${exclusion_flags}"

    echo "${pack_bin}:"
    echo "${pack_command}"

    # TODO: Need to port the packer to Windows and Linux
    ${pack_command}
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

