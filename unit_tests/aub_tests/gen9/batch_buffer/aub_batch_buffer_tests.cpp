/*
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_batch_buffer_tests.h"

#include "unit_tests/fixtures/device_fixture.h"

using AubBatchBufferTests = Test<OCLRT::DeviceFixture>;

static constexpr auto gpuBatchBufferAddr = 0x800400001000ull; // 48-bit GPU address

GEN9TEST_F(AubBatchBufferTests, givenSimpleRCSWithBatchBufferWhenItHasMSBSetInGpuAddressThenAUBShouldBeSetupSuccessfully) {
    setupAUBWithBatchBuffer<FamilyType>(pDevice, OCLRT::EngineType::ENGINE_RCS, gpuBatchBufferAddr);
}
