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

#include "number_identity_pinyin_helper.h"
#include "singleton.h"
#include "string_ex.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
class PinyinHelperGtest : public testing::Test {
  public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void PinyinHelperGtest::SetUp() {}

void PinyinHelperGtest::TearDown() {}

void PinyinHelperGtest::SetUpTestCase() {}

void PinyinHelperGtest::TearDownTestCase() {}

HWTEST_F(PinyinHelperGtest, Telephony_Pinyin_convert_pinyin, Function | MediumTest | Level1)
{
    auto &pinyinHelper = DelayedRefSingleton<PinyinHelper>::GetInstance();
    auto pinyin = pinyinHelper.ConvertChineseToPinyin("\u957f");
    EXPECT_EQ(UpperStr(pinyin), " CHANG");
}

HWTEST_F(PinyinHelperGtest, Telephony_Pinyin_get_pinyin_abbreviation, Function | MediumTest | Level1)
{
    auto &pinyinHelper = DelayedRefSingleton<PinyinHelper>::GetInstance();
    auto abbr = pinyinHelper.GetPinyinAbbreviation(" YI LONG LV XING WANG JI PIAO JIU DIAN YU DING DIAN HUA");
    EXPECT_EQ(UpperStr(abbr), "YLLXWJPJDYDDH");
}

HWTEST_F(PinyinHelperGtest, get_pinyin_sub_string, Function | MediumTest | Level1)
{
    auto &pinyinHelper = DelayedRefSingleton<PinyinHelper>::GetInstance();
    auto abbr = pinyinHelper.GetPinyinSubString(" YI LONG LV XING WANG JI PIAO JIU DIAN YU DING DIAN HUA");
    EXPECT_EQ(UpperStr(abbr), " HUA DIANHUA DINGDIANHUA YUDINGDIANHUA DIANYUDINGDIANHUA JIUDIANYUDINGDIANHUA "
                              "PIAOJIUDIANYUDINGDIANHUA JIPIAOJIUDIANYUDINGDIANHUA WANGJIPIAOJIUDIANYUDINGDIANHUA "
                              "XINGWANGJIPIAOJIUDIANYUDINGDIANHUA LVXINGWANGJIPIAOJIUDIANYUDINGDIANHUA "
                              "LONGLVXINGWANGJIPIAOJIUDIANYUDINGDIANHUA YILONGLVXINGWANGJIPIAOJIUDIANYUDINGDIANHUA");
}

HWTEST_F(PinyinHelperGtest, get_multi_pinyin, Function | MediumTest | Level1)
{
    auto &pinyinHelper = DelayedRefSingleton<PinyinHelper>::GetInstance();
    auto p1 = pinyinHelper.ConvertChineseToPinyin("锦江会员客服");
    auto p2 = pinyinHelper.ConvertChineseToPinyin("员");
    EXPECT_NE(p1.find(p2), string::npos);
    EXPECT_NE(UpperStr(p2).find("YUAN"), string::npos);
}

HWTEST_F(PinyinHelperGtest, get_pinyin_seperate, Function | MediumTest | Level1)
{
    auto &pinyinHelper = DelayedRefSingleton<PinyinHelper>::GetInstance();
    auto p1 = pinyinHelper.ConvertChineseToPinyin("中国联通VIP客服电话");
    EXPECT_NE(p1.find("V I P"), string::npos);
    EXPECT_EQ(p1.find("VIP"), string::npos);
}

} // namespace Telephony
} // namespace OHOS
