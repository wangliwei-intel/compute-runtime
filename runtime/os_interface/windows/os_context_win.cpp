/*
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/os_interface/windows/os_context_win.h"

#include "runtime/os_interface/windows/os_interface.h"
#include "runtime/os_interface/windows/wddm/wddm.h"
#include "runtime/os_interface/windows/wddm/wddm_interface.h"

namespace OCLRT {

OsContext *OsContext::create(OSInterface *osInterface, uint32_t contextId, DeviceBitfield deviceBitfield,
                             EngineType engineType, PreemptionMode preemptionMode, bool lowPriority) {
    if (osInterface) {
        return new OsContextWin(*osInterface->get()->getWddm(), contextId, deviceBitfield, engineType, preemptionMode, lowPriority);
    }
    return new OsContext(contextId, deviceBitfield, engineType, preemptionMode, lowPriority);
}

OsContextWin::OsContextWin(Wddm &wddm, uint32_t contextId, DeviceBitfield deviceBitfield,
                           EngineType engineType, PreemptionMode preemptionMode, bool lowPriority)
    : OsContext(contextId, deviceBitfield, engineType, preemptionMode, lowPriority), wddm(wddm), residencyController(wddm, contextId) {

    UNRECOVERABLE_IF(!wddm.isInitialized());

    auto wddmInterface = wddm.getWddmInterface();
    if (!wddm.createContext(*this)) {
        return;
    }
    if (wddmInterface->hwQueuesSupported()) {
        if (!wddmInterface->createHwQueue(*this)) {
            return;
        }
    }
    initialized = wddmInterface->createMonitoredFence(residencyController);
    residencyController.registerCallback();
};

OsContextWin::~OsContextWin() {
    wddm.getWddmInterface()->destroyHwQueue(hwQueueHandle);
    wddm.destroyContext(wddmContextHandle);
}

} // namespace OCLRT
