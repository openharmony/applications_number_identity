/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "number_mark_result_set_bridge.h"
#include "number_identity_result_set_bridge_template.h"
#include <gtest/gtest.h>
#include <cstring>
#include <string>

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class NumberMarkResultSetBridgeGTest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};
void NumberMarkResultSetBridgeGTest::SetUp() {}

void NumberMarkResultSetBridgeGTest::TearDown() {}

void NumberMarkResultSetBridgeGTest::SetUpTestCase() {}

void NumberMarkResultSetBridgeGTest::TearDownTestCase() {}

class NumberMarkResultSetBridgeMock final : public DataShare::ResultSetBridge::Writer {
public:
    NumberMarkResultSetBridgeMock();
    virtual ~NumberMarkResultSetBridgeMock();
    int AllocRow() override;
    int FreeLastRow() override;
    int Write(uint32_t column) override;
    int Write(uint32_t column, int64_t value) override;
    int Write(uint32_t column, double value) override;
    int Write(uint32_t column, const uint8_t *value, size_t size) override;
    int Write(uint32_t column, const char *value, size_t size) override;
    void SetAllocRowStatue(int status);
    DistributedKv::Key GetKey() const;
    DistributedKv::Value GetValue() const;

private:
    int allocStatus_ = DataShare::E_OK;
    std::vector<uint8_t> key_;
    std::vector<uint8_t> value_;
};

NumberMarkResultSetBridgeMock::NumberMarkResultSetBridgeMock()
{
}

NumberMarkResultSetBridgeMock::~NumberMarkResultSetBridgeMock()
{
}

void NumberMarkResultSetBridgeMock::SetAllocRowStatue(int status)
{
    allocStatus_ = status;
}

DistributedKv::Key NumberMarkResultSetBridgeMock::GetKey() const
{
    return key_;
}

DistributedKv::Value NumberMarkResultSetBridgeMock::GetValue() const
{
    return value_;
}

int NumberMarkResultSetBridgeMock::AllocRow()
{
    return allocStatus_;
}

int NumberMarkResultSetBridgeMock::FreeLastRow()
{
    return DataShare::E_OK;
}

int NumberMarkResultSetBridgeMock::Write(uint32_t column)
{
    return DataShare::E_OK;
}

int NumberMarkResultSetBridgeMock::Write(uint32_t column, int64_t value)
{
    return DataShare::E_OK;
}

int NumberMarkResultSetBridgeMock::Write(uint32_t column, double value)
{
    return DataShare::E_OK;
}

int NumberMarkResultSetBridgeMock::Write(uint32_t column, const uint8_t *value, size_t size)
{
    return DataShare::E_OK;
}

int NumberMarkResultSetBridgeMock::Write(uint32_t column, const char *value, size_t size)
{
    if (column < 0 || column > 1 || value == nullptr) {
        return DataShare::E_ERROR;
    }
    auto vec = std::vector<uint8_t>(value, value + size - 1);
    if (column == 0) {
        key_.insert(key_.end(), vec.begin(), vec.end());
    } else {
        value_.insert(value_.end(), vec.begin(), vec.end());
    }
    return DataShare::E_OK;
}

/**
 * @tc.number   NumberMarkResultSetBridgeGTest_001
 * @tc.name     NumberMarkResultSetBridge.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(NumberMarkResultSetBridgeGTest, NumberMarkResultSetBridgeGTest_001, Function | MediumTest | Level1)
{
    NumberMarkResultSetBridgeMock numberMarkResultSetBridge;
    NumberMarkInfo markInfo;
    NumberMarkResultSetBridge bridge({});
    uint32_t columnIndex = 0;
    EXPECT_GE((bridge.columns).size(), 0);
    for (const auto& item : bridge.columns) {
        item.write(numberMarkResultSetBridge, markInfo, columnIndex++);
    }
}
}
}