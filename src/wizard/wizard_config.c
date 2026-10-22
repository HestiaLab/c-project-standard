#include "wizard.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int is_ascii_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int is_ascii_digit(char c) {
    return c >= '0' && c <= '9';
}

static int is_separator(char c) {
    return c == '-' || c == '_' || c == ' ';
}

static char ascii_upper(char c) {
    return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

static char ascii_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

static void set_error(char *err, size_t err_size, const char *message) {
    if (!err || err_size == 0)
        return;
    snprintf(err, err_size, "%s", message);
}

void wizard_config_defaults(WizardConfig *cfg) {
    if (!cfg)
        return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->features[WIZARD_FEATURE_INSTALLABLE] = 1;
    cfg->features[WIZARD_FEATURE_SUBPROJECT] = 1;
    cfg->features[WIZARD_FEATURE_CI] = 1;
    cfg->features[WIZARD_FEATURE_AI_AGENT] = 0;
    cfg->features[WIZARD_FEATURE_CLI] = 1;
}

void wizard_config_derive(WizardConfig *cfg) {
    size_t i;
    if (!cfg)
        return;
    for (i = 0; cfg->name[i] && i + 1 < sizeof(cfg->lower); i++) {
        cfg->lower[i] = ascii_lower(cfg->name[i]);
        cfg->upper[i] = ascii_upper(cfg->name[i]);
    }
    cfg->lower[i] = '\0';
    cfg->upper[i] = '\0';
    if (cfg->description[0] == '\0')
        snprintf(cfg->description, sizeof(cfg->description), "A C library");
    if (cfg->version[0] == '\0')
        snprintf(cfg->version, sizeof(cfg->version), "0.1.0");
    if (cfg->license[0] == '\0')
        snprintf(cfg->license, sizeof(cfg->license), "MIT");
    if (cfg->author[0] == '\0')
        snprintf(cfg->author, sizeof(cfg->author), "%s contributors", cfg->name);
}

int wizard_normalize_name(const char *input, char *out, size_t out_size, char *err,
                          size_t err_size) {
    size_t in_len;
    size_t write = 0;
    int in_segment = 0;
    int previous_separator = 0;

    if (err && err_size > 0)
        err[0] = '\0';
    if (!input || !out || out_size == 0) {
        set_error(err, err_size, "名称不能为空。");
        return 0;
    }
    in_len = strlen(input);
    if (in_len == 0) {
        set_error(err, err_size, "名称不能为空。");
        return 0;
    }
    if (is_ascii_digit(input[0])) {
        set_error(err, err_size, "名称不能以数字开头。");
        return 0;
    }
    if (!is_ascii_alpha(input[0])) {
        set_error(err, err_size, "名称必须以 ASCII 字母开头。");
        return 0;
    }

    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = (unsigned char)input[i];
        if (c >= 0x80) {
            set_error(err, err_size, "名称只能包含 ASCII 字母、数字、空格、'-' 或 '_'。");
            return 0;
        }
        if (is_separator((char)c)) {
            if (i == 0 || i + 1 == in_len || previous_separator) {
                set_error(err, err_size, "名称不能包含连续或首尾分隔符。");
                return 0;
            }
            previous_separator = 1;
            continue;
        }
        previous_separator = 0;
        if (!is_ascii_alpha((char)c) && !is_ascii_digit((char)c)) {
            set_error(err, err_size, "名称只能包含 ASCII 字母、数字、空格、'-' 或 '_'。");
            return 0;
        }
    }

    in_segment = 0;
    for (size_t i = 0; i <= in_len; i++) {
        char c = input[i];
        if (i == in_len || is_separator(c)) {
            in_segment = 0;
            continue;
        }
        if (!in_segment) {
            size_t end = i;
            int all_upper = 1;
            int has_alpha = 0;
            while (end < in_len && !is_separator(input[end])) {
                if (is_ascii_alpha(input[end])) {
                    has_alpha = 1;
                    if (input[end] != ascii_upper(input[end]))
                        all_upper = 0;
                }
                end++;
            }
            if (write + 1 >= out_size) {
                set_error(err, err_size, "名称过长。");
                return 0;
            }
            out[write++] = ascii_upper(c);
            in_segment = 1;
            if (all_upper && has_alpha) {
                for (size_t j = i + 1; j < end; j++) {
                    if (write + 1 >= out_size) {
                        set_error(err, err_size, "名称过长。");
                        return 0;
                    }
                    out[write++] = ascii_lower(input[j]);
                }
                i = end - 1;
            }
            continue;
        }
        if (write + 1 >= out_size) {
            set_error(err, err_size, "名称过长。");
            return 0;
        }
        out[write++] = c;
    }
    out[write] = '\0';
    if (!is_ascii_alpha(out[0])) {
        set_error(err, err_size, "名称必须包含字母。");
        return 0;
    }
    return 1;
}

int wizard_validate_version(const char *version) {
    int dots = 0;
    int digits = 0;
    if (!version || !version[0])
        return 0;
    for (size_t i = 0; version[i]; i++) {
        if (is_ascii_digit(version[i])) {
            digits++;
            continue;
        }
        if (version[i] == '.') {
            if (digits == 0)
                return 0;
            dots++;
            digits = 0;
            if (dots > 2)
                return 0;
            continue;
        }
        return 0;
    }
    return dots == 2 && digits > 0;
}

int wizard_feature_from_name(const char *name) {
    if (!name)
        return -1;
    if (strcmp(name, "installable") == 0)
        return WIZARD_FEATURE_INSTALLABLE;
    if (strcmp(name, "subproject") == 0)
        return WIZARD_FEATURE_SUBPROJECT;
    if (strcmp(name, "ci") == 0)
        return WIZARD_FEATURE_CI;
    if (strcmp(name, "ai-agent") == 0 || strcmp(name, "ai") == 0)
        return WIZARD_FEATURE_AI_AGENT;
    if (strcmp(name, "cli") == 0)
        return WIZARD_FEATURE_CLI;
    return -1;
}

const char *wizard_feature_name(int feature) {
    switch (feature) {
    case WIZARD_FEATURE_INSTALLABLE:
        return "installable";
    case WIZARD_FEATURE_SUBPROJECT:
        return "subproject";
    case WIZARD_FEATURE_CI:
        return "ci";
    case WIZARD_FEATURE_AI_AGENT:
        return "ai-agent";
    case WIZARD_FEATURE_CLI:
        return "cli";
    default:
        return "";
    }
}

int wizard_parse_feature_option(const char *arg, int *feature, int *enabled) {
    const char *name;
    int index;
    if (!arg || !feature || !enabled)
        return 0;
    if (strncmp(arg, "--with-", 7) == 0) {
        name = arg + 7;
        *enabled = 1;
    } else if (strncmp(arg, "--no-", 5) == 0) {
        name = arg + 5;
        *enabled = 0;
    } else {
        return 0;
    }
    index = wizard_feature_from_name(name);
    if (index < 0)
        return 0;
    *feature = index;
    return 1;
}
