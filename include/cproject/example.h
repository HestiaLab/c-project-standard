#ifndef CPROJECT_EXAMPLE_H
#define CPROJECT_EXAMPLE_H

#include <cproject/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CProjectVersion {
    int major;
    int minor;
    int patch;
} CProjectVersion;

/* Returns the library version compiled into this build. */
CPROJECT_API CProjectVersion cproject_version(void);

/*
 * Adds two integers and writes the result to out_value.
 * Returns 1 on success and 0 when out_value is NULL or the sum would overflow.
 */
CPROJECT_API int cproject_add_checked(int left, int right, int *out_value);

#ifdef __cplusplus
}
#endif

#endif /* CPROJECT_EXAMPLE_H */
