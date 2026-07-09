# C Project Standard

> [English](README.md)

用一套可靠的工程基础开始你的下一个 C 库或 CLI 工具 — CMake、C11、CTest、
跨平台 CI，以及清晰的贡献者文档，全部集成在一个模板中。

## 目录结构

```text
include/        公共头文件
src/            实现代码
tests/          CTest 测试
cmake/          CMake 模块
scripts/        引导、检查、重命名和发布脚本
docs/           项目文档
docs/guides/    主题指南
.github/        CI 工作流和仓库模板
```

## 使用此模板

1. **构建并运行初始化向导：**

   ```sh
   cmake --preset ninja-debug
   cmake --build --preset ninja-debug
   ./build/ninja-debug/bin/cproject_example
   ```

   向导交互式引导你完成项目名称、描述、特性选择，并自动执行
   重命名和清理。

   跳过交互：

   ```sh
   ./build/ninja-debug/bin/cproject_example MyLib --yes
   ```

   预览变更（不实际修改）：

   ```sh
   ./build/ninja-debug/bin/cproject_example MyLib --dry-run --yes
   ```

2. **替换** 示例代码和测试为你的真实模块。

3. **保持** 公共 API 在 `include/` 中，私有辅助在 `src/` 中。

4. **关联** 版本元数据到 CMake 的 `project(... VERSION ...)` 值。

5. **更新** 文档当构建、测试、发布或 API 规则变化时。

详见 [docs/zh-CN/template.md](docs/zh-CN/template.md) 获取完整的重命名清单
和裁剪指南。

## 快速开始

**所需工具：** CMake 3.21+、C11 编译器（MSVC、GCC、Clang 或 AppleClang）、
以及默认预设所需的 Ninja。

Windows 上请在 Developer PowerShell、Developer Command Prompt 或初始化了
MSVC 的 VS Code CMake kit 中运行 Ninja 预设。各平台详细安装说明见
[docs/guides/environment.md](docs/guides/environment.md)（英文）。

**首次设置：**

Windows PowerShell
```powershell
./scripts/bootstrap.ps1
```

Linux/macOS
```sh
./scripts/bootstrap.sh
```

引导脚本会检查必要工具并运行 configure，以便编辑器加载
`build/ninja-debug/compile_commands.json`。

**日常检查：**

Windows PowerShell
```powershell
./scripts/check.ps1
```

Linux/macOS
```sh
./scripts/check.sh
```

详细的 CMake 工作流说明见 [docs/guides/cmake.md](docs/guides/cmake.md)（英文）。

## 包含内容

- 按职责拆分的 CMake 模块（编译器选项、依赖、源码、测试、打包）。
- C11 作为目标级别语言基线。
- 跨平台支持 MSVC、GCC、Clang 和 AppleClang。
- debug、release、shared-library、sanitizer 和 coverage 构建预设。
- 基于 CTest 的单元测试。
- 可安装的 CMake 包配置，含 `find_package` 冒烟测试。
- `add_subdirectory()` 冒烟测试（子项目消费者）。
- GitHub Actions 用于构建、测试和静态分析。
- PR、issue 和 Dependabot 模板。
- VS Code 推荐、CMake 任务和 clangd 默认配置。
- 展示预期布局的示例库和可执行文件。

## 包消费

```cmake
find_package(CProjectStandard CONFIG REQUIRED)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

`tests/package_smoke/` 项目验证已安装的 C 和 C++ 消费者。
通过 `add_subdirectory()` 直接消费源码树由 `tests/subproject_smoke/` 验证。

详见 [C_PROJECT_STANDARD.md](C_PROJECT_STANDARD.md)（英文）。

## 文档

完整文档索引见 [docs/zh-CN/README.md](docs/zh-CN/README.md)。

## 许可证

MIT
