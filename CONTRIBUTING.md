# Contributing to PrettyMemory

Thank you for contributing.

## Prerequisites

- CMake 3.15 or newer
- Visual Studio 2022 or Visual Studio 2026
- PowerShell

## Local development

Generate a Visual Studio 2026 solution:

```powershell
.\make-solution-vs-2026.ps1
```

Generate a Visual Studio 2022 solution:

```powershell
.\make-solution-vs-2022.ps1
```

Open the preferred solution:

```powershell
.\start-with-visual-studio.ps1
```

## Running tests

Build and run the test suite in `Release` mode:

```powershell
.\build-and-run-tests.ps1
```

If you prefer to run the commands manually:

```powershell
cmake -S . -B build\vs2022 -G "Visual Studio 17 2022" -A x64
cmake --build build\vs2022 --config Release
ctest --test-dir build\vs2022 -C Release --output-on-failure
```

## Project structure

- `include\PrettyMemory.h` contains the current public header and implementation details.
- `test\` contains the GoogleTest-based test target.
- `.github\workflows\ci.yml` runs the Windows build and test pipeline in GitHub Actions.

## Pull requests

- Keep changes focused and small enough to review.
- Add or update tests when behavior changes.
- Update documentation when commands, workflow, or public behavior changes.
