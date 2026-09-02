/* SETUP: replace with tests for your public API.
 * Keep the Unity test pattern: each test function registers with RUN_TEST(),
 * main() runs UNITY_BEGIN() / UNITY_END(). */
#include <cproject/example.h>
#include <cproject/version.h>

#include "unity.h"

#include <limits.h>

/* Unity calls setUp() before each test and tearDown() after. Both are required
 * even when empty. */
void setUp(void) {}

void tearDown(void) {}

static void test_version(void) {
    cproject_version_t version = cproject_version();
    TEST_ASSERT_EQUAL_INT(CPROJECT_VERSION_MAJOR, version.major);
    TEST_ASSERT_EQUAL_INT(CPROJECT_VERSION_MINOR, version.minor);
    TEST_ASSERT_EQUAL_INT(CPROJECT_VERSION_PATCH, version.patch);
}

static void test_get_version(void) {
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    cproject_version_t version = cproject_get_version();
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
    TEST_ASSERT_EQUAL_INT(CPROJECT_VERSION_MAJOR, version.major);
    TEST_ASSERT_EQUAL_INT(CPROJECT_VERSION_MINOR, version.minor);
    TEST_ASSERT_EQUAL_INT(CPROJECT_VERSION_PATCH, version.patch);
}

static void test_checked_add(void) {
    int value = 0;

    TEST_ASSERT_TRUE(cproject_add_checked(2, 3, &value));
    TEST_ASSERT_EQUAL_INT(5, value);

    TEST_ASSERT_FALSE(cproject_add_checked(INT_MAX, 1, &value));
    TEST_ASSERT_FALSE(cproject_add_checked(INT_MIN, -1, &value));
    TEST_ASSERT_FALSE(cproject_add_checked(1, 1, NULL));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_version);
    RUN_TEST(test_get_version);
    RUN_TEST(test_checked_add);
    return UNITY_END();
}
