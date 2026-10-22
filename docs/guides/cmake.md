# CMake 指南

本指南介绍项目中的常用 CMake 工作流。当你需要添加源文件、测试、安装规则或
下游消费者时，可使用本指南。

## 前置要求

以下工具不由仓库提供，请在配置项目之前安装。

- CMake 3.21 或更高版本。
- C11 编译器。
- 以下受支持的编译器家族之一：
  - Windows 上的 MSVC。
  - Linux 上的 GCC 或 Clang。
  - macOS 上的 AppleClang 或 Clang。
- 默认 preset 使用 Ninja；或通过本地 preset 或命令行 generator 选择来使用
  CMake 支持的其他 generator。

Windows 开发环境的安装步骤见 [environment.md](environment.md)。

项目通过 `c_std_11` 在目标级别要求 C11。请保持公共头文件在受支持的编译器
家族之间可移植，并避免使用编译器扩展，除非它们被隔离在小型 CMake/编译器
检查之后。

## 编辑器默认配置

VS Code 是可选的。如果使用它，请安装 CMake Tools 和 clangd。默认工作区设置
使用 `ninja-debug` preset，`.clangd` 将 clangd 指向
`build/ninja-debug/compile_commands.json`。不要为同一工作区启用多个 C/C++
语言服务，除非你有意管理它们之间的重叠。

项目提交了可移植的编辑器默认配置：

- `.vscode/extensions.json` 推荐使用 CMake Tools 和 clangd。
- `.vscode/settings.json` 使用 `ninja-debug` preset，并让 clangd 成为生效的
  语言服务。
- `.vscode/c_cpp_properties.example.json` 提供可选的 Microsoft C/C++ 后备
  include 路径。
- `.vscode/tasks.json` 提供 configure、build、test 任务。
- `.clangd` 指向 Ninja 编译数据库。

不要提交本地 Visual Studio、MSYS2、Homebrew 或系统 SDK 安装路径等绝对路径。
将这些路径放在 VS Code 用户设置、CMake kit、`CMakeUserPresets.json`、shell
环境配置或本地 toolchain 文件中。

## 心智模型

CMake 工作通常分为四个独立步骤：

1. Configure：读取 `CMakeLists.txt`、选择选项并生成构建文件。
2. Build：使用生成的构建文件编译源文件并链接目标。
3. Test：从已配置的构建树运行 CTest 测试。
4. Install：将公共制品复制到安装前缀，供下游使用。

这些步骤被有意地分开。如果 configure 失败，构建文件就不可靠；如果 build
失败，测试和安装就没有意义。

## 快速命令

对于新检出的代码，先运行 bootstrap。它会检查所需的命令行工具并执行
configure，以便 clangd 能读取编译数据库：

**Windows PowerShell**

```powershell
./scripts/bootstrap.ps1
```

**Linux/macOS**

```sh
./scripts/bootstrap.sh
```

日常开发使用本地 check 封装脚本：

**Windows PowerShell**

```powershell
./scripts/check.ps1
```

**Linux/macOS**

```sh
./scripts/check.sh
```

需要逐步执行时，可直接使用 CMake 命令：

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

<!-- CPROJECT_FEATURE_CLI_BEGIN -->
构建后运行示例 CLI：

```sh
./build/ninja-debug/bin/cproject
```
<!-- CPROJECT_FEATURE_CLI_END -->

## Presets

Preset 是存储在 `CMakePresets.json` 中的命名构建配置。

| Preset | 用途 |
| --- | --- |
| `ninja-debug` | 带调试符号和 `compile_commands.json` 的默认跨平台开发构建。 |
| `ninja-release` | 使用 Ninja 的优化 release 风格验证。 |
| `ninja-shared` | 带 `BUILD_SHARED_LIBS=ON` 的 `ninja-debug` 构建。 |
| `ninja-asan` | 在受支持平台上带 AddressSanitizer 和 UBSan 的 Ninja debug 构建。 |
| `ninja-coverage` | 在受支持平台上带覆盖率标志的 Ninja debug 构建。 |
| `debug` | 带调试符号的快速本地开发构建。 |
| `release` | 用于 release 风格验证的优化构建。 |
| `shared` | 带 `BUILD_SHARED_LIBS=ON` 的 `debug` 构建。 |
| `asan` | 在受支持平台上带 AddressSanitizer 和 UBSan 的 debug 构建。 |
| `coverage` | 在受支持平台上带覆盖率标志的 debug 构建。 |

configure、build 和 test preset 共用相同名称：

```sh
cmake --preset ninja-shared
cmake --build --preset ninja-shared
ctest --preset ninja-shared --output-on-failure
```

