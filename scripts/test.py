# coding: utf-8

from argparse import ArgumentParser
from typing import Final, Set, MutableSequence

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

configs: MutableSequence[str] = []
if args.config is not None and args.config in VALID_CONFIGS:
    configs.append(args.config)
else:
    configs.extend(VALID_CONFIGS)

build_dir = get_build_dir()
for config in configs:
    cmd = [
        'ctest',
        '--test-dir', path_to_string(build_dir),
        '-C', config,
        '--output-on-failure'
    ]
    run_command(cmd)
