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

#include "result_set.h"
#include "number_identity_models.h"
#include "number_identity_ddl.h"
#include "number_identity_log_wrapper.h"
#include "rdb_errno.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <gmock/gmock.h>

using namespace std;

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using namespace NativeRdb;
using namespace std;
class ModelTest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

class ModelTestMock : public NativeRdb::ResultSet {
  public:
    ModelTestMock() {}
    MOCK_METHOD1(GetAllColumnNames, int(std::vector<std::string> &));
    MOCK_METHOD1(GetColumnCount, int(int &));
    MOCK_METHOD2(GetColumnType, int(int, ColumnType &));
    MOCK_METHOD2(GetColumnIndex, int(const std::string &, int &));
    MOCK_METHOD2(GetColumnName, int(int, std::string &));
    MOCK_METHOD1(GetRowCount, int(int &));
    MOCK_METHOD1(GoTo, int(int));
    MOCK_METHOD1(GoToRow, int(int));
    MOCK_METHOD0(GoToFirstRow, int());
    MOCK_METHOD0(GoToLastRow, int());
    MOCK_METHOD0(GoToNextRow, int());
    MOCK_METHOD0(GoToPreviousRow, int());
    MOCK_METHOD1(IsEnded, int(bool &));
    MOCK_METHOD1(IsAtLastRow, int(bool &));
    MOCK_METHOD2(GetBlob, int(int, std::vector<uint8_t> &));
    MOCK_METHOD2(GetString, int(int, std::string &));
    MOCK_METHOD2(GetInt, int(int, int &));
    MOCK_METHOD2(GetLong, int(int, int64_t &));
    MOCK_METHOD2(GetDouble, int(int, double &));
    MOCK_METHOD2(IsColumnNull, int(int, bool &));
    MOCK_METHOD0(Close, int());
    MOCK_METHOD1(GetRow, int(RowEntity &));

    MOCK_CONST_METHOD0(IsClosed, bool());
    MOCK_CONST_METHOD1(GetRowIndex, int(int &));
    MOCK_CONST_METHOD1(IsStarted, int(bool &));
    MOCK_CONST_METHOD1(IsAtFirstRow, int(bool &));

    MOCK_METHOD2(GetAsset, int(int32_t, ValueObject::Asset &));
    MOCK_METHOD2(GetAssets, int(int32_t, ValueObject::Assets &));
    MOCK_METHOD2(Get, int(int32_t, ValueObject &));
    MOCK_METHOD1(GetModifyTime, int(std::string &));
    MOCK_METHOD2(GetSize, int(int32_t, size_t &));
};

void ModelTest::SetUp()
{
}

void ModelTest::TearDown() {}

void ModelTest::SetUpTestCase() {}

void ModelTest::TearDownTestCase() {}

HWTEST_F(ModelTest, ModelTest_001, Function | MediumTest | Level1)
{
    ModelTestMock resultSet;
    YellowPageViewModel yellowModel;
    EXPECT_EQ(yellowModel.CreateFromResultSet(resultSet), E_OK);
}

HWTEST_F(ModelTest, ModelTest_002, Function | MediumTest | Level1)
{
    NumberMarkModel markModel;
    ModelTestMock resultSet;
    EXPECT_EQ(markModel.CreateFromResultSet(resultSet), E_OK);
}

HWTEST_F(ModelTest, ModelTest_003, Function | MediumTest | Level1)
{
    NumberMarkInfo marInfo;
    YellowPageViewModel yellowPage;
    marInfo.FromYellowPage(yellowPage);
    EXPECT_EQ(marInfo.markType, MarkType::MARK_TYPE_YELLOW_PAGE);
}

HWTEST_F(ModelTest, ModelTest_004, Function | MediumTest | Level1)
{
    NumberMarkInfo marInfo;
    NumberMarkModel numberMark;
    numberMark.classify = "crank";
    marInfo.FromNumberMark(numberMark);
    EXPECT_EQ(marInfo.isCloud, false);
}

HWTEST_F(ModelTest, ModelTest_005, Function | MediumTest | Level1)
{
    NumberMarkModel numberMark;
    numberMark.ToDataShareValuesBucket();
    numberMark.ToIdentityDataSharePredicates();
    MarkType markType = MarkType::MARK_TYPE_CRANK;
    EXPECT_TRUE("crank"==GetClassify(markType));
}
} // namespace Telephony
} // namespace OHOS
