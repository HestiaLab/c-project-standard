#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "wizard.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <process.h>
#include <sys/stat.h>
#define WIZARD_ACCESS _access
#define WIZARD_CHDIR _chdir
#define WIZARD_GETCWD _getcwd
#define WIZARD_MKDIR(p) _mkdir(p)
#define WIZARD_RMDIR _rmdir
#define WIZARD_UNLINK _unlink
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define WIZARD_ACCESS access
#define WIZARD_CHDIR chdir
#define WIZARD_GETCWD getcwd
#define WIZARD_MKDIR(p) mkdir((p), 0777)
#define WIZARD_RMDIR rmdir
#define WIZARD_UNLINK unlink
#endif

#ifndef F_OK
#define F_OK 0
#endif

static int is_sep(char c) {
    return c == '/' || c == '\\';
}

int wizard_join_path(char *out, size_t size, const char *a, const char *b) {
    size_t len;
    int need_sep;
    if (!out || size == 0 || !a || !b)
        return -1;
    len = strlen(a);
    need_sep = len > 0 && !is_sep(a[len - 1]);
#ifdef _WIN32
    if (snprintf(out, size, "%s%s%s", a, need_sep ? "\\" : "", b) >= (int)size)
        return -1;
#else
    if (snprintf(out, size, "%s%s%s", a, need_sep ? "/" : "", b) >= (int)size)
        return -1;
#endif
    return 0;
}

int wizard_read_text(const char *path, char **out, size_t *out_len) {
    FILE *file;
    long length;
    char *buffer;
    size_t read;

    if (out)
        *out = NULL;
    if (out_len)
        *out_len = 0;
    if (!path || !out)
        return -1;
    file = fopen(path, "rb");
    if (!file)
        return -1;
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }
    length = ftell(file);
    if (length < 0 || length > (long)(64 * 1024 * 1024)) {
        fclose(file);
        return -1;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return -1;
    }
    buffer = (char *)malloc((size_t)length + 1);
    if (!buffer) {
        fclose(file);
        return -1;
    }
    read = fread(buffer, 1, (size_t)length, file);
    fclose(file);
    if (read != (size_t)length) {
        free(buffer);
        return -1;
    }
    buffer[read] = '\0'; /* NOLINT(clang-analyzer-security.ArrayBound) */
    *out = buffer;
    if (out_len)
        *out_len = read;
    return 0;
}

int wizard_write_bytes(const char *path, const void *data, size_t len) {
    FILE *file;
    size_t written;
    if (!path || (!data && len != 0))
        return -1;
    file = fopen(path, "wb");
    if (!file)
        return -1;
    written = len == 0 ? 0 : fwrite(data, 1, len, file);
    if (fclose(file) != 0)
        return -1;
    return written == len ? 0 : -1;
}

int wizard_write_text(const char *path, const char *text) {
    if (!text)
        return -1;
    return wizard_write_bytes(path, text, strlen(text));
}

int wizard_file_exists(const char *path) {
    return path && WIZARD_ACCESS(path, F_OK) == 0;
}

int wizard_dir_exists(const char *path) {
    struct stat st;
    if (!path || stat(path, &st) != 0)
        return 0;
#ifdef _WIN32
    return (st.st_mode & _S_IFDIR) != 0;
#else
    return S_ISDIR(st.st_mode) != 0;
#endif
}

int wizard_dir_is_empty(const char *path) {
    if (!wizard_dir_exists(path))
        return -1;
#ifdef _WIN32
    {
        char pattern[WIZARD_MAX_PATH];
        struct _finddata_t entry;
        intptr_t handle;
        if (wizard_join_path(pattern, sizeof(pattern), path, "*") != 0)
            return -1;
        handle = _findfirst(pattern, &entry);
        if (handle == -1)
            return 1;
        do {
            if (strcmp(entry.name, ".") != 0 && strcmp(entry.name, "..") != 0) {
                _findclose(handle);
                return 0;
            }
        } while (_findnext(handle, &entry) == 0);
        _findclose(handle);
    }
#else
    {
        DIR *dir = opendir(path);
        struct dirent *entry;
        if (!dir)
            return -1;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                closedir(dir);
                return 0;
            }
        }
        closedir(dir);
    }
#endif
    return 1;
}

static int mkdir_one(const char *path) {
    if (!path || path[0] == '\0' || wizard_dir_exists(path))
        return 0;
    if (WIZARD_MKDIR(path) == 0)
        return 0;
    if (errno == EEXIST && wizard_dir_exists(path))
        return 0;
    return -1;
}

