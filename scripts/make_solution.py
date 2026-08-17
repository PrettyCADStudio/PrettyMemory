# coding: utf-8

from argparse import ArgumentParser

from pretty_memory import get_source_dir, get_build_dir, path_to_string, run_command


parser = ArgumentParser()
parser.add_argument("--std", required=False, type=int, default=17, help="C++ standard")
args = parser.parse_args()

std = 17
if args.std is not None:
    std = args.std

source_dir = get_source_dir()
build_dir = get_build_dir()
cmd = [
    'cmake',
    '-G', 'Visual Studio 18 2026',
    '-A', 'x64',
    '-S', path_to_string(source_dir),
    '-B', path_to_string(build_dir),
    f'-DCXX_STD={std}'
]
run_command(cmd)
