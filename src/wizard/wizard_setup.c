#include "wizard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} TextBuffer;

typedef struct {
    const char *from;
    const char *to;
} Replacement;

static void set_error(char *err, size_t err_size, const char *message) {
    if (err && err_size > 0)
        snprintf(err, err_size, "%s", message);
}

static int buffer_reserve(TextBuffer *buffer, size_t extra) {
    size_t needed = buffer->len + extra + 1;
    char *data;
    if (buffer->data != NULL && needed <= buffer->cap)
        return 0;
    {
        size_t cap = buffer->cap == 0 ? 256 : buffer->cap;
        while (cap < needed)
            cap *= 2;
        data = (char *)realloc(buffer->data, cap);
        if (!data)
            return -1;
        buffer->data = data;
        buffer->cap = cap;
    }
    return 0;
}

static int buffer_append_n(TextBuffer *buffer, const char *text, size_t len) {
    if (buffer_reserve(buffer, len) != 0)
        return -1;
    if (len > 0) {
        // NOLINTNEXTLINE(clang-analyzer-unix.cstring.NullArg)
        memcpy(buffer->data + buffer->len, text, len);
    }
    buffer->len += len;
    buffer->data[buffer->len] = '\0';
    return 0;
}

static int buffer_append(TextBuffer *buffer, const char *text) {
    return buffer_append_n(buffer, text, strlen(text));
}

static void buffer_free(TextBuffer *buffer) {
    free(buffer->data);
    buffer->data = NULL;
    buffer->len = 0;
    buffer->cap = 0;
}

static char *duplicate_text(const char *text) {
    size_t len = strlen(text);
    char *copy = (char *)malloc(len + 1);
    if (copy)
        memcpy(copy, text, len + 1);
    return copy;
}

static char *replace_all(const char *source, const char *from, const char *to) {
    size_t source_len;
    size_t from_len;
    size_t to_len;
    size_t count = 0;
    size_t result_len;
    const char *cursor;
    char *result;
    char *write;

    if (!source || !from || !to)
        return NULL;
    source_len = strlen(source);
    from_len = strlen(from);
    to_len = strlen(to);
    if (from_len == 0)
        return duplicate_text(source);
    cursor = source;
    while ((cursor = strstr(cursor, from)) != NULL) {
        count++;
        cursor += from_len;
    }
    if (count == 0)
        return duplicate_text(source);
    result_len = source_len;
    if (to_len >= from_len)
        result_len += count * (to_len - from_len);
    else
        result_len -= count * (from_len - to_len);
    result = (char *)malloc(result_len + 1);
    if (!result)
        return NULL;
    cursor = source;
    write = result;
    for (;;) {
        const char *match = strstr(cursor, from);
        size_t chunk;
        if (!match) {
            size_t tail = strlen(cursor) + 1;
            memcpy(write, cursor, tail);
            break;
        }
        chunk = (size_t)(match - cursor);
        if (chunk > 0)
            memcpy(write, cursor, chunk);
        write += chunk;
        if (to_len > 0)
            memcpy(write, to, to_len);
        write += to_len;
        cursor = match + from_len;
    }
    return result;
}

static char *replace_pairs(const char *source, const Replacement *replacements, size_t count) {
    char *current = duplicate_text(source);
    if (!current)
        return NULL;
    for (size_t i = 0; i < count; i++) {
        char *next = replace_all(current, replacements[i].from, replacements[i].to);
        free(current);
        if (!next)
            return NULL;
        current = next;
    }
    return current;
}

static int has_any_identifier(const char *text) {
    return strstr(text, "CProjectStandard") || strstr(text, "cproject") ||
           strstr(text, "CPROJECT") || strstr(text, "CProject") || strstr(text, "Cproject");
}

static int is_text_name(const char *name) {
    static const char *const extensions[] = {".c",   ".h",    ".cpp", ".cmake", ".txt", ".json",
                                             ".yml", ".yaml", ".md",  ".in",    ".ps1", ".sh",
                                             ".bat", ".wxs",  ".py",  ".xml",   ".rc",  NULL};
    const char *dot = strrchr(name, '.');
    if (!dot)
        return 0;
    for (int i = 0; extensions[i]; i++) {
        if (strcmp(dot, extensions[i]) == 0)
            return 1;
    }
    return 0;
}

static int should_skip_walk_name(const char *name) {
    return strcmp(name, "build") == 0 || strcmp(name, ".git") == 0 || strcmp(name, ".vs") == 0 ||
           strcmp(name, ".cache") == 0 || strcmp(name, ".hermes") == 0 ||
           strcmp(name, "dist") == 0 || strcmp(name, "out") == 0 ||
           strcmp(name, "_CPack_Packages") == 0 || strcmp(name, "Testing") == 0 ||
           strcmp(name, "__pycache__") == 0 || strcmp(name, ".wix") == 0 ||
           strcmp(name, "install") == 0 || strncmp(name, "install-", 8) == 0 ||
           strcmp(name, "node_modules") == 0 || strcmp(name, "Okk") == 0 ||
           strcmp(name, "TestProj") == 0 || strcmp(name, ".static-analysis-files.txt") == 0 ||
           (strlen(name) >= 19 && strcmp(name + strlen(name) - 19, "-cppcheck-build-dir") == 0);
}

static int line_bounds(const char *text, size_t offset, size_t *start, size_t *end) {
    const char *line_start = text;
    const char *newline;
    for (const char *p = text; *p && (size_t)(p - text) < offset; p++) {
        if (*p == '\n')
            line_start = p + 1;
    }
    newline = strchr(text + offset, '\n');
    *start = (size_t)(line_start - text);
    *end = newline ? (size_t)(newline - text) + 1 : strlen(text);
    return 0;
}

