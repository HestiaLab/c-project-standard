#include "wizard.h"

#include <cproject/version.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#endif
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <stdint.h>
#endif

#define WIZARD_EXIT_OK 0
#define WIZARD_EXIT_RUNTIME 1
#define WIZARD_EXIT_USAGE 2

typedef struct {
    WizardConfig cfg;
    WizardOptions options;
} CommandState;

static const char *tr(WizardLang lang, const char *en, const char *zh) {
    return lang == WIZARD_LANG_ZH ? zh : en;
}

static int stdin_is_tty(void) {
#ifdef _WIN32
    return _isatty(_fileno(stdin));
#else
    return isatty(fileno(stdin));
#endif
}

static int current_directory(char *out, size_t size) {
#ifdef _WIN32
    return _getcwd(out, (int)size) ? 0 : -1;
#else
    return getcwd(out, size) ? 0 : -1;
#endif
}

static int ascii_case_equal(const char *left, const char *right) {
    while (*left && *right) {
        char a = (char)tolower((unsigned char)*left++);
        char b = (char)tolower((unsigned char)*right++);
        if (a != b)
            return 0;
    }
    return *left == '\0' && *right == '\0';
}

static int detect_language(void) {
    static const char *const variables[] = {"LC_ALL", "LC_MESSAGES", "LANG"};
    for (size_t i = 0; i < sizeof(variables) / sizeof(variables[0]); i++) {
        const char *value = getenv(variables[i]);
        if (value && strstr(value, "zh"))
            return WIZARD_LANG_ZH;
    }
    return WIZARD_LANG_EN;
}

static void print_version(void) {
    printf("%s\n", CPROJECT_VERSION_STRING);
}

static void print_banner(WizardLang lang) {
    printf("\n  %s v%s\n", tr(lang, "C Project Standard", "C 项目工程标准"),
           CPROJECT_VERSION_STRING);
    printf("  ========================================\n\n");
}

static void print_help(void) {
    printf("Usage:\n");
    printf("  cproject\n");
    printf("  cproject new [Name] [options]\n");
    printf("  cproject init --in-place [Name] [options]\n\n");
    printf("Commands:\n");
    printf("  new       Create a project from the template (default)\n");
    printf("  init      Initialize the current template tree in place\n\n");
    printf("Options:\n");
    printf("  --output <dir>             Parent directory for a new project\n");
    printf("  --template-root <dir>      Template source directory\n");
    printf("  --yes, -y                  Accept defaults and never prompt\n");
    printf("  --dry-run                  Preview without changing the destination\n");
    printf("  --json                     Emit one JSON object on stdout\n");
    printf("  --lang <en|zh>             Interface language (explicit wins over LANG)\n");
    printf("  --description <text>       Project description\n");
    printf("  --author <text>            Author or vendor\n");
    printf("  --project-version <x.y.z>  Project version\n");
    printf("  --license <name>           License name (default: MIT)\n");
    printf("  --license-file <path>      License text for non-MIT licenses\n");
    printf("  --git-init                 Initialize a git repository after creation\n\n");
    printf("Feature flags (--with-* / --no-*):\n");
    printf("  installable, subproject, ci, ai-agent, cli\n");
    printf("  Defaults: installable/subproject/ci/cli on, ai-agent off\n\n");
    printf("Exit codes:\n");
    printf("  0  success\n");
    printf("  1  runtime failure\n");
    printf("  2  usage or validation error\n\n");
    printf("JSON output contains project metadata, feature flags, and the actions applied.\n");
    printf("`--version` always reports the wizard version; use `--project-version` for the\n");
    printf("version written into the generated project.\n");
}

static void json_string(const char *value) {
    const unsigned char *p = (const unsigned char *)value;
    if (!p || p[0] == '\0') {
        fputs("\"\"", stdout);
        return;
    }
    putchar('"');
    for (; *p; p++) {
        switch (*p) {
        case '"':
            fputs("\\\"", stdout);
            break;
        case '\\':
            fputs("\\\\", stdout);
            break;
        case '\b':
            fputs("\\b", stdout);
            break;
        case '\f':
            fputs("\\f", stdout);
            break;
        case '\n':
            fputs("\\n", stdout);
            break;
        case '\r':
            fputs("\\r", stdout);
            break;
        case '\t':
            fputs("\\t", stdout);
            break;
        default:
            if (*p < 0x20)
                printf("\\u%04x", *p);
            else
                putchar(*p);
            break;
        }
    }
    putchar('"');
}

