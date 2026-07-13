#ifndef CPROJECT_EXPORT_H
#define CPROJECT_EXPORT_H

#if defined(_WIN32) && defined(CPROJECT_SHARED)
#if defined(CPROJECT_BUILDING_LIBRARY)
#define CPROJECT_API __declspec(dllexport)
#else
#define CPROJECT_API __declspec(dllimport)
#endif
#else
#define CPROJECT_API
#endif

#endif /* CPROJECT_EXPORT_H */
