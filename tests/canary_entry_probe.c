// SPDX-License-Identifier: GPL-3.0-or-later
// Exercise the full candidate entry point in a disposable, isolated process.
#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) return 1;
    void *module = dlopen(argv[1], RTLD_NOW);
    if (!module) {
        fprintf(stderr, "Module load failed: %s\n", dlerror());
        return 2;
    }
    bool (*entry)(void *) = (bool (*)(void *))dlsym(module, "entry");
    if (!entry || !entry(module)) {
        fputs("Candidate entry failed\n", stderr);
        return 3;
    }
    puts("Candidate module entry passed in isolated process");
    fflush(stdout);
    // Do not unload a library while its hook is installed. Process exit removes it.
    _exit(0);
}
