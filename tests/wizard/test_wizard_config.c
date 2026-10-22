#include <cproject/example.h>

#include "unity.h"
#include "wizard.h"

#include <string.h>

void setUp(void) {}
void tearDown(void) {}

static void test_normalizes_common_name_forms(void) {
    char output[WIZARD_MAX_NAME];
    char error[128];
    const char *inputs[] = {"MyLib", "my_lib", "my-lib", "my lib", "myLib", "MY_LIB"};
    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++) {
        TEST_ASSERT_TRUE(
            wizard_normalize_name(inputs[i], output, sizeof(output), error, sizeof(error)));
        TEST_ASSERT_EQUAL_STRING("MyLib", output);
    }
}

static void test_rejects_invalid_names(void) {
    char output[WIZARD_MAX_NAME];
    char error[128];
    const char *inputs[] = {"",       "1Lib",     "my.lib", "mylib/", " mylib",
                            "mylib-", "mylib--x", "mylib!", "mylibé"};
    for (size_t i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++)
        TEST_ASSERT_FALSE(
            wizard_normalize_name(inputs[i], output, sizeof(output), error, sizeof(error)));
}

static void test_derives_namespace_and_defaults(void) {
    WizardConfig config;
    wizard_config_defaults(&config);
    strcpy(config.name, "MyLib");
    wizard_config_derive(&config);
    TEST_ASSERT_EQUAL_STRING("mylib", config.lower);
    TEST_ASSERT_EQUAL_STRING("MYLIB", config.upper);
    TEST_ASSERT_EQUAL_STRING("A C library", config.description);
    TEST_ASSERT_EQUAL_STRING("0.1.0", config.version);
    TEST_ASSERT_EQUAL_STRING("MIT", config.license);
    TEST_ASSERT_EQUAL_STRING("MyLib contributors", config.author);
}

static void test_validates_versions(void) {
    TEST_ASSERT_TRUE(wizard_validate_version("0.1.0"));
    TEST_ASSERT_TRUE(wizard_validate_version("12.34.56"));
    TEST_ASSERT_FALSE(wizard_validate_version("1.2"));
    TEST_ASSERT_FALSE(wizard_validate_version("1.2.3.4"));
    TEST_ASSERT_FALSE(wizard_validate_version("v1.2.3"));
    TEST_ASSERT_FALSE(wizard_validate_version("1.2.x"));
    TEST_ASSERT_FALSE(wizard_validate_version(""));
}

static void test_parses_feature_flags(void) {
    int feature = -1;
    int enabled = 0;
    TEST_ASSERT_TRUE(wizard_parse_feature_option("--with-ci", &feature, &enabled));
    TEST_ASSERT_EQUAL_INT(WIZARD_FEATURE_CI, feature);
    TEST_ASSERT_EQUAL_INT(1, enabled);
    TEST_ASSERT_TRUE(wizard_parse_feature_option("--no-ai-agent", &feature, &enabled));
    TEST_ASSERT_EQUAL_INT(WIZARD_FEATURE_AI_AGENT, feature);
    TEST_ASSERT_EQUAL_INT(0, enabled);
    TEST_ASSERT_FALSE(wizard_parse_feature_option("--no-shared", &feature, &enabled));
    TEST_ASSERT_FALSE(wizard_parse_feature_option("--with-unknown", &feature, &enabled));
}

static void test_manifest_matches_directories_and_files(void) {
    char *entries[] = {"src/wizard", "cmake/PackagingMSI.cmake", ".github/workflows/release.yml"};
    WizardManifest manifest = {entries, 3};
    TEST_ASSERT_TRUE(wizard_manifest_matches(&manifest, "src/wizard/wizard_cli.c"));
    TEST_ASSERT_TRUE(wizard_manifest_matches(&manifest, "src\\wizard\\wizard_util.c"));
    TEST_ASSERT_TRUE(wizard_manifest_matches(&manifest, "cmake/PackagingMSI.cmake"));
    TEST_ASSERT_TRUE(wizard_manifest_matches(&manifest, "cmake/PackagingMSI.cmake/"));
    TEST_ASSERT_FALSE(wizard_manifest_matches(&manifest, "cmake/Packaging.cmake"));
    TEST_ASSERT_FALSE(wizard_manifest_matches(&manifest, "src/wizardly/file.c"));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_normalizes_common_name_forms);
    RUN_TEST(test_rejects_invalid_names);
    RUN_TEST(test_derives_namespace_and_defaults);
    RUN_TEST(test_validates_versions);
    RUN_TEST(test_parses_feature_flags);
    RUN_TEST(test_manifest_matches_directories_and_files);
    return UNITY_END();
}
