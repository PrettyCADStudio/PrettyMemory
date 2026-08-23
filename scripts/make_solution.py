# coding: utf-8

import sys
from argparse import ArgumentParser

from pretty_memory import get_source_dir, get_build_dir, path_to_string, run_command, get_cmake_generator


parser = ArgumentParser()
parser.add_argument("--std", required=False, type=int, default=17, help="C++ standard")
args = parser.parse_args()

std = 17
if args.std is not None:
    std = args.std

source_dir = get_source_dir()
build_dir = get_build_dir()
cmake_generator = get_cmake_generator()
cmd = [
    'cmake',
    '-G', cmake_generator,
    '-S', path_to_string(source_dir),
    '-B', path_to_string(build_dir),
    f'-DCXX_STD={std}'
]
if sys.platform == "Windows":
    cmd.append('-A')
    cmd.append('x64')
else:
    cmd.append("-DCMAKE_C_COMPILER=/usr/bin/gcc")
    cmd.append("-DCMAKE_CXX_COMPILER=/usr/bin/g++")
run_command(cmd)
