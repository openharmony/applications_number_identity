/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "fix_phone_number.h"
#include <gtest/gtest.h>
#include <cstring>
#include <string>

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

class FixPhoneNumberGtest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};
void FixPhoneNumberGtest::SetUp() {}

void FixPhoneNumberGtest::TearDown() {}

void FixPhoneNumberGtest::SetUpTestCase() {}

void FixPhoneNumberGtest::TearDownTestCase() {}

/**
 * @tc.number   FixPhoneNumber_001
 * @tc.name     DownloadFileAbility Create.
 * @tc.desc     Function test
 * @tc.require: DTS2024061803720
 */
HWTEST_F(FixPhoneNumberGtest, FixPhoneNumber_001, Function | MediumTest | Level1)
{
    FixPhoneNumber fixNumber("0218012580103");
    EXPECT_NE(fixNumber.SubstringFixedPhoneNumber(), "");
}

}
}