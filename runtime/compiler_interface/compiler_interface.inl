/*
 * Copyright (C) 2017-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "runtime/helpers/validators.h"
#include "runtime/os_interface/os_library.h"

#include "cif/builtins/memory/buffer/buffer.h"
#include "cif/common/cif.h"
#include "cif/import/library_api.h"
#include "ocl_igc_interface/ocl_translation_output.h"

namespace OCLRT {
using CIFBuffer = CIF::Builtins::BufferSimple;

template <typename TranslationCtx>
inline CIF::RAII::UPtr_t<IGC::OclTranslationOutputTagOCL> translate(TranslationCtx *tCtx, CIFBuffer *src, CIFBuffer *options,
                                                                    CIFBuffer *internalOptions) {
    if (false == OCLRT::areNotNullptr(tCtx, src, options, internalOptions)) {
        return nullptr;
    }

    auto ret = tCtx->Translate(src, options, internalOptions, nullptr, 0);
    if (ret == nullptr) {
        return nullptr; // assume OOM or internal error
    }

    if ((ret->GetOutput() == nullptr) || (ret->GetBuildLog() == nullptr) || (ret->GetDebugData() == nullptr)) {
        return nullptr; // assume OOM or internal error
    }

    return ret;
}
template <typename TranslationCtx>
inline CIF::RAII::UPtr_t<IGC::OclTranslationOutputTagOCL> translate(TranslationCtx *tCtx, CIFBuffer *src, CIFBuffer *options,
                                                                    CIFBuffer *internalOptions, void *gtpinInit) {
    if (false == OCLRT::areNotNullptr(tCtx, src, options, internalOptions)) {
        return nullptr;
    }

    auto ret = tCtx->Translate(src, options, internalOptions, nullptr, 0, gtpinInit);
    if (ret == nullptr) {
        return nullptr; // assume OOM or internal error
    }

    if ((ret->GetOutput() == nullptr) || (ret->GetBuildLog() == nullptr) || (ret->GetDebugData() == nullptr)) {
        return nullptr; // assume OOM or internal error
    }

    return ret;
}

CIF::CIFMain *createMainNoSanitize(CIF::CreateCIFMainFunc_t createFunc);

template <template <CIF::Version_t> class EntryPointT>
inline bool loadCompiler(const char *libName, std::unique_ptr<OsLibrary> &outLib,
                         CIF::RAII::UPtr_t<CIF::CIFMain> &outLibMain) {
    auto lib = std::unique_ptr<OsLibrary>(OsLibrary::load(libName));
    if (lib == nullptr) {
        DEBUG_BREAK_IF(true); // could not load library
        return false;
    }

    auto createMain = reinterpret_cast<CIF::CreateCIFMainFunc_t>(lib->getProcAddress(CIF::CreateCIFMainFuncName));
    UNRECOVERABLE_IF(createMain == nullptr); // invalid compiler library

    auto main = CIF::RAII::UPtr(createMainNoSanitize(createMain));
    if (main == nullptr) {
        DEBUG_BREAK_IF(true); // could not create main entry point
        return false;
    }

    if (false == main->IsCompatible<EntryPointT>()) {
        DEBUG_BREAK_IF(true); // given compiler library is not compatible
        return false;
    }

    outLib = std::move(lib);
    outLibMain = std::move(main);

    return true;
}

} // namespace OCLRT
