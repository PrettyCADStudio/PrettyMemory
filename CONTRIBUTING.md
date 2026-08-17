# Contributing to PrettyMemory

## Prerequisites

- CMake 3.28 or newer
- C++17 compiler (Visual Studio 2022 or 2026 on Windows)
- Python 3.13+ and Pipenv (for helper scripts)

## Build and test

Standard CMake workflow:

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Or use Pipenv helper scripts (default config is `RelWithDebInfo`):

```powershell
pipenv run build
pipenv run test
```

CI builds in Release with `-G "Visual Studio 17 2022" -A x64`.

## Project structure

- `include/PrettyMemory.h` — the entire library implementation (header-only, namespace `prtm`)
- `test/` — GoogleTest unit tests, one file per public type
- `CMakeLists.txt` — top-level build config
- `Pipfile` — Python helper scripts for build/test/clean/doc
- `.github/workflows/ci.yml` — Windows CI pipeline
- `.github/workflows/deploy-docs.yml` — GitHub Pages doc deployment

## Adding tests

Tests use GoogleTest (v1.14.0, fetched via `FetchContent`). All sources under `test/` compile into a single `PrettyMemoryTests` executable.

Use the custom test macros from `test/PrettyMemoryTest.h`:

```cpp
#include "PrettyMemoryTest.h"

using namespace prtm;

DEFINE_TEST_BEGIN(SuiteName, TestName, Subject)
{
    // TestableObject::Balance tracks construction/destruction counts
    OwnerPtr<TestableObject> obj = OwnerPtr<TestableObject>::Create();
    EXPECT_EQ(TestableObject::Balance, 1);
}
DEFINE_TEST_END
```

`DEFINE_TEST_END` automatically asserts `TestableObject::Balance == 0` to catch leaks. Every test must use this pattern — do not use raw `TEST()`.

## Code conventions

- Namespace: `prtm` (public), `prtm::detail` (internal)
- `#pragma once` for include guards
- Standard library only, no third-party runtime dependencies
- C++17 is the default standard
- MSVC gets `/utf-8` automatically

## Pull requests

- Keep changes focused and small enough to review.
- Add or update tests when behavior changes.
- Update documentation when commands, workflow, or public behavior changes.
