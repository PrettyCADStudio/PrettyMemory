# coding: utf-8

import shutil

from pretty_memory import get_bin_dir, get_build_dir, get_repo_dir


repo_dir = get_repo_dir()
directories_to_remove = [
    get_bin_dir(),
    get_build_dir(),
    repo_dir / ".idea",
    repo_dir / ".vs"
]
for directory in repo_dir.iterdir():
    if directory.is_dir() and directory.name.startswith("cmake-build-"):
        directories_to_remove.append(directory)
for directory in directories_to_remove:
    if directory.is_dir():
        print(f'Removing directory: {directory}')
        try:
            shutil.rmtree(directory)
        except PermissionError as e:
            print(e)
