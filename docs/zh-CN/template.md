<!-- 对应英文版：docs/guides/template.md | 最后同步：2026-07-09 -->

# 使用此模板

从本仓库启动新项目时，请参照以下清单。

## 推荐流程

1. 使用你的 PascalCase 项目名称运行重命名脚本：

   **Linux/macOS**
   ```sh
   ./scripts/rename-project.sh MyNewLib
   ```

   **Windows PowerShell**
   ```powershell
   .\scripts\rename-project.ps1 MyNewLib
   ```

   脚本会将所有源码、cmake、CI 和文档文件中的 `CProjectStandard`、
   `cproject` 和 `CPROJECT` 替换为你的项目名，并相应重命名 include、
   source 目录和 cmake 配置文件。

2. 运行 `./scripts/bootstrap.sh`（或 `.ps1`）验证重命名后的项目能
   正常配置和构建。

3. 替换示例 API、实现、可执行文件和测试。

4. 决定项目是否需要支持安装、包配置、`add_subdirectory()`、动态库、
   覆盖率和 AI 代理文档。

5. 裁剪不需要的可选部分。

6. 运行本文档和 `cmake.md` 中的验证命令。

### 手动重命名（备选）

如果你偏好手动重命名，需要同步更新以下名称：

- `CMakeLists.txt` 中的 `project(CProjectStandard ...)`。
- `include/cproject/` 下的公共 include 命名空间。
- `src/cproject/` 下的实现命名空间。
- CMake 目标名称，如 `cproject_core`。
- `cmake/Packaging.cmake` 和 `cmake/CProjectStandardConfig.cmake.in` 中的
  包配置名称。
- `cmake/Tests.cmake` 中的测试目标名称。
- README 标题、CI 工作流名称和文档示例。

## 替换示例代码

- 将 `include/cproject/example.h` 替换为第一个真实的公共 API。
- 将 `src/cproject/example.c` 替换为相应的实现。
- 如果项目不包含 CLI 或示例程序，删除 `src/main.c`。
- 保持测试聚焦于公共行为，放在 `tests/` 中。

## 保留生成的元数据

版本元数据从 `project(... VERSION ...)` 生成到 `cproject/version.h` 中。
保持运行时版本报告连接到该生成头文件，而不是在源文件中复制字面版本号。

## 验证重命名后的项目

重命名后运行本地检查：

**Linux/macOS**

```sh
./scripts/bootstrap.sh
./scripts/check.sh
cmake --preset ninja-shared
cmake --build --preset ninja-shared
ctest --preset ninja-shared --output-on-failure
```

**Windows PowerShell**

```powershell
./scripts/bootstrap.ps1
./scripts/check.ps1
cmake --preset ninja-shared
cmake --build --preset ninja-shared
ctest --preset ninja-shared --output-on-failure
```

如果项目保持可安装，还需使用 `tests/package_smoke/` 运行包冒烟测试。

如果项目保持可通过 `add_subdirectory()` 消费，保留并运行
`tests/subproject_smoke/`。

## 裁剪可选内容

重命名后保持模板精简。删除与新项目不匹配的内容：

- 如果项目没有 CLI 或可运行示例，删除 `src/main.c` 和
  `CPROJECT_BUILD_EXAMPLE`。
- 如果项目不打算安装或通过 CMake 包配置消费，删除
  `tests/package_smoke/` 和包安装文档。
- 如果项目永远不会通过 `add_subdirectory()` 消费，删除
  `tests/subproject_smoke/`。
- 如果不使用覆盖率数据，删除 `coverage` 预设和 CI 任务。
- 如果下游项目不接受 AI 辅助改动，删除 AI 代理文档。
- 保留下游项目没有更强策略时的 `.gitattributes`、格式化规则和本地
  引导/检查脚本。

## 需要更新的文档

重命名后需要更新：

- `README.md` — 新项目的概述和快速开始。
- `docs/guides/cmake.md` — 项目特定的目标、选项和冒烟测试。
- `docs/guides/testing.md` — 测试分层和必要检查。
- `docs/guides/release.md` — 如果项目发布制品。
- `docs/guides/ai-agent.md` — 如果项目接受 AI 辅助改动。
- `SECURITY.md` — 如果支持窗口或漏洞报告流程不同。
