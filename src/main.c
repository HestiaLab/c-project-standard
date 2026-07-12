/*
 * C Project Standard — Interactive Project Setup Wizard
 *
 * This is the example CLI entry point. On first run it guides the developer
 * through project initialization: name, metadata, feature selection, and
 * automated in-place rename + cleanup. After setup it replaces itself with
 * a project-specific stub.
 *
 * Language: C11 | No external dependencies beyond the standard library
 */

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define F_OK 0
#define access _access
#define mkdir(p, m) _mkdir(p)
#define rmdir(p) _rmdir(p)
#define unlink(p) _unlink(p)
#define getcwd _getcwd
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <cproject/example.h>
#include <cproject/version.h>

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* =========================================================================
 * Types & constants
 * ========================================================================= */

typedef enum { LANG_EN, LANG_ZH, LANG_COUNT } Lang;

#define FEAT_INSTALLABLE 0
#define FEAT_SUBPROJECT 1
#define FEAT_SHARED 2
#define FEAT_SANITIZERS 3
#define FEAT_COVERAGE 4
#define FEAT_CI 5
#define FEAT_AI_AGENT 6
#define FEAT_CLI 7
#define FEAT_COUNT 8

typedef struct {
    char pascal[64];
    char lower[64];
    char upper[64];
    char description[256];
    char author[128];
    char version[32];
    char license[64];
    int features[FEAT_COUNT]; /* 1 = enabled */
} Config;

/* =========================================================================
 * Bilingual message table — compile-time initialized, O(1) lookup
 * ========================================================================= */

enum Msg {
    M_BANNER,
    M_LANG_SELECT,
    M_NAME_PROMPT,
    M_NAME_ERROR,
    M_DESC_PROMPT,
    M_DESC_DEFAULT,
    M_AUTHOR_PROMPT,
    M_VERSION_PROMPT,
    M_VERSION_DEFAULT,
    M_LICENSE_PROMPT,
    M_LICENSE_DEFAULT,
    M_FEAT_HEADER,
    M_FEAT_INSTALLABLE,
    M_FEAT_SUBPROJECT,
    M_FEAT_SHARED,
    M_FEAT_SANITIZERS,
    M_FEAT_COVERAGE,
    M_FEAT_CI,
    M_FEAT_AI_AGENT,
    M_FEAT_CLI,
    M_REVIEW_HEADER,
    M_REVIEW_NAME,
    M_REVIEW_NAMESPACE,
    M_REVIEW_MACRO,
    M_REVIEW_DESC,
    M_REVIEW_VERSION,
    M_REVIEW_LICENSE,
    M_REVIEW_YES,
    M_REVIEW_NO,
    M_REVIEW_OPS,
    M_REVIEW_RENAME,
    M_REVIEW_DIRS,
    M_REVIEW_STRIP,
    M_REVIEW_GIT,
    M_PROCEED,
    M_OK_RENAME,
    M_OK_DIRS,
    M_OK_STRIP,
    M_OK_GIT,
    M_OK_STUB,
    M_NEXT_TITLE,
    M_NEXT_1,
    M_NEXT_2,
    M_NEXT_3,
    M_NEXT_4,
    M_NEXT_5,
    M_NEXT_6,
    M_NOT_ROOT,
    M_DRYRUN_DONE,
    M_ABORTED,
    M_MSG_COUNT
};

typedef struct {
    const char *en;
    const char *zh;
} Bilingual;

static const Bilingual M[M_MSG_COUNT] = {
    [M_BANNER] = {"C Project Standard v%d.%d.%d — Project Setup",
                  "C 项目工程标准 v%d.%d.%d — 项目初始化"},
    [M_LANG_SELECT] = {"Language [EN/zh]: ", "语言 [EN/zh]: "},
    [M_NAME_PROMPT] = {"Project name (PascalCase): ", "项目名称 (PascalCase): "},
    [M_NAME_ERROR] = {"Must start with uppercase letter, letters and digits only.",
                      "必须以大写字母开头，仅含字母和数字。"},
    [M_DESC_PROMPT] = {"Description", "项目描述"},
    [M_DESC_DEFAULT] = {"A C library", "一个 C 库"},
    [M_AUTHOR_PROMPT] = {"Author/Vendor", "作者/组织"},
    [M_VERSION_PROMPT] = {"Version", "版本"},
    [M_VERSION_DEFAULT] = {"0.1.0", "0.1.0"},
    [M_LICENSE_PROMPT] = {"License", "许可证"},
    [M_LICENSE_DEFAULT] = {"MIT", "MIT"},
    [M_FEAT_HEADER] = {"Features (y/n, Enter=default):", "可选特性 (y/n, 回车=默认):"},
    [M_FEAT_INSTALLABLE] = {"Installable library (find_package + packaging)",
                            "可安装库 (find_package + 打包)"},
    [M_FEAT_SUBPROJECT] = {"Subproject consumer (add_subdirectory)",
                           "子项目消费 (add_subdirectory)"},
    [M_FEAT_SHARED] = {"Shared library support (BUILD_SHARED_LIBS)",
                       "动态库支持 (BUILD_SHARED_LIBS)"},
    [M_FEAT_SANITIZERS] = {"Sanitizer presets (ASan + UBSan)", "Sanitizer 预设 (ASan + UBSan)"},
    [M_FEAT_COVERAGE] = {"Code coverage preset", "代码覆盖率预设"},
    [M_FEAT_CI] = {"GitHub Actions CI", "GitHub Actions CI"},
    [M_FEAT_AI_AGENT] = {"AI agent policy (AGENTS.md)", "AI 代理策略 (AGENTS.md)"},
    [M_FEAT_CLI] = {"Example CLI (stub after setup)", "示例 CLI (初始化后变为桩代码)"},
    [M_REVIEW_HEADER] = {"Review:", "预览:"},
    [M_REVIEW_NAME] = {"  Name         %s", "  项目名       %s"},
    [M_REVIEW_NAMESPACE] = {"  Namespace    %s", "  命名空间     %s"},
    [M_REVIEW_MACRO] = {"  Macro prefix %s", "  宏前缀       %s"},
    [M_REVIEW_DESC] = {"  Description  %s", "  描述         %s"},
    [M_REVIEW_VERSION] = {"  Version      %s", "  版本         %s"},
    [M_REVIEW_LICENSE] = {"  License      %s", "  许可证       %s"},
    [M_REVIEW_YES] = {"yes", "是"},
    [M_REVIEW_NO] = {"no", "否"},
    [M_REVIEW_OPS] = {"Operations:", "操作:"},
    [M_REVIEW_RENAME] = {"  Rename  files  (CProjectStandard -> %s)",
                         "  重命名  文件  (CProjectStandard -> %s)"},
    [M_REVIEW_DIRS] = {"  Rename  dirs   (cproject -> %s)", "  重命名  目录  (cproject -> %s)"},
    [M_REVIEW_STRIP] = {"  Strip   %s", "  移除    %s"},
    [M_REVIEW_GIT] = {"  Clear  git remote", "  清理    git remote"},
    [M_PROCEED] = {"Proceed? [Y/n]: ", "确认执行? [Y/n]: "},
    [M_OK_RENAME] = {"  Renamed        %d files", "  已重命名        %d 个文件"},
    [M_OK_DIRS] = {"  Renamed        %d directories", "  已重命名        %d 个目录"},
    [M_OK_STRIP] = {"  Stripped       %s", "  已移除          %s"},
    [M_OK_GIT] = {"  Removed        git remote", "  已移除          git remote"},
    [M_OK_STUB] = {"  Wrote          src/main.c (stub)", "  已写入          src/main.c (桩代码)"},
    [M_NEXT_TITLE] = {"Next steps:", "下一步:"},
    [M_NEXT_1] = {"  1. cmake --build --preset ninja-debug",
                  "  1. cmake --build --preset ninja-debug"},
    [M_NEXT_2] = {"  2. Search for SETUP: to find placeholder code",
                  "  2. 搜索 SETUP: 找到所有待替换的占位代码"},
    [M_NEXT_3] = {"  3. Replace include/%s/example.h with your first API",
                  "  3. 将 include/%s/example.h 替换为你的第一个 API"},
    [M_NEXT_4] = {"  4. Add source files to cmake/Sources.cmake",
                  "  4. 向 cmake/Sources.cmake 添加源文件"},
    [M_NEXT_5] = {"  5. Add tests under tests/", "  5. 在 tests/ 下添加测试"},
    [M_NEXT_6] = {"  6. Run ./scripts/check.sh to verify", "  6. 运行 ./scripts/check.sh 验证"},
    [M_NOT_ROOT] = {"ERROR: Not in project root (CMakeLists.txt not found).",
                    "错误：不在项目根目录（找不到 CMakeLists.txt）。"},
    [M_DRYRUN_DONE] = {"No changes made (--dry-run).", "未做任何修改 (--dry-run)。"},
    [M_ABORTED] = {"Aborted.", "已取消。"},
};