static int find_marker_line(const char *text, const char *token, size_t from, size_t *start,
                            size_t *end) {
    const char *found = strstr(text + from, token);
    if (!found)
        return 0;
    line_bounds(text, (size_t)(found - text), start, end);
    return 1;
}

static int replace_block(const char *path, const char *begin_token, const char *end_token,
                         const char *replacement, int remove) {
    char *text = NULL;
    size_t begin_start = 0;
    size_t begin_end = 0;
    size_t end_start = 0;
    size_t end_end = 0;
    TextBuffer output = {0};
    int result = -1;

    if (wizard_read_text(path, &text, NULL) != 0)
        return -1;
    if (!find_marker_line(text, begin_token, 0, &begin_start, &begin_end)) {
        result = 0;
        goto done;
    }
    if (!find_marker_line(text, end_token, begin_end, &end_start, &end_end))
        goto done;
    if (buffer_append_n(&output, text, begin_start) != 0)
        goto done;
    if (!remove && replacement && buffer_append(&output, replacement) != 0)
        goto done;
    if (buffer_append(&output, text + end_end) != 0)
        goto done;
    if (wizard_write_bytes(path, output.data, output.len) != 0)
        goto done;
    result = 1;
done:
    buffer_free(&output);
    free(text);
    return result;
}

static int process_block(const char *path, const char *begin_token, const char *end_token,
                         int keep) {
    char *text = NULL;
    size_t begin_start = 0;
    size_t begin_end = 0;
    size_t end_start = 0;
    size_t end_end = 0;
    TextBuffer output = {0};
    int result = -1;

    if (wizard_read_text(path, &text, NULL) != 0)
        return -1;
    if (!find_marker_line(text, begin_token, 0, &begin_start, &begin_end)) {
        free(text);
        return 0;
    }
    if (!find_marker_line(text, end_token, begin_end, &end_start, &end_end)) {
        free(text);
        return -1;
    }
    if (buffer_append_n(&output, text, begin_start) != 0)
        goto done;
    if (keep && buffer_append_n(&output, text + begin_end, end_start - begin_end) != 0)
        goto done;
    if (buffer_append(&output, text + end_end) != 0)
        goto done;
    if (wizard_write_bytes(path, output.data, output.len) != 0)
        goto done;
    result = 1;
done:
    buffer_free(&output);
    free(text);
    return result;
}

static int join_relative(char *out, size_t size, const char *directory, const char *name) {
    if (!directory || !directory[0])
        return snprintf(out, size, "%s", name) >= (int)size ? -1 : 0;
    if (snprintf(out, size, "%s/%s", directory, name) >= (int)size)
        return -1;
    return 0;
}

typedef int (*TextVisitor)(const char *path, const char *relative, void *context);

static int visit_text_tree(const char *root, const char *relative, TextVisitor visitor,
                           void *context);

static int visit_text_tree_impl(const char *root, const char *relative, TextVisitor visitor,
                                void *context) {
    char directory[WIZARD_MAX_PATH];
    if (!relative || !relative[0]) {
        snprintf(directory, sizeof(directory), "%s", root);
    } else if (wizard_join_path(directory, sizeof(directory), root, relative) != 0) {
        return -1;
    }
#ifdef _WIN32
    {
        char pattern[WIZARD_MAX_PATH];
        struct _finddata_t entry;
        intptr_t handle;
        if (wizard_join_path(pattern, sizeof(pattern), directory, "*") != 0)
            return -1;
        handle = _findfirst(pattern, &entry);
        if (handle == -1)
            return -1;
        do {
            char child_rel[WIZARD_MAX_PATH];
            char child_path[WIZARD_MAX_PATH];
            if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0)
                continue;
            if (should_skip_walk_name(entry.name))
                continue;
            if (join_relative(child_rel, sizeof(child_rel), relative, entry.name) != 0)
                goto fail;
            if (wizard_join_path(child_path, sizeof(child_path), root, child_rel) != 0)
                goto fail;
            if (entry.attrib & _A_SUBDIR) {
                if (visit_text_tree_impl(root, child_rel, visitor, context) != 0)
                    goto fail;
            } else if (is_text_name(entry.name)) {
                int visited = visitor(child_path, child_rel, context);
                if (visited < 0)
                    goto fail;
            }
        } while (_findnext(handle, &entry) == 0);
        _findclose(handle);
        return 0;
    fail:
        _findclose(handle);
        return -1;
    }
#else
    {
        DIR *dir = opendir(directory);
        struct dirent *entry;
        if (!dir)
            return -1;
        while ((entry = readdir(dir)) != NULL) {
            char child_rel[WIZARD_MAX_PATH];
            char child_path[WIZARD_MAX_PATH];
            struct stat st;
            int visited;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            if (should_skip_walk_name(entry->d_name))
                continue;
            if (join_relative(child_rel, sizeof(child_rel), relative, entry->d_name) != 0 ||
                wizard_join_path(child_path, sizeof(child_path), root, child_rel) != 0) {
                closedir(dir);
                return -1;
            }
            if (lstat(child_path, &st) != 0 || S_ISLNK(st.st_mode))
                continue;
            if (S_ISDIR(st.st_mode)) {
                if (visit_text_tree_impl(root, child_rel, visitor, context) != 0) {
                    closedir(dir);
                    return -1;
                }
            } else if (S_ISREG(st.st_mode) && is_text_name(entry->d_name)) {
                visited = visitor(child_path, child_rel, context);
                if (visited < 0) {
                    closedir(dir);
                    return -1;
                }
            }
        }
        closedir(dir);
        return 0;
    }
