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

#include <cstring>
#include <string>

#include <gtest/gtest.h>
#include <vector>

#include "number_mark_caller_info.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using namespace std;

inline bool operator==(const CallerInfoProvider &a, const CallerInfoProvider &b)
{
    return a.bundleName == b.bundleName && a.extensionAbilityName == b.extensionAbilityName;
}

class NumberMarkCallerInfoGTest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};
void NumberMarkCallerInfoGTest::SetUp() {}

void NumberMarkCallerInfoGTest::TearDown() {}

void NumberMarkCallerInfoGTest::SetUpTestCase() {}

void NumberMarkCallerInfoGTest::TearDownTestCase() {}

/**
 * @tc.number   NumberMarkCallerInfoGTest_001
 * @tc.name     Parse provider info with empty settings value.
 * @tc.desc     Function test
 */
HWTEST_F(NumberMarkCallerInfoGTest, NumberMarkCallerInfoGTest_001, Function | MediumTest | Level1)
{
    auto providers = CallerInfoProvider::Parse("");
    EXPECT_EQ(providers.size(), 0);
}

/**
 * @tc.number   NumberMarkCallerInfoGTest_002
 * @tc.name     Parse provider info with empty array value.
 * @tc.desc     Function test
 */
HWTEST_F(NumberMarkCallerInfoGTest, NumberMarkCallerInfoGTest_002, Function | MediumTest | Level1)
{
    auto providers = CallerInfoProvider::Parse("[]");
    EXPECT_EQ(providers.size(), 0);
}

/**
 * @tc.number   NumberMarkCallerInfoGTest_003
 * @tc.name     Parse provider info with leagacy settings value.
 * @tc.desc     Function test
 */
HWTEST_F(NumberMarkCallerInfoGTest, NumberMarkCallerInfoGTest_003, Function | MediumTest | Level1)
{
    auto json = R"([{"bundleName":"com.example.app","extensionAbilityName":"MainAbility"}])";
    auto providers = CallerInfoProvider::Parse(json);
    vector<CallerInfoProvider> expected = {
        { .bundleName = "com.example.app", .extensionAbilityName = "MainAbility", .switchState = nullopt },
    };
    EXPECT_EQ(providers, expected);
}

/**
 * @tc.number   NumberMarkCallerInfoGTest_004
 * @tc.name     Parse provider info with newer value.
 * @tc.desc     Function test
 */
HWTEST_F(NumberMarkCallerInfoGTest, NumberMarkCallerInfoGTest_004, Function | MediumTest | Level1)
{
    auto json = R"([
        {
            "bundleName":"com.example.app","extensionAbilityName":"MainAbility", "switchState":true
        },
        {
            "bundleName":"com.example.app2","extensionAbilityName":"MainAbility2", "switchState":false
        }
    ])";
    auto newProviders = CallerInfoProvider::Parse(json);
    vector<CallerInfoProvider> expected = {
        { .bundleName = "com.example.app", .extensionAbilityName = "MainAbility", .switchState = true },
        { .bundleName = "com.example.app2", .extensionAbilityName = "MainAbility2", .switchState = false },
    };
    EXPECT_EQ(newProviders, expected);
}

/**
 * @tc.number   CallerInfoNumberMarkVendorGTest_001
 * @tc.name     GetNumberMarkInfoFromCallerInfo.
 * @tc.desc     Function test
 */
HWTEST_F(NumberMarkCallerInfoGTest, CallerInfoNumberMarkVendorGTest_001, Function | MediumTest | Level1)
{
    Want want;
    string bundleName = "";
    string abilityName = "";
    string phoneNumber = "";
    want.SetElementName(bundleName, abilityName);
    shared_ptr<CallerInfoAbilityNumberMarkVendor> vendor = make_shared<CallerInfoAbilityNumberMarkVendor>(want);
    if (vendor == nullptr) {
        std::cout << "vendor is nullptr!" << std::endl;
    }
    std::cout << "vendor is not nullptr!" << std::endl;
    future<NumberMarkVendorResult> futureNumberMarkVendorResult = vendor->QueryNumberMark(phoneNumber);
    NumberMarkVendorResult numberMarkVendorResult = futureNumberMarkVendorResult.get();
    EXPECT_EQ(numberMarkVendorResult.numberMarkInfo.markType, MarkType::MARK_TYPE_NONE);
    CallerInfoQueryResult callerInfoQueryResult = vendor->QueryCallerInfo(phoneNumber);
    EXPECT_EQ(callerInfoQueryResult.errCode, NumberMarkVendorErrCode::OTHER_ERROR);
    vendor->appName_ = "abc";
    callerInfoQueryResult = vendor->QueryCallerInfo(phoneNumber);
    EXPECT_EQ(callerInfoQueryResult.errCode, NumberMarkVendorErrCode::OTHER_ERROR);
}

} // namespace Telephony
} // namespace OHOS