static void print_json_error(int code, const char *command, const char *message) {
    printf("{\"ok\":false,\"command\":");
    json_string(command ? command : "");
    printf(",\"error\":{\"exitCode\":%d,\"message\":", code);
    json_string(message ? message : "");
    printf("}}\n");
}

static int fail(const CommandState *state, int code, const char *message) {
    if (state->options.json)
        print_json_error(code, state->options.command, message);
    else
        fprintf(stderr, "ERROR: %s\n", message);
    return code;
}

static int fail_parse(int json, const char *command, int code, const char *message) {
    if (json)
        print_json_error(code, command, message);
    else
        fprintf(stderr, "ERROR: %s\n", message);
    return code;
}

static void print_report_json(const CommandState *state, const char *template_root,
                              const char *destination, const WizardReport *report) {
    const WizardConfig *cfg = &state->cfg;
    printf("{\"ok\":true,\"command\":");
    json_string(state->options.command);
    printf(",\"dryRun\":%s,\"template\":", state->options.dry_run ? "true" : "false");
    json_string(template_root);
    printf(",\"destination\":");
    json_string(destination);
    printf(",\"project\":{\"name\":");
    json_string(cfg->name);
    printf(",\"namespace\":");
    json_string(cfg->lower);
    printf(",\"macro\":");
    json_string(cfg->upper);
    printf(",\"description\":");
    json_string(cfg->description);
    printf(",\"author\":");
    json_string(cfg->author);
    printf(",\"version\":");
    json_string(cfg->version);
    printf(",\"license\":");
    json_string(cfg->license);
    printf("},\"features\":{");
    for (int feature = 0; feature < WIZARD_FEATURE_COUNT; feature++) {
        if (feature > 0)
            putchar(',');
        json_string(wizard_feature_name(feature));
        printf(":%s", cfg->features[feature] ? "true" : "false");
    }
    printf("},\"changes\":{\"renamedFiles\":%d,\"renamedDirs\":%d,\"gitInitialized\":%s,",
           report->renamed_files, report->renamed_dirs, report->git_initialized ? "true" : "false");
    printf("\"removed\":[");
    for (int i = 0; i < report->removed_count; i++) {
        if (i > 0)
            putchar(',');
        json_string(report->removed[i]);
    }
    printf("],\"written\":[");
    for (int i = 0; i < report->written_count; i++) {
        if (i > 0)
            putchar(',');
        json_string(report->written[i]);
    }
    printf("]}}\n");
}

static void print_report_text(const CommandState *state, const char *template_root,
                              const char *destination, const WizardReport *report) {
    const WizardConfig *cfg = &state->cfg;
    WizardLang lang = state->options.lang;
    printf("\n  %s\n", tr(lang, "Project", "项目"));
    printf("  %-12s %s\n", tr(lang, "Name", "名称"), cfg->name);
    printf("  %-12s %s\n", tr(lang, "Namespace", "命名空间"), cfg->lower);
    printf("  %-12s %s\n", tr(lang, "Version", "版本"), cfg->version);
    printf("  %-12s %s\n", tr(lang, "License", "许可证"), cfg->license);
    printf("  %-12s %s\n", tr(lang, "Template", "模板"), template_root);
    printf("  %-12s %s\n", tr(lang, "Destination", "目标"), destination);
    printf("  %s:", tr(lang, "Features", "特性"));
    for (int feature = 0; feature < WIZARD_FEATURE_COUNT; feature++)
        printf(" %s=%s", wizard_feature_name(feature), cfg->features[feature] ? "on" : "off");
    printf("\n");
    if (state->options.dry_run)
        printf("\n  %s\n",
               tr(lang, "Dry run: no destination changes were made.", "试运行：未修改目标目录。"));
    else
        printf("\n  %s\n", tr(lang, "Project created.", "项目已创建。"));
    if (report->renamed_files > 0)
        printf("  %s: %d\n", tr(lang, "Renamed files", "重命名文件"), report->renamed_files);
    for (int i = 0; i < report->removed_count; i++)
        printf("  %s: %s\n", tr(lang, "Removed", "已移除"), report->removed[i]);
    for (int i = 0; i < report->written_count; i++)
        printf("  %s: %s\n", tr(lang, "Wrote", "已写入"), report->written[i]);
    if (report->git_initialized)
        printf("  %s\n", tr(lang, "Initialized git repository", "已初始化 git 仓库"));
}

