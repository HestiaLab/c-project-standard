#include <cproject/example.h>
#include <cproject/version.h>

#include <limits.h>
#include <stdio.h>

#define EXPECT_TRUE(expr)                                                                          \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            fputs("EXPECT_TRUE failed: " #expr "\n", stderr);                                      \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

#define EXPECT_INT_EQ(actual, expected)                                                            \
    do {                                                                                           \
        int actual_value = (actual);                                                               \
        int expected_value = (expected);                                                           \
        if (actual_value != expected_value) {                                                      \
            fputs("EXPECT_INT_EQ failed: " #actual " != " #expected "\n", stderr);                 \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

static int test_version(void) {
    CProjectVersion version = cproject_version();
    EXPECT_INT_EQ(version.major, CPROJECT_VERSION_MAJOR);
    EXPECT_INT_EQ(version.minor, CPROJECT_VERSION_MINOR);
    EXPECT_INT_EQ(version.patch, CPROJECT_VERSION_PATCH);
    return 0;
}

static int test_checked_add(void) {
    int value = 0;

    EXPECT_TRUE(cproject_add_checked(2, 3, &value));
    EXPECT_INT_EQ(value, 5);
    EXPECT_TRUE(!cproject_add_checked(INT_MAX, 1, &value));
    EXPECT_TRUE(!cproject_add_checked(INT_MIN, -1, &value));
    EXPECT_TRUE(!cproject_add_checked(1, 1, NULL));
    return 0;
}

int main(void) {
    if (test_version()) {
        return 1;
    }
    if (test_checked_add()) {
        return 1;
    }

    printf("[PASS] example tests\n");
    return 0;
}
