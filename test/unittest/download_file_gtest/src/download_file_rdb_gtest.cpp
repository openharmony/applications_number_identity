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
#define private public
#include "download_file_rdb.h"
#undef private
#include "number_identity_errors.h"
#include "number_identity_ddl.h"
#include "rdb_predicates.h"
#include "number_identity_database.h"
#include <gtest/gtest.h>
#include <cstring>
#include <string>

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class DownloadFileRdbGtest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};
void DownloadFileRdbGtest::SetUp() {}

void DownloadFileRdbGtest::TearDown() {}

void DownloadFileRdbGtest::SetUpTestCase() {}

void DownloadFileRdbGtest::TearDownTestCase() {}

/**
 * @tc.number   DownloadFileRdbGtest_001
 * @tc.name     DownloadFileRdbGtest Query.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileRdbGtest, DownloadFileRdbGtest_001, Function | MediumTest | Level1)
{
    DownloadFileRdb::GetInstance().SetDBDirectory("/data/test");
    EXPECT_EQ(DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, ""),
        NUMBER_IDENTITY_ERR_SUCCESS);
    EXPECT_EQ(DownloadFileRdb::GetInstance().Query(PropertyKeys::NETWORK_TYPE, PropertyValues::WLAN_ONLY), "");
    EXPECT_EQ(DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::ALL_NETWORK),
        NUMBER_IDENTITY_ERR_SUCCESS);
    EXPECT_EQ(DownloadFileRdb::GetInstance().QueryNetworkType(nullptr), PropertyValues::ALL_NETWORK);
    NativeRdb::AbsRdbPredicates predicates(NumberIdentityTables::PROPERTIES);
    predicates.EqualTo(PropertiesColumns::PROPERTY_KEY, PropertyKeys::NETWORK_TYPE);
    NumberIdentityDatabase::GetInstance()->Delete(predicates);
    EXPECT_EQ(DownloadFileRdb::GetInstance().QueryNetworkType(nullptr), PropertyValues::WLAN_ONLY);
}
}
}