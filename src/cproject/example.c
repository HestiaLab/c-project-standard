#include <cproject/example.h>

#include <limits.h>

CProjectVersion cproject_version(void)
{
    CProjectVersion version = {0, 1, 0};
    return version;
}

int cproject_add_checked(int left, int right, int* out_value)
{
    if (!out_value) {
        return 0;
    }

    if ((right > 0 && left > INT_MAX - right) ||
        (right < 0 && left < INT_MIN - right)) {
        return 0;
    }

    *out_value = left + right;
    return 1;
}