/* =========================================================================
 * Global state
 * ========================================================================= */

static Lang g_lang = LANG_EN;
static int g_dry_run = 0;
static int g_yes = 0;
static int g_renamed_files = 0;
static int g_renamed_dirs = 0;

/* ---- helpers ---- */
static const char *T(enum Msg id) {
    return g_lang == LANG_ZH ? M[id].zh : M[id].en;
}
static void pln(enum Msg id) {
    printf("%s\n", T(id));
}

/* =========================================================================
 * Utility: safe terminal I/O
 * ========================================================================= */

static char *safe_gets(char *buf, int size) {
    if (!fgets(buf, size, stdin))
        return NULL;
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        buf[--len] = '\0';
    return buf;
}

static int ask_yesno(enum Msg prompt, int default_yes) {
    char buf[8];
    for (;;) {
        printf("%s", T(prompt));
        fflush(stdout);
        if (!safe_gets(buf, (int)sizeof(buf)))
            return default_yes;
        if (buf[0] == '\0')
            return default_yes;
        if (buf[0] == 'y' || buf[0] == 'Y')
            return 1;
        if (buf[0] == 'n' || buf[0] == 'N')
            return 0;
    }
}

static void ask_string(enum Msg prompt, char *buf, size_t size, enum Msg def_msg) {
    printf("%s", T(prompt));
    printf("[%s]: ", T(def_msg));
    fflush(stdout);
    if (!safe_gets(buf, (int)size)) {
        strncpy(buf, T(def_msg), size - 1);
        buf[size - 1] = '\0';
        return;
    }
    if (buf[0] == '\0') {
        strncpy(buf, T(def_msg), size - 1);
        buf[size - 1] = '\0';
    }
}

static void ask_raw(enum Msg prompt, char *buf, size_t size, const char *def) {
    printf("%s", T(prompt));
    if (def && def[0])
        printf("[%s]: ", def);
    else
        printf(": ");
    fflush(stdout);
    if (!safe_gets(buf, (int)size)) {
        if (def) {
            strncpy(buf, def, size - 1);
            buf[size - 1] = '\0';
        }
        return;
    }
    if (buf[0] == '\0' && def) {
        strncpy(buf, def, size - 1);
        buf[size - 1] = '\0';
    }
}

/* =========================================================================
 * Utility: string operations
 * ========================================================================= */

static char *strdup_safe(const char *s) {
    if (!s)
        return NULL;
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (p)
        memcpy(p, s, n);
    return p;
}

static char *str_replace_all(const char *s, const char *from, const char *to) {
    if (!s || !from || !to)
        return NULL;
    size_t slen = strlen(s), flen = strlen(from), tlen = strlen(to);
    if (flen == 0)
        return strdup_safe(s);

    size_t count = 0;
    const char *p = s;
    while ((p = strstr(p, from))) {
        count++;
        p += flen;
    }

    size_t newlen = slen + count * (tlen - flen) + 1;
    char *result = malloc(newlen);
    if (!result)
        return NULL;

    char *wp = result;
    p = s;
    while (*p) {
        const char *match = strstr(p, from);
        if (match == p) {
            memcpy(wp, to, tlen);
            wp += tlen;
            p += flen;
        } else {
            const char *end = match ? match : p + strlen(p);
            size_t n = (size_t)(end - p);
            memcpy(wp, p, n);
            wp += n;
            p = end;
        }
    }
    *wp = '\0';
    return result;
}

/* =========================================================================
 * Utility: file & directory operations
 * ========================================================================= */

