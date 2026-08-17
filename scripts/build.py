# coding: utf-8

from argparse import ArgumentParser
from typing import Final, Set

from pretty_memory import get_build_dir, run_command, path_to_string


VALID_CONFIGS: Final[Set[str]] = {
    'Debug',
    'Release',
    'RelWithDebInfo',
    'MinSizeRel'
}

parser = ArgumentParser()
parser.add_argument("--config", type=str, default='RelWithDebInfo',
                    help="Compilation configuration, default 'RelWithDebInfo'")
args = parser.parse_args()

config = 'RelWithDebInfo'
if args.config is not None and args.config in VALID_CONFIGS:
    config = args.config

build_dir = get_build_dir()
cmd = [
    'cmake',
    '--build', path_to_string(build_dir),
    '--config', config
]
run_command(cmd)
