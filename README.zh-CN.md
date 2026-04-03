# PrettyMemory

PrettyMemory 是一个小型的、仅头文件（header-only）的 C++17 内存池库。

[English README](README.md)
[贡献指南](CONTRIBUTING.md)

## 项目结构

- `include\PrettyMemory.h`：`PrettyMemory::MemoryPool` 的公开 API 与当前实现
- `test\`：基于 GoogleTest 的测试目标与测试入口
- `build\`：生成出的构建目录和 Visual Studio 解决方案
- `bin\`：编译产物输出目录

## 环境要求

- CMake 3.15 或更高版本
- Visual Studio 2022 或 Visual Studio 2026

## 生成 Visual Studio 解决方案

生成 Visual Studio 2022 解决方案：

```powershell
.\make-solution-vs-2022.ps1
```

生成 Visual Studio 2026 解决方案：

```powershell
.\make-solution-vs-2026.ps1
```

生成结果分别位于：

- `build\vs2022`
- `build\vs2026`

## 用 Visual Studio 打开解决方案

执行下面的脚本：

```powershell
.\start-with-visual-studio.ps1
```

脚本会优先打开 `build\vs2026` 下的解决方案；如果不存在，再回退到 `build\vs2022`。

## 编译并运行测试

以 `Release` 配置编译优先选择的解决方案，并运行全部已发现的测试：

```powershell
.\build-and-run-test.ps1
```

同时也提供了一个复数形式的别名脚本：

```powershell
.\build-and-run-tests.ps1
```

两个脚本都会在执行结束后等待你按下 Enter，再退出 PowerShell 窗口，这样即使测试失败也不会错过输出信息。

如果你想手动执行，也可以使用：

```powershell
cmake --build .\build\vs2026 --config Release
ctest --test-dir .\build\vs2026 -C Release --output-on-failure
```

## 说明

- 当前库为 header-only 结构，因此大多数实现修改都发生在 `include\PrettyMemory.h`。
- 测试工程通过 CMake `FetchContent` 集成 GoogleTest。
- 仓库里已经包含基础的 GoogleTest 用例，并且本地脚本与 GitHub Actions CI 都会执行这些测试。

