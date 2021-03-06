/*
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/os_interface/windows/os_interface.h"

#include "runtime/memory_manager/memory_constants.h"
namespace OCLRT {

bool OSInterface::osEnableLocalMemory = true && is64bit;

} // namespace OCLRT