`ninja-*` preset 是编辑器与 CI 的默认路径。它们强制使用 Ninja generator，
因此 CMake 会将 `compile_commands.json` 写入 preset 构建目录。

`debug`、`release`、`shared`、`asan` 和 `coverage` preset 是与 generator
无关的兼容性条目。当你需要使用 Visual Studio、Xcode、Make 或其他 generator
时，可以通过 `CMakeUserPresets.json`、CMake kit 或显式 generator 选择：

```sh
cmake --preset debug -G "<your generator>"
```

要创建本地 preset，请将 `CMakeUserPresets.example.json` 复制为
`CMakeUserPresets.json`，并根据你的机器修改 generator 名称。真正的
`CMakeUserPresets.json` 被 Git 忽略。

## 重要选项

以下选项可在 configure 阶段传入：

```sh
cmake --preset ninja-debug -DCPROJECT_BUILD_TESTING=OFF
```

| 选项 | 含义 |
| --- | --- |
| `BUILD_SHARED_LIBS` | 在受支持时将库构建为共享库。 |
<!-- CPROJECT_FEATURE_CLI_BEGIN -->
| `CPROJECT_BUILD_EXAMPLE` | 将 `src/main.c` 构建为示例可执行文件。 |
<!-- CPROJECT_FEATURE_CLI_END -->
| `CPROJECT_BUILD_TESTING` | 构建 CTest 测试目标。 |
<!-- CPROJECT_FEATURE_INSTALLABLE_BEGIN -->
| `CPROJECT_INSTALL` | 生成安装规则。 |
<!-- CPROJECT_FEATURE_INSTALLABLE_END -->
| `CPROJECT_ENABLE_ASAN` | 在受支持平台启用 AddressSanitizer。 |
| `CPROJECT_ENABLE_UBSAN` | 在受支持平台启用 UndefinedBehaviorSanitizer。 |
| `CPROJECT_ENABLE_COVERAGE` | 在受支持平台启用覆盖率标志。 |

对于顶层构建，示例、测试和安装规则默认为开启。当本项目通过
`add_subdirectory()` 被包含时，它们默认为关闭。

检查可用缓存选项：

```sh
cmake -LAH -S . -B build/ninja-debug
```

## 多配置 Generator

Visual Studio 和 Xcode 是 multi-config generator。它们可以从同一构建目录
生成 Debug 和 Release 制品，因此 build 和 test 命令需要显式指定配置：

```sh
cmake --build build/package-smoke --config Release
ctest --test-dir build/package-smoke -C Release --output-on-failure
```

Ninja 和 Make 等 single-config generator 则在 configure 阶段使用
`CMAKE_BUILD_TYPE`。

## 工具链策略

构建定义是语言模式与 include 目录的唯一事实来源：

- CMake 最低版本：3.21。
- C 语言标准：C11，以目标特性 `c_std_11` 表示。
- 受支持的编译器家族：MSVC、GCC、Clang 和 AppleClang。
- 默认 generator：通过 `ninja-*` preset 使用 Ninja。
- 兼容 generator：任何能与所选编译器配合的 CMake generator。

不要在已提交的 preset 中放入特定于机器的编译器路径、SDK 路径或 Visual
Studio 安装路径。请使用用户 preset、CMake kit、toolchain 文件或环境配置脚本
选择本地工具链。

请将生成头文件保留在构建树下，并通过目标 include 目录暴露它们。源文件不应
通过相对文件系统路径包含生成头文件。

## 添加源文件

将实现文件放在 `src/cproject/` 下，公共头文件放在 `include/cproject/` 下。

1. 在 `src/cproject/` 下创建 `.c` 文件。
2. 在 `include/cproject/` 下创建或更新对应的公共头文件。
3. 将新的 `.c` 文件添加到 `cmake/Sources.cmake` 的 `CPROJECT_CORE_SOURCES` 中。
4. 在 `tests/` 下添加测试。
5. 运行 `./scripts/check.ps1` 或 `./scripts/check.sh`。

公共头文件应只包含其所需的内容，并在暴露 C API 时保持可供 C++ 调用方使用。

## 添加测试

使用 `cmake/Tests.cmake` 中的 `cproject_add_test()`：

```cmake
cproject_add_test(cproject_feature_tests
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_feature.c
    LIBS
        cproject_core
)
```

然后运行：

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

测试应断言公共行为。请保持实现细节测试的稀缺性，并记录它们所保护的内部契约。

<!-- CPROJECT_FEATURE_INSTALLABLE_BEGIN -->
## 安装并通过 find_package 使用

配置并构建一个 release 包：

```sh
cmake --preset ninja-release
cmake --build --preset ninja-release
cmake --install build/ninja-release --prefix install
```

下游 CMake 项目随后即可消费它：