static char *safe_gets(char *buffer, int size) {
    size_t len;
    if (!fgets(buffer, size, stdin))
        return NULL;
    len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
        buffer[--len] = '\0';
    return buffer;
}

static int ask_raw(WizardLang lang, const char *en, const char *zh, char *buffer, size_t size,
                   const char *default_value) {
    printf("%s", tr(lang, en, zh));
    if (default_value && default_value[0])
        printf("[%s]: ", default_value);
    else
        printf(": ");
    fflush(stdout);
    if (!safe_gets(buffer, (int)size))
        return 0;
    if (buffer[0] == '\0' && default_value)
        snprintf(buffer, size, "%s", default_value);
    return 1;
}

static int ask_yes_no(WizardLang lang, const char *en, const char *zh, int default_yes) {
    char buffer[16];
    for (;;) {
        printf("%s", tr(lang, en, zh));
        fflush(stdout);
        if (!safe_gets(buffer, (int)sizeof(buffer)))
            return default_yes;
        if (buffer[0] == '\0')
            return default_yes;
        if (buffer[0] == 'y' || buffer[0] == 'Y')
            return 1;
        if (buffer[0] == 'n' || buffer[0] == 'N')
            return 0;
    }
}

static const char *feature_label(int feature, WizardLang lang) {
    switch (feature) {
    case WIZARD_FEATURE_INSTALLABLE:
        return tr(lang, "Installable library and packaging", "可安装库与打包");
    case WIZARD_FEATURE_SUBPROJECT:
        return tr(lang, "Subproject consumer support", "子项目消费支持");
    case WIZARD_FEATURE_CI:
        return tr(lang, "GitHub Actions CI", "GitHub Actions CI");
    case WIZARD_FEATURE_AI_AGENT:
        return tr(lang, "AI agent policy (AGENTS.md)", "AI 代理策略 (AGENTS.md)");
    case WIZARD_FEATURE_CLI:
        return tr(lang, "Example CLI stub", "示例 CLI 桩程序");
    default:
        return "";
    }
}

static void print_feature_list(const WizardConfig *cfg, WizardLang lang) {
    for (int feature = 0; feature < WIZARD_FEATURE_COUNT; feature++)
        printf("  %d. [%c] %s\n", feature + 1, cfg->features[feature] ? 'Y' : 'N',
               feature_label(feature, lang));
}

static int edit_features(WizardConfig *cfg, WizardLang lang) {
    for (;;) {
        char buffer[128];
        printf("\n%s\n", tr(lang, "Features:", "可选特性:"));
        print_feature_list(cfg, lang);
        printf("%s", tr(lang, "Numbers to toggle (for example 1 3), Enter to accept: ",
                        "输入编号切换（例如 1 3），回车接受: "));
        fflush(stdout);
        if (!safe_gets(buffer, (int)sizeof(buffer)))
            return 0;
        if (buffer[0] == '\0')
            return 1;
        for (char *p = buffer; *p; p++) {
            if (*p >= '1' && *p <= '0' + WIZARD_FEATURE_COUNT)
                cfg->features[*p - '1'] = !cfg->features[*p - '1'];
        }
    }
}

static void print_review(const WizardConfig *cfg, WizardLang lang, const char *destination) {
    printf("\n%s\n", tr(lang, "Review:", "预览:"));
    printf("  %-12s %s\n", tr(lang, "Name", "名称"), cfg->name);
    printf("  %-12s %s\n", tr(lang, "Namespace", "命名空间"), cfg->lower);
    printf("  %-12s %s\n", tr(lang, "Description", "描述"), cfg->description);
    printf("  %-12s %s\n", tr(lang, "Author", "作者"), cfg->author);
    printf("  %-12s %s\n", tr(lang, "Version", "版本"), cfg->version);
    printf("  %-12s %s\n", tr(lang, "License", "许可证"), cfg->license);
    if (destination)
        printf("  %-12s %s\n", tr(lang, "Destination", "目标"), destination);
    print_feature_list(cfg, lang);
}