#endif
}

static int visit_text_tree(const char *root, const char *relative, TextVisitor visitor,
                           void *context) {
    return visit_text_tree_impl(root, relative ? relative : "", visitor, context);
}

typedef struct {
    const WizardConfig *cfg;
    WizardReport *report;
} RenameContext;

static int replace_file_content(const char *path, const WizardConfig *cfg, WizardReport *report) {
    char *text = NULL;
    char *next;
    Replacement replacements[5];
    replacements[0].from = "CProjectStandard";
    replacements[0].to = cfg->name;
    replacements[1].from = "CPROJECT";
    replacements[1].to = cfg->upper;
    replacements[2].from = "cproject";
    replacements[2].to = cfg->lower;
    replacements[3].from = "CProject";
    replacements[3].to = cfg->name;
    replacements[4].from = "Cproject";
    replacements[4].to = cfg->name;
    if (wizard_read_text(path, &text, NULL) != 0)
        return -1;
    if (!has_any_identifier(text)) {
        free(text);
        return 0;
    }
    next = replace_pairs(text, replacements, 5);
    free(text);
    if (!next)
        return -1;
    if (wizard_write_text(path, next) != 0) {
        free(next);
        return -1;
    }
    free(next);
    report->renamed_files++;
    return 1;
}

static int rename_visitor(const char *path, const char *relative, void *context) {
    RenameContext *rename_context = (RenameContext *)context;
    (void)relative;
    return replace_file_content(path, rename_context->cfg, rename_context->report);
}

static int rename_identifiers(const char *root, const WizardConfig *cfg, WizardReport *report,
                              char *err, size_t err_size) {
    RenameContext context;
    context.cfg = cfg;
    context.report = report;
    if (visit_text_tree(root, "", rename_visitor, &context) != 0) {
        set_error(err, err_size, "替换项目标识符失败。");
        return -1;
    }
    return 0;
}

static int replace_file_name(const char *directory, const char *name, const WizardConfig *cfg,
                             char *out, size_t out_size) {
    Replacement replacements[5];
    char *next;
    (void)directory;
    replacements[0].from = "CProjectStandard";
    replacements[0].to = cfg->name;
    replacements[1].from = "CPROJECT";
    replacements[1].to = cfg->upper;
    replacements[2].from = "cproject";
    replacements[2].to = cfg->lower;
    replacements[3].from = "CProject";
    replacements[3].to = cfg->name;
    replacements[4].from = "Cproject";
    replacements[4].to = cfg->name;
    next = replace_pairs(name, replacements, 5);
    if (!next)
        return -1;
    if (strcmp(next, name) == 0) {
        free(next);
        return 0;
    }
    {
        size_t length = strlen(next);
        if (length >= out_size) {
            free(next);
            return -1;
        }
        memcpy(out, next, length + 1);
    }
    free(next);
    return 1;
}

static int rename_file_tree(const char *root, const char *relative, const WizardConfig *cfg,
                            WizardReport *report, char *err, size_t err_size) {
    char directory[WIZARD_MAX_PATH];
    if (!relative || !relative[0])
        snprintf(directory, sizeof(directory), "%s", root);
    else if (wizard_join_path(directory, sizeof(directory), root, relative) != 0)
        return -1;
#ifdef _WIN32
    {
        char pattern[WIZARD_MAX_PATH];
        struct _finddata_t entry;
        intptr_t handle;
        if (wizard_join_path(pattern, sizeof(pattern), directory, "*") != 0)
            return -1;
        handle = _findfirst(pattern, &entry);
        if (handle == -1)
            return -1;
        do {
            char child_rel[WIZARD_MAX_PATH];
            char child_path[WIZARD_MAX_PATH];
            char new_name[WIZARD_MAX_PATH];
            char new_path[WIZARD_MAX_PATH];
            int renamed;
            if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0 ||
                should_skip_walk_name(entry.name))
                continue;
            if (join_relative(child_rel, sizeof(child_rel), relative, entry.name) != 0 ||
                wizard_join_path(child_path, sizeof(child_path), root, child_rel) != 0)
                goto fail;
            if (entry.attrib & _A_SUBDIR) {
                if (rename_file_tree(root, child_rel, cfg, report, err, err_size) != 0)
                    goto fail;
                continue;
            }
            renamed = replace_file_name(directory, entry.name, cfg, new_name, sizeof(new_name));
            if (renamed < 0)
                goto fail;
            if (renamed == 0)
                continue;
            if (wizard_join_path(new_path, sizeof(new_path), directory, new_name) != 0)
                goto fail;
            if (wizard_file_exists(new_path) || wizard_dir_exists(new_path)) {
                char message[WIZARD_MAX_PATH + 64];
                snprintf(message, sizeof(message), "目标文件已存在，无法重命名: %s", new_name);
                set_error(err, err_size, message);
                goto fail;
            }
            if (rename(child_path, new_path) != 0)
                goto fail;
            report->renamed_files++;
        } while (_findnext(handle, &entry) == 0);
        _findclose(handle);
        return 0;
    fail:
        _findclose(handle);
        return -1;
    }
