#ifndef CPROJECT_EXAMPLE_H
#define CPROJECT_EXAMPLE_H

typedef struct CProjectVersion {
    int major;
    int minor;
    int patch;
} CProjectVersion;

CProjectVersion cproject_version(void);
int cproject_add_checked(int left, int right, int* out_value);

#endif /* CPROJECT_EXAMPLE_H */
