#include <cproject/example.h>
#include <cproject/version.h>

#include <limits.h>

/* SETUP: replace with your library implementation.
 * Wire public APIs declared in include/<project>/example.h */
cproject_version_t cproject_version(void) {
    cproject_version_t version = {
        CPROJECT_VERSION_MAJOR,
        CPROJECT_VERSION_MINOR,
        CPROJECT_VERSION_PATCH,
    };
    return version;
}

cproject_version_t cproject_get_version(void) {
    return cproject_version();
}

int cproject_add_checked(int left, int right, int *out_value) {
    if (!out_value) {
        return 0;
    }

    if ((right > 0 && left > INT_MAX - right) || (right < 0 && left < INT_MIN - right)) {
        return 0;
    }

    *out_value = left + right;
    return 1;
}

const char *cproject_platform_name(void) {
#if CPROJECT_PLATFORM_WINDOWS
    return "windows";
#elif CPROJECT_PLATFORM_MACOS
    return "macos";
#elif CPROJECT_PLATFORM_LINUX
    return "linux";
#else
    return "unknown";
#endif
}