static int ask_project_name(WizardConfig *cfg, WizardLang lang) {
    for (;;) {
        char raw[WIZARD_MAX_NAME];
        char error[256];
        if (!ask_raw(lang, "Project name", "项目名称", raw, sizeof(raw), NULL))
            return 0;
        if (raw[0] == '\0')
            return 0;
        if (wizard_normalize_name(raw, cfg->name, sizeof(cfg->name), error, sizeof(error))) {
            wizard_config_derive(cfg);
            return 1;
        }
        printf("ERROR: %s\n", error);
    }
}

static int prompt_metadata(WizardConfig *cfg, WizardLang lang) {
    char buffer[WIZARD_MAX_PATH];
    if (!ask_raw(lang, "Description", "项目描述", cfg->description, sizeof(cfg->description),
                 "A C library"))
        return 0;
    if (!ask_raw(lang, "Author/Vendor", "作者/组织", cfg->author, sizeof(cfg->author), NULL))
        return 0;
    if (!ask_raw(lang, "Version", "版本", cfg->version, sizeof(cfg->version), "0.1.0"))
        return 0;
    if (!ask_raw(lang, "License", "许可证", cfg->license, sizeof(cfg->license), "MIT"))
        return 0;
    if (ascii_case_equal(cfg->license, "MIT"))
        snprintf(cfg->license, sizeof(cfg->license), "MIT");
    if (!ascii_case_equal(cfg->license, "MIT")) {
        buffer[0] = '\0';
        if (!ask_raw(lang, "License file", "许可证文件", buffer, sizeof(buffer), NULL))
            return 0;
        snprintf(cfg->license_file, sizeof(cfg->license_file), "%s", buffer);
        if (!wizard_file_exists(cfg->license_file)) {
            printf("ERROR: %s\n", tr(lang, "License file does not exist.", "许可证文件不存在。"));
            return 0;
        }
    }
    wizard_config_derive(cfg);
    return 1;
}

static int has_file_under(const char *root, const char *relative) {
    char path[WIZARD_MAX_PATH];
    return wizard_join_path(path, sizeof(path), root, relative) == 0 && wizard_file_exists(path);
}

static int is_template_root(const char *root) {
    return has_file_under(root, "CMakeLists.txt") && has_file_under(root, "cmake/Sources.cmake") &&
           has_file_under(root, "include/cproject/example.h");
}

static int actual_executable_path(char *out, size_t size) {
#ifdef _WIN32
    typedef unsigned long path_length_t;
    __declspec(dllimport) path_length_t __stdcall GetModuleFileNameA(void *module, char *filename,
                                                                     path_length_t length);
    path_length_t length = GetModuleFileNameA(NULL, out, (path_length_t)size);
    return (length == 0 || length >= (path_length_t)size) ? -1 : 0;
#elif defined(__APPLE__)
    uint32_t required = (uint32_t)size;
    return _NSGetExecutablePath(out, &required) == 0 ? 0 : -1;
#else
    ssize_t length = readlink("/proc/self/exe", out, size - 1);
    if (length <= 0 || (size_t)length >= size)
        return -1;
    out[length] = '\0';
    return 0;
#endif
}

static char *executable_dir(const char *argv0) {
    const char *slash = strrchr(argv0, '/');
    size_t length;
    char *result;
#ifdef _WIN32
    const char *backslash = strrchr(argv0, '\\');
    if (backslash && (!slash || backslash > slash))
        slash = backslash;
#endif
    if (!slash || slash == argv0)
        return NULL;
    length = (size_t)(slash - argv0);
    result = (char *)malloc(length + 1);
    if (!result)
        return NULL;
    memcpy(result, argv0, length);
    result[length] = '\0';
    return result;
}

static int find_upward_template_root(const char *start, char *out, size_t size) {
    char directory[WIZARD_MAX_PATH];
    snprintf(directory, sizeof(directory), "%s", start);
    for (int depth = 0; depth < 12; depth++) {
        char *slash;
        if (is_template_root(directory)) {
            snprintf(out, size, "%s", directory);
            return 1;
        }
        slash = strrchr(directory, '/');
#ifdef _WIN32
        {
            char *backslash = strrchr(directory, '\\');
            if (backslash && (!slash || backslash > slash))
                slash = backslash;
        }
#endif
        if (!slash)
            break;
        *slash = '\0';
        if (directory[0] == '\0')
            break;
    }
    return 0;
}

