# coding: utf-8

from pretty_memory import get_build_dir, get_source_dir, get_repo_dir, path_to_string, run_command


source_dir = get_source_dir()
build_dir = get_build_dir() / "docs"
doc_dir = get_repo_dir() / "doc"
if not doc_dir.is_dir():
    doc_dir.mkdir()

cmd = [
    'cmake',
    '-S', path_to_string(source_dir),
    '-B', path_to_string(build_dir),
    '-DBUILD_DOCS=ON',
    f'-DDOXYGEN_OUTPUT_DIRECTORY={path_to_string(doc_dir)}'
]
run_command(cmd)

cmd = [
    'cmake',
    '--build', path_to_string(build_dir),
    '--target', 'docs'
]
run_command(cmd)

doc_index_file = doc_dir / "html" / "index.html"
if doc_index_file.is_file():
    print(f"Documentation generated: {doc_index_file}")
