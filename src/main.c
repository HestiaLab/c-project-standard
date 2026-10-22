/*
 * C Project Standard — project setup wizard entry point.
 *
 * The wizard implementation lives under src/wizard/ and is linked only into
 * the template build. Generated projects receive a project-specific stub.
 */

#include "wizard/wizard.h"

#ifdef _WIN32
#ifndef CP_UTF8
#define CP_UTF8 65001
#endif
#endif

int main(int argc, char *argv[]) {
#ifdef _WIN32
    {
        typedef unsigned int UINT;
        __declspec(dllimport) int __stdcall SetConsoleOutputCP(UINT);
        __declspec(dllimport) int __stdcall SetConsoleCP(UINT);
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }
#endif
    return wizard_cli_main(argc, argv);
}
