// SPDX-License-Identifier: GPL-3.0-or-later
// Runs in its own process. Never inject this probe into keystore2.
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include "plti.h"

static int (*original_ioctl)(int, int, ...);

static int probe_ioctl(int fd, int request, ...) {
    va_list args;
    va_start(args, request);
    void *argument = va_arg(args, void *);
    va_end(args);
    return original_ioctl(fd, request, argument);
}

int main(void) {
    // The linker resolves libbinder.so to libcore_bundle.so on affected Canary.
    void *binder = dlopen("libbinder.so", RTLD_NOW);
    if (!binder) {
        fprintf(stderr, "Cannot load Binder: %s\n", dlerror());
        return 1;
    }
    struct plti ctx;
    plti_init(&ctx);
    const char *target = "libbinder.so";
    if (!plti_add_lib(&ctx, target)) {
        target = "libcore_bundle.so";
        if (!plti_add_lib(&ctx, target)) {
            fprintf(stderr, "No loaded Binder library found\n");
            return 2;
        }
    }
    printf("Binder target: %s\n", target);
    if (!plti_add_hook(&ctx, target, "ioctl", (void *)probe_ioctl,
                       (void **)&original_ioctl) || !original_ioctl) {
        fprintf(stderr, "Could not install ioctl hook\n");
        return 3;
    }
    printf("ioctl PLT hook installed; original callback is available\n");
    if (!plti_remove_hook(&ctx, target, "ioctl", (void **)&original_ioctl)) {
        fprintf(stderr, "Could not remove ioctl hook\n");
        return 4;
    }
    plti_deinit(&ctx);
    dlclose(binder);
    puts("ioctl hook removed; isolated probe passed");
    return 0;
}
