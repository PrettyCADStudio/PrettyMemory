# PrettyMemory Copilot Instructions

## Build and test commands

Use CMake from the repository root:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run a single test by name pattern:

```powershell
ctest --test-dir build -R ShadowPtrTest --output-on-failure
```

List discovered tests without running:

```powershell
ctest --test-dir build -N
```

After building with the Visual Studio generator, you can also run a single GoogleTest case directly:

```powershell
.\bin\Debug\PrettyMemoryTests.exe --gtest_filter=SuiteName.TestName
```

Pipenv helper scripts are also available (default config is `RelWithDebInfo`, not Release):

```powershell
pipenv run build              # configure + build
pipenv run test               # run tests
pipenv run clean              # remove build/, bin/, .vs/, .idea/
```

CI builds in Release with `-G "Visual Studio 17 2022" -A x64`.

## Architecture

Header-only C++17 smart pointer library. The entire implementation lives in `include/PrettyMemory.h`. There is no `src/` directory or compiled library target.

Public API (namespace `prtm`):
- `OwnerPtr<T, Deleter>` — owning smart pointer, move-only, custom deleter support, `Create`, `Transfer`, `Cast`, `Shadow`
- `ShadowPtr<T>` — non-owning observer that detects when the target is destroyed; `Get`, `Expired`, `Cast`, `Swap`
- `EnableShadowFromThis<T>` — CRTP base for creating `ShadowPtr` from `this` via `ShadowFromThis()`

Internal implementation lives in `prtm::detail` (`ControlBlock`, `DefaultDeleter`).

The top-level `CMakeLists.txt` sets the C++ standard, output directories, and the `BUILD_TESTS` option. When tests are enabled it delegates to `test/CMakeLists.txt`, which defines an `INTERFACE` target named `PrettyMemory`, fetches GoogleTest v1.14.0 with `FetchContent`, builds `PrettyMemoryTests`, and registers tests with `gtest_discover_tests()`.

Build outputs: executables go to `bin/`, intermediate files and fetched dependencies stay under `build/`.

## Test structure

All test sources under `test/` compile into a single `PrettyMemoryTests` executable. Test files: `OwnerPtr.Test.cpp`, `ShadowPtr.Test.cpp`, `EnableShadowFromThis.Test.cpp`, `ControlBlock.Test.cpp`, `CountableObject.Test.cpp`, `Deleter.Test.cpp`.

Custom test macros in `test/PrettyMemoryTest.h`:
- `DEFINE_TEST_BEGIN(SuiteName, TestName, Subject)` / `DEFINE_TEST_END` — wraps GoogleTest with automatic leak checking via `CountableObject::Balance`. Every test MUST end with `DEFINE_TEST_END`.
- `CountableObject<T>` (in `test/CountableObject.h`) — tracks construction/destruction counts. Use as the test type to verify no leaks.

Do not use raw `TEST()` macros. Always use the `DEFINE_TEST_BEGIN`/`DEFINE_TEST_END` pattern.

## Code conventions

- Namespace: `prtm` (public), `prtm::detail` (internal). NOT `PrettyMemory`.
- The header uses `#pragma once`.
- Standard library only — no third-party runtime dependencies.
- C++17 is the default; overridable via `-DCXX_STD=N`.
- MSVC gets `/utf-8` flag automatically.
- Keep library implementation inline in headers. New functionality goes in `include/PrettyMemory.h`.
