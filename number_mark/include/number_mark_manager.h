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

#ifndef NUMBER_MARK_MANAGER_H
#define NUMBER_MARK_MANAGER_H

#include "singleton.h"

#include <string>

namespace OHOS {
namespace Telephony {
using std::string;

/**
 * Only implemented part of `NumberMarkManager` in double framework repo.
 */
class NumberMarkManager {
    DECLARE_DELAYED_REF_SINGLETON(NumberMarkManager)

  public:
    static string StandardizationPhoneNum(const string &number);
    static bool GetFormatNumber(const string &number, string &endString, string &formatNumber);
    static string RemoveDashesAndBlanksBrackets(const string &number);
    static bool IsMobilePhoneNumber(const string &num);
    static string IpHeadBarber(const string &oriNumber);
    static bool IsMaritimeSatelliteNumber(const string &number);
    /**
     * Same to `CommonUtilMethods.parseFixedPhoneNumber(Context context, String number)` in double framework repo.
     */
    static string ParseFixedPhoneNumber(const string &number);
    static bool ShouldQueryWeLink(const string &phoneNumber, string &welinkQueryPhoneNumber);
};

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_MARK_MANAGER_H */