#else
    {
        DIR *dir = opendir(directory);
        struct dirent *entry;
        if (!dir)
            return -1;
        while ((entry = readdir(dir)) != NULL) {
            char child_rel[WIZARD_MAX_PATH];
            char child_path[WIZARD_MAX_PATH];
            char new_name[WIZARD_MAX_PATH];
            char new_path[WIZARD_MAX_PATH];
            struct stat st;
            int renamed;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 ||
                should_skip_walk_name(entry->d_name))
                continue;
            if (join_relative(child_rel, sizeof(child_rel), relative, entry->d_name) != 0 ||
                wizard_join_path(child_path, sizeof(child_path), root, child_rel) != 0) {
                closedir(dir);
                return -1;
            }
            if (lstat(child_path, &st) != 0 || S_ISLNK(st.st_mode))
                continue;
            if (S_ISDIR(st.st_mode)) {
                if (rename_file_tree(root, child_rel, cfg, report, err, err_size) != 0) {
                    closedir(dir);
                    return -1;
                }
                continue;
            }
            renamed = replace_file_name(directory, entry->d_name, cfg, new_name, sizeof(new_name));
            if (renamed < 0) {
                closedir(dir);
                return -1;
            }
            if (renamed == 0)
                continue;
            if (wizard_join_path(new_path, sizeof(new_path), directory, new_name) != 0) {
                closedir(dir);
                return -1;
            }
            if (wizard_file_exists(new_path) || wizard_dir_exists(new_path)) {
                char message[WIZARD_MAX_PATH + 64];
                snprintf(message, sizeof(message), "目标文件已存在，无法重命名: %s", new_name);
                set_error(err, err_size, message);
                closedir(dir);
                return -1;
            }
            if (rename(child_path, new_path) != 0) {
                closedir(dir);
                return -1;
            }
            report->renamed_files++;
        }
        closedir(dir);
        return 0;
    }
#endif
}

static int rename_known_directories(const char *root, const WizardConfig *cfg, WizardReport *report,
                                    char *err, size_t err_size) {
    static const char *const relative_directories[] = {"include/cproject", "src/cproject", NULL};
    for (int i = 0; relative_directories[i]; i++) {
        char old_path[WIZARD_MAX_PATH];
        char new_relative[WIZARD_MAX_PATH];
        char new_path[WIZARD_MAX_PATH];
        const char *slash;
        if (strcmp(cfg->lower, "cproject") == 0)
            break;
        if (wizard_join_path(old_path, sizeof(old_path), root, relative_directories[i]) != 0)
            return -1;
        if (!wizard_dir_exists(old_path))
            continue;
        slash = strrchr(relative_directories[i], '/');
        snprintf(new_relative, sizeof(new_relative), "%.*s%s",
                 (int)(slash - relative_directories[i]) + 1, relative_directories[i], cfg->lower);
        if (wizard_join_path(new_path, sizeof(new_path), root, new_relative) != 0)
            return -1;
        if (wizard_file_exists(new_path) || wizard_dir_exists(new_path)) {
            set_error(err, err_size, "目标目录已存在，无法重命名。");
            return -1;
        }
        if (rename(old_path, new_path) != 0) {
            set_error(err, err_size, "重命名项目目录失败。");
            return -1;
        }
        report->renamed_dirs++;
    }
    return 0;
}

static const char *feature_marker_name(int feature) {
    switch (feature) {
    case WIZARD_FEATURE_INSTALLABLE:
        return "INSTALLABLE";
    case WIZARD_FEATURE_SUBPROJECT:
        return "SUBPROJECT";
    case WIZARD_FEATURE_CI:
        return "CI";
    case WIZARD_FEATURE_AI_AGENT:
        return "AI_AGENT";
    case WIZARD_FEATURE_CLI:
        return "CLI";
    default:
        return "";
    }
}

typedef struct {
    const WizardConfig *cfg;
    int error;
} MarkerContext;

static int marker_visitor(const char *path, const char *relative, void *context) {
    MarkerContext *marker_context = (MarkerContext *)context;
    (void)relative;
    for (int feature = 0; feature < WIZARD_FEATURE_COUNT; feature++) {
        char begin[96];
        char end[96];
        int result;
        snprintf(begin, sizeof(begin), "CPROJECT_FEATURE_%s_BEGIN", feature_marker_name(feature));
        snprintf(end, sizeof(end), "CPROJECT_FEATURE_%s_END", feature_marker_name(feature));
        do {
            result = process_block(path, begin, end, marker_context->cfg->features[feature]);
            if (result < 0) {
                marker_context->error = 1;
                return -1;
            }
        } while (result > 0);
    }
    return 0;
}

static int process_feature_markers(const char *root, const WizardConfig *cfg, char *err,
                                   size_t err_size) {
    MarkerContext context;
    context.cfg = cfg;
    context.error = 0;
    if (visit_text_tree(root, "", marker_visitor, &context) != 0 || context.error) {
        set_error(err, err_size, "处理特性标记失败。");
        return -1;
    }
    return 0;
}

static int remove_path(const char *root, const char *relative, WizardReport *report, char *err,
                       size_t err_size) {
    char path[WIZARD_MAX_PATH];
    if (wizard_join_path(path, sizeof(path), root, relative) != 0)
        return -1;
    if (!wizard_file_exists(path) && !wizard_dir_exists(path))
        return 0;
    if (wizard_remove_tree(path) != 0) {
        char message[512];
        snprintf(message, sizeof(message), "无法删除: %s", relative);
        set_error(err, err_size, message);
        return -1;
    }
    if (report->removed_count < WIZARD_MAX_ITEMS) {
        snprintf(report->removed[report->removed_count], WIZARD_MAX_ITEM, "%s", relative);
        report->removed_count++;
    }
    return 0;
}

