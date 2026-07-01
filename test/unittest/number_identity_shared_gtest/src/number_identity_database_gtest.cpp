/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "number_identity_database.h"
#include "number_identity_ddl.h"
#include "number_identity_log_wrapper.h"
#include "rdb_errno.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
class NumberIdentityDatabaseGtest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void NumberIdentityDatabaseGtest::SetUp()
{
    NUMBER_IDENTITY_LOGI("NumberIdentityDatabaseGtest::SetUp");
    NumberIdentityDatabase::SetDBDirectory("/data/test");
}

void NumberIdentityDatabaseGtest::TearDown() {}

void NumberIdentityDatabaseGtest::SetUpTestCase() {}

void NumberIdentityDatabaseGtest::TearDownTestCase() {}

HWTEST_F(NumberIdentityDatabaseGtest, Telephony_ImportYellowPageData, Function | MediumTest | Level1)
{
    auto db = NumberIdentityDatabase::GetInstance();
    EXPECT_NE(db, nullptr);
    db->ImportYellowPageData();
    RdbPredicates rdbPredicates(NumberIdentityTables::YELLOW_PAGE_VIEW);
    vector<string> columns = { YellowPageViewColumns::MATCH_PATTERN };
    auto result = db->Query(rdbPredicates, columns);
    int count;
    EXPECT_EQ(result->GetRowCount(count), E_OK);
    EXPECT_GT(count, 0);
    result->Close();
}

} // namespace Telephony
} // namespace OHOS
