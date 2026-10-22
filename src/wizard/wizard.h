#ifndef CPROJECT_WIZARD_H
#define CPROJECT_WIZARD_H

#include <stddef.h>

#define WIZARD_MAX_PATH 1024
#define WIZARD_MAX_NAME 128
#define WIZARD_MAX_DESCRIPTION 256
#define WIZARD_MAX_AUTHOR 160
#define WIZARD_MAX_VERSION 32
#define WIZARD_MAX_LICENSE 64
#define WIZARD_MAX_ITEM 256
#define WIZARD_MAX_ITEMS 64

typedef enum {
    WIZARD_LANG_EN = 0,
    WIZARD_LANG_ZH = 1,
} WizardLang;

typedef enum {
    WIZARD_FEATURE_INSTALLABLE = 0,
    WIZARD_FEATURE_SUBPROJECT,
    WIZARD_FEATURE_CI,
    WIZARD_FEATURE_AI_AGENT,
    WIZARD_FEATURE_CLI,
    WIZARD_FEATURE_COUNT,
} WizardFeature;

typedef struct {
    char name[WIZARD_MAX_NAME];
    char lower[WIZARD_MAX_NAME];
    char upper[WIZARD_MAX_NAME];
    char description[WIZARD_MAX_DESCRIPTION];
    char author[WIZARD_MAX_AUTHOR];
    char version[WIZARD_MAX_VERSION];
    char license[WIZARD_MAX_LICENSE];
    char license_file[WIZARD_MAX_PATH];
    int features[WIZARD_FEATURE_COUNT];
} WizardConfig;

typedef struct {
    WizardLang lang;
    int lang_explicit;
    int dry_run;
    int json;
    int yes;
    int git_init;
    int in_place;
    int non_interactive;
    char command[16];
    char template_root[WIZARD_MAX_PATH];
    char output[WIZARD_MAX_PATH];
} WizardOptions;

typedef struct {
    int renamed_files;
    int renamed_dirs;
    int removed_count;
    int written_count;
    int git_initialized;
    char removed[WIZARD_MAX_ITEMS][WIZARD_MAX_ITEM];
    char written[16][WIZARD_MAX_ITEM];
} WizardReport;

typedef struct {
    char **entries;
    int count;
} WizardManifest;

void wizard_config_defaults(WizardConfig *cfg);
void wizard_config_derive(WizardConfig *cfg);
int wizard_normalize_name(const char *input, char *out, size_t out_size, char *err,
                          size_t err_size);
int wizard_validate_version(const char *version);
int wizard_feature_from_name(const char *name);
const char *wizard_feature_name(int feature);
int wizard_parse_feature_option(const char *arg, int *feature, int *enabled);

int wizard_read_text(const char *path, char **out, size_t *out_len);
int wizard_write_text(const char *path, const char *text);
int wizard_write_bytes(const char *path, const void *data, size_t len);
int wizard_file_exists(const char *path);
int wizard_dir_exists(const char *path);
int wizard_dir_is_empty(const char *path);
int wizard_mkdir_p(const char *path);
int wizard_remove_tree(const char *path);
int wizard_join_path(char *out, size_t size, const char *a, const char *b);
int wizard_make_temp_dir(char *out, size_t size, const char *tag);
int wizard_copy_tree(const char *src, const char *dst, const WizardManifest *manifest);
int wizard_run_git_init(const char *root);

int wizard_manifest_load(const char *root, WizardManifest *manifest);
void wizard_manifest_free(WizardManifest *manifest);
int wizard_manifest_matches(const WizardManifest *manifest, const char *relative_path);

int wizard_setup_project(const char *root, const WizardConfig *cfg, const WizardManifest *manifest,
                         int git_init, WizardReport *report, char *err, size_t err_size);

int wizard_cli_main(int argc, char **argv);

#endif