static int remove_manifest_paths(const char *root, const WizardManifest *manifest,
                                 WizardReport *report, char *err, size_t err_size) {
    if (!manifest)
        return 0;
    for (int i = 0; i < manifest->count; i++) {
        if (remove_path(root, manifest->entries[i], report, err, err_size) != 0)
            return -1;
    }
    return 0;
}

static int strip_feature_paths(const char *root, const WizardConfig *cfg, WizardReport *report,
                               char *err, size_t err_size) {
    static const char *const installable_paths[] = {
        "cmake/Packaging.cmake", "cmake/InstallRules.cmake",      "cmake/PackagingMSI.cmake",
        "tests/package_smoke",   "scripts/check-cpack-config.py", NULL};
    static const char *const subproject_paths[] = {"tests/subproject_smoke", NULL};
    static const char *const ci_paths[] = {".github", NULL};
    static const char *const ai_paths[] = {"AGENTS.md", NULL};
    static const char *const cli_paths[] = {"src/main.c", "res", NULL};
    if (!cfg->features[WIZARD_FEATURE_INSTALLABLE]) {
        for (int i = 0; installable_paths[i]; i++) {
            if (remove_path(root, installable_paths[i], report, err, err_size) != 0)
                return -1;
        }
        {
            char config_path[WIZARD_MAX_PATH];
            snprintf(config_path, sizeof(config_path), "cmake/%sConfig.cmake.in", cfg->name);
            if (remove_path(root, config_path, report, err, err_size) != 0)
                return -1;
        }
    }
    if (!cfg->features[WIZARD_FEATURE_SUBPROJECT]) {
        for (int i = 0; subproject_paths[i]; i++) {
            if (remove_path(root, subproject_paths[i], report, err, err_size) != 0)
                return -1;
        }
    }
    if (!cfg->features[WIZARD_FEATURE_CI]) {
        for (int i = 0; ci_paths[i]; i++) {
            if (remove_path(root, ci_paths[i], report, err, err_size) != 0)
                return -1;
        }
    }
    if (!cfg->features[WIZARD_FEATURE_AI_AGENT]) {
        for (int i = 0; ai_paths[i]; i++) {
            if (remove_path(root, ai_paths[i], report, err, err_size) != 0)
                return -1;
        }
    }
    if (!cfg->features[WIZARD_FEATURE_CLI]) {
        for (int i = 0; cli_paths[i]; i++) {
            if (remove_path(root, cli_paths[i], report, err, err_size) != 0)
                return -1;
        }
    }
    return 0;
}

static char *cmake_quote(const char *value) {
    size_t length = 0;
    char *result;
    char *write;
    for (const char *p = value; *p; p++)
        length += (*p == '"' || *p == '\\') ? 2 : 1;
    result = (char *)malloc(length + 1);
    if (!result)
        return NULL;
    write = result;
    for (const char *p = value; *p; p++) {
        if (*p == '"' || *p == '\\')
            *write++ = '\\';
        *write++ = *p;
    }
    *write = '\0';
    return result;
}

static int write_metadata_block(const char *root, const WizardConfig *cfg, char *err,
                                size_t err_size) {
    char path[WIZARD_MAX_PATH];
    char *description = cmake_quote(cfg->description);
    char *author = cmake_quote(cfg->author);
    char *license = cmake_quote(cfg->license);
    char replacement[2048];
    int result;
    if (!description || !author || !license) {
        free(description);
        free(author);
        free(license);
        set_error(err, err_size, "内存不足。");
        return -1;
    }
    if (wizard_join_path(path, sizeof(path), root, "CMakeLists.txt") != 0 ||
        snprintf(replacement, sizeof(replacement),
                 "project(%s\n"
                 "    VERSION %s\n"
                 "    DESCRIPTION \"%s\"\n"
                 "    LANGUAGES C\n"
                 ")\n"
                 "set(CPROJECT_PROJECT_AUTHOR \"%s\")\n"
                 "set(CPROJECT_PROJECT_LICENSE \"%s\")\n",
                 cfg->name, cfg->version, description, author,
                 license) >= (int)sizeof(replacement)) {
        free(description);
        free(author);
        free(license);
        set_error(err, err_size, "写入项目元信息失败。");
        return -1;
    }
    result = replace_block(path, "CPROJECT_PROJECT_METADATA_BEGIN", "CPROJECT_PROJECT_METADATA_END",
                           replacement, 0);
    free(description);
    free(author);
    free(license);
    if (result != 1) {
        set_error(err, err_size, "CMakeLists.txt 缺少项目元信息标记。");
        return -1;
    }
    return 0;
}

static int current_date(char *out, size_t size) {
    time_t now = time(NULL);
    struct tm local;
#ifdef _WIN32
    if (localtime_s(&local, &now) != 0)
        return -1;
#else
    if (!localtime_r(&now, &local))
        return -1;
#endif
    return strftime(out, size, "%Y-%m-%d", &local) == 0 ? -1 : 0;
}

static int current_year(void) {
    time_t now = time(NULL);
    struct tm local;
#ifdef _WIN32
    if (localtime_s(&local, &now) != 0)
        return 1970;
#else
    if (!localtime_r(&now, &local))
        return 1970;
#endif
    return local.tm_year + 1900;
}

