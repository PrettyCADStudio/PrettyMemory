# PrettyMemory

PrettyMemory is a small header-only C++17 memory-pool library.

[中文说明 / Chinese README](README.zh-CN.md)
[Contribution guide](CONTRIBUTING.md)

## Project layout

- `include\PrettyMemory.h`: public API and current implementation of `PrettyMemory::MemoryPool`
- `test\`: GoogleTest-based test target and test entry point
- `build\`: generated build trees and Visual Studio solutions
- `bin\`: built executables and libraries

## Requirements

- CMake 3.15 or newer
- Visual Studio 2022 or Visual Studio 2026

## Generate Visual Studio solutions

Generate a Visual Studio 2022 solution:

```powershell
.\make-solution-vs-2022.ps1
```

Generate a Visual Studio 2026 solution:

```powershell
.\make-solution-vs-2026.ps1
```

The solutions are generated under:

- `build\vs2022`
- `build\vs2026`

## Open the solution in Visual Studio

Open the preferred solution with:

```powershell
.\start-with-visual-studio.ps1
```

This script looks for a solution under `build\vs2026` first and falls back to `build\vs2022`.

## Build and run tests

Build the preferred solution in `Release` mode and run all discovered tests:

```powershell
.\build-and-run-test.ps1
```

An alias with a plural file name is also available:

```powershell
.\build-and-run-tests.ps1
```

Both scripts wait for you to press Enter before the PowerShell window closes, so test failures remain visible.

If you want to run the steps manually, you can also use:

```powershell
cmake --build .\build\vs2026 --config Release
ctest --test-dir .\build\vs2026 -C Release --output-on-failure
```

## Notes

- The library is currently header-only, so implementation changes usually happen in `include\PrettyMemory.h`.
- The test project uses GoogleTest via CMake `FetchContent`.
- Basic GoogleTest cases are checked in and run through both local scripts and GitHub Actions CI.
