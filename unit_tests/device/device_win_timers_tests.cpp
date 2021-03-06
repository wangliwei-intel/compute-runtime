/*
 * Copyright (C) 2017-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "test.h"
#include "unit_tests/fixtures/device_fixture.h"
#include "unit_tests/mocks/mock_ostime.h"
#include "unit_tests/mocks/mock_ostime_win.h"
#include "unit_tests/mocks/mock_wddm.h"

#include "gtest/gtest.h"

using namespace OCLRT;

namespace ULT {

typedef ::testing::Test MockOSTimeWinTest;

TEST_F(MockOSTimeWinTest, DynamicResolution) {
    auto wddmMock = std::unique_ptr<WddmMock>(new WddmMock());
    auto mDev = std::unique_ptr<MockDevice>(MockDevice::createWithNewExecutionEnvironment<MockDevice>(nullptr));

    bool success = wddmMock->init(mDev->getPreemptionMode());
    EXPECT_TRUE(success);

    std::unique_ptr<MockOSTimeWin> timeWin(new MockOSTimeWin(wddmMock.get()));

    double res = 0.0;
    res = timeWin->getDynamicDeviceTimerResolution(mDev->getHardwareInfo());
    EXPECT_EQ(res, 1e+09);
}

} // namespace ULT