static int resolve_template_root(const char *argv0, const char *explicit_root, char *out,
                                 size_t size) {
    if (explicit_root && explicit_root[0]) {
        if (!is_template_root(explicit_root))
            return 0;
        snprintf(out, size, "%s", explicit_root);
        return 1;
    }
    {
        const char *environment = getenv("CPROJECT_TEMPLATE_ROOT");
        if (environment && environment[0] && is_template_root(environment)) {
            snprintf(out, size, "%s", environment);
            return 1;
        }
    }
    {
        char actual_path[WIZARD_MAX_PATH];
        char *directory = NULL;
        if (actual_executable_path(actual_path, sizeof(actual_path)) == 0)
            directory = executable_dir(actual_path);
        if (!directory)
            directory = executable_dir(argv0);
        if (directory) {
            char candidate[WIZARD_MAX_PATH];
            if (wizard_join_path(candidate, sizeof(candidate), directory,
                                 "../share/c-project-standard") == 0 &&
                is_template_root(candidate)) {
                snprintf(out, size, "%s", candidate);
                free(directory);
                return 1;
            }
            if (wizard_join_path(candidate, sizeof(candidate), directory,
                                 "../installer-staging/share/c-project-standard") == 0 &&
                is_template_root(candidate)) {
                snprintf(out, size, "%s", candidate);
                free(directory);
                return 1;
            }
            if (find_upward_template_root(directory, out, size)) {
                free(directory);
                return 1;
            }
            free(directory);
        }
    }
    if (is_template_root(".")) {
        snprintf(out, size, ".");
        return 1;
    }
    return 0;
}

static int default_output_parent(int interactive, char *out, size_t size) {
    const char *environment = getenv("CPROJECT_DEFAULT_OUTPUT");
    if (environment && environment[0]) {
        snprintf(out, size, "%s", environment);
        return 0;
    }
    if (interactive) {
        const char *home = getenv("USERPROFILE");
        char desktop[WIZARD_MAX_PATH];
        if (!home || !home[0])
            home = getenv("HOME");
        if (home && home[0] && wizard_join_path(desktop, sizeof(desktop), home, "Desktop") == 0 &&
            wizard_dir_exists(desktop)) {
            snprintf(out, size, "%s", desktop);
            return 0;
        }
    }
    if (current_directory(out, size) == 0)
        return 0;
    snprintf(out, size, ".");
    return 0;
}

static int check_destination(const char *destination, char *err, size_t err_size) {
    if (!wizard_file_exists(destination) && !wizard_dir_exists(destination))
        return 0;
    if (!wizard_dir_exists(destination)) {
        snprintf(err, err_size, "输出路径已存在且不是目录: %s", destination);
        return -1;
    }
    if (wizard_dir_is_empty(destination) != 1) {
        snprintf(err, err_size, "输出目录非空: %s", destination);
        return -1;
    }
    return 0;
}

