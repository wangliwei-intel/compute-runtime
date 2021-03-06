/*
 * Copyright (C) 2017-2019 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "runtime/mem_obj/mem_obj.h"
#include "runtime/memory_manager/allocations_list.h"
#include "runtime/os_interface/os_context.h"
#include "runtime/platform/platform.h"
#include "test.h"
#include "unit_tests/libult/ult_command_stream_receiver.h"
#include "unit_tests/mocks/mock_context.h"
#include "unit_tests/mocks/mock_device.h"
#include "unit_tests/mocks/mock_memory_manager.h"

using namespace OCLRT;

template <typename Family>
class MyCsr : public UltCommandStreamReceiver<Family> {
  public:
    MyCsr(const HardwareInfo &hwInfo, const ExecutionEnvironment &executionEnvironment) : UltCommandStreamReceiver<Family>(hwInfo, const_cast<ExecutionEnvironment &>(executionEnvironment)) {}
    MOCK_METHOD3(waitForCompletionWithTimeout, bool(bool enableTimeout, int64_t timeoutMs, uint32_t taskCountToWait));
};

void CL_CALLBACK emptyDestructorCallback(cl_mem memObj, void *userData) {
}

class MemObjDestructionTest : public ::testing::TestWithParam<bool> {
  public:
    void SetUp() override {
        executionEnvironment = platformImpl->peekExecutionEnvironment();
        memoryManager = new MockMemoryManager(*executionEnvironment);
        executionEnvironment->memoryManager.reset(memoryManager);
        device.reset(MockDevice::create<MockDevice>(*platformDevices, executionEnvironment, 0));
        context.reset(new MockContext(device.get()));

        allocation = memoryManager->allocateGraphicsMemoryWithProperties(MockAllocationProperties{size});
        memObj = new MemObj(context.get(), CL_MEM_OBJECT_BUFFER,
                            CL_MEM_READ_WRITE,
                            size,
                            nullptr, nullptr, allocation, true, false, false);
        *device->getDefaultEngine().commandStreamReceiver->getTagAddress() = 0;
        contextId = device->getDefaultEngine().osContext->getContextId();
    }

    void TearDown() override {
        context.reset();
    }

    void makeMemObjUsed() {
        memObj->getGraphicsAllocation()->updateTaskCount(taskCountReady, contextId);
    }

    void makeMemObjNotReady() {
        makeMemObjUsed();
        *device->getDefaultEngine().commandStreamReceiver->getTagAddress() = taskCountReady - 1;
    }

    void makeMemObjReady() {
        makeMemObjUsed();
        *device->getDefaultEngine().commandStreamReceiver->getTagAddress() = taskCountReady;
    }

    constexpr static uint32_t taskCountReady = 3u;
    ExecutionEnvironment *executionEnvironment;
    std::unique_ptr<MockDevice> device;
    uint32_t contextId = 0;
    MockMemoryManager *memoryManager;
    std::unique_ptr<MockContext> context;
    GraphicsAllocation *allocation;
    MemObj *memObj;
    size_t size = MemoryConstants::pageSize;
};

class MemObjAsyncDestructionTest : public MemObjDestructionTest {
  public:
    void SetUp() override {
        DebugManager.flags.EnableAsyncDestroyAllocations.set(true);
        MemObjDestructionTest::SetUp();
    }
    void TearDown() override {
        MemObjDestructionTest::TearDown();
        DebugManager.flags.EnableAsyncDestroyAllocations.set(defaultFlag);
    }
    bool defaultFlag = DebugManager.flags.EnableAsyncDestroyAllocations.get();
};

class MemObjSyncDestructionTest : public MemObjDestructionTest {
  public:
    void SetUp() override {
        DebugManager.flags.EnableAsyncDestroyAllocations.set(false);
        MemObjDestructionTest::SetUp();
    }
    void TearDown() override {
        MemObjDestructionTest::TearDown();
        DebugManager.flags.EnableAsyncDestroyAllocations.set(defaultFlag);
    }
    bool defaultFlag = DebugManager.flags.EnableAsyncDestroyAllocations.get();
};

TEST_P(MemObjAsyncDestructionTest, givenMemObjWithDestructableAllocationWhenAsyncDestructionsAreEnabledAndAllocationIsNotReadyAndMemObjectIsDestructedThenAllocationIsDeferred) {
    bool isMemObjReady;
    bool expectedDeferration;
    isMemObjReady = GetParam();
    expectedDeferration = !isMemObjReady;

    if (isMemObjReady) {
        makeMemObjReady();
    } else {
        makeMemObjNotReady();
    }
    auto &allocationList = memoryManager->getDefaultCommandStreamReceiver(0)->getTemporaryAllocations();
    EXPECT_TRUE(allocationList.peekIsEmpty());

    delete memObj;

    EXPECT_EQ(!expectedDeferration, allocationList.peekIsEmpty());
    if (expectedDeferration) {
        EXPECT_EQ(allocation, allocationList.peekHead());
    }
}

HWTEST_P(MemObjAsyncDestructionTest, givenUsedMemObjWithAsyncDestructionsEnabledThatHasDestructorCallbacksWhenItIsDestroyedThenDestructorWaitsOnTaskCount) {
    makeMemObjUsed();

    bool hasCallbacks = GetParam();

    if (hasCallbacks) {
        memObj->setDestructorCallback(emptyDestructorCallback, nullptr);
    }

    auto mockCsr = new ::testing::NiceMock<MyCsr<FamilyType>>(device->getHardwareInfo(), *device->executionEnvironment);
    device->resetCommandStreamReceiver(mockCsr);

    bool desired = true;

    auto waitForCompletionWithTimeoutMock = [=](bool enableTimeout, int64_t timeoutMs, uint32_t taskCountToWait) -> bool { return desired; };
    auto osContextId = mockCsr->getOsContext().getContextId();

    ON_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(waitForCompletionWithTimeoutMock));

    if (hasCallbacks) {
        EXPECT_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, TimeoutControls::maxTimeout, allocation->getTaskCount(osContextId)))
            .Times(1);
    } else {
        EXPECT_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
    }
    delete memObj;
}

HWTEST_P(MemObjAsyncDestructionTest, givenUsedMemObjWithAsyncDestructionsEnabledThatHasAllocatedMappedPtrWhenItIsDestroyedThenDestructorWaitsOnTaskCount) {
    makeMemObjUsed();

    bool hasAllocatedMappedPtr = GetParam();

    if (hasAllocatedMappedPtr) {
        auto allocatedPtr = alignedMalloc(size, MemoryConstants::pageSize);
        memObj->setAllocatedMapPtr(allocatedPtr);
    }

    auto mockCsr = new ::testing::NiceMock<MyCsr<FamilyType>>(device->getHardwareInfo(), *device->executionEnvironment);
    device->resetCommandStreamReceiver(mockCsr);
    auto osContextId = mockCsr->getOsContext().getContextId();

    bool desired = true;

    auto waitForCompletionWithTimeoutMock = [=](bool enableTimeout, int64_t timeoutMs, uint32_t taskCountToWait) -> bool { return desired; };

    ON_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(waitForCompletionWithTimeoutMock));

    if (hasAllocatedMappedPtr) {
        EXPECT_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, TimeoutControls::maxTimeout, allocation->getTaskCount(osContextId)))
            .Times(1);
    } else {
        EXPECT_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
    }
    delete memObj;
}

HWTEST_P(MemObjAsyncDestructionTest, givenUsedMemObjWithAsyncDestructionsEnabledThatHasDestructableMappedPtrWhenItIsDestroyedThenDestructorWaitsOnTaskCount) {
    auto storage = alignedMalloc(size, MemoryConstants::pageSize);

    bool hasAllocatedMappedPtr = GetParam();

    if (!hasAllocatedMappedPtr) {
        delete memObj;
        allocation = memoryManager->allocateGraphicsMemoryWithProperties(MockAllocationProperties{size});
        MemObjOffsetArray origin = {{0, 0, 0}};
        MemObjSizeArray region = {{1, 1, 1}};
        cl_map_flags mapFlags = CL_MAP_READ;
        memObj = new MemObj(context.get(), CL_MEM_OBJECT_BUFFER,
                            CL_MEM_READ_WRITE,
                            size,
                            storage, nullptr, allocation, true, false, false);
        memObj->addMappedPtr(storage, 1, mapFlags, region, origin, 0);
    } else {
        memObj->setAllocatedMapPtr(storage);
    }

    makeMemObjUsed();
    auto mockCsr = new ::testing::NiceMock<MyCsr<FamilyType>>(device->getHardwareInfo(), *device->executionEnvironment);
    device->resetCommandStreamReceiver(mockCsr);

    bool desired = true;

    auto waitForCompletionWithTimeoutMock = [=](bool enableTimeout, int64_t timeoutMs, uint32_t taskCountToWait) -> bool { return desired; };
    auto osContextId = mockCsr->getOsContext().getContextId();

    ON_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(waitForCompletionWithTimeoutMock));

    if (hasAllocatedMappedPtr) {
        EXPECT_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, TimeoutControls::maxTimeout, allocation->getTaskCount(osContextId)))
            .Times(1);
    } else {
        EXPECT_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, ::testing::_, ::testing::_))
            .Times(0);
    }
    delete memObj;

    if (!hasAllocatedMappedPtr) {
        alignedFree(storage);
    }
}

HWTEST_P(MemObjSyncDestructionTest, givenMemObjWithDestructableAllocationWhenAsyncDestructionsAreDisabledThenDestructorWaitsOnTaskCount) {
    bool isMemObjReady;
    isMemObjReady = GetParam();

    if (isMemObjReady) {
        makeMemObjReady();
    } else {
        makeMemObjNotReady();
    }
    auto mockCsr = new ::testing::NiceMock<MyCsr<FamilyType>>(device->getHardwareInfo(), *device->executionEnvironment);
    device->resetCommandStreamReceiver(mockCsr);

    bool desired = true;

    auto waitForCompletionWithTimeoutMock = [=](bool enableTimeout, int64_t timeoutMs, uint32_t taskCountToWait) -> bool { return desired; };
    auto osContextId = mockCsr->getOsContext().getContextId();

    ON_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(waitForCompletionWithTimeoutMock));

    EXPECT_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, TimeoutControls::maxTimeout, allocation->getTaskCount(osContextId)))
        .Times(1);

    delete memObj;
}

HWTEST_P(MemObjSyncDestructionTest, givenMemObjWithDestructableAllocationWhenAsyncDestructionsAreDisabledThenAllocationIsNotDeferred) {
    bool isMemObjReady;
    isMemObjReady = GetParam();

    if (isMemObjReady) {
        makeMemObjReady();
    } else {
        makeMemObjNotReady();
    }
    auto mockCsr = new ::testing::NiceMock<MyCsr<FamilyType>>(device->getHardwareInfo(), *device->executionEnvironment);
    device->resetCommandStreamReceiver(mockCsr);

    bool desired = true;

    auto waitForCompletionWithTimeoutMock = [=](bool enableTimeout, int64_t timeoutMs, uint32_t taskCountToWait) -> bool { return desired; };

    ON_CALL(*mockCsr, waitForCompletionWithTimeout(::testing::_, ::testing::_, ::testing::_))
        .WillByDefault(::testing::Invoke(waitForCompletionWithTimeoutMock));

    delete memObj;
    auto &allocationList = memoryManager->getDefaultCommandStreamReceiver(0)->getTemporaryAllocations();
    EXPECT_TRUE(allocationList.peekIsEmpty());
}

INSTANTIATE_TEST_CASE_P(
    MemObjTests,
    MemObjAsyncDestructionTest,
    testing::Bool());

INSTANTIATE_TEST_CASE_P(
    MemObjTests,
    MemObjSyncDestructionTest,
    testing::Bool());
