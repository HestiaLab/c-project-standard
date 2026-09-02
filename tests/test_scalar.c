/* SETUP: width and value guarantees for <cproject/scalar.h>. */
#include <cproject/scalar.h>

#include "unity.h"

/* Compile-time width guarantees. These fail the build if a typedef drifts to a
 * platform-dependent width (for example, ulong mapped to native
 * `unsigned long`, which is 64-bit on LP64 Linux). */
_Static_assert(sizeof(cproject_uchar_t) == 1, "cproject_uchar_t must be 8-bit");
_Static_assert(sizeof(cproject_ushort_t) == 2, "cproject_ushort_t must be 16-bit");
_Static_assert(sizeof(cproject_ulong_t) == 4, "cproject_ulong_t must be 32-bit");
_Static_assert(sizeof(cproject_short_t) == 2, "cproject_short_t must be 16-bit");
_Static_assert(sizeof(cproject_long_t) == 4, "cproject_long_t must be 32-bit");
_Static_assert(sizeof(cproject_int64_t) == 8, "cproject_int64_t must be 64-bit");
_Static_assert(sizeof(cproject_uint64_t) == 8, "cproject_uint64_t must be 64-bit");
_Static_assert(sizeof(cproject_float_t) == sizeof(float), "cproject_float_t must be float");
_Static_assert(sizeof(cproject_double_t) == sizeof(double), "cproject_double_t must be double");

/* Unity calls setUp() before each test and tearDown() after. Both are required
 * even when empty. */
void setUp(void) {}

void tearDown(void) {}

static void test_scalar_max_macros(void) {
    TEST_ASSERT_EQUAL_UINT8(UINT8_MAX, CPROJECT_UCHAR_MAX);
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, CPROJECT_USHORT_MAX);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, CPROJECT_ULONG_MAX);
    TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, CPROJECT_UINT64_MAX);
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, CPROJECT_SHORT_MIN);
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, CPROJECT_SHORT_MAX);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, CPROJECT_LONG_MIN);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, CPROJECT_LONG_MAX);
    TEST_ASSERT_EQUAL_INT64(INT64_MIN, CPROJECT_INT64_MIN);
    TEST_ASSERT_EQUAL_INT64(INT64_MAX, CPROJECT_INT64_MAX);
}

static void test_scalar_bool(void) {
    cproject_bool_t yes = true;
    cproject_bool_t no = false;
    TEST_ASSERT_TRUE(yes);
    TEST_ASSERT_FALSE(no);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_scalar_max_macros);
    RUN_TEST(test_scalar_bool);
    return UNITY_END();
}