static int parse_options(int argc, char **argv, int first, int command_is_init, CommandState *state,
                         int *in_place, char *err, size_t err_size) {
    for (int i = first; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            print_help();
            return 1;
        }
        if (strcmp(arg, "--version") == 0) {
            snprintf(err, err_size, "--version reports the tool version; use --project-version.");
            return -1;
        }
        if (strcmp(arg, "--dry-run") == 0) {
            state->options.dry_run = 1;
            continue;
        }
        if (strcmp(arg, "--json") == 0) {
            state->options.json = 1;
            state->options.yes = 1;
            continue;
        }
        if (strcmp(arg, "--yes") == 0 || strcmp(arg, "-y") == 0) {
            state->options.yes = 1;
            continue;
        }
        if (strcmp(arg, "--git-init") == 0) {
            state->options.git_init = 1;
            continue;
        }
        if (strcmp(arg, "--in-place") == 0) {
            if (!command_is_init) {
                snprintf(err, err_size, "--in-place is only valid with the init command.");
                return -1;
            }
            *in_place = 1;
            continue;
        }
        if (strncmp(arg, "--with-", 7) == 0 || strncmp(arg, "--no-", 5) == 0) {
            int feature = -1;
            int enabled = 0;
            if (!wizard_parse_feature_option(arg, &feature, &enabled)) {
                snprintf(err, err_size, "unknown feature flag: %s", arg);
                return -1;
            }
            state->cfg.features[feature] = enabled;
            continue;
        }
        if (strcmp(arg, "--lang") == 0) {
            const char *value;
            if (i + 1 >= argc) {
                snprintf(err, err_size, "--lang requires a value.");
                return -1;
            }
            value = argv[++i];
            if (strcmp(value, "en") == 0)
                state->options.lang = WIZARD_LANG_EN;
            else if (strcmp(value, "zh") == 0)
                state->options.lang = WIZARD_LANG_ZH;
            else {
                snprintf(err, err_size, "--lang must be en or zh.");
                return -1;
            }
            state->options.lang_explicit = 1;
            continue;
        }
        if (strcmp(arg, "--output") == 0 || strcmp(arg, "--template-root") == 0 ||
            strcmp(arg, "--description") == 0 || strcmp(arg, "--author") == 0 ||
            strcmp(arg, "--project-version") == 0 || strcmp(arg, "--license") == 0 ||
            strcmp(arg, "--license-file") == 0) {
            const char *value;
            if (i + 1 >= argc) {
                snprintf(err, err_size, "%s requires a value.", arg);
                return -1;
            }
            value = argv[++i];
            if (strcmp(arg, "--output") == 0) {
                if (command_is_init) {
                    snprintf(err, err_size, "--output is not valid with init --in-place.");
                    return -1;
                }
                snprintf(state->options.output, sizeof(state->options.output), "%s", value);
            } else if (strcmp(arg, "--template-root") == 0) {
                if (command_is_init) {
                    snprintf(err, err_size, "--template-root is not valid with init --in-place.");
                    return -1;
                }
                snprintf(state->options.template_root, sizeof(state->options.template_root), "%s",
                         value);
            } else if (strcmp(arg, "--description") == 0) {
                snprintf(state->cfg.description, sizeof(state->cfg.description), "%s", value);
            } else if (strcmp(arg, "--author") == 0) {
                snprintf(state->cfg.author, sizeof(state->cfg.author), "%s", value);
            } else if (strcmp(arg, "--project-version") == 0) {
                snprintf(state->cfg.version, sizeof(state->cfg.version), "%s", value);
            } else if (strcmp(arg, "--license") == 0) {
                snprintf(state->cfg.license, sizeof(state->cfg.license), "%s", value);
            } else {
                snprintf(state->cfg.license_file, sizeof(state->cfg.license_file), "%s", value);
            }
            continue;
        }
        if (strncmp(arg, "--", 2) == 0) {
            snprintf(err, err_size, "unknown option: %s", arg);
            return -1;
        }
        if (state->cfg.name[0] != '\0') {
            snprintf(err, err_size, "unexpected argument: %s", arg);
            return -1;
        }
        snprintf(state->cfg.name, sizeof(state->cfg.name), "%s", arg);
    }
    return 0;
}

static int validate_config(CommandState *state, char *err, size_t err_size) {
    char normalized[WIZARD_MAX_NAME];
    char normalize_error[256];
    if (!wizard_normalize_name(state->cfg.name, normalized, sizeof(normalized), normalize_error,
                               sizeof(normalize_error))) {
        snprintf(err, err_size, "%s", normalize_error);
        return -1;
    }
    snprintf(state->cfg.name, sizeof(state->cfg.name), "%s", normalized);
    wizard_config_derive(&state->cfg);
    if (!wizard_validate_version(state->cfg.version)) {
        snprintf(err, err_size, "版本必须使用 x.y.z 格式: %s", state->cfg.version);
        return -1;
    }
    if (ascii_case_equal(state->cfg.license, "MIT"))
        snprintf(state->cfg.license, sizeof(state->cfg.license), "MIT");
    if (!ascii_case_equal(state->cfg.license, "MIT")) {
        if (state->cfg.license_file[0] == '\0' || !wizard_file_exists(state->cfg.license_file)) {
            snprintf(err, err_size, "非 MIT 许可证需要可读取的 --license-file。");
            return -1;
        }
    }
    return 0;
}

