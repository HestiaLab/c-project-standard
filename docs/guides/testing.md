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

## 质量门禁（CTest 注册）

除单元测试外，CTest 还注册了以下门禁。门禁的验收口径（扫描面下限、牙齿测试、空目录探针、四步注册）见 [C_PROJECT_STANDARD.md](../../C_PROJECT_STANDARD.md) 的「验证标准」一节。

| CTest 名称 | 脚本 | 守护什么 | 失败输出怎么读 |
| --- | --- | --- | --- |
| `cproject_decl_def_check` | `scripts/check_decl_def.py` | include/ 每个公共声明在 src/ 有定义；绝对扫描面下限 | `ERROR: 文件:行: public declaration without definition: 符号`；下限不足时首行 `FAIL: 解析失败：…仅 N 项（下限 M）` |
| `cproject_decl_def_floor` | 同上（严格下限 3/46，仅模板） | 模板自身扫描面不因正则/布局漂移静默缩水 | 同上；仅存在于 wizard markers 内，生成项目不继承 |
| `cproject_gate_behaviors` | `scripts/tests/test_gate_behaviors.py` | 门禁脚本自身「能失败」的性质（unittest） | unittest 标准输出，FAIL 行指名测试与断言 |
| `cproject_version_consistency` | `scripts/check_version_consistency.py` | CMakeLists VERSION、生成头、CLI --version、CHANGELOG 最新段四处一致 | 首行 `FAIL:` 指名缺失/异常源，随后逐源列出四个值 |
| `cproject_docs_check` | `scripts/check_docs.py` | 相对链接/锚点可达（URL 解码后判定）、代码围栏配对、README.md 文档索引与 docs/ 双向覆盖 | `ERROR: 文件:行: …`（死链/锚点/围栏/孤儿/索引死链） |
| `cproject_doc_paths_check` | `scripts/check_doc_paths.py` | 活文档反引号路径在 git 视角（已入库 ∪ 已忽略 ∪ 历史并集）可达 | `ERROR: 文件:行: …未入库/不存在`；非 git 上下文报 `FAIL: 无法取证` |
| `cproject_changelog_tag_check` | `scripts/check_changelog_tag.py` | CHANGELOG 段落形态、每个 tag 有同版本段落、已发布段落与 tag 处逐字一致 | `ERROR:` 指名版本与 tag；无 tag 段落打印 `SKIP:`（跳过 ≠ 通过） |
| `cproject_cpack_config` | `scripts/check-cpack-config.py` | NSIS 启用时的发布敏感 CPack 设置 | 输出缺失/违规项名称 |

依赖 git 取证或构建产物的门禁注册在 `cmake/Tests.cmake` 的 wizard markers 内：生成项目创建时不是 git 仓库、也没有向导 CLI，markers 内的测试会被自动注释掉。

可选入口（默认不接入 CI 与 check.sh）：

- `scripts/coverage_ratchet.py` —— 覆盖率棘轮（基线只升不降；基线口径唯一平台，当前为 Linux/GCC）。
- `scripts/gate_floor_probe.sh` —— 把全部 `scripts/check_*.py` 指向空目录跑一遍，验证每条门禁都能失败（VACUOUS-PASS 探测器）。
- `scripts/static_analysis.sh` —— clang-format / clang-tidy / cppcheck 的单一实现入口，CI 与 `scripts/check.sh` 调同一份。

## 手动检查

当行为无法通过 CTest 验证时，在 PR 中记录手动检查。手动检查应包含命令、平台与观察到的结果。
