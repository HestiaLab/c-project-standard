#include <cproject/example.h>

int main(void)
{
    CProjectVersion version = cproject_version();
    int value = 0;

    if (version.major < 0 || version.minor < 0 || version.patch < 0) {
        return 1;
    }

    if (!cproject_add_checked(20, 22, &value) || value != 42) {
        return 1;
    }

    return 0;
}
