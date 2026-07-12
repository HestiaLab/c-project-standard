#ifndef CPROJECT_COMPILER_H
#define CPROJECT_COMPILER_H

/*
 * Compiler and platform detection macros.
 *
 * This header provides a reference pattern for portable C code across the
 * four supported compiler families (MSVC, GCC, Clang, AppleClang) and three
 * target platforms (Windows, Linux, macOS).
 */

/* ---- Compiler family detection ---- */

#define CPROJECT_COMPILER_MSVC 0
#define CPROJECT_COMPILER_APPLECLANG 0
#define CPROJECT_COMPILER_CLANG 0
#define CPROJECT_COMPILER_GCC 0

#if defined(_MSC_VER)
#undef CPROJECT_COMPILER_MSVC
#define CPROJECT_COMPILER_MSVC 1
#elif defined(__clang__) && defined(__APPLE__)
#undef CPROJECT_COMPILER_APPLECLANG
#define CPROJECT_COMPILER_APPLECLANG 1
#elif defined(__clang__)
#undef CPROJECT_COMPILER_CLANG
#define CPROJECT_COMPILER_CLANG 1
#elif defined(__GNUC__)
#undef CPROJECT_COMPILER_GCC
#define CPROJECT_COMPILER_GCC 1
#else
#warning "Unknown compiler. Portable macros may not be available."
#endif

/* ---- Platform detection ---- */

#define CPROJECT_PLATFORM_WINDOWS 0
#define CPROJECT_PLATFORM_MACOS 0
#define CPROJECT_PLATFORM_LINUX 0

#if defined(_WIN32)
#undef CPROJECT_PLATFORM_WINDOWS
#define CPROJECT_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
#undef CPROJECT_PLATFORM_MACOS
#define CPROJECT_PLATFORM_MACOS 1
#elif defined(__linux__)
#undef CPROJECT_PLATFORM_LINUX
#define CPROJECT_PLATFORM_LINUX 1
#endif

/* ---- Portable function attributes ---- */

#if CPROJECT_COMPILER_MSVC
#define CPROJECT_NORETURN __declspec(noreturn)
#define CPROJECT_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#define CPROJECT_NORETURN __attribute__((__noreturn__))
#define CPROJECT_DEPRECATED(msg) __attribute__((__deprecated__(msg)))
#endif

#endif /* CPROJECT_COMPILER_H */