int wizard_mkdir_p(const char *path) {
    char buffer[WIZARD_MAX_PATH];
    size_t len;
    size_t start = 0;
    if (!path || !path[0])
        return -1;
    len = strlen(path);
    if (len >= sizeof(buffer))
        return -1;
    memcpy(buffer, path, len + 1);
    while (len > 0 && is_sep(buffer[len - 1]))
        buffer[--len] = '\0';
    if (len == 0)
        return 0;
#ifdef _WIN32
    if (len >= 2 && buffer[1] == ':')
        start = 2;
#endif
    for (size_t i = start + 1; i < len; i++) {
        if (is_sep(buffer[i])) {
            char saved = buffer[i];
            buffer[i] = '\0';
            if (mkdir_one(buffer) != 0)
                return -1;
            buffer[i] = saved;
        }
    }
    return mkdir_one(buffer);
}

int wizard_remove_tree(const char *path) {
    if (!path || !path[0] || strcmp(path, ".") == 0 || strcmp(path, "/") == 0)
        return -1;
    if (!wizard_file_exists(path) && !wizard_dir_exists(path))
        return 0;
#ifdef _WIN32
    {
        struct _finddata_t entry;
        char pattern[WIZARD_MAX_PATH];
        intptr_t handle;
        if (!wizard_dir_exists(path))
            return WIZARD_UNLINK(path) == 0 ? 0 : -1;
        if (wizard_join_path(pattern, sizeof(pattern), path, "*") != 0)
            return -1;
        handle = _findfirst(pattern, &entry);
        if (handle != -1) {
            do {
                char child[WIZARD_MAX_PATH];
                if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0)
                    continue;
                if (wizard_join_path(child, sizeof(child), path, entry.name) != 0) {
                    _findclose(handle);
                    return -1;
                }
                if (wizard_remove_tree(child) != 0) {
                    _findclose(handle);
                    return -1;
                }
            } while (_findnext(handle, &entry) == 0);
            _findclose(handle);
        }
        return WIZARD_RMDIR(path) == 0 ? 0 : -1;
    }
#else
    {
        struct stat st;
        if (lstat(path, &st) != 0)
            return -1;
        if (!S_ISDIR(st.st_mode))
            return WIZARD_UNLINK(path) == 0 ? 0 : -1;
        DIR *dir = opendir(path);
        struct dirent *entry;
        if (!dir)
            return -1;
        while ((entry = readdir(dir)) != NULL) {
            char child[WIZARD_MAX_PATH];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            if (wizard_join_path(child, sizeof(child), path, entry->d_name) != 0) {
                closedir(dir);
                return -1;
            }
            if (wizard_remove_tree(child) != 0) {
                closedir(dir);
                return -1;
            }
        }
        closedir(dir);
        return WIZARD_RMDIR(path) == 0 ? 0 : -1;
    }
#endif
}

static int should_skip_name(const char *name) {
    if (strcmp(name, ".git") == 0 || strcmp(name, ".vs") == 0 || strcmp(name, ".cache") == 0 ||
        strcmp(name, ".hermes") == 0 || strcmp(name, "build") == 0 || strcmp(name, "dist") == 0 ||
        strcmp(name, "out") == 0 || strcmp(name, "_CPack_Packages") == 0 ||
        strcmp(name, "Testing") == 0 || strcmp(name, "__pycache__") == 0 ||
        strcmp(name, ".wix") == 0 || strcmp(name, "Okk") == 0 || strcmp(name, "TestProj") == 0 ||
        strcmp(name, "template-exclude.txt") == 0 || strcmp(name, "CMakeUserPresets.json") == 0 ||
        strcmp(name, ".stamp") == 0 || strcmp(name, ".static-analysis-files.txt") == 0 ||
        strcmp(name, "node_modules") == 0) {
        return 1;
    }
    if (strncmp(name, "install-", 8) == 0)
        return 1;
    if (strcmp(name, "install") == 0)
        return 1;
    {
        size_t len = strlen(name);
        if (len >= 4 && strcmp(name + len - 4, ".pyc") == 0)
            return 1;
        if (len >= 19 && strcmp(name + len - 19, "-cppcheck-build-dir") == 0)
            return 1;
    }
    return 0;
}

static int manifest_skip(const WizardManifest *manifest, const char *relative) {
    return manifest && manifest->count > 0 && wizard_manifest_matches(manifest, relative);
}

