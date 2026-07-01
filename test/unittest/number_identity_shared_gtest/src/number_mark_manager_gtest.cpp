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

#include "number_mark_manager.h"
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
class NumberMarkManagerTest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void NumberMarkManagerTest::SetUp() {}

void NumberMarkManagerTest::TearDown() {}

void NumberMarkManagerTest::SetUpTestCase() {}

void NumberMarkManagerTest::TearDownTestCase() {}

HWTEST_F(NumberMarkManagerTest, NumberMarkManagerTest_001, Function | MediumTest | Level1)
{
    std::string number = "17900345678";
    NumberMarkManager::StandardizationPhoneNum(number);
    number = "12593035894";
    number = NumberMarkManager::RemoveDashesAndBlanksBrackets(number);
    NumberMarkManager::IsMobilePhoneNumber(number);
    EXPECT_NE(NumberMarkManager::IpHeadBarber(number), "");
}

} // namespace Telephony
} // namespace OHOS