static char *slurp(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len < 0) {
        fclose(f);
        return NULL;
    }
    char *buf = malloc((size_t)len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t n = fread(buf, 1, (size_t)len, f);
    fclose(f);
    buf[n] = '\0';
    if (out_len)
        *out_len = n;
    return buf;
}

static int spew(const char *path, const char *content) {
    FILE *f = fopen(path, "wb");
    if (!f)
        return -1;
    size_t len = strlen(content);
    size_t n = fwrite(content, 1, len, f);
    fclose(f);
    return (n == len) ? 0 : -1;
}

static int replace_in_file(const char *path, const char *s1, const char *r1, const char *s2,
                           const char *r2, const char *s3, const char *r3, const char *s4,
                           const char *r4) {
    size_t len;
    char *orig = slurp(path, &len);
    if (!orig)
        return 0;

    char *tmp = NULL;
    char *cur = orig;
    const char *searches[] = {s1, s2, s3, s4};
    const char *replaces[] = {r1, r2, r3, r4};

    for (int i = 0; i < 4; i++) {
        if (!searches[i])
            break;
        tmp = str_replace_all(cur, searches[i], replaces[i]);
        if (i > 0)
            free(cur);
        if (!tmp) {
            free(orig);
            return -1;
        }
        cur = tmp;
    }

    int changed = (strcmp(cur, orig) != 0);
    int ret = 0;
    if (changed) {
        if (g_dry_run) {
            ret = 1;
        } else {
            ret = (spew(path, cur) == 0) ? 1 : -1;
        }
    }
    if (cur != orig)
        free(cur);
    free(orig);
    return ret;
}

static int file_has_any(const char *path) {
    size_t len;
    char *buf = slurp(path, &len);
    if (!buf)
        return 0;
    int has = (strstr(buf, "CProjectStandard") || strstr(buf, "cproject") ||
               strstr(buf, "CPROJECT") || strstr(buf, "CProject"));
    free(buf);
    return has;
}

static int rmrf(const char *path) {
    if (g_dry_run)
        return 0;
#ifdef _WIN32
    if (_unlink(path) == 0)
        return 0;
    return _rmdir(path);
#else
    if (unlink(path) == 0)
        return 0;
    return rmdir(path);
#endif
}

static int rename_path(const char *old, const char *newpath) {
    if (g_dry_run)
        return 1;
    return rename(old, newpath) == 0 ? 1 : 0;
}

/* =========================================================================
 * Core: recursive file walk for rename
 * ========================================================================= */

static int has_rename_ext(const char *name) {
    static const char *exts[] = {".c",  ".h",  ".cmake", ".txt", ".json", ".yml",
                                 ".md", ".in", ".cpp",   ".ps1", ".sh",   NULL};
    const char *dot = strrchr(name, '.');
    if (!dot)
        return 0;
    for (int i = 0; exts[i]; i++)
        if (strcmp(dot, exts[i]) == 0)
            return 1;
    return 0;
}

static int should_skip_dir(const char *name) {
    return (strcmp(name, "build") == 0 || strcmp(name, ".git") == 0);
}

static void walk_and_rename(const char *dir, const char *pascal, const char *lower,
                            const char *upper) {
#ifdef _WIN32
    char pattern[1024];
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    struct _finddata_t fd;
    intptr_t h = _findfirst(pattern, &fd);
    if (h == -1)
        return;
    do {
        if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0)
            continue;
        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", dir, fd.name);
        if (fd.attrib & _A_SUBDIR) {
            if (should_skip_dir(fd.name))
                continue;
            walk_and_rename(full, pascal, lower, upper);
        } else if (has_rename_ext(fd.name) && file_has_any(full)) {
            int r = replace_in_file(full, "CProjectStandard", pascal, "cproject", lower, "CPROJECT",
                                    upper, "CProject", pascal);
            if (r > 0) {
                g_renamed_files++;
                printf("    %s\n", full);
            }
        }
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(dir);
    if (!d)
        return;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        char full[1024];
        snprintf(full, sizeof(full), "%s/%s", dir, e->d_name);
        struct stat st;
        if (stat(full, &st) != 0)
            continue;
        if (S_ISDIR(st.st_mode)) {
            if (should_skip_dir(e->d_name))
                continue;
            walk_and_rename(full, pascal, lower, upper);
        } else if (has_rename_ext(e->d_name) && file_has_any(full)) {
            int r = replace_in_file(full, "CProjectStandard", pascal, "cproject", lower, "CPROJECT",
                                    upper, "CProject", pascal);
            if (r > 0) {
                g_renamed_files++;
                printf("    %s\n", full);
            }
        }
    }
    closedir(d);
#endif
}

/* =========================================================================
 * Core: directory & cmake file renames
 * ========================================================================= */

static int do_dir_renames(const char *lower) {
    int count = 0;
    char old[512], newpath[512];

    snprintf(old, sizeof(old), "include/cproject");
    snprintf(newpath, sizeof(newpath), "include/%s", lower);
    if (!access(old, F_OK) && rename_path(old, newpath)) {
        printf("    %s -> %s\n", old, newpath);
        count++;
    }
    snprintf(old, sizeof(old), "src/cproject");
    snprintf(newpath, sizeof(newpath), "src/%s", lower);
    if (!access(old, F_OK) && rename_path(old, newpath)) {
        printf("    %s -> %s\n", old, newpath);
        count++;
    }
    return count;
}

static void do_cmake_renames(const char *pascal, const char *lower) {
    char old[256], newpath[256];
    snprintf(old, sizeof(old), "cmake/CProjectStandardConfig.cmake.in");
    snprintf(newpath, sizeof(newpath), "cmake/%sConfig.cmake.in", pascal);
    if (!access(old, F_OK) && rename_path(old, newpath))
        printf("    %s -> %s\n", old, newpath);

    snprintf(old, sizeof(old), "cmake/cproject_version.h.in");
    snprintf(newpath, sizeof(newpath), "cmake/%s_version.h.in", lower);
    if (!access(old, F_OK) && rename_path(old, newpath))
        printf("    %s -> %s\n", old, newpath);

    snprintf(old, sizeof(old), "res/cproject_example.rc.in");
    snprintf(newpath, sizeof(newpath), "res/%s_example.rc.in", lower);
    if (!access(old, F_OK) && rename_path(old, newpath))
        printf("    %s -> %s\n", old, newpath);
}

