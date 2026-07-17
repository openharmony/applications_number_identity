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

#include "download_file_result_set_bridge.h"
#include "number_identity_ddl.h"
#include "rdb_errno.h"
#include <gtest/gtest.h>
#include <cstring>
#include <string>

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class DownloadFileResultSetBridgeGtest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};
void DownloadFileResultSetBridgeGtest::SetUp() {}

void DownloadFileResultSetBridgeGtest::TearDown() {}

void DownloadFileResultSetBridgeGtest::SetUpTestCase() {}

void DownloadFileResultSetBridgeGtest::TearDownTestCase() {}


class DownloadFileResultSetBridgeWriter : public DataShare::ResultSetBridge::Writer {
  public:
    DownloadFileResultSetBridgeWriter() {}
    int AllocRow()
    {
        return 0;
    }
    int FreeLastRow()
    {
        return 0;
    }
    int Write(uint32_t column)
    {
        return 0;
    }
    int Write(uint32_t column, int64_t value)
    {
        return 0;
    }
    int Write(uint32_t column, double value)
    {
        return 0;
    }
    int Write(uint32_t column, const uint8_t *value, size_t size)
    {
        return 0;
    }
    int Write(uint32_t column, const char *value, size_t size)
    {
        return 0;
    }
};
/**
 * @tc.number   DownloadFileResultSetBridgeGtest_001
 * @tc.name     DownloadFileAbility GetRowCount.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileResultSetBridgeGtest, DownloadFileResultSetBridgeGtest_001, TestSize.Level0)
{
    std::vector<std::string> propertyKey;
    propertyKey.push_back(PropertyKeys::NETWORK_TYPE);
    std::vector<std::string> propertyValue;
    propertyValue.push_back(PropertyValues::WLAN_ONLY);
    NativeRdb::DownloadFileResultSetBridge downloadBridge(propertyKey, propertyValue, propertyValue.size());
    int32_t count = 0;
    EXPECT_EQ(downloadBridge.GetRowCount(count), NativeRdb::E_OK);
}

/**
 * @tc.number   DownloadFileResultSetBridgeGtest_002
 * @tc.name     DownloadFileResultSetBridgeGtest GetAllColumnNames.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileResultSetBridgeGtest, DownloadFileResultSetBridgeGtest_002, TestSize.Level0)
{
    std::vector<std::string> propertyKey;
    propertyKey.push_back(PropertyKeys::NETWORK_TYPE);
    std::vector<std::string> propertyValue;
    propertyValue.push_back(PropertyValues::WLAN_ONLY);
    NativeRdb::DownloadFileResultSetBridge downloadBridge(propertyKey, propertyValue, propertyValue.size());
    std::vector<std::string> columnNames;
    EXPECT_EQ(downloadBridge.GetAllColumnNames(columnNames), NativeRdb::E_OK);
}

/**
 * @tc.number   DownloadFileResultSetBridgeGtest_003
 * @tc.name     DownloadFileResultSetBridgeGtest FillBlock.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileResultSetBridgeGtest, DownloadFileResultSetBridgeGtest_003, TestSize.Level0)
{
    std::vector<std::string> propertyKey;
    propertyKey.push_back(PropertyKeys::NETWORK_TYPE);
    std::vector<std::string> propertyValue;
    propertyValue.push_back(PropertyValues::WLAN_ONLY);
    NativeRdb::DownloadFileResultSetBridge downloadBridge(propertyKey, propertyValue, propertyValue.size());
    DownloadFileResultSetBridgeWriter writer;
    downloadBridge.FillBlock(0, writer);
    EXPECT_NE(propertyValue.size(), 0);
}

/**
 * @tc.number   DownloadFileResultSetBridgeGtest_004
 * @tc.name     DownloadFileResultSetBridgeGtest OnGo.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileResultSetBridgeGtest, DownloadFileResultSetBridgeGtest_004, TestSize.Level0)
{
    std::vector<std::string> propertyKey;
    propertyKey.push_back(PropertyKeys::NETWORK_TYPE);
    propertyKey.push_back(PropertyKeys::NUMBER_LOCATION_VERSION_TIME_STAMP);
    std::vector<std::string> propertyValue;
    propertyValue.push_back(PropertyValues::WLAN_ONLY);
    propertyValue.push_back(PropertyValues::WORK_SHEDULER);
    NativeRdb::DownloadFileResultSetBridge downloadBridge(propertyKey, propertyValue, propertyValue.size());
    DownloadFileResultSetBridgeWriter writer;
    EXPECT_NE(downloadBridge.OnGo(-1, 0, writer), NativeRdb::E_ERROR);
    downloadBridge.OnGo(0, 1, writer);
}
}
}