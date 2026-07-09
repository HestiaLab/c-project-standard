#ifndef CPROJECT_EXAMPLE_H
#define CPROJECT_EXAMPLE_H

#include <cproject/compiler.h>
#include <cproject/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CProjectVersion {
    int major;
    int minor;
    int patch;
} CProjectVersion;

/* SETUP: replace with your first public API header.
 * Keep the include guard, export macro, and extern "C" guard pattern. */
CPROJECT_API CProjectVersion cproject_version(void);

/*
 * Deprecated alias for cproject_version(). Use cproject_version() instead.
 * This declaration exists to demonstrate the CPROJECT_DEPRECATED annotation.
 */
CPROJECT_API CPROJECT_DEPRECATED("Use cproject_version() instead") CProjectVersion
    cproject_get_version(void);

/*
 * Adds two integers and writes the result to out_value.
 * Returns 1 on success and 0 when out_value is NULL or the sum would overflow.
 */
CPROJECT_API int cproject_add_checked(int left, int right, int *out_value);

/* Returns a human-readable platform name for the current target. */
CPROJECT_API const char *cproject_platform_name(void);

#ifdef __cplusplus
}
#endif

#endif /* CPROJECT_EXAMPLE_H */