/* =========================================================================
 * Core: stub main.c generation
 * ========================================================================= */

static void write_stub_main(const char *pascal, const char *lower, int has_cli) {
    char content[512];
    if (has_cli) {
        snprintf(content, sizeof(content),
                 "#include <%s/example.h>\n"
                 "#include <stdio.h>\n\n"
                 "int main(void) {\n"
                 "    %sVersion v = %s_version();\n"
                 "    printf(\"%s v%%d.%%d.%%d\\n\", v.major, v.minor, v.patch);\n"
                 "    return 0;\n"
                 "}\n",
                 lower, pascal, lower, pascal);
    } else {
        snprintf(content, sizeof(content),
                 "/* SETUP: Your entry point goes here.\n"
                 " * Remove this file if %s is a library-only project. */\n"
                 "int main(void) { return 0; }\n",
                 pascal);
    }
    if (g_dry_run) {
        printf("    [dry-run] src/main.c\n");
    } else {
        spew("src/main.c", content);
    }
}

/* =========================================================================
 * Core: feature stripping
 * ========================================================================= */

typedef struct {
    const char *name;
    const char *path;
} StripItem;

static const StripItem FEAT_STRIPS[FEAT_COUNT][4] = {
    [FEAT_INSTALLABLE] =
        {
            {"installable packaging", "cmake/Packaging.cmake"},
            {"package smoke test", "tests/package_smoke"},
            {NULL, NULL},
        },
    [FEAT_SUBPROJECT] =
        {
            {"subproject smoke test", "tests/subproject_smoke"},
            {NULL, NULL},
        },
    [FEAT_SHARED] = {{NULL, NULL}},
    [FEAT_SANITIZERS] = {{NULL, NULL}},
    [FEAT_COVERAGE] = {{NULL, NULL}},
    [FEAT_CI] =
        {
            {"GitHub Actions CI", ".github/workflows"},
            {NULL, NULL},
        },
    [FEAT_AI_AGENT] =
        {
            {"AI agent policy doc", "docs/guides/ai-agent.md"},
            {NULL, NULL},
        },
    [FEAT_CLI] = {{NULL, NULL}},
};

static void strip_features(const Config *cfg) {
    for (int f = 0; f < FEAT_COUNT; f++) {
        if (cfg->features[f])
            continue;
        const StripItem *items = FEAT_STRIPS[f];
        for (int i = 0; items[i].name; i++) {
            if (access(items[i].path, F_OK) == 0) {
                int ok = rmrf(items[i].path);
                if (ok == 0 || g_dry_run)
                    printf("    %s: %s\n", items[i].name, items[i].path);
            }
        }
    }
}

/* =========================================================================
 * Core: git remote cleanup
 * ========================================================================= */

static void clear_git_remote(void) {
    if (access(".git", F_OK) != 0)
        return;
    if (g_dry_run) {
        printf("    git remote origin ...\n");
        return;
    }
    (void)system("git remote remove origin 2>/dev/null");
}

/* =========================================================================
 * Detect language from environment
 * ========================================================================= */

static int detect_lang(void) {
    const char *v;
    v = getenv("LANG");
    if (v && strstr(v, "zh"))
        return LANG_ZH;
    v = getenv("LC_ALL");
    if (v && strstr(v, "zh"))
        return LANG_ZH;
    v = getenv("LC_MESSAGES");
    if (v && strstr(v, "zh"))
        return LANG_ZH;
    return LANG_EN;
}

/* =========================================================================
 * Interactive screens
 * ========================================================================= */

static void print_banner(void) {
    CProjectVersion v = cproject_version();
    const char *s = T(M_BANNER);
    printf("\n  ");
    printf(s, v.major, v.minor, v.patch);
    printf("\n  ");
    for (int i = 0; i < 40; i++)
        putchar('=');
    printf("\n\n");
}

static void select_language(void) {
    char buf[8];
    int detected = detect_lang();
    g_lang = (Lang)detected;
    printf("%s", T(M_LANG_SELECT));
    fflush(stdout);
    if (!safe_gets(buf, (int)sizeof(buf)) || buf[0] == '\0')
        return;
    if (buf[0] == 'z' || buf[0] == 'Z')
        g_lang = LANG_ZH;
    else
        g_lang = LANG_EN;
}

static int validate_name(const char *s) {
    if (!s[0] || !isupper((unsigned char)s[0]))
        return 0;
    for (size_t i = 0; s[i]; i++)
        if (!isalnum((unsigned char)s[i]))
            return 0;
    return 1;
}

static const enum Msg FEAT_MSGS[FEAT_COUNT] = {
    M_FEAT_INSTALLABLE, M_FEAT_SUBPROJECT, M_FEAT_SHARED,   M_FEAT_SANITIZERS,
    M_FEAT_COVERAGE,    M_FEAT_CI,         M_FEAT_AI_AGENT, M_FEAT_CLI,
};

static int edit_features(Config *cfg) {
    for (;;) {
        printf("\n%s\n\n", T(M_FEAT_HEADER));
        for (int i = 0; i < FEAT_COUNT; i++) {
            char yn = cfg->features[i] ? 'Y' : 'N';
            printf("  [%c] %s\n", yn, T(FEAT_MSGS[i]));
        }
        printf("\n  All correct? [Y/n]: ");
        fflush(stdout);
        char buf[8];
        if (!safe_gets(buf, (int)sizeof(buf)))
            return 1;
        if (buf[0] == '\0' || buf[0] == 'y' || buf[0] == 'Y')
            return 1;
        if (buf[0] == 'n' || buf[0] == 'N') {
            printf("\n  Enter number to toggle (1-%d, 0=done): ", FEAT_COUNT);
            fflush(stdout);
            if (!safe_gets(buf, (int)sizeof(buf)))
                return 1;
            int n = atoi(buf);
            if (n >= 1 && n <= FEAT_COUNT)
                cfg->features[n - 1] = !cfg->features[n - 1];
        }
    }
}

