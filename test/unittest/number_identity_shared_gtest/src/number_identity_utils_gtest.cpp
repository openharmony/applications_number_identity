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

#include "cJSON.h"
#include <cstdint>
#define private public
#include "number_identity_utils.h"
#undef private
#include "string_ex.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include <gtest/gtest.h>
#include <iostream>
#include <optional>

using namespace std;

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
class NumberIdentityUtilsGTest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void NumberIdentityUtilsGTest::SetUp() {}

void NumberIdentityUtilsGTest::TearDown() {}

void NumberIdentityUtilsGTest::SetUpTestCase() {}

void NumberIdentityUtilsGTest::TearDownTestCase() {}

HWTEST_F(NumberIdentityUtilsGTest, Telephony_RandomLong, TestSize.Level0)
{
    auto v1 = GenerateRandomLong();
    auto v2 = GenerateRandomLong();
    EXPECT_NE(v1, v2);
}

HWTEST_F(NumberIdentityUtilsGTest, Telephony_Base64_Encode, TestSize.Level0)
{
    const char bytes[] = { 1, 2, 3, 4 };
    auto encoded = EncodeBase64(bytes, sizeof(bytes));
    EXPECT_EQ(encoded, "AQIDBA==");
}

HWTEST_F(NumberIdentityUtilsGTest, Telephony_Base64_Decode, TestSize.Level0)
{
    auto decoded = DecodeBase64("AQIDBA==");
    const char bytes[] = { 1, 2, 3, 4 };
    EXPECT_EQ(decoded, "\x01\x02\x03\x04");
    EXPECT_EQ(decoded.compare(0, decoded.size(), bytes, decoded.size()), 0);
}

HWTEST_F(NumberIdentityUtilsGTest, Telephony_To_HexString, TestSize.Level0)
{
    auto hexstr = ToHexString("abc");
    cout << "encoded = " << hexstr << endl;
    EXPECT_EQ(hexstr, "616263");
}

HWTEST_F(NumberIdentityUtilsGTest, Telephony_Generate_MD5, TestSize.Level0)
{
    vector<uint8_t> bytes = {'a', 'b', 'c'};
    auto md5 = GenerateMD5(bytes);
    EXPECT_NE(md5, nullopt);
    auto md5Hex = ToHexString(*md5);
    EXPECT_EQ(md5Hex, "900150983cd24fb0d6963f7d28e17f72");
}

HWTEST_F(NumberIdentityUtilsGTest, Telephony_To_GetUriPathName, TestSize.Level0)
{
    string path = "";
    Uri uri(path);
    string uriPath = GetUriPathName(uri);
    EXPECT_GE(uriPath, "");
}

HWTEST_F(NumberIdentityUtilsGTest, Telephony_To_TimeStart, TestSize.Level0)
{
    TimeLogger logger;
    logger.TimeStart();
    EXPECT_GE(logger.startTime_, 0);
    logger.TimeEnd("tag");
}
} // namespace Telephony
} // namespace OHOS