static int interactive_config(CommandState *state, int is_init, char *destination,
                              size_t destination_size) {
    WizardConfig *cfg = &state->cfg;
    WizardLang lang = state->options.lang;
    char parent[WIZARD_MAX_PATH];
    if (!state->options.lang_explicit && stdin_is_tty()) {
        char buffer[16];
        printf("%s", tr(lang, "Language [EN/zh]: ", "语言 [EN/zh]: "));
        fflush(stdout);
        if (safe_gets(buffer, (int)sizeof(buffer)) && (buffer[0] == 'z' || buffer[0] == 'Z'))
            state->options.lang = WIZARD_LANG_ZH;
        lang = state->options.lang;
    }
    print_banner(lang);
    if (cfg->name[0] == '\0' && !ask_project_name(cfg, lang))
        return 0;
    if (!is_init) {
        if (state->options.output[0] != '\0')
            snprintf(parent, sizeof(parent), "%s", state->options.output);
        else
            default_output_parent(1, parent, sizeof(parent));
        {
            char suggested[WIZARD_MAX_PATH];
            snprintf(suggested, sizeof(suggested), "%s", parent);
            if (!ask_raw(lang, "Output directory", "输出目录", parent, sizeof(parent), suggested))
                return 0;
        }
        snprintf(state->options.output, sizeof(state->options.output), "%s", parent);
        if (wizard_join_path(destination, destination_size, parent, cfg->name) != 0)
            return 0;
    } else {
        if (current_directory(destination, destination_size) != 0)
            return 0;
        printf("\n%s\n", tr(lang, "WARNING: init --in-place modifies the current template tree.",
                            "警告：init --in-place 会修改当前模板目录。"));
    }
    if (!prompt_metadata(cfg, lang))
        return 0;
    if (!edit_features(cfg, lang))
        return 0;
    print_review(cfg, lang, destination);
    if (!ask_yes_no(lang, "Proceed? [Y/n]: ", "确认执行? [Y/n]: ", 1))
        return 0;
    return 1;
}

static int copy_and_setup(CommandState *state, const char *template_root, const char *destination,
                          char *err, size_t err_size) {
    WizardManifest manifest;
    WizardReport report;
    char working_root[WIZARD_MAX_PATH];
    char temp_root[WIZARD_MAX_PATH] = "";
    int destination_existed = wizard_dir_exists(destination);
    memset(&report, 0, sizeof(report));
    if (wizard_manifest_load(template_root, &manifest) != 0) {
        snprintf(err, err_size, "无法读取模板排除清单。");
        return WIZARD_EXIT_RUNTIME;
    }
    if (state->options.dry_run) {
        if (wizard_make_temp_dir(temp_root, sizeof(temp_root), "dry-run") != 0) {
            wizard_manifest_free(&manifest);
            snprintf(err, err_size, "无法创建试运行临时目录。");
            return WIZARD_EXIT_RUNTIME;
        }
        if (wizard_join_path(working_root, sizeof(working_root), temp_root, "Project") != 0 ||
            wizard_copy_tree(template_root, working_root, &manifest) != 0) {
            wizard_remove_tree(temp_root);
            wizard_manifest_free(&manifest);
            snprintf(err, err_size, "复制模板到试运行目录失败。");
            return WIZARD_EXIT_RUNTIME;
        }
        if (wizard_setup_project(working_root, &state->cfg, &manifest, 0, &report, err, err_size) !=
            0) {
            wizard_remove_tree(temp_root);
            wizard_manifest_free(&manifest);
            return WIZARD_EXIT_RUNTIME;
        }
        report.git_initialized = state->options.git_init;
        if (state->options.json)
            print_report_json(state, template_root, destination, &report);
        else
            print_report_text(state, template_root, destination, &report);
        wizard_remove_tree(temp_root);
        wizard_manifest_free(&manifest);
        return WIZARD_EXIT_OK;
    }
    if (!state->options.in_place && wizard_copy_tree(template_root, destination, &manifest) != 0) {
        wizard_manifest_free(&manifest);
        snprintf(err, err_size, "复制模板到目标目录失败: %s", destination);
        return WIZARD_EXIT_RUNTIME;
    }
    if (wizard_setup_project(destination, &state->cfg, &manifest, state->options.git_init, &report,
                             err, err_size) != 0) {
        if (!destination_existed)
            wizard_remove_tree(destination);
        wizard_manifest_free(&manifest);
        return WIZARD_EXIT_RUNTIME;
    }
    if (state->options.json)
        print_report_json(state, template_root, destination, &report);
    else
        print_report_text(state, template_root, destination, &report);
    wizard_manifest_free(&manifest);
    return WIZARD_EXIT_OK;
}