static void print_review(const Config *cfg) {
    printf("\n\n  %s\n", T(M_REVIEW_HEADER));
    printf(T(M_REVIEW_NAME), cfg->pascal);
    printf("\n");
    printf(T(M_REVIEW_NAMESPACE), cfg->lower);
    printf("\n");
    printf(T(M_REVIEW_MACRO), cfg->upper);
    printf("\n");
    printf(T(M_REVIEW_DESC), cfg->description);
    printf("\n");
    printf(T(M_REVIEW_VERSION), cfg->version);
    printf("\n");
    printf(T(M_REVIEW_LICENSE), cfg->license);
    printf("\n");
    printf("\n");
    for (int i = 0; i < FEAT_COUNT; i++)
        printf("  %-30s %s\n", T(FEAT_MSGS[i]),
               cfg->features[i] ? T(M_REVIEW_YES) : T(M_REVIEW_NO));

    printf("\n  %s\n", T(M_REVIEW_OPS));
    printf(T(M_REVIEW_RENAME), cfg->pascal);
    printf("\n");
    printf(T(M_REVIEW_DIRS), cfg->lower);
    printf("\n");
    for (int f = 0; f < FEAT_COUNT; f++) {
        if (cfg->features[f])
            continue;
        const StripItem *items = FEAT_STRIPS[f];
        for (int i = 0; items[i].name; i++)
            printf(T(M_REVIEW_STRIP), items[i].name), printf("\n");
    }
    printf("%s\n", T(M_REVIEW_GIT));
}

static void print_results(const Config *cfg) {
    printf("\n  Results:\n");
    printf(T(M_OK_RENAME), g_renamed_files);
    printf("\n");
    printf(T(M_OK_DIRS), g_renamed_dirs);
    printf("\n");
    for (int f = 0; f < FEAT_COUNT; f++) {
        if (cfg->features[f])
            continue;
        const StripItem *items = FEAT_STRIPS[f];
        for (int i = 0; items[i].name; i++)
            printf(T(M_OK_STRIP), items[i].name), printf("\n");
    }
    printf("%s\n", T(M_OK_GIT));
    printf("%s\n", T(M_OK_STUB));
    printf("\n  %s\n", T(M_NEXT_TITLE));
    pln(M_NEXT_1);
    pln(M_NEXT_2);
    printf(T(M_NEXT_3), cfg->lower);
    printf("\n");
    pln(M_NEXT_4);
    pln(M_NEXT_5);
    pln(M_NEXT_6);
    printf("\n");
}

/* =========================================================================
 * Interactive: full flow
 * ========================================================================= */

static int run_interactive(void) {
    Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.features[FEAT_INSTALLABLE] = 1;
    cfg.features[FEAT_SUBPROJECT] = 1;
    cfg.features[FEAT_SHARED] = 1;
    cfg.features[FEAT_SANITIZERS] = 1;
    cfg.features[FEAT_COVERAGE] = 0;
    cfg.features[FEAT_CI] = 1;
    cfg.features[FEAT_AI_AGENT] = 0;
    cfg.features[FEAT_CLI] = 1;

    select_language();
    printf("\n");
    print_banner();

    /* ── project name ── */
    for (;;) {
        ask_raw(M_NAME_PROMPT, cfg.pascal, sizeof(cfg.pascal), "");
        if (cfg.pascal[0] == '\0') {
            pln(M_ABORTED);
            return 1;
        }
        if (validate_name(cfg.pascal))
            break;
        pln(M_NAME_ERROR);
        printf("\n");
    }
    for (size_t i = 0; cfg.pascal[i]; i++) {
        cfg.lower[i] = (char)tolower((unsigned char)cfg.pascal[i]);
        cfg.upper[i] = (char)toupper((unsigned char)cfg.pascal[i]);
    }

    /* ── metadata ── */
    printf("\n");
    ask_string(M_DESC_PROMPT, cfg.description, sizeof(cfg.description), M_DESC_DEFAULT);
    ask_raw(M_AUTHOR_PROMPT, cfg.author, sizeof(cfg.author), "");
    ask_string(M_VERSION_PROMPT, cfg.version, sizeof(cfg.version), M_VERSION_DEFAULT);
    ask_string(M_LICENSE_PROMPT, cfg.license, sizeof(cfg.license), M_LICENSE_DEFAULT);

    /* ── features ── */
    if (!edit_features(&cfg))
        return 1;

    /* ── review ── */
    print_review(&cfg);
    printf("\n");
    if (!ask_yesno(M_PROCEED, 1)) {
        pln(M_ABORTED);
        return 0;
    }

    /* ── execute ── */
    printf("\n  Renaming identifiers...\n");
    g_renamed_files = 0;
    walk_and_rename(".", cfg.pascal, cfg.lower, cfg.upper);

    printf("  Renaming directories...\n");
    g_renamed_dirs = do_dir_renames(cfg.lower);

    printf("  Renaming cmake files...\n");
    do_cmake_renames(cfg.pascal, cfg.lower);

    printf("  Stripping features...\n");
    strip_features(&cfg);

    printf("  Clearing git remote...\n");
    clear_git_remote();

    printf("  Writing stub main.c...\n");
    write_stub_main(cfg.pascal, cfg.lower, cfg.features[FEAT_CLI]);

    print_results(&cfg);
    return 0;
}

/* =========================================================================
 * CLI helpers: paths, copying, commands
 * ========================================================================= */

static void init_default_config(Config *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->features[FEAT_INSTALLABLE] = 1;
    cfg->features[FEAT_SUBPROJECT] = 1;
    cfg->features[FEAT_SHARED] = 1;
    cfg->features[FEAT_SANITIZERS] = 1;
    cfg->features[FEAT_COVERAGE] = 0;
    cfg->features[FEAT_CI] = 1;
    cfg->features[FEAT_AI_AGENT] = 0;
    cfg->features[FEAT_CLI] = 1;
}

static void derive_names(Config *cfg) {
    size_t i;
    for (i = 0; cfg->pascal[i] && i + 1 < sizeof(cfg->lower); i++) {
        cfg->lower[i] = (char)tolower((unsigned char)cfg->pascal[i]);
        cfg->upper[i] = (char)toupper((unsigned char)cfg->pascal[i]);
    }
    cfg->lower[i] = '\0';
    cfg->upper[i] = '\0';
    if (cfg->description[0] == '\0')
        strcpy(cfg->description, "A C library");
    if (cfg->version[0] == '\0')
        strcpy(cfg->version, "0.1.0");
    if (cfg->license[0] == '\0')
        strcpy(cfg->license, "MIT");
}

