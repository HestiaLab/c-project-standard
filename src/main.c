#include <cproject/example.h>

#include <stdio.h>

int main(void) {
    CProjectVersion version = cproject_version();
    printf("CProjectStandard example %d.%d.%d\n", version.major, version.minor, version.patch);
    return 0;
}
