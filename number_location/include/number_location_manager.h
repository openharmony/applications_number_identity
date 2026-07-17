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

#ifndef NUMBER_LCATION_MANAGER_H
#define NUMBER_LCATION_MANAGER_H

#include <list>
#include "number_identity_inner_type.h"
#include "pac_map.h"
#include "singleton.h"

namespace OHOS {
namespace Telephony {
class NumberLocationManager {
    DECLARE_DELAYED_SINGLETON(NumberLocationManager)
public:
    std::string GetNumberLocation(std::string number, bool isExactMatch, const std::string &currentIso);
    std::string DeleteInternationalPrefix(std::string number, std::string countryIso);
    std::string GetAreaCode(std::string number);
    std::string GetCountryIso();

private:
    std::string GetNumberLocationDefault(std::string number, std::string countryIso, bool isExactMatch);
    std::string GetNumberLocationNotCn(std::string number, std::string countryIso, bool isExactMatch);
    std::string GetAttributionInfo(std::string number, std::string countryIso, bool isExactMatch);
    std::string GetNationalAttributionInfo(std::string number);
    std::string IpHeadBarber(std::string number);
    std::string GetNatAttributionInfo(std::string number, std::string countryIso);
    std::string GetDescription(std::string number, std::string countryIso);
    std::string DeleteInternationalPrefix(std::string number, std::string countryIso, bool isExactMatch);
    void Trim(std::string &number);
    std::string GetCountryIsoFromNetwork();
    std::string GetCountryIsoFromSim();
    std::string GetCountryIsoFromLocal();

private:
    std::map<std::string, std::list<std::string>> internationPrefix;
};
} // namespace Telephony
} // namespace OHOS

#endif // NUMBER_LCATION_MANAGER_H