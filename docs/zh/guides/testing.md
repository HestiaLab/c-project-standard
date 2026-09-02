<!-- 对应英文版：docs/en/guides/testing.md | 最后同步：2026-09-02 -->

# 测试

使用 CTest 进行自动化测试。

```sh
ctest --preset ninja-debug --output-on-failure
```

命令详情和生成器相关标志见 `cmake.md`。

## 测试框架

测试使用 vendored 的 [Unity](https://github.com/ThrowTheSwitch/Unity) 框架（MIT，位于 `tests/vendor/unity/` 下）。每个测试可执行文件在 `UNITY_BEGIN()` 和 `UNITY_END()` 之间用 `RUN_TEST()` 注册其测试用例，并且必须提供 `setUp()` 和 `tearDown()`，即使它们为空。vendored 的 Unity 源码不参与 `clang-format`、`clang-tidy` 和 `cppcheck` 检查。

## 期望

- 缺陷修复应包含回归测试。
- 新增公共 API 应包含成功与失败用例。
- 数值类 API 应覆盖相关的边界与溢出用例。
- 测试应断言行为，而非行覆盖率。
- 对于小型模块，优先编写聚焦的测试程序，而非宽泛的集成测试。
- 可安装的库应保留一个小型的下游包冒烟测试。

## 静态分析

CI 运行 clang-format、clang-tidy、cppcheck 和 YAML lint 检查。本地 clang-tidy 检查是可选的，因为工具可用性因平台而异：

**Linux/macOS**

```sh
./scripts/check.sh --enable-tidy
```

**Windows PowerShell**

```powershell
./scripts/check.ps1 -EnableTidy
```

## 需要运行的检查

| 变更类型 | 最低限度的有效检查 |
| --- | --- |
| C 源码或公共头文件 | `./scripts/check.ps1` 或 `./scripts/check.sh` |
| 动态库或导出宏变更 | `ninja-shared` 预设与动态库包冒烟测试 |
| 安装/包元数据变更 | 静态与动态包冒烟测试 |
| 子项目行为变更 | `tests/subproject_smoke/` |
| CI 或 YAML 变更 | `git diff --check` 与 CI 静态分析 |
| 发布敏感变更 | `ninja-release`、`ninja-shared`、`ninja-asan` 与包冒烟测试 |

## 包冒烟测试

`tests/package_smoke/` 是一个下游消费者，用于验证已安装的包元数据。它应保持精简，只断言已安装的头文件、导出的目标、C 链接以及基本库调用能够通过 `find_package` 正常工作。

在支持动态库时，为静态与动态两种安装运行包冒烟测试。

## 子项目冒烟测试

`tests/subproject_smoke/` 验证项目可以通过 `add_subdirectory()` 被消费，且默认不启用顶层示例、测试或安装规则。

## 手动检查

当行为无法通过 CTest 验证时，在 PR 中记录手动检查。手动检查应包含命令、平台与观察到的结果。
