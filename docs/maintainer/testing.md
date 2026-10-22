# 模板测试

本页只覆盖模板仓库自身的验证。生成项目共享的门禁见
[测试指南](../guides/testing.md)。

## 测试分层

| 层 | 内容 | 运行方式 |
| --- | --- | --- |
| 单元测试 | 名称归一化、版本校验、特性参数、载荷清单匹配 | `cproject_wizard_config_tests` |
| CLI 契约 | 退出码、JSON、严格非交互、语言优先级、写盘保护 | `cproject_cli_contract` |
| 落盘门禁 | 声明/定义对齐、版本一致性、文档链接/锚点、门禁自测 | CTest |
| 模板专属门禁 | 向导测试标记、严格扫描下限、git 视角文档、CHANGELOG/tag | CTest wizard 标记 |
| 生成物矩阵 | default/minimal 项目 configure、build、CTest | CI 与人工 |
| 安装/载荷 | 安装暂存、模板专属文件泄漏、安装后向导 | 人工/CI |
| MSI | Windows MSI 构建、安装后向导生成项目 | Windows CI |

## 向导标记

`cmake/Tests.cmake` 中的 `CPROJECT_WIZARD_TESTS_BEGIN/END` 块只属于模板。
生成项目时向导会直接删除该块。生成项目保留块外的通用门禁。

修改测试注册时必须确认：

1. 模板专属测试在标记内；
2. 通用测试在标记外；
3. 生成项目删除标记后仍能 configure、build、CTest。

## 生成物矩阵

至少验证两个生成物：

- default：installable/subproject/CI/CLI 开启，AI 关闭；
- minimal：5 个结构开关全部关闭。

每个生成物都要在仓库外创建，并执行：

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug --output-on-failure
```

default 生成物必须包含 package smoke、subproject smoke、CI 工作流和 CLI
桩。minimal 生成物必须不含 `.github/`、`src/main.c`、`res/`、
`tests/package_smoke/`、`tests/subproject_smoke/` 和安装规则。

## 载荷与安装

安装暂存必须与生成器使用同一份 `template-exclude.txt`。验证步骤：

```sh
cmake --install build/ninja-debug --prefix /tmp/cproject-install
```

然后检查安装树的 `share/c-project-standard/`：

- 不得出现 `installer/`、`src/wizard/`、`tests/wizard/`、
  `cmake/Wizard.cmake`、`cmake/PackagingMSI.cmake`、`docs/maintainer/`、
  release/package/publish 脚本或 `res/run-wizard.bat`；
- 必须保留 `template-exclude.txt`，安装后的向导依赖它复制载荷。

再用安装后的 `bin/cproject` 生成一个项目并跑完整 CTest。该步骤捕获
“安装树能安装、但不能生成可构建项目”的回归。

## CI 作业

模板 CI 中的两个模板专属作业：

- `generated-projects`（Linux/macOS/Windows）：构建向导，运行
  `tests/wizard/test_generated_matrix.py` 与
  `tests/wizard/test_install_payload.py`；
- `msi-smoke`（Windows）：构建 MSI，运行 `scripts/msi-smoke.ps1`，验证安装后的
  向导、模板载荷、库包消费与生成项目。

## 模板专属门禁

- `cproject_decl_def_floor`：模板扫描面严格下限，防止正则/布局漂移导致
  门禁空转；
- `cproject_doc_paths_check`：活文档反引号路径在 git 视角可达；
- `cproject_changelog_tag_check`：CHANGELOG 段落与 tag 一致；
- `cproject_wizard_config_tests` 与 `cproject_cli_contract`：向导行为契约。

`scripts/gate_floor_probe.sh` 把扫描型门禁指向空目录，任何 exit 0 都是
VACUOUS-PASS。该脚本仅存在于模板仓库，不进入生成物。

## 发布前

1. 模板 CTest 全绿；
2. default 与 minimal 生成物全绿；
3. 安装暂存无泄漏，安装后的向导能生成可构建项目；
4. Windows MSI 构建成功，安装后向导能生成、配置、构建项目；
5. 更新 `CHANGELOG.md` 与版本后，再打 tag。
