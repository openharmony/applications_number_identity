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
#include "download_file_ability.h"
#undef private
#include "number_identity_errors.h"
#include "download_file_rdb.h"
#include "number_identity_ddl.h"
#include "download_file.h"
#include <gtest/gtest.h>
#include <cstring>
#include <string>

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class DownloadFileAbilityGtest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};
void DownloadFileAbilityGtest::SetUp() {}

void DownloadFileAbilityGtest::TearDown() {}

void DownloadFileAbilityGtest::SetUpTestCase() {}

void DownloadFileAbilityGtest::TearDownTestCase() {}

/**
 * @tc.number   DownloadFileAbilityGtest_001
 * @tc.name     DownloadFileAbility Create.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileAbilityGtest, DownloadFileAbilityGtest_001, TestSize.Level0)
{
    EXPECT_NE(DownloadFileAbility::Create(nullptr), nullptr);
}

/**
 * @tc.number   DownloadFileAbilityGtest_004
 * @tc.name     DownloadFileAbilityGtest Insert.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileAbilityGtest, DownloadFileAbilityGtest_004, TestSize.Level0)
{
    DownloadFileAbility dFileAbility(nullptr);
    Uri uri("datashare:///com.ohos.numberlocationability");
    DataShare::DataShareValuesBucket value;
    EXPECT_EQ(dFileAbility.Insert(uri, value), NUMBER_IDENTITY_ERROR);
}

/**
 * @tc.number   DownloadFileAbilityGtest_005
 * @tc.name     DownloadFileAbilityGtest Query.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileAbilityGtest, DownloadFileAbilityGtest_005, Function | MediumTest | Level1)
{
    DownloadFileAbility dFileAbility(nullptr);
    Uri uri("datashare:///com.ohos.numberlocationability");
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    DataShare::DatashareBusinessError businessError;
    dFileAbility.Query(uri, predicates, columns, businessError);
    EXPECT_EQ(columns.size(), 0);
    EXPECT_EQ(DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, ""),
        NUMBER_IDENTITY_ERROR);
    DownloadFileRdb::GetInstance().SetDBDirectory("/data/test");
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::WLAN_ONLY);
    dFileAbility.Query(uri, predicates, columns, businessError);
    EXPECT_EQ(columns.size(), 0);
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::TIME_WORKER_START_TIME_SECOND, "123");
    dFileAbility.Query(uri, predicates, columns, businessError);
    EXPECT_EQ(columns.size(), 0);
}

/**
 * @tc.number   DownloadFileAbilityGtest_TaskType_Is_WorkSheduler
 * @tc.name     DownloadFileAbilityGtest Update.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileAbilityGtest, DownloadFileAbilityGtest_TaskType_Is_WorkSheduler, TestSize.Level0)
{
    DownloadFileAbility dFileAbility(nullptr);
    Uri uri("datashare:///com.ohos.numberlocationability");
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
    values.Put("task_type", PropertyValues::WORK_SHEDULER);
    values.Put("work_id", BOOT_WORK_ID);
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
}

/**
 * @tc.number   DownloadFileAbilityGtest_WorkId
 * @tc.name     DownloadFileAbilityGtest DealTaskTypeOfWorkSheduler.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileAbilityGtest, DownloadFileAbilityGtest_WorkId, Function | MediumTest | Level1)
{
    DownloadFileAbility dFileAbility(nullptr);
    Uri uri("datashare:///com.ohos.numberlocationability");
    DataShare::DataShareValuesBucket values;
    values.Put("work_id", "1");
    ASSERT_NO_THROW(dFileAbility.DealTaskTypeOfWorkSheduler(values));
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::CLOSE_UPDATE);
    ASSERT_NO_THROW(dFileAbility.DealTaskTypeOfWorkSheduler(values));
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::FORCE_UPDATED, "1");
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, "");
    ASSERT_NO_THROW(dFileAbility.DealTaskTypeOfWorkSheduler(values));
    values.Clear();
    values.Put("work_id", "2");
    ASSERT_NO_THROW(dFileAbility.DealTaskTypeOfWorkSheduler(values));
}

/**
 * @tc.number   DownloadFileAbilityGtest_008
 * @tc.name     DownloadFileAbilityGtest Delete.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(DownloadFileAbilityGtest, DownloadFileAbilityGtest_008, TestSize.Level0)
{
    DownloadFileAbility dFileAbility(nullptr);
    Uri uri("datashare:///com.ohos.numberlocationability");
    DataShare::DataSharePredicates predicates;
    EXPECT_EQ(dFileAbility.Delete(uri, predicates), NUMBER_IDENTITY_ERROR);
}

/**
 * @tc.number   DownloadFileAbilityGtest_TaskType_Is_WlanOnly
 * @tc.name     DownloadFileAbilityGtest
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileAbilityGtest, DownloadFileAbilityGtest_TaskType_Is_WlanOnly, Function | MediumTest | Level1)
{
    DownloadFileAbility dFileAbility(nullptr);
    Uri uri("datashare:///com.ohos.numberlocationability");
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    values.Put("task_type", PropertyValues::WLAN_ONLY);
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
    DownloadFileRdb::GetInstance().SetDBDirectory("/data/test");
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::WLAN_ONLY);
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::CLOSE_UPDATE);
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
}

/**
 * @tc.number   DownloadFileAbilityGtest_TaskType_Is_AllNetwork
 * @tc.name     DownloadFileAbilityGtest
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileAbilityGtest, DownloadFileAbilityGtest_TaskType_Is_AllNetwork, Function | MediumTest | Level1)
{
    DownloadFileAbility dFileAbility(nullptr);
    Uri uri("datashare:///com.ohos.numberlocationability");
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    values.Put("task_type", PropertyValues::ALL_NETWORK);
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::ALL_NETWORK);
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::CLOSE_UPDATE);
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
}

/**
 * @tc.number   DownloadFileAbilityGtest_TaskType_Is_CloseUpdate
 * @tc.name     DownloadFileAbilityGtest
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileAbilityGtest, DownloadFileAbilityGtest_TaskType_Is_CloseUpdate, Function | MediumTest | Level1)
{
    DownloadFileAbility dFileAbility(nullptr);
    Uri uri("datashare:///com.ohos.numberlocationability");
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket values;
    values.Put("task_type", PropertyValues::CLOSE_UPDATE);
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::CLOSE_UPDATE);
    EXPECT_EQ(dFileAbility.Update(uri, predicates, values), NUMBER_IDENTITY_ERR_SUCCESS);
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, "");
}
}
}