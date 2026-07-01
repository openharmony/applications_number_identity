/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef NUMBER_IDENTITY_PINYIN_HELPER_H
#define NUMBER_IDENTITY_PINYIN_HELPER_H

#include <map>
#include <string>

#include "unicode/rep.h"
#include "singleton.h"
#include "unicode/umachine.h"
#include "unicode/translit.h"

namespace OHOS {
namespace Telephony {
using std::map;
using std::string;
class PinyinHelper {
    DECLARE_DELAYED_REF_SINGLETON(PinyinHelper);

  public:
    string GetPinyinAbbreviation(const string &pinyin);
    string GetPinyinSubString(const string &pinyin);
    string ConvertChineseToPinyin(const string &chinese);

  private:
    /**
     * Some chinese characters with multiple pronunciations cannot be converted to chinese pinyin directly.
     * This method provides the mapping of them (from chinese to pinyin) for yellow page.
     */
    map<UChar32, string> GetYellowPageMultiPinyins();
    void ForceMultiPinyinAsDefined(UChar32 chinese, string &pinyin);
    icu::Transliterator *transliterator;
};
} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_PINYIN_HELPER_H */