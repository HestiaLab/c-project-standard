#ifndef CPROJECT_EXAMPLE_H
#define CPROJECT_EXAMPLE_H

/**
 * @file example.h
 * @brief Example public API demonstrating the expected header layout.
 *
 * Replace this file with your first public API. Keep the include guard, export
 * macro, and `extern "C"` guard pattern shown here.
 */

#include <cproject/compiler.h>
#include <cproject/export.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Version triple returned by cproject_version(). */
typedef struct cproject_version_t {
    int major; /**< Major version. */
    int minor; /**< Minor version. */
    int patch; /**< Patch version. */
} cproject_version_t;

/**
 * @brief Return the project version declared in CMake.
 * @return The project version.
 */
CPROJECT_API cproject_version_t cproject_version(void);

/**
 * @brief Deprecated alias for cproject_version().
 * @deprecated Use cproject_version() instead.
 * @return The project version.
 */
CPROJECT_API CPROJECT_DEPRECATED("Use cproject_version() instead") cproject_version_t
    cproject_get_version(void);

/**
 * @brief Add two integers with overflow checking.
 * @param left First operand.
 * @param right Second operand.
 * @param[out] out_value Receives the sum on success. Must not be NULL.
 * @return 1 on success, 0 when out_value is NULL or the sum would overflow.
 */
CPROJECT_API int cproject_add_checked(int left, int right, int *out_value);

/**
 * @brief Return a human-readable platform name for the current target.
 * @return One of "windows", "macos", "linux", or "unknown".
 */
CPROJECT_API const char *cproject_platform_name(void);

#ifdef __cplusplus
}
#endif

#endif /* CPROJECT_EXAMPLE_H */