static int write_report_file(const char *root, const char *relative, const char *content,
                             WizardReport *report, char *err, size_t err_size) {
    char path[WIZARD_MAX_PATH];
    if (wizard_join_path(path, sizeof(path), root, relative) != 0 ||
        wizard_write_text(path, content) != 0) {
        char message[512];
        snprintf(message, sizeof(message), "无法写入: %s", relative);
        set_error(err, err_size, message);
        return -1;
    }
    if (report->written_count < (int)(sizeof(report->written) / sizeof(report->written[0]))) {
        snprintf(report->written[report->written_count], WIZARD_MAX_ITEM, "%s", relative);
        report->written_count++;
    }
    return 0;
}

static int write_license_file(const char *root, const WizardConfig *cfg, WizardReport *report,
                              char *err, size_t err_size) {
    if (strcmp(cfg->license, "MIT") != 0) {
        char *content = NULL;
        char path[WIZARD_MAX_PATH];
        size_t length = 0;
        if (cfg->license_file[0] == '\0' ||
            wizard_read_text(cfg->license_file, &content, &length) != 0) {
            set_error(err, err_size, "非 MIT 许可证需要提供可读取的 --license-file。");
            return -1;
        }
        if (wizard_join_path(path, sizeof(path), root, "LICENSE") != 0 ||
            wizard_write_bytes(path, content, length) != 0) {
            free(content);
            set_error(err, err_size, "无法写入 LICENSE。");
            return -1;
        }
        free(content);
    } else {
        char content[4096];
        if (snprintf(
                content, sizeof(content),
                "MIT License\n\nCopyright (c) %d %s\n\n"
                "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
                "of this software and associated documentation files (the \"Software\"), to deal\n"
                "in the Software without restriction, including without limitation the rights\n"
                "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
                "copies of the Software, and to permit persons to whom the Software is\n"
                "furnished to do so, subject to the following conditions:\n\n"
                "The above copyright notice and this permission notice shall be included in all\n"
                "copies or substantial portions of the Software.\n\n"
                "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
                "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
                "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
                "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
                "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
                "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
                "SOFTWARE.\n",
                current_year(), cfg->author) >= (int)sizeof(content)) {
            set_error(err, err_size, "LICENSE 内容过长。");
            return -1;
        }
        if (write_report_file(root, "LICENSE", content, report, err, err_size) != 0)
            return -1;
        return 0;
    }
    if (report->written_count < (int)(sizeof(report->written) / sizeof(report->written[0]))) {
        snprintf(report->written[report->written_count], WIZARD_MAX_ITEM, "%s", "LICENSE");
        report->written_count++;
    }
    return 0;
}

static int write_changelog(const char *root, const WizardConfig *cfg, WizardReport *report,
                           char *err, size_t err_size) {
    char date[32];
    char content[1024];
    if (current_date(date, sizeof(date)) != 0)
        snprintf(date, sizeof(date), "1970-01-01");
    if (snprintf(content, sizeof(content),
                 "# 更新日志\n\n"
                 "> 新增段落用中文书写；已发布段落保留发布时的原文，不重写。\n\n"
                 "## %s — %s\n\n"
                 "- 初始项目版本。\n",
                 cfg->version, date) >= (int)sizeof(content)) {
        set_error(err, err_size, "CHANGELOG 内容过长。");
        return -1;
    }
    return write_report_file(root, "CHANGELOG.md", content, report, err, err_size);
}

