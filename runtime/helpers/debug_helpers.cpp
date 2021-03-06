/*
 * Copyright (C) 2017-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/helpers/debug_helpers.h"

#include "runtime/os_interface/debug_settings_manager.h"

#include <assert.h>
#include <cstdio>

namespace OCLRT {
void debugBreak(int line, const char *file) {
    if (DebugManager.flags.EnableDebugBreak.get()) {
        printf("Assert was called at %d line in file:\n%s\n", line, file);
        assert(false);
    }
}
void abortUnrecoverable(int line, const char *file) {
    printf("Abort was called at %d line in file:\n%s\n", line, file);
    abortExecution();
}
} // namespace OCLRT