/*
 * Copyright (C) 2018-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/os_interface/linux/drm_neo.h"
#include "test.h"

#include "hw_cmds.h"

#include <array>

using namespace OCLRT;

TEST(SklDeviceIdTest, supportedDeviceId) {
    std::array<DeviceDescriptor, 30> expectedDescriptors = {{
        {ISKL_GT1_DESK_DEVICE_F0_ID, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},
        {ISKL_GT1_DT_DEVICE_F0_ID, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},
        {ISKL_GT1_HALO_MOBL_DEVICE_F0_ID, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},
        {ISKL_GT1_SERV_DEVICE_F0_ID, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},
        {ISKL_GT1_ULT_DEVICE_F0_ID, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},
        {ISKL_GT1_ULX_DEVICE_F0_ID, &SKL_1x2x6::hwInfo, &SKL_1x2x6::setupHardwareInfo, GTTYPE_GT1},

        {ISKL_GT1_5_DT_DEVICE_F0_ID, &SKL_1x3x6::hwInfo, &SKL_1x3x6::setupHardwareInfo, GTTYPE_GT1_5},
        {ISKL_GT1_5_ULT_DEVICE_F0_ID, &SKL_1x3x6::hwInfo, &SKL_1x3x6::setupHardwareInfo, GTTYPE_GT1_5},
        {ISKL_GT1_5_ULX_DEVICE_F0_ID, &SKL_1x3x6::hwInfo, &SKL_1x3x6::setupHardwareInfo, GTTYPE_GT1_5},

        {ISKL_GT2_DESK_DEVICE_F0_ID, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {ISKL_GT2_DT_DEVICE_F0_ID, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {ISKL_GT2_HALO_MOBL_DEVICE_F0_ID, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {ISKL_GT2_SERV_DEVICE_F0_ID, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {ISKL_GT2_ULT_DEVICE_F0_ID, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {ISKL_GT2_ULX_DEVICE_F0_ID, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {ISKL_GT2_WRK_DEVICE_F0_ID, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {ISKL_GT2F_ULT_DEVICE_F0_ID, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},
        {ISKL_LP_DEVICE_F0_ID, &SKL_1x3x8::hwInfo, &SKL_1x3x8::setupHardwareInfo, GTTYPE_GT2},

        {ISKL_GT3_DESK_DEVICE_F0_ID, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {ISKL_GT3_HALO_MOBL_DEVICE_F0_ID, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {ISKL_GT3_MEDIA_SERV_DEVICE_F0_ID, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {ISKL_GT3_SERV_DEVICE_F0_ID, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {ISKL_GT3_ULT_DEVICE_F0_ID, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {ISKL_GT3e_ULT_DEVICE_F0_ID_540, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},
        {ISKL_GT3e_ULT_DEVICE_F0_ID_550, &SKL_2x3x8::hwInfo, &SKL_2x3x8::setupHardwareInfo, GTTYPE_GT3},

        {ISKL_GT4_DESK_DEVICE_F0_ID, &SKL_3x3x8::hwInfo, &SKL_3x3x8::setupHardwareInfo, GTTYPE_GT4},
        {ISKL_GT4_DT_DEVICE_F0_ID, &SKL_3x3x8::hwInfo, &SKL_3x3x8::setupHardwareInfo, GTTYPE_GT4},
        {ISKL_GT4_HALO_MOBL_DEVICE_F0_ID, &SKL_3x3x8::hwInfo, &SKL_3x3x8::setupHardwareInfo, GTTYPE_GT4},
        {ISKL_GT4_SERV_DEVICE_F0_ID, &SKL_3x3x8::hwInfo, &SKL_3x3x8::setupHardwareInfo, GTTYPE_GT4},
        {ISKL_GT4_WRK_DEVICE_F0_ID, &SKL_3x3x8::hwInfo, &SKL_3x3x8::setupHardwareInfo, GTTYPE_GT4},
    }};

    auto compareStructs = [](const DeviceDescriptor *first, const DeviceDescriptor *second) {
        return first->deviceId == second->deviceId && first->pHwInfo == second->pHwInfo &&
               first->setupHardwareInfo == second->setupHardwareInfo && first->eGtType == second->eGtType;
    };

    size_t startIndex = 0;
    while (!compareStructs(&expectedDescriptors[0], &deviceDescriptorTable[startIndex]) &&
           deviceDescriptorTable[startIndex].deviceId != 0) {
        startIndex++;
    };
    EXPECT_NE(0u, deviceDescriptorTable[startIndex].deviceId);

    for (auto &expected : expectedDescriptors) {
        EXPECT_TRUE(compareStructs(&expected, &deviceDescriptorTable[startIndex]));
        startIndex++;
    }
}
