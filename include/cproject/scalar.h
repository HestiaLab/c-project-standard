#ifndef CPROJECT_SCALAR_H
#define CPROJECT_SCALAR_H

/**
 * @file scalar.h
 * @brief Platform-stable scalar type aliases.
 *
 * Fixed-width aliases over <stdint.h> and <stdbool.h> so public APIs do not
 * depend on the platform width of the C built-in types. In particular,
 * `unsigned long` is 32-bit on Windows (LLP64) but 64-bit on Linux (LP64);
 * `cproject_ulong_t` pins the 32-bit Windows convention so a field means the
 * same width on every platform.
 *
 * The aliases are transparent `typedef`s; they do not add strong-type checks.
 * Use the exact-width stdint names (`uint32_t`, ...) when a specific width is
 * the point, and these aliases when the intent is the conventional name.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Unsigned 8-bit integer, 0..255. */
typedef uint8_t cproject_uchar_t;

/** @brief Unsigned 16-bit integer, 0..65535. */
typedef uint16_t cproject_ushort_t;

/** @brief Unsigned 32-bit integer, 0..4294967295 (Windows `unsigned long`). */
typedef uint32_t cproject_ulong_t;

/** @brief Signed 16-bit integer. */
typedef int16_t cproject_short_t;

/** @brief Signed 32-bit integer (Windows `long`). */
typedef int32_t cproject_long_t;

/** @brief Signed 64-bit integer. */
typedef int64_t cproject_int64_t;

/** @brief Unsigned 64-bit integer. */
typedef uint64_t cproject_uint64_t;

/** @brief IEEE-754 single-precision float. */
typedef float cproject_float_t;

/** @brief IEEE-754 double-precision float. */
typedef double cproject_double_t;

/** @brief Boolean, `true` or `false`. */
typedef bool cproject_bool_t;

/** @brief Maximum value of cproject_uchar_t. */
#define CPROJECT_UCHAR_MAX UINT8_MAX
/** @brief Maximum value of cproject_ushort_t. */
#define CPROJECT_USHORT_MAX UINT16_MAX
/** @brief Maximum value of cproject_ulong_t; differs from platform ULONG_MAX. */
#define CPROJECT_ULONG_MAX UINT32_MAX
/** @brief Maximum value of cproject_uint64_t. */
#define CPROJECT_UINT64_MAX UINT64_MAX
/** @brief Minimum value of cproject_short_t. */
#define CPROJECT_SHORT_MIN INT16_MIN
/** @brief Maximum value of cproject_short_t. */
#define CPROJECT_SHORT_MAX INT16_MAX
/** @brief Minimum value of cproject_long_t. */
#define CPROJECT_LONG_MIN INT32_MIN
/** @brief Maximum value of cproject_long_t. */
#define CPROJECT_LONG_MAX INT32_MAX
/** @brief Minimum value of cproject_int64_t. */
#define CPROJECT_INT64_MIN INT64_MIN
/** @brief Maximum value of cproject_int64_t. */
#define CPROJECT_INT64_MAX INT64_MAX

#ifdef __cplusplus
}
#endif

#endif /* CPROJECT_SCALAR_H */
