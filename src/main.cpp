#include "app.h"
#include <cstdio>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    printf("Crynos Executor v2.0 - Advanced Roblox Script Executor\n");
    printf("Built with C++ / Dear ImGui / OpenGL\n");
    printf("===================================================\n\n");

    App app;

    if (!app.init()) {
        fprintf(stderr, "Failed to initialize Crynos Executor\n");
        return 1;
    }

    printf("Initialization complete. Starting main loop...\n");
    app.run();
    printf("Shutting down...\n");

    return 0;
}
