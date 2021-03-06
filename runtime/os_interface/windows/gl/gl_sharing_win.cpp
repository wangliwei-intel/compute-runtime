/*
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/helpers/timestamp_packet.h"
#include "runtime/os_interface/windows/windows_wrapper.h"
#include "runtime/sharings/gl/gl_arb_sync_event.h"
#include "runtime/sharings/gl/gl_sharing.h"

#include <cstdint>
#include <memory>

namespace Os {
extern const char *openglDllName;
}

namespace OCLRT {

GLboolean GLSharingFunctions::makeCurrent(GLContext contextHandle, GLDisplay displayHandle) {
    if (displayHandle == 0) {
        displayHandle = GLHDCHandle;
    }
    return this->wglMakeCurrent(displayHandle, contextHandle);
}

GLSharingFunctions::~GLSharingFunctions() {
    if (pfnWglDeleteContext) {
        pfnWglDeleteContext(GLHGLRCHandleBkpCtx);
    }
}

GLboolean GLSharingFunctions::initGLFunctions() {
    glLibrary.reset(OsLibrary::load(Os::openglDllName));

    if (glLibrary->isLoaded()) {
        glFunctionHelper wglLibrary(glLibrary.get(), "wglGetProcAddress");
        GLGetCurrentContext = (*glLibrary)["wglGetCurrentContext"];
        GLGetCurrentDisplay = (*glLibrary)["wglGetCurrentDC"];
        glGetString = (*glLibrary)["glGetString"];
        glGetIntegerv = (*glLibrary)["glGetIntegerv"];
        pfnWglCreateContext = (*glLibrary)["wglCreateContext"];
        pfnWglDeleteContext = (*glLibrary)["wglDeleteContext"];
        pfnWglShareLists = (*glLibrary)["wglShareLists"];
        wglMakeCurrent = (*glLibrary)["wglMakeCurrent"];

        GLSetSharedOCLContextState = wglLibrary["wglSetSharedOCLContextStateINTEL"];
        GLAcquireSharedBuffer = wglLibrary["wglAcquireSharedBufferINTEL"];
        GLReleaseSharedBuffer = wglLibrary["wglReleaseSharedBufferINTEL"];
        GLAcquireSharedRenderBuffer = wglLibrary["wglAcquireSharedRenderBufferINTEL"];
        GLReleaseSharedRenderBuffer = wglLibrary["wglReleaseSharedRenderBufferINTEL"];
        GLAcquireSharedTexture = wglLibrary["wglAcquireSharedTextureINTEL"];
        GLReleaseSharedTexture = wglLibrary["wglReleaseSharedTextureINTEL"];
        GLRetainSync = wglLibrary["wglRetainSyncINTEL"];
        GLReleaseSync = wglLibrary["wglReleaseSyncINTEL"];
        GLGetSynciv = wglLibrary["wglGetSyncivINTEL"];
        glGetStringi = wglLibrary["glGetStringi"];
    }
    this->pfnGlArbSyncObjectCleanup = cleanupArbSyncObject;
    this->pfnGlArbSyncObjectSetup = setupArbSyncObject;
    this->pfnGlArbSyncObjectSignal = signalArbSyncObject;
    this->pfnGlArbSyncObjectWaitServer = serverWaitForArbSyncObject;

    return 1;
}

bool GLSharingFunctions::isGlSharingEnabled() {
    static bool oglLibAvailable = std::unique_ptr<OsLibrary>(OsLibrary::load(Os::openglDllName)).get() != nullptr;
    return oglLibAvailable;
}

bool GLSharingFunctions::isOpenGlExtensionSupported(const unsigned char *pExtensionString) {
    bool LoadedNull = (glGetStringi == nullptr) || (glGetIntegerv == nullptr);
    if (LoadedNull) {
        return false;
    }

    cl_int NumberOfExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions);
    for (cl_int i = 0; i < NumberOfExtensions; i++) {
        std::basic_string<unsigned char> pString = glGetStringi(GL_EXTENSIONS, i);
        if (pString == pExtensionString) {
            return true;
        }
    }
    return false;
}

bool GLSharingFunctions::isOpenGlSharingSupported() {

    std::basic_string<unsigned char> Vendor = glGetString(GL_VENDOR);
    const unsigned char intelVendor[] = "Intel";

    if ((Vendor.empty()) || (Vendor != intelVendor)) {
        return false;
    }
    std::basic_string<unsigned char> Version = glGetString(GL_VERSION);
    if (Version.empty()) {
        return false;
    }

    bool IsOpenGLES = false;
    const unsigned char versionES[] = "OpenGL ES";
    if (Version.find(versionES) != std::string::npos) {
        IsOpenGLES = true;
    }

    if (IsOpenGLES == true) {
        const unsigned char versionES1[] = "OpenGL ES 1.";
        if (Version.find(versionES1) != std::string::npos) {
            const unsigned char supportGLOES[] = "GL_OES_framebuffer_object";
            if (isOpenGlExtensionSupported(supportGLOES) == false) {
                return false;
            }
        }
    } else {
        if (Version[0] < '3') {
            const unsigned char supportGLEXT[] = "GL_EXT_framebuffer_object";
            if (isOpenGlExtensionSupported(supportGLEXT) == false) {
                return false;
            }
        }
    }

    return true;
}

void GLSharingFunctions::createBackupContext() {
    if (pfnWglCreateContext) {
        GLHGLRCHandleBkpCtx = pfnWglCreateContext(GLHDCHandle);
        pfnWglShareLists(GLHGLRCHandle, GLHGLRCHandleBkpCtx);
    }
}
} // namespace OCLRT
