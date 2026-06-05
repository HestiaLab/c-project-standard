#include <cproject/example.h>

#include <limits.h>
#include <stdio.h>

#define EXPECT_TRUE(expr)                                                     \
    do {                                                                      \
        if (!(expr)) {                                                        \
            fprintf(stderr, "EXPECT_TRUE failed: %s:%d: %s\n",               \
                    __FILE__, __LINE__, #expr);                              \
            return 1;                                                         \
        }                                                                     \
    } while (0)

#define EXPECT_INT_EQ(actual, expected)                                       \
    do {                                                                      \
        int actual_value = (actual);                                          \
        int expected_value = (expected);                                      \
        if (actual_value != expected_value) {                                 \
            fprintf(stderr, "EXPECT_INT_EQ failed: %s:%d: %d != %d\n",        \
                    __FILE__, __LINE__, actual_value, expected_value);        \
            return 1;                                                         \
        }                                                                     \
    } while (0)

static int test_version(void)
{
    CProjectVersion version = cproject_version();
    EXPECT_INT_EQ(version.major, 0);
    EXPECT_INT_EQ(version.minor, 1);
    EXPECT_INT_EQ(version.patch, 0);
    return 0;
}

static int test_checked_add(void)
{
    int value = 0;

    EXPECT_TRUE(cproject_add_checked(2, 3, &value));
    EXPECT_INT_EQ(value, 5);
    EXPECT_TRUE(!cproject_add_checked(INT_MAX, 1, &value));
    EXPECT_TRUE(!cproject_add_checked(1, 1, NULL));
    return 0;
}

int main(void)
{
    if (test_version()) {
        return 1;
    }
    if (test_checked_add()) {
        return 1;
    }

    printf("[PASS] example tests\n");
    return 0;
}
