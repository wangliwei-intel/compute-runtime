/*
 * Copyright (C) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

#include "runtime/aub_mem_dump/aub_mem_dump.h"
#include "runtime/gmm_helper/gmm_helper.h"
#include "runtime/gmm_helper/resource_info.h"
#include "runtime/helpers/options.h"
#include "runtime/memory_manager/graphics_allocation.h"
#include "runtime/os_interface/debug_settings_manager.h"

using namespace OCLRT;

namespace AubAllocDump {

enum DumpFormat {
    NONE,
    BUFFER_BIN,
    BUFFER_TRE,
    IMAGE_BMP,
    IMAGE_TRE,
};

inline bool isWritableBuffer(GraphicsAllocation &gfxAllocation) {
    return (gfxAllocation.getAllocationType() == GraphicsAllocation::AllocationType::BUFFER ||
            gfxAllocation.getAllocationType() == GraphicsAllocation::AllocationType::BUFFER_COMPRESSED ||
            gfxAllocation.getAllocationType() == GraphicsAllocation::AllocationType::BUFFER_HOST_MEMORY ||
            gfxAllocation.getAllocationType() == GraphicsAllocation::AllocationType::EXTERNAL_HOST_PTR) &&
           gfxAllocation.isMemObjectsAllocationWithWritableFlags();
}

inline bool isWritableImage(GraphicsAllocation &gfxAllocation) {
    return (gfxAllocation.getAllocationType() == GraphicsAllocation::AllocationType::IMAGE) &&
           gfxAllocation.isMemObjectsAllocationWithWritableFlags();
}

inline DumpFormat getDumpFormat(GraphicsAllocation &gfxAllocation) {
    auto dumpBufferFormat = DebugManager.flags.AUBDumpBufferFormat.get();
    auto dumpImageFormat = DebugManager.flags.AUBDumpImageFormat.get();
    auto isDumpableBuffer = isWritableBuffer(gfxAllocation);
    auto isDumpableImage = isWritableImage(gfxAllocation);
    auto dumpFormat = DumpFormat::NONE;

    if (isDumpableBuffer) {
        if (0 == dumpBufferFormat.compare("BIN")) {
            dumpFormat = DumpFormat::BUFFER_BIN;
        } else if (0 == dumpBufferFormat.compare("TRE")) {
            dumpFormat = DumpFormat::BUFFER_TRE;
        }
    } else if (isDumpableImage) {
        if (0 == dumpImageFormat.compare("BMP")) {
            dumpFormat = DumpFormat::IMAGE_BMP;
        } else if (0 == dumpImageFormat.compare("TRE")) {
            dumpFormat = DumpFormat::IMAGE_TRE;
        }
    }

    return dumpFormat;
}

template <typename GfxFamily>
uint32_t getImageSurfaceTypeFromGmmResourceType(GMM_RESOURCE_TYPE gmmResourceType);

template <typename GfxFamily>
void dumpBufferInBinFormat(GraphicsAllocation &gfxAllocation, AubMemDump::AubFileStream *stream, uint32_t context);

template <typename GfxFamily>
void dumpImageInBmpFormat(GraphicsAllocation &gfxAllocation, AubMemDump::AubFileStream *stream, uint32_t context);

template <typename GfxFamily>
void dumpBufferInTreFormat(GraphicsAllocation &gfxAllocation, AubMemDump::AubFileStream *stream, uint32_t context);

template <typename GfxFamily>
void dumpImageInTreFormat(GraphicsAllocation &gfxAllocation, AubMemDump::AubFileStream *stream, uint32_t context);

template <typename GfxFamily>
void dumpAllocation(DumpFormat dumpFormat, GraphicsAllocation &gfxAllocation, AubMemDump::AubFileStream *stream, uint32_t context);
} // namespace AubAllocDump
