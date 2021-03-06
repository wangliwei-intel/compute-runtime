/*
 * Copyright (C) 2017-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/mem_obj/buffer.inl"

#include "hw_cmds.h"

namespace OCLRT {

typedef SKLFamily Family;
static auto gfxCore = IGFX_GEN9_CORE;

#include "runtime/mem_obj/buffer_factory_init.inl"
} // namespace OCLRT