static void print_help(void) {
    printf("Usage:\n");
    printf("  cproject\n  cproject new [Name] [options]\n  cproject init --in-place [Name] "
           "[options]\n");
    printf("  cproject --help\n  cproject --version\n\n");
    printf("Commands:\n  new       Create a new project from the installed template (default)\n");
    printf("  init      Initialize the current template directory in place\n\n");
    printf("Options:\n  --output <dir>          Parent directory for the new project\n");
    printf("  --template-root <dir>   Template source directory\n  --yes, -y               Accept "
           "defaults\n");
    printf("  --dry-run               Preview changes without writing\n  --lang <en|zh>          "
           "Language\n");
    printf(
        "  --description <text>    Project description\n  --author <text>         Author/vendor\n");
    printf("  --license <text>        License\n");
}

static void print_version(void) {
    CProjectVersion v = cproject_version();
    printf("%d.%d.%d\n", v.major, v.minor, v.patch);
}

static int path_exists(const char *path) {
    return access(path, F_OK) == 0;
}

static int is_dir_path(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode) != 0;
}

static void join_path(char *out, size_t size, const char *a, const char *b) {
    size_t len = strlen(a);
    if (len > 0 && (a[len - 1] == '/' || a[len - 1] == '\\'))
        snprintf(out, size, "%s%s", a, b);
    else
        snprintf(out, size, "%s/%s", a, b);
}

static int has_file_under(const char *root, const char *rel) {
    char path[1024];
    join_path(path, sizeof(path), root, rel);
    return path_exists(path);
}

static int is_template_root(const char *path) {
    return has_file_under(path, "CMakeLists.txt") && has_file_under(path, "cmake/Sources.cmake") &&
           has_file_under(path, "include/cproject/example.h");
}

static int mkdir_one(const char *path) {
    if (path[0] == '\0' || is_dir_path(path))
        return 0;
#ifdef _WIN32
    return (_mkdir(path) == 0 || errno == EEXIST) ? 0 : -1;
#else
    return (mkdir(path, 0777) == 0 || errno == EEXIST) ? 0 : -1;
#endif
}

static int mkdir_recursive(const char *path) {
    char tmp[1024];
    size_t len = strlen(path);
    if (len >= sizeof(tmp))
        return -1;
    strcpy(tmp, path);
    for (char *q = tmp + 1; *q; q++) {
        if (*q == '/' || *q == '\\') {
            char old = *q;
            *q = '\0';
            if (mkdir_one(tmp) != 0)
                return -1;
            *q = old;
        }
    }
    return mkdir_one(tmp);
}

static int is_skip_copy_name(const char *name) {
    return strcmp(name, ".git") == 0 || strcmp(name, "build") == 0 || strcmp(name, ".vs") == 0 ||
           strcmp(name, ".cache") == 0 || strcmp(name, "install") == 0 ||
           strncmp(name, "install-", 8) == 0 || strcmp(name, "dist") == 0 ||
           strcmp(name, "out") == 0 || strcmp(name, "_CPack_Packages") == 0 ||
           strcmp(name, "Testing") == 0 || strcmp(name, ".hermes") == 0;
}

static int copy_file_binary(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in)
        return -1;
    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }
    char buf[16384];
    size_t n;
    int ok = 0;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            ok = -1;
            break;
        }
    }
    if (ferror(in))
        ok = -1;
    fclose(out);
    fclose(in);
    return ok;
}

static int copy_tree(const char *src, const char *dst) {
    if (mkdir_recursive(dst) != 0)
        return -1;
#ifdef _WIN32
    char pattern[1024];
    snprintf(pattern, sizeof(pattern), "%s\\*", src);
    struct _finddata_t fd;
    intptr_t h = _findfirst(pattern, &fd);
    if (h == -1)
        return -1;
    do {
        if (strcmp(fd.name, ".") == 0 || strcmp(fd.name, "..") == 0 || is_skip_copy_name(fd.name))
            continue;
        char sfull[1024], dfull[1024];
        join_path(sfull, sizeof(sfull), src, fd.name);
        join_path(dfull, sizeof(dfull), dst, fd.name);
        if (fd.attrib & _A_SUBDIR) {
            if (copy_tree(sfull, dfull) != 0) {
                _findclose(h);
                return -1;
            }
        } else if (copy_file_binary(sfull, dfull) != 0) {
            _findclose(h);
            return -1;
        }
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(src);
    if (!d)
        return -1;
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0 ||
            is_skip_copy_name(e->d_name))
            continue;
        char sfull[1024], dfull[1024];
        join_path(sfull, sizeof(sfull), src, e->d_name);
        join_path(dfull, sizeof(dfull), dst, e->d_name);
        struct stat st;
        if (stat(sfull, &st) != 0)
            continue;
        if (S_ISDIR(st.st_mode)) {
            if (copy_tree(sfull, dfull) != 0) {
                closedir(d);
                return -1;
            }
        } else if (copy_file_binary(sfull, dfull) != 0) {
            closedir(d);
            return -1;
        }
    }
    closedir(d);
#endif
    return 0;
}

static int apply_setup_to_current_directory(const Config *cfg) {
    char wizard_begin[128], wizard_end[128];
    snprintf(wizard_begin, sizeof(wizard_begin), "# %s_WIZARD_TESTS_BEGIN", cfg->upper);
    snprintf(wizard_end, sizeof(wizard_end), "# %s_WIZARD_TESTS_END", cfg->upper);
    g_renamed_files = g_renamed_dirs = 0;
    printf("Renaming identifiers...\n");
    walk_and_rename(".", cfg->pascal, cfg->lower, cfg->upper);
    printf("  %d files\n", g_renamed_files);
    printf("Renaming directories...\n");
    g_renamed_dirs = do_dir_renames(cfg->lower);
    printf("  %d directories\n", g_renamed_dirs);
    printf("Renaming cmake files...\n");
    do_cmake_renames(cfg->pascal, cfg->lower);
    printf("Stripping features...\n");
    strip_features(cfg);
    printf("Clearing git remote...\n");
    clear_git_remote();
    printf("Writing stub main.c...\n");
    write_stub_main(cfg->pascal, cfg->lower, cfg->features[FEAT_CLI]);
    if (!g_dry_run) {
        replace_in_file("cmake/Tests.cmake", wizard_begin, "#[[ WIZARD_TESTS_BEGIN", wizard_end,
                        "WIZARD_TESTS_END ]]", NULL, NULL, NULL, NULL);
    }
    return 0;
}