static int write_readme(const char *root, const WizardConfig *cfg, WizardReport *report, char *err,
                        size_t err_size) {
    TextBuffer buffer = {0};
    char line[512];
    int has_install = cfg->features[WIZARD_FEATURE_INSTALLABLE];
    int has_ci = cfg->features[WIZARD_FEATURE_CI];
    int has_ai = cfg->features[WIZARD_FEATURE_AI_AGENT];
    int has_cli = cfg->features[WIZARD_FEATURE_CLI];

    if (buffer_append(&buffer, "# ") != 0 || buffer_append(&buffer, cfg->name) != 0 ||
        buffer_append(&buffer, "\n\n") != 0 || buffer_append(&buffer, cfg->description) != 0 ||
        buffer_append(&buffer, "\n\n作者/维护者：") != 0 ||
        buffer_append(&buffer, cfg->author) != 0 ||
        buffer_append(&buffer, "\n\n## 快速开始\n\n") != 0 ||
        buffer_append(&buffer, "```sh\n"
                               "./scripts/bootstrap.sh\n"
                               "./scripts/check.sh\n"
                               "```\n\n"
                               "**Windows PowerShell**\n\n"
                               "```powershell\n"
                               "./scripts/bootstrap.ps1\n"
                               "./scripts/check.ps1\n"
                               "```\n\n") != 0)
        goto fail;

    if (has_cli) {
        if (buffer_append(&buffer, "构建后运行 CLI 桩程序：\n\n```sh\n") != 0 ||
            snprintf(line, sizeof(line),
                     "cmake --build --preset ninja-debug\n"
                     "./build/ninja-debug/bin/%s\n",
                     cfg->lower) >= (int)sizeof(line) ||
            buffer_append(&buffer, line) != 0 || buffer_append(&buffer, "```\n\n") != 0)
            goto fail;
    }

    if (buffer_append(&buffer, "## 项目结构\n\n"
                               "```text\n"
                               "include/        公共头文件\n"
                               "src/            实现\n"
                               "tests/          CTest 测试\n"
                               "cmake/          构建模块\n"
                               "scripts/        本地引导与检查脚本\n"
                               "docs/           项目文档\n"
                               "```\n\n") != 0)
        goto fail;

    if (has_install) {
        if (snprintf(line, sizeof(line),
                     "## 安装与消费\n\n```sh\n"
                     "cmake --preset ninja-release\n"
                     "cmake --build --preset ninja-release\n"
                     "cmake --install build/ninja-release --prefix install\n"
                     "```\n\n```cmake\nfind_package(%s CONFIG REQUIRED)\n"
                     "target_link_libraries(app PRIVATE %s::%s_core)\n```\n\n",
                     cfg->name, cfg->name, cfg->lower) >= (int)sizeof(line) ||
            buffer_append(&buffer, line) != 0)
            goto fail;
    }

    if (has_ci &&
        buffer_append(
            &buffer, "## 持续集成\n\n本项目包含 GitHub Actions 构建、测试与静态分析工作流。\n\n") !=
            0)
        goto fail;

    if (buffer_append(&buffer, "## 文档\n\n") != 0 ||
        buffer_append(&buffer, "- [CMake 与构建](docs/guides/cmake.md)\n") != 0 ||
        buffer_append(&buffer, "- [测试与质量门禁](docs/guides/testing.md)\n") != 0 ||
        buffer_append(&buffer, "- [编码风格](docs/guides/coding-style.md)\n") != 0 ||
        buffer_append(&buffer, "- [环境配置](docs/guides/environment.md)\n") != 0 ||
        buffer_append(&buffer, "- [架构决策记录](docs/adr/README.md)\n") != 0 ||
        buffer_append(&buffer,
                      "- [工具链策略 ADR](docs/adr/0001-c11-cmake-toolchain-policy.md)\n") != 0 ||
        buffer_append(&buffer, "- [工程标准](C_PROJECT_STANDARD.md)\n") != 0 ||
        buffer_append(&buffer, "- [贡献指南](CONTRIBUTING.md)\n") != 0 ||
        buffer_append(&buffer, "- [安全策略](SECURITY.md)\n") != 0)
        goto fail;
    if (has_ai && buffer_append(&buffer, "- [AI 代理入口](AGENTS.md)\n") != 0)
        goto fail;
    if (snprintf(line, sizeof(line), "\n## 许可证\n\n%s\n", cfg->license) >= (int)sizeof(line) ||
        buffer_append(&buffer, line) != 0)
        goto fail;

    if (write_report_file(root, "README.md", buffer.data, report, err, err_size) != 0)
        goto fail;
    buffer_free(&buffer);
    return 0;
fail:
    buffer_free(&buffer);
    set_error(err, err_size, "无法生成 README.md。");
    return -1;
}

static int write_agents(const char *root, const WizardConfig *cfg, WizardReport *report, char *err,
                        size_t err_size) {
    TextBuffer buffer = {0};
    char line[512];
    if (!cfg->features[WIZARD_FEATURE_AI_AGENT])
        return 0;
    if (buffer_append(&buffer, "# AGENTS.md\n\n面向在 ") != 0 ||
        buffer_append(&buffer, cfg->name) != 0 ||
        buffer_append(&buffer, " 项目中工作的 AI 代理。\n\n"
                               "## 必读\n\n"
                               "- `CONTRIBUTING.md`\n"
                               "- `docs/guides/testing.md`\n"
                               "- `docs/guides/cmake.md`\n"
                               "- `SECURITY.md`\n\n"
                               "## 项目形状\n\n"
                               "```text\n"
                               "include/        公共头文件\n"
                               "src/            实现\n"
                               "tests/          CTest 测试\n"
                               "cmake/          构建模块\n"
                               "scripts/        本地引导与检查脚本\n"
                               "docs/           项目文档\n"
                               "```\n\n"
                               "## 验证\n\n"
                               "```sh\n"
                               "cmake --preset ninja-debug\n"
                               "cmake --build --preset ninja-debug\n"
                               "ctest --preset ninja-debug --output-on-failure\n"
                               "```\n\n"
                               "## 规则\n\n"
                               "- 公共 API 属于 `include/`，私有实现属于 `src/`。\n"
                               "- 测试应断言行为，而不是偶然的实现细节。\n"
                               "- 不为了让改动通过而放宽警告、测试或 CI。\n"
                               "- 修改构建、测试、发布或安全策略时同步更新文档。\n") != 0)
        goto fail;
    if (snprintf(line, sizeof(line), "\n许可证：%s\n", cfg->license) >= (int)sizeof(line) ||
        buffer_append(&buffer, line) != 0)
        goto fail;
    if (write_report_file(root, "AGENTS.md", buffer.data, report, err, err_size) != 0)
        goto fail;
    buffer_free(&buffer);
    return 0;
fail:
    buffer_free(&buffer);
    set_error(err, err_size, "无法生成 AGENTS.md。");
    return -1;
}

static int write_generated_ci(const char *root, const WizardConfig *cfg, WizardReport *report,
                              char *err, size_t err_size) {
    char template_path[WIZARD_MAX_PATH];
    char workflows[WIZARD_MAX_PATH];
    char output_path[WIZARD_MAX_PATH];
    char assets[WIZARD_MAX_PATH];
    char *content = NULL;
    size_t length = 0;
    int result = 0;

    if (wizard_join_path(assets, sizeof(assets), root, "templates") != 0)
        return -1;
    if (!cfg->features[WIZARD_FEATURE_CI]) {
        if (wizard_remove_tree(assets) != 0)
            return -1;
        return 0;
    }
    if (wizard_join_path(template_path, sizeof(template_path), root, "templates/ci.yml") != 0 ||
        wizard_read_text(template_path, &content, &length) != 0) {
        set_error(err, err_size, "无法读取生成 CI 模板。");
        return -1;
    }
    if (wizard_join_path(workflows, sizeof(workflows), root, ".github/workflows") != 0 ||
        wizard_mkdir_p(workflows) != 0 ||
        wizard_join_path(output_path, sizeof(output_path), workflows, "ci.yml") != 0) {
        free(content);
        set_error(err, err_size, "无法创建 CI 目录。");
        return -1;
    }
    if (wizard_write_bytes(output_path, content, length) != 0) {
        free(content);
        set_error(err, err_size, "无法写入生成 CI 工作流。");
        return -1;
    }
    free(content);
    if (report->written_count < (int)(sizeof(report->written) / sizeof(report->written[0]))) {
        snprintf(report->written[report->written_count], WIZARD_MAX_ITEM, "%s",
                 ".github/workflows/ci.yml");
        report->written_count++;
    }
    if (wizard_remove_tree(assets) != 0) {
        set_error(err, err_size, "无法清理生成资产目录。");
        result = -1;
    }
    return result;
}

