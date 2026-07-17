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
#include "number_identity_json_helper.h"
#include "string_ex.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
class JSONHelperGtest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void JSONHelperGtest::SetUp() {}

void JSONHelperGtest::TearDown() {}

void JSONHelperGtest::SetUpTestCase() {}

void JSONHelperGtest::TearDownTestCase() {}

HWTEST_F(JSONHelperGtest, Telephony_JSON_cJSONHelper_GetJSONType_Object, TestSize.Level0)
{
    cJSON *json;
    json = cJSON_Parse("{}");
    EXPECT_EQ(LowerStr(cJSONHelper_GetJSONType(json)), "object");
    cJSON_Delete(json);
}

HWTEST_F(JSONHelperGtest, Telephony_JSON_cJSONHelper_GetJSONType_Array, TestSize.Level0)
{
    cJSON *json;
    json = cJSON_Parse("[]");
    EXPECT_EQ(LowerStr(cJSONHelper_GetJSONType(json)), "array");
    cJSON_Delete(json);
}

HWTEST_F(JSONHelperGtest, Telephony_JSON_cJSONHelper_GetJSONType_String, TestSize.Level0)
{
    cJSON *json;
    json = cJSON_Parse(R"("abc")");
    EXPECT_EQ(LowerStr(cJSONHelper_GetJSONType(json)), "string");
    cJSON_Delete(json);
}

HWTEST_F(JSONHelperGtest, Telephony_JSON_cJSONHelper_GetJSONType_Number, TestSize.Level0)
{
    cJSON *json;
    json = cJSON_Parse("123");
    EXPECT_EQ(LowerStr(cJSONHelper_GetJSONType(json)), "number");
    cJSON_Delete(json);
}

HWTEST_F(JSONHelperGtest, Telephony_JSON_cJSONHelper_GetJSONType_Boolean, TestSize.Level0)
{
    cJSON *json;
    json = cJSON_Parse("true");
    EXPECT_EQ(LowerStr(cJSONHelper_GetJSONType(json)), "boolean");
    cJSON_Delete(json);
}

HWTEST_F(JSONHelperGtest, Telephony_JSON_cJSONHelper_GetJSONType_Null, TestSize.Level0)
{
    cJSON *json;
    json = cJSON_Parse("null");
    EXPECT_EQ(LowerStr(cJSONHelper_GetJSONType(json)), "null");
    cJSON_Delete(json);
}

HWTEST_F(JSONHelperGtest, Telephony_JSON_cJSONHelper_cJSONHelper_Stringify, TestSize.Level0)
{
    cJSON *json;
    json = cJSON_Parse("true");
    string result;
    EXPECT_TRUE(cJSONHelper_Stringify(json, result));
    cJSON_Delete(json);
    const string wrapped = "wrapped";
    EXPECT_TRUE(cJSONHelper_UnwrapJSONString(wrapped, result));
}
} // namespace Telephony
} // namespace OHOS
