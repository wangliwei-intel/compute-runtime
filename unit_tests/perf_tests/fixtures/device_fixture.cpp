/*
 * Copyright (C) 2017-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "device_fixture.h"

#include "runtime/command_stream/command_stream_receiver.h"
#include "runtime/helpers/options.h"

#include "gtest/gtest.h"

using OCLRT::Device;
using OCLRT::HardwareInfo;
using OCLRT::platformDevices;

void DeviceFixture::SetUp() {
    pDevice = DeviceHelper<>::create();
    ASSERT_NE(nullptr, pDevice);

    auto &commandStreamReceiver = pDevice->getCommandStreamReceiver();
    pTagMemory = commandStreamReceiver.getTagAddress();
    ASSERT_NE(nullptr, const_cast<uint32_t *>(pTagMemory));
}

void DeviceFixture::TearDown() {
    delete pDevice;
}
