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
#define private public
#include "number_identity_settings.h"
#undef private
#include "number_identity_ddl.h"
#include "number_identity_log_wrapper.h"
#include "number_identity_errors.h"
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
class NumberIdentitySettingTest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void NumberIdentitySettingTest::SetUp() {}

void NumberIdentitySettingTest::TearDown() {}

void NumberIdentitySettingTest::SetUpTestCase() {}

void NumberIdentitySettingTest::TearDownTestCase() {}

HWTEST_F(NumberIdentitySettingTest, NumberIdentitySettingTest_001, Function | MediumTest | Level1)
{
    GetDeviceType();
    GetOsVersion();
    sptr<IRemoteObject> token = nullptr;
    const string key = "setting";
    string value;
    EXPECT_NE(GetSettingsData(key, value, token), NUMBER_IDENTITY_ERR_SUCCESS);
    IsNetworkRoaming();
}

HWTEST_F(NumberIdentitySettingTest, NumberIdentitySettingTest_002, Function | MediumTest | Level1)
{
    auto &net = DelayedRefSingleton<NetworkInfo>::GetInstance();
    net.Refresh();
    net.SetIp("127.0.0.1");
    EXPECT_TRUE(net.hasNetWork);
    net.Clear();
}

HWTEST_F(NumberIdentitySettingTest, NumberIdentitySettingTest_003, Function | MediumTest | Level1)
{
    NetworkInfo networkInfo;
    networkInfo.Refresh();
    networkInfo.SetIp("127.0.0.1");
    EXPECT_TRUE(networkInfo.hasNetWork);
    networkInfo.Clear();
}
} // namespace Telephony
} // namespace OHOS
