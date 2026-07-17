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

#ifndef MOBILE_PHONE_NUMBER_H
#define MOBILE_PHONE_NUMBER_H

#include <cstdio>
#include <string>
#include "pac_map.h"

namespace OHOS {
namespace Telephony {
class MobilePhoneNumber {
public:
    MobilePhoneNumber(std::string number);
    ~MobilePhoneNumber();
    std::string GetMobileParseResult();
    std::string GetPhoneOperator();
    int32_t ParseMobilePhoneNumber();
    std::string GetLocation();

private:
    std::string mobilePhoneNumber_ = "";
    std::string phoneOperator_ = "";
    std::string location_ = "";
    std::string ndc3String_ = "";
    std::string ndc7String_ = "";
};
} // namespace Telephony
} // namespace OHOS

#endif // MOBILE_PHONE_NUMBER_H