static int copy_file_binary(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    FILE *out;
    char buffer[16384];
    size_t read;
    int ok = 0;
    if (!in)
        return -1;
    out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }
    /* Standard read-to-EOF loop; clang-analyzer misreports the second read. */
    // NOLINTNEXTLINE(clang-analyzer-unix.Stream)
    while ((read = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, read, out) != read) {
            ok = -1;
            break;
        }
    }
    if (ferror(in))
        ok = -1;
    if (fclose(out) != 0)
        ok = -1;
    fclose(in);
    return ok;
}

static int copy_tree_inner(const char *src, const char *dst, const char *relative,
                           const WizardManifest *manifest) {
#ifdef _WIN32
    char pattern[WIZARD_MAX_PATH];
    struct _finddata_t entry;
    intptr_t handle;
    if (wizard_join_path(pattern, sizeof(pattern), src, "*") != 0)
        return -1;
    handle = _findfirst(pattern, &entry);
    if (handle == -1)
        return -1;
    do {
        char child_src[WIZARD_MAX_PATH];
        char child_dst[WIZARD_MAX_PATH];
        char child_rel[WIZARD_MAX_PATH];
        if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0)
            continue;
        if (should_skip_name(entry.name))
            continue;
        if (wizard_join_path(child_src, sizeof(child_src), src, entry.name) != 0 ||
            wizard_join_path(child_dst, sizeof(child_dst), dst, entry.name) != 0)
            goto fail;
        if (relative && relative[0])
            snprintf(child_rel, sizeof(child_rel), "%s/%s", relative, entry.name);
        else
            snprintf(child_rel, sizeof(child_rel), "%s", entry.name);
        if (manifest_skip(manifest, child_rel))
            continue;
        if (entry.attrib & _A_SUBDIR) {
            if (wizard_mkdir_p(child_dst) != 0 ||
                copy_tree_inner(child_src, child_dst, child_rel, manifest) != 0)
                goto fail;
        } else if (copy_file_binary(child_src, child_dst) != 0) {
            goto fail;
        }
    } while (_findnext(handle, &entry) == 0);
    _findclose(handle);
    return 0;
fail:
    _findclose(handle);
    return -1;
#else
    DIR *dir;
    struct dirent *entry;
    dir = opendir(src);
    if (!dir)
        return -1;
    while ((entry = readdir(dir)) != NULL) {
        char child_src[WIZARD_MAX_PATH];
        char child_dst[WIZARD_MAX_PATH];
        char child_rel[WIZARD_MAX_PATH];
        struct stat st;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (should_skip_name(entry->d_name))
            continue;
        if (wizard_join_path(child_src, sizeof(child_src), src, entry->d_name) != 0 ||
            wizard_join_path(child_dst, sizeof(child_dst), dst, entry->d_name) != 0) {
            closedir(dir);
            return -1;
        }
        if (relative && relative[0])
            snprintf(child_rel, sizeof(child_rel), "%s/%s", relative, entry->d_name);
        else
            snprintf(child_rel, sizeof(child_rel), "%s", entry->d_name);
        if (manifest_skip(manifest, child_rel))
            continue;
        if (lstat(child_src, &st) != 0)
            continue;
        if (S_ISLNK(st.st_mode))
            continue;
        if (S_ISDIR(st.st_mode)) {
            if (wizard_mkdir_p(child_dst) != 0 ||
                copy_tree_inner(child_src, child_dst, child_rel, manifest) != 0) {
                closedir(dir);
                return -1;
            }
        } else if (S_ISREG(st.st_mode)) {
            if (copy_file_binary(child_src, child_dst) != 0) {
                closedir(dir);
                return -1;
            }
        }
    }
    closedir(dir);
    return 0;
#endif
}

int wizard_copy_tree(const char *src, const char *dst, const WizardManifest *manifest) {
    if (!src || !dst)
        return -1;
    if (wizard_mkdir_p(dst) != 0)
        return -1;
    return copy_tree_inner(src, dst, "", manifest);
}

int wizard_make_temp_dir(char *out, size_t size, const char *tag) {
    const char *base;
    if (!out || size == 0)
        return -1;
    base = getenv("TMPDIR");
    if (!base || !base[0])
        base = getenv("TEMP");
    if (!base || !base[0])
        base = getenv("TMP");
    if (!base || !base[0])
        base = ".";
    for (int attempt = 0; attempt < 100; attempt++) {
        unsigned long stamp = (unsigned long)time(NULL);
#ifdef _WIN32
        int pid = _getpid();
#else
        int pid = (int)getpid();
#endif
        if (snprintf(out, size, "%s%ccproject-%s-%lu-%d-%d", base,
#ifdef _WIN32
                     '\\',
#else
                     '/',
#endif
                     tag ? tag : "tmp", stamp, pid, attempt) >= (int)size)
            return -1;
        if (wizard_file_exists(out))
            continue;
        if (wizard_mkdir_p(out) == 0)
            return 0;
    }
    return -1;
}

int wizard_run_git_init(const char *root) {
    char previous[WIZARD_MAX_PATH];
    int result;
    if (!root || !wizard_dir_exists(root))
        return -1;
    if (!WIZARD_GETCWD(previous, (int)sizeof(previous)))
        return -1;
    if (WIZARD_CHDIR(root) != 0)
        return -1;
    result = system("git init -q"); /* NOLINT(bugprone-command-processor) */
    if (WIZARD_CHDIR(previous) != 0)
        return -1;
    return result == 0 ? 0 : -1;
}

static int manifest_add(WizardManifest *manifest, const char *entry) {
    char **items;
    char *copy;
    if (!entry[0] || strstr(entry, "..") != NULL || entry[0] == '/')
        return 0;
    items = (char **)realloc(manifest->entries, sizeof(char *) * (size_t)(manifest->count + 1));
    if (!items)
        return -1;
    manifest->entries = items;
    {
        size_t length = strlen(entry);
        copy = (char *)malloc(length + 1);
        if (!copy)
            return -1;
        memcpy(copy, entry, length + 1);
    }
    manifest->entries[manifest->count++] = copy;
    return 0;
}

static void normalize_relative(char *path) {
    char *start = path;
    char *write = path;
    int previous_sep = 0;
    while (*path) {
        char c = *path++;
        if (c == '\\')
            c = '/';
        if (c == '/' && previous_sep)
            continue;
        previous_sep = c == '/';
        *write++ = c;
    }
    *write = '\0';
    while (write > start && write[-1] == '/')
        *--write = '\0';
}

int wizard_manifest_load(const char *root, WizardManifest *manifest) {
    char path[WIZARD_MAX_PATH];
    char *text = NULL;
    char *line;
    char *next;
    if (!manifest)
        return -1;
    manifest->entries = NULL;
    manifest->count = 0;
    if (!root || wizard_join_path(path, sizeof(path), root, "template-exclude.txt") != 0)
        return -1;
    if (!wizard_file_exists(path))
        return 0;
    if (wizard_read_text(path, &text, NULL) != 0)
        return -1;
    line = text;
    while (line) {
        char *end;
        next = strchr(line, '\n');
        if (next)
            *next++ = '\0';
        end = line + strlen(line);
        while (end > line && (end[-1] == '\r' || end[-1] == ' ' || end[-1] == '\t'))
            *--end = '\0';
        while (*line == ' ' || *line == '\t')
            line++;
        if (*line && *line != '#') {
            normalize_relative(line);
            if (manifest_add(manifest, line) != 0) {
                free(text);
                wizard_manifest_free(manifest);
                return -1;
            }
        }
        line = next;
    }
    free(text);
    return 0;
}

void wizard_manifest_free(WizardManifest *manifest) {
    if (!manifest)
        return;
    for (int i = 0; i < manifest->count; i++)
        free(manifest->entries[i]);
    free(manifest->entries);
    manifest->entries = NULL;
    manifest->count = 0;
}

int wizard_manifest_matches(const WizardManifest *manifest, const char *relative_path) {
    char normalized[WIZARD_MAX_PATH];
    size_t len;
    if (!manifest || !relative_path || !relative_path[0])
        return 0;
    len = strlen(relative_path);
    if (len >= sizeof(normalized))
        return 1;
    memcpy(normalized, relative_path, len + 1);
    for (size_t i = 0; i < len; i++) {
        if (normalized[i] == '\\')
            normalized[i] = '/';
    }
    while (len > 0 && normalized[len - 1] == '/')
        normalized[--len] = '\0';
    for (int i = 0; i < manifest->count; i++) {
        const char *entry = manifest->entries[i];
        size_t entry_len = strlen(entry);
        if (strcmp(normalized, entry) == 0)
            return 1;
        if (strncmp(normalized, entry, entry_len) == 0 && normalized[entry_len] == '/')
            return 1;
    }
    return 0;
}
