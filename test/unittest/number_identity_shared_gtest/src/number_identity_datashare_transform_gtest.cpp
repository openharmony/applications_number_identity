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

#include "datashare_predicates.h"
#include "number_identity_database.h"
#include "number_identity_datashare_transform.h"
#include "number_identity_ddl.h"
#include "number_identity_log_wrapper.h"
#include "rdb_errno.h"
#include "rdb_utils.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
namespace OHOS {
using namespace RdbDataShareAdapter;
namespace Telephony {
using namespace testing::ext;
class NumberIdentityDatashareTransformTest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
    NativeQuery GetQuery(const vector<string> &columns, const string &table);
    StringFieldProxy GetProxy();
    shared_ptr<NumberIdentityDatabase> db;
};

void NumberIdentityDatashareTransformTest::SetUp()
{
    NumberIdentityDatabase::SetDBDirectory("/data/test");
    db = NumberIdentityDatabase::GetInstance();
    EXPECT_NE(db, nullptr);
}

void NumberIdentityDatashareTransformTest::TearDown() {}

void NumberIdentityDatashareTransformTest::SetUpTestCase() {}

void NumberIdentityDatashareTransformTest::TearDownTestCase() {}

NativeQuery NumberIdentityDatashareTransformTest::GetQuery(const vector<string> &columns, const string &table)
{
    return [=](const auto &predicates) {
        string s = table;
        auto rdbPred = RdbUtils::ToPredicates(predicates, s);
        auto result = db->Query(rdbPred, columns);
        auto queryResultSet = RdbUtils::ToResultSetBridge(result);
        return std::make_shared<DataShareResultSet>(queryResultSet);
    };
}

StringFieldProxy NumberIdentityDatashareTransformTest::GetProxy()
{
    vector<string> columns = { YellowPageViewColumns::ID, YellowPageViewColumns::NUMBER, YellowPageViewColumns::NAME };
    return {
        .field = YellowPageViewColumns::NUMBER,
        .query = GetQuery(columns, NumberIdentityTables::YELLOW_PAGE_VIEW),
        .transform = [](const string &number) -> string {
            if (number == "01010086") {
                return "10086";
            }
            return number;
        },
    };
}

HWTEST_F(NumberIdentityDatashareTransformTest, Telephony_TransformAreaCode, Function | MediumTest | Level1)
{
    DataSharePredicates predicates;
    OperationTransform transform;
    TransformOperation(predicates, transform);
    predicates.EqualTo(YellowPageViewColumns::NUMBER, "01010086");
    auto resultSetPtr = GetProxy().ProxyQuery(predicates);
    EXPECT_NE(resultSetPtr, nullptr);
    auto &resultSet = *resultSetPtr;
    int count, columnIndex;
    int errCode = resultSet.GetRowCount(count);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(count, 1);
    errCode = resultSet.GoToFirstRow();
    EXPECT_EQ(errCode, E_OK);
    errCode = resultSet.GetColumnIndex(YellowPageViewColumns::NUMBER, columnIndex);
    EXPECT_EQ(errCode, E_OK);
    string number, name;
    errCode = resultSet.GetString(columnIndex, number);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(number, "01010086");
    errCode = resultSet.GetColumnIndex(YellowPageViewColumns::NAME, columnIndex);
    errCode = resultSet.GetString(columnIndex, name);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_NE(name.find("中国移动"), string::npos);
    resultSet.Close();
}

} // namespace Telephony
} // namespace OHOS