static char *executable_dir(const char *argv0) {
    const char *slash = strrchr(argv0, '/');
#ifdef _WIN32
    const char *bs = strrchr(argv0, '\\');
    if (bs && (!slash || bs > slash))
        slash = bs;
#endif
    if (slash && slash != argv0) {
        size_t n = (size_t)(slash - argv0);
        char *copy = malloc(n + 1);
        if (!copy)
            return NULL;
        memcpy(copy, argv0, n);
        copy[n] = '\0';
        return copy;
    }
    char *copy = malloc(2);
    if (copy) {
        copy[0] = '.';
        copy[1] = '\0';
    }
    return copy;
}

static int find_upward_template_root(const char *start, char *out, size_t size) {
    char dir[1024];
    strncpy(dir, start, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';
    for (int depth = 0; depth < 8; depth++) {
        if (is_template_root(dir)) {
            strncpy(out, dir, size - 1);
            out[size - 1] = '\0';
            return 1;
        }
        char *slash = strrchr(dir, '/');
#ifdef _WIN32
        char *bs = strrchr(dir, '\\');
        if (bs && (!slash || bs > slash))
            slash = bs;
#endif
        if (!slash)
            break;
        *slash = '\0';
    }
    return 0;
}

static int resolve_template_root(const char *argv0, const char *explicit_root, char *out,
                                 size_t size) {
    if (explicit_root && explicit_root[0]) {
        if (!is_template_root(explicit_root))
            return 0;
        strncpy(out, explicit_root, size - 1);
        out[size - 1] = '\0';
        return 1;
    }
    const char *env = getenv("CPROJECT_TEMPLATE_ROOT");
    if (env && env[0] && is_template_root(env)) {
        strncpy(out, env, size - 1);
        out[size - 1] = '\0';
        return 1;
    }
    char *exe = executable_dir(argv0);
    if (exe) {
        char installed[1024];
        join_path(installed, sizeof(installed), exe, "../share/c-project-standard");
        if (is_template_root(installed)) {
            strncpy(out, installed, size - 1);
            out[size - 1] = '\0';
            free(exe);
            return 1;
        }
        if (find_upward_template_root(exe, out, size)) {
            free(exe);
            return 1;
        }
        free(exe);
    }
    if (is_template_root(".")) {
        strncpy(out, ".", size - 1);
        out[size - 1] = '\0';
        return 1;
    }
    return 0;
}

static int parse_common_option(int argc, char **argv, int *i, Config *cfg, char *template_root,
                               size_t template_root_size, char *output, size_t output_size) {
    if (strcmp(argv[*i], "--dry-run") == 0)
        g_dry_run = 1;
    else if (strcmp(argv[*i], "--yes") == 0 || strcmp(argv[*i], "-y") == 0)
        g_yes = 1;
    else if (strcmp(argv[*i], "--lang") == 0 && *i + 1 < argc) {
        (*i)++;
        g_lang = (strcmp(argv[*i], "zh") == 0) ? LANG_ZH : LANG_EN;
    } else if (strcmp(argv[*i], "--description") == 0 && *i + 1 < argc)
        strncpy(cfg->description, argv[++(*i)], sizeof(cfg->description) - 1);
    else if (strcmp(argv[*i], "--author") == 0 && *i + 1 < argc)
        strncpy(cfg->author, argv[++(*i)], sizeof(cfg->author) - 1);
    else if (strcmp(argv[*i], "--version") == 0 && *i + 1 < argc)
        strncpy(cfg->version, argv[++(*i)], sizeof(cfg->version) - 1);
    else if (strcmp(argv[*i], "--license") == 0 && *i + 1 < argc)
        strncpy(cfg->license, argv[++(*i)], sizeof(cfg->license) - 1);
    else if (strcmp(argv[*i], "--template-root") == 0 && *i + 1 < argc) {
        strncpy(template_root, argv[++(*i)], template_root_size - 1);
        template_root[template_root_size - 1] = '\0';
    } else if (strcmp(argv[*i], "--output") == 0 && *i + 1 < argc) {
        strncpy(output, argv[++(*i)], output_size - 1);
        output[output_size - 1] = '\0';
    } else if (strcmp(argv[*i], "--no-installable") == 0)
        cfg->features[FEAT_INSTALLABLE] = 0;
    else if (strcmp(argv[*i], "--no-subproject") == 0)
        cfg->features[FEAT_SUBPROJECT] = 0;
    else if (strcmp(argv[*i], "--no-shared") == 0)
        cfg->features[FEAT_SHARED] = 0;
    else if (strcmp(argv[*i], "--no-sanitizers") == 0)
        cfg->features[FEAT_SANITIZERS] = 0;
    else if (strcmp(argv[*i], "--no-coverage") == 0)
        cfg->features[FEAT_COVERAGE] = 0;
    else if (strcmp(argv[*i], "--no-ci") == 0)
        cfg->features[FEAT_CI] = 0;
    else if (strcmp(argv[*i], "--no-ai-agent") == 0)
        cfg->features[FEAT_AI_AGENT] = 0;
    else if (strcmp(argv[*i], "--no-cli") == 0)
        cfg->features[FEAT_CLI] = 0;
    else
        return 0;
    return 1;
}

static void default_output_parent(char *out, size_t size) {
    const char *env = getenv("CPROJECT_DEFAULT_OUTPUT");
    if (env && env[0]) {
        strncpy(out, env, size - 1);
        out[size - 1] = '\0';
        return;
    }
#ifdef _WIN32
    if (!getcwd(out, (int)size)) {
#else
    if (!getcwd(out, size)) {
#endif
        strncpy(out, ".", size - 1);
        out[size - 1] = '\0';
    }
}

static int run_new_command(int argc, char **argv, int first, const char *argv0) {
    Config cfg;
    char template_arg[1024] = "", template_root[1024] = "", output_parent[1024] = "",
         dest[1024] = "";
    init_default_config(&cfg);
    for (int i = first; i < argc; i++) {
        if (argv[i][0] != '-' && cfg.pascal[0] == '\0') {
            strncpy(cfg.pascal, argv[i], sizeof(cfg.pascal) - 1);
            continue;
        }
        if (strcmp(argv[i], "--interactive") == 0)
            continue;
        if (!parse_common_option(argc, argv, &i, &cfg, template_arg, sizeof(template_arg),
                                 output_parent, sizeof(output_parent))) {
            fprintf(stderr, "ERROR: unknown option or extra argument: %s\n", argv[i]);
            return 2;
        }
    }
    if (g_lang == LANG_EN)
        g_lang = (Lang)detect_lang();
    if (cfg.pascal[0] == '\0') {
        select_language();
        print_banner();
        for (;;) {
            ask_raw(M_NAME_PROMPT, cfg.pascal, sizeof(cfg.pascal), "");
            if (cfg.pascal[0] == '\0') {
                pln(M_ABORTED);
                return 1;
            }
            if (validate_name(cfg.pascal))
                break;
            pln(M_NAME_ERROR);
        }
    }
    if (!validate_name(cfg.pascal)) {
        pln(M_NAME_ERROR);
        return 1;
    }
    derive_names(&cfg);
    if (output_parent[0] == '\0')
        default_output_parent(output_parent, sizeof(output_parent));
    if (!g_yes && !g_dry_run) {
        ask_string(M_DESC_PROMPT, cfg.description, sizeof(cfg.description), M_DESC_DEFAULT);
        ask_raw(M_AUTHOR_PROMPT, cfg.author, sizeof(cfg.author), "");
        ask_string(M_VERSION_PROMPT, cfg.version, sizeof(cfg.version), M_VERSION_DEFAULT);
        ask_string(M_LICENSE_PROMPT, cfg.license, sizeof(cfg.license), M_LICENSE_DEFAULT);
        edit_features(&cfg);
        derive_names(&cfg);
    }
    if (!resolve_template_root(argv0, template_arg, template_root, sizeof(template_root))) {
        fprintf(stderr, "ERROR: template root not found. Use --template-root <dir>.\n");
        return 1;
    }
    join_path(dest, sizeof(dest), output_parent, cfg.pascal);
    printf("Template: %s\nOutput:   %s\n", template_root, dest);
    if (!g_yes && !g_dry_run && !ask_yesno(M_PROCEED, 1)) {
        pln(M_ABORTED);
        return 0;
    }
    if (g_dry_run) {
        CProjectVersion v = cproject_version();
        printf(T(M_BANNER), v.major, v.minor, v.patch);
        printf(" (--dry-run)\n");
        printf("Would copy template and create project at %s\n", dest);
        printf("\n%s\n", T(M_DRYRUN_DONE));
        return 0;
    }
    if (path_exists(dest)) {
        fprintf(stderr, "ERROR: output path already exists: %s\n", dest);
        return 1;
    }
    printf("Copying template...\n");
    if (copy_tree(template_root, dest) != 0) {
        fprintf(stderr, "ERROR: failed to copy template to %s\n", dest);
        return 1;
    }
#ifdef _WIN32
    if (_chdir(dest) != 0)
#else
    if (chdir(dest) != 0)
#endif
    {
        fprintf(stderr, "ERROR: failed to enter output path: %s\n", dest);
        return 1;
    }
    apply_setup_to_current_directory(&cfg);
    printf("\nDone. Project created at %s\n", dest);
    return 0;
}

static int run_init_command(int argc, char **argv, int first) {
    Config cfg;
    char unused_template[2] = "", unused_output[2] = "";
    int in_place = 0;
    init_default_config(&cfg);
    for (int i = first; i < argc; i++) {
        if (strcmp(argv[i], "--in-place") == 0) {
            in_place = 1;
            continue;
        }
        if (argv[i][0] != '-' && cfg.pascal[0] == '\0') {
            strncpy(cfg.pascal, argv[i], sizeof(cfg.pascal) - 1);
            continue;
        }
        if (!parse_common_option(argc, argv, &i, &cfg, unused_template, sizeof(unused_template),
                                 unused_output, sizeof(unused_output))) {
            fprintf(stderr, "ERROR: unknown option or extra argument: %s\n", argv[i]);
            return 2;
        }
    }
    if (!in_place) {
        fprintf(stderr,
                "ERROR: init modifies the current directory. Pass --in-place to continue.\n");
        return 2;
    }
    if (!is_template_root(".")) {
        fprintf(stderr, "%s\n", T(M_NOT_ROOT));
        return 1;
    }
    if (cfg.pascal[0] == '\0')
        return run_interactive();
    if (!validate_name(cfg.pascal)) {
        pln(M_NAME_ERROR);
        return 1;
    }
    derive_names(&cfg);
    if (g_dry_run) {
        CProjectVersion v = cproject_version();
        printf(T(M_BANNER), v.major, v.minor, v.patch);
        printf(" (--dry-run)\n\n");
    }
    if (!g_yes && !g_dry_run) {
        printf("Initialize current template directory as %s.\n", cfg.pascal);
        if (!ask_yesno(M_PROCEED, 1)) {
            pln(M_ABORTED);
            return 0;
        }
    }
    apply_setup_to_current_directory(&cfg);
    if (g_dry_run)
        printf("\n%s\n", T(M_DRYRUN_DONE));
    else
        printf("\nDone. Run: cmake --build --preset ninja-debug\n");
    return 0;
}

/* =========================================================================
 * Entry point
 * ========================================================================= */

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        print_version();
        return 0;
    }
    if (argc <= 1)
        return run_new_command(argc, argv, 1, argv[0]);
    if (strcmp(argv[1], "new") == 0)
        return run_new_command(argc, argv, 2, argv[0]);
    if (strcmp(argv[1], "init") == 0)
        return run_init_command(argc, argv, 2);
    if (argv[1][0] != '-')
        return run_new_command(argc, argv, 1, argv[0]);
    fprintf(stderr, "ERROR: unknown command: %s\nRun 'cproject --help' for usage.\n", argv[1]);
    return 2;
}
