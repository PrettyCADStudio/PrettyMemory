# PrettyMemory

PrettyMemory 是一个仅头文件（header-only）的 C++17 智能指针库，重点提供显式所有权管理和安全的观察能力。

[在线 API 文档](https://prettycadstudio.github.io/PrettyMemory/)
[English README](README.md)
[贡献指南](CONTRIBUTING.md)

## 当前提供的能力

- `prtm::OwnerPtr<T, Deleter>`：拥有型智能指针，支持移动语义、自定义删除器、`Create`、`Transfer`、`Cast` 和 `Shadow`。
- `prtm::ShadowPtr<T>`：非拥有型观察指针；当目标对象被销毁或释放后会自动失效。
- `prtm::EnableShadowFromThis<T>`：基类，可让对象从 `this` 生成 `ShadowPtr`。

当前公开实现全部位于 `include\PrettyMemory.h`。

## 仓库结构

- `include\PrettyMemory.h`：公开 API 与头文件实现
- `test\`：基于 GoogleTest 的单元测试
- `doxygen-assets\`：双语文档生成所需的 Doxygen 过滤器和静态资源
- `.github\workflows\ci.yml`：Windows 平台的 CI 构建与测试工作流
- `.github\workflows\deploy-docs.yml`：GitHub Pages 文档部署工作流

## 环境要求

- 支持 C++17 的编译器
- CMake 3.15 或更高版本
- 本地生成接口文档时需要 Doxygen 和 Python 3

仓库同时提供了适用于 Windows 的 Visual Studio 2022 / 2026 辅助脚本；如果不使用这些脚本，也可以直接使用标准 CMake 命令。

## 编译并运行测试

推荐在仓库根目录执行：

```powershell
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

如果只想查看当前发现了哪些测试：

```powershell
ctest --test-dir build -N
```

如果只想运行某一组测试：

```powershell
ctest --test-dir build -R ShadowPtrTest
```

## Windows 辅助脚本

生成 Visual Studio 2022 解决方案：

```powershell
.\make-solution-vs-2022.ps1
```

生成 Visual Studio 2026 解决方案：

```powershell
.\make-solution-vs-2026.ps1
```

打开优先选择的解决方案：

```powershell
.\start-with-visual-studio.ps1
```

以 `Release` 配置编译优先选择的解决方案并运行全部测试：

```powershell
.\build-and-run-tests.ps1
```

`start-with-visual-studio.ps1` 和 `build-and-run-tests.ps1` 会优先使用 `build\vs2026`，找不到时回退到 `build\vs2022`。

## 生成接口文档

`include\PrettyMemory.h` 中的公开接口已经使用 `@~english` 和 `@~chinese` 编写双语 Doxygen 注释。生成文档时，Python 过滤器会把它们转换为可切换的语言块。

本地生成文档：

```powershell
cmake -S . -B build\docs -DBUILD_DOCS=ON
cmake --build build\docs --target docs
```

或者直接使用 Windows 辅助脚本：

```powershell
.\generate-document.ps1
```

生成后的 HTML 位于：

- `doc\html`

当前文档输出具备以下特性：

- 不显示本地绝对路径
- 页面提供中文 / English 语言切换按钮

## GitHub Actions 与 Pages

- `.github\workflows\ci.yml` 会在 push 到 `main`、`master` 以及提交 Pull Request 时，在 Windows 上构建并运行测试。
- `.github\workflows\deploy-docs.yml` 会在 push 到 `main` 后重新生成文档，并自动部署到 GitHub Pages。

如果仓库是第一次启用 Pages，需要在 **Settings > Pages > Source** 中选择 **GitHub Actions**。

## 示例

```cpp
#include <PrettyMemory.h>

struct Object
{
    int Value{ 0 };
};

auto owner = prtm::OwnerPtr<Object>::Create();
owner->Value = 42;

auto shadow = owner.Shadow();
owner.Reset();

bool expired = shadow.Expired();
```