static int write_stub_main(const char *root, const WizardConfig *cfg, WizardReport *report,
                           char *err, size_t err_size) {
    char content[1024];
    if (!cfg->features[WIZARD_FEATURE_CLI])
        return 0;
    if (snprintf(
            content, sizeof(content),
            "#include <%s/example.h>\n"
            "#include <stdio.h>\n\n"
            "int main(void) {\n"
            "    %s_version_t version = %s_version();\n"
            "    printf(\"%s v%%d.%%d.%%d\\n\", version.major, version.minor, version.patch);\n"
            "    return 0;\n"
            "}\n",
            cfg->lower, cfg->lower, cfg->lower, cfg->name) >= (int)sizeof(content)) {
        set_error(err, err_size, "无法生成 CLI 桩代码。");
        return -1;
    }
    return write_report_file(root, "src/main.c", content, report, err, err_size);
}

static int prepare_template_files(const char *root, char *err, size_t err_size) {
    char path[WIZARD_MAX_PATH];
    int result;
    if (wizard_join_path(path, sizeof(path), root, "CMakeLists.txt") != 0 ||
        !wizard_file_exists(path)) {
        set_error(err, err_size, "找不到 CMakeLists.txt。");
        return -1;
    }
    result = replace_block(path, "CPROJECT_TEMPLATE_BUILD_BEGIN", "CPROJECT_TEMPLATE_BUILD_END",
                           NULL, 1);
    if (result < 0) {
        set_error(err, err_size, "处理模板构建标记失败。");
        return -1;
    }
    for (;;) {
        result = replace_block(path, "CPROJECT_TEMPLATE_ONLY_BEGIN", "CPROJECT_TEMPLATE_ONLY_END",
                               NULL, 1);
        if (result < 0) {
            set_error(err, err_size, "处理模板构建标记失败。");
            return -1;
        }
        if (result == 0)
            break;
    }
    if (wizard_join_path(path, sizeof(path), root, "cmake/InstallRules.cmake") == 0) {
        for (;;) {
            result = replace_block(path, "CPROJECT_TEMPLATE_ONLY_BEGIN",
                                   "CPROJECT_TEMPLATE_ONLY_END", NULL, 1);
            if (result < 0) {
                set_error(err, err_size, "处理安装模板标记失败。");
                return -1;
            }
            if (result == 0)
                break;
        }
    }
    if (wizard_join_path(path, sizeof(path), root, "cmake/Tests.cmake") != 0 ||
        replace_block(path, "CPROJECT_WIZARD_TESTS_BEGIN", "CPROJECT_WIZARD_TESTS_END", NULL, 1) <
            0) {
        set_error(err, err_size, "处理向导测试标记失败。");
        return -1;
    }
    return 0;
}

int wizard_setup_project(const char *root, const WizardConfig *cfg, const WizardManifest *manifest,
                         int git_init, WizardReport *report, char *err, size_t err_size) {
    if (!root || !cfg || !report) {
        set_error(err, err_size, "向导参数无效。");
        return -1;
    }
    if (err && err_size > 0)
        err[0] = '\0';
    if (prepare_template_files(root, err, err_size) != 0)
        return -1;
    if (write_metadata_block(root, cfg, err, err_size) != 0)
        return -1;
    if (process_feature_markers(root, cfg, err, err_size) != 0)
        return -1;
    if (remove_manifest_paths(root, manifest, report, err, err_size) != 0)
        return -1;
    if (remove_path(root, "template-exclude.txt", report, err, err_size) != 0)
        return -1;
    if (rename_identifiers(root, cfg, report, err, err_size) != 0)
        return -1;
    if (rename_file_tree(root, "", cfg, report, err, err_size) != 0)
        return -1;
    if (rename_known_directories(root, cfg, report, err, err_size) != 0)
        return -1;
    if (strip_feature_paths(root, cfg, report, err, err_size) != 0)
        return -1;
    if (write_license_file(root, cfg, report, err, err_size) != 0)
        return -1;
    if (write_changelog(root, cfg, report, err, err_size) != 0)
        return -1;
    if (write_readme(root, cfg, report, err, err_size) != 0)
        return -1;
    if (write_agents(root, cfg, report, err, err_size) != 0)
        return -1;
    if (write_stub_main(root, cfg, report, err, err_size) != 0)
        return -1;
    if (write_generated_ci(root, cfg, report, err, err_size) != 0)
        return -1;
    if (git_init && wizard_run_git_init(root) != 0) {
        set_error(err, err_size, "git init 失败，请确认 git 已安装。");
        return -1;
    }
    report->git_initialized = git_init;
    return 0;
}
