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

#ifndef FIX_PHONE_NUMBER_H
#define FIX_PHONE_NUMBER_H

#include <cstdio>
#include <string>
#include "pac_map.h"

namespace OHOS {
namespace Telephony {
class FixPhoneNumber {
public:
    FixPhoneNumber(std::string number);
    ~FixPhoneNumber();
    /**
     * In double framework repo, this method is called `getParseResult()`.
     */
    std::string GetFixParseResult();
    /**
     * In double framework repo, this method is called `parseFixedPhoneNumber()`.
     */
    int32_t ParseFixPhoneNumber();
    /**
     * In double framework repo, this method is implemented in sub class `FixedPhoneNumberCache`.
     */
    std::string SubstringFixedPhoneNumber();

private:
    std::string fixPhoneNumber_ = "";
    std::string areaCode_ = "";
};
} // namespace Telephony
} // namespace OHOS

#endif // FIX_PHONE_NUMBER_H