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

#include "string_ex.h"
#include "yellow_page_parser.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

using namespace std;

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
class YellowPageParserGtest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
    YellowPageParser parser;
};

void YellowPageParserGtest::SetUp() {}

void YellowPageParserGtest::TearDown() {}

void YellowPageParserGtest::SetUpTestCase() {}

void YellowPageParserGtest::TearDownTestCase() {}

HWTEST_F(YellowPageParserGtest, Telephony_YellowPageParser_ParseVersion, TestSize.Level0)
{
    string line = R"({"version":"1234"})";
    string result;
    EXPECT_TRUE(parser.ParseVersion(line, result));
    EXPECT_EQ(result, "1234");
}

HWTEST_F(YellowPageParserGtest, Telephony_YellowPageParser_ParseRecord_ErrorData, TestSize.Level0)
{
    auto errorData =
        R"({"group":"公共服务","photo":"","phone":[{"hot_points":-1,"dial_map":"24264 5426 744 746 226 5464","phone":"110","name":"畅连视频报警",match_pattern="610800",alias_name="榆林110",device_type=15}],"name":"畅连视频报警"})";
    YellowPageRecord result;
    EXPECT_TRUE(parser.ParseRecord(errorData, result));
    EXPECT_EQ(result.rawData, errorData);
    EXPECT_EQ(result.phone.size(), 1);
    auto phone = result.phone[0];
    EXPECT_EQ(phone.hot_points, -1);
    EXPECT_EQ(phone.dial_map, "24264 5426 744 746 226 5464");
    EXPECT_EQ(phone.phone, "110");
    EXPECT_EQ(phone.name, "畅连视频报警");
    EXPECT_EQ(phone.match_pattern, "610800");
    EXPECT_EQ(phone.alias_name, "榆林110");
    EXPECT_EQ(phone.device_type, 15);
}

} // namespace Telephony
} // namespace OHOS
