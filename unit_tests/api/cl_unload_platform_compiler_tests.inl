/*
 * Copyright (C) 2017-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "unit_tests/fixtures/buffer_fixture.h"

#include "cl_api_tests.h"

using namespace OCLRT;

typedef api_tests clUnloadPlatformCompilerTests;

namespace ULT {

TEST_F(clUnloadPlatformCompilerTests, notImplemented) {
    auto retVal = clUnloadPlatformCompiler(nullptr);
    EXPECT_EQ(CL_OUT_OF_HOST_MEMORY, retVal);
}
} // namespace ULT