int wizard_cli_main(int argc, char **argv) {
    CommandState state;
    char error[512] = "";
    char template_root[WIZARD_MAX_PATH] = "";
    char destination[WIZARD_MAX_PATH] = "";
    int first = 1;
    int command_is_init = 0;
    int in_place = 0;
    int parse_result;
    int json_requested = 0;
    int interactive;
    const char *command_name = "new";
    const char *argv0 = argc > 0 ? argv[0] : "cproject";

    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        print_help();
        return WIZARD_EXIT_OK;
    }
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        print_version();
        return WIZARD_EXIT_OK;
    }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--json") == 0) {
            json_requested = 1;
            break;
        }
    }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0 && i > 1) {
            return fail_parse(json_requested, command_name, WIZARD_EXIT_USAGE,
                              "--version reports the tool version; use --project-version.");
        }
    }
    if (argc >= 2 && strcmp(argv[1], "init") == 0) {
        command_is_init = 1;
        command_name = "init";
        first = 2;
    } else if (argc >= 2 && strcmp(argv[1], "new") == 0) {
        first = 2;
    }
    memset(&state, 0, sizeof(state));
    wizard_config_defaults(&state.cfg);
    state.options.lang = (WizardLang)detect_language();
    snprintf(state.options.command, sizeof(state.options.command), "%s", command_name);
    state.options.in_place = command_is_init;
    parse_result =
        parse_options(argc, argv, first, command_is_init, &state, &in_place, error, sizeof(error));
    if (parse_result > 0)
        return WIZARD_EXIT_OK;
    if (parse_result < 0)
        return fail_parse(json_requested, command_name, WIZARD_EXIT_USAGE, error);
    if (command_is_init && !in_place)
        return fail_parse(json_requested, command_name, WIZARD_EXIT_USAGE,
                          "init 需要 --in-place；它会修改当前模板目录。");
    if (state.options.json)
        state.options.yes = 1;
    state.options.non_interactive =
        state.options.yes || state.options.json || state.options.dry_run || !stdin_is_tty();
    interactive = !state.options.non_interactive;

    if (!interactive && state.cfg.name[0] == '\0')
        return fail(&state, WIZARD_EXIT_USAGE, "缺少项目名称。");

    if (command_is_init) {
        if (current_directory(destination, sizeof(destination)) != 0 ||
            !is_template_root(destination))
            return fail(&state, WIZARD_EXIT_RUNTIME, "当前目录不是模板根目录。");
        if (!state.options.dry_run && !state.options.yes && !interactive)
            return fail(&state, WIZARD_EXIT_USAGE,
                        "init --in-place requires --yes when stdin is not interactive.");
    } else if (!resolve_template_root(argv0, state.options.template_root, template_root,
                                      sizeof(template_root))) {
        return fail(
            &state, WIZARD_EXIT_RUNTIME,
            "找不到模板根目录。请使用 --template-root <dir>（Windows 路径含空格时请加引号）。");
    }

    if (interactive) {
        if (!interactive_config(&state, command_is_init, destination, sizeof(destination)))
            return fail(&state, WIZARD_EXIT_RUNTIME, "操作已取消。");
    }

    if (validate_config(&state, error, sizeof(error)) != 0)
        return fail(&state, WIZARD_EXIT_USAGE, error);

    if (command_is_init) {
        if (current_directory(destination, sizeof(destination)) != 0)
            return fail(&state, WIZARD_EXIT_RUNTIME, "无法读取当前目录。");
        snprintf(template_root, sizeof(template_root), "%s", destination);
    } else {
        if (state.options.output[0] == '\0')
            default_output_parent(interactive, state.options.output, sizeof(state.options.output));
        if (wizard_join_path(destination, sizeof(destination), state.options.output,
                             state.cfg.name) != 0)
            return fail(&state, WIZARD_EXIT_USAGE, "输出路径过长。");
        if (check_destination(destination, error, sizeof(error)) != 0)
            return fail(&state, WIZARD_EXIT_RUNTIME, error);
    }

    if (copy_and_setup(&state, template_root, destination, error, sizeof(error)) != WIZARD_EXIT_OK)
        return fail(&state, WIZARD_EXIT_RUNTIME, error[0] ? error : "生成项目失败。");
    return WIZARD_EXIT_OK;
}