```cmake
find_package(CProjectStandard CONFIG REQUIRED)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

包冒烟测试项目验证这条路径：

**Linux/macOS**

```sh
cmake -G Ninja -S tests/package_smoke -B build/package-smoke -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build build/package-smoke --config Release
ctest --test-dir build/package-smoke -C Release --output-on-failure
```

**Windows PowerShell**

```powershell
cmake -G Ninja -S tests/package_smoke -B build/package-smoke -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build build/package-smoke --config Release
ctest --test-dir build/package-smoke -C Release --output-on-failure
```

对于共享库安装，请使用 `ninja-shared` preset 和单独的安装前缀：

```sh
cmake --preset ninja-shared
cmake --build --preset ninja-shared
cmake --install build/ninja-shared --config Debug --prefix install-shared
cmake -G Ninja -S tests/package_smoke -B build/package-smoke-shared -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PWD/install-shared"
cmake --build build/package-smoke-shared --config Debug
ctest --test-dir build/package-smoke-shared -C Debug --output-on-failure
```

在 Windows 上，共享库消费者需要将已安装的 DLL 放在测试可执行文件旁边或加入
`PATH`。冒烟项目会在 CMake 暴露运行时 DLL 元数据时复制该 DLL。
<!-- CPROJECT_FEATURE_INSTALLABLE_END -->

<!-- CPROJECT_FEATURE_SUBPROJECT_BEGIN -->
## 作为子项目使用

项目也可以直接消费本仓库：

```cmake
add_subdirectory(path/to/c-project-standard)
target_link_libraries(app PRIVATE CProjectStandard::cproject_core)
```

子项目冒烟测试验证这种模式：

```sh
cmake -G Ninja -S tests/subproject_smoke -B build/subproject-smoke -DCPROJECT_SOURCE_DIR="$PWD"
cmake --build build/subproject-smoke --config Debug
ctest --test-dir build/subproject-smoke -C Debug --output-on-failure
```

作为子项目被包含时，示例、测试和安装规则默认为关闭；下游项目如有需要可以
显式开启它们。
<!-- CPROJECT_FEATURE_SUBPROJECT_END -->

## 生成的版本头文件

项目版本只声明一次：

```cmake
project(<Project> VERSION <x.y.z> LANGUAGES C)
```

CMake 根据该值生成 `cproject/version.h`。源文件应使用生成的宏，而不是重复
字面版本号。CTest 中的版本一致性门禁会比较 CMake、生成头、CHANGELOG，以及
存在时的 CLI 输出。

## 本地质量检查

在克隆之后、安装工具之后或切换到新机器之后运行 bootstrap：

```powershell
./scripts/bootstrap.ps1
```

```sh
./scripts/bootstrap.sh
```

check 脚本会执行 configure、build 和测试，并在工具可用时运行可选工具：

```powershell
./scripts/check.ps1 -EnableTidy
```

```sh
./scripts/check.sh --enable-tidy
```

如果本地没有安装 `clang-format`、`clang-tidy` 或 `yamllint`，请将 CI 作为
这些工具的权威检查。

## 故障排除

旧的 CMake 缓存指向其他路径：

- 症状：configure 提示了不同的源目录。
- 解决方法：删除受影响的构建目录（例如 `build/ninja-release`），重新运行
  `cmake --preset ninja-release`。

缺少 Ninja 或 C 编译器：

- 症状：configure 提示找不到 Ninja，或未设置 `CMAKE_C_COMPILER`。
- 解决方法：运行 `./scripts/bootstrap.ps1` 或 `./scripts/bootstrap.sh`。
  Windows 上请打开 Developer PowerShell 或选择 MSVC CMake kit。

VS Code 使用 Ninja 构建 `ALL_BUILD`：

- 症状：Ninja 报告 `unknown target 'ALL_BUILD'`。
- 解决方法：运行 `CMake: Set Build Target` 并选择 `all`。如果旧目标仍被缓存，
  运行 `CMake: Reset CMake Tools Extension State` 和
  `CMake: Delete Cache and Reconfigure`。

`find_package(CProjectStandard)` 找不到包：

- 症状：CMake 找不到 `CProjectStandardConfig.cmake`。
- 解决方法：先安装项目，并通过 `-DCMAKE_PREFIX_PATH=/path/to/install` 传入
  绝对安装前缀。

Windows 共享库测试找不到 DLL：

- 症状：共享库消费者能构建，但运行时失败。
- 解决方法：将安装的 DLL 复制到可执行文件旁边，或将安装的 `bin` 目录加入
  `PATH`。

缺少 `clang-format`、`clang-tidy` 或 `yamllint`：

- 症状：本地脚本跳过工具，或 shell 找不到它。
- 解决方法：安装该工具，或依赖会安装工具链的 CI。
