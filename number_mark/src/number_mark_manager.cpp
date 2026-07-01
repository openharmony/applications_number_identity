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

#include "number_mark_manager.h"
#include "call_manager_client.h"
#include "fix_phone_number.h"
#include "number_identity_settings.h"
#include "number_identity_utils.h"
#include "number_location_manager.h"
#include "singleton.h"
#include "telephony_errors.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <set>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {
using namespace std;
const size_t IPHEAD_LENGTH = 5;
const size_t MIN_LEN_OF_MS_NUMBER = 6;
const size_t MAX_LEN_OF_MS_NUMBER = 20;
const size_t MAX_CHINA_MOBILE_NUMBER_LENGTH = 11;
const size_t MOBILE_PHONE_LENGTH = 11;
const char *LOCAL_MARK = "0";
const char *VALID_1ST_0_PATTERN = "0(13[0-9]|14[5-9]|15[012356789]|16[2567]|17[0-8]|18[0-9]|19[012356789])[0-9]{8}";
const char *HW_CALL_API_FACTORY_MPN_PATTERN = "1\\d{10}$";
const char *E164_CHINA_MAINLAND_PREFIX1 = "86";
const char *E164_CHINA_MAINLAND_PREFIX2 = "+86";
const char *E164_CHINA_MAINLAND_PREFIX3 = "0086";
constexpr int QUERY_NUM_LENGTH_LIMIT = 7;
const int32_t TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID = 4005;

NumberMarkManager::NumberMarkManager() {}

NumberMarkManager::~NumberMarkManager() {}

string NumberMarkManager::RemoveDashesAndBlanksBrackets(const string &number)
{
    static string charsToRemove = " -()";
    string result;
    std::copy_if(number.cbegin(), number.cend(), std::back_inserter(result), [](char ch) {
        return !std::any_of(charsToRemove.begin(), charsToRemove.end(), [ch](char c) { return c == ch; });
    });
    return result;
}

string NumberMarkManager::StandardizationPhoneNum(const string &number)
{
    if (number.empty()) {
        return "";
    }
    auto num = RemoveDashesAndBlanksBrackets(number);
    num = IpHeadBarber(num);
    if (LOGI_EXPR(IsNetworkRoaming()) && !StartsWith(num, E164_CHINA_MAINLAND_PREFIX2)) {
        auto numberLocationManager = DelayedSingleton<NumberLocationManager>().GetInstance();
        u16string formatNumber;
        u16string countryCode = Str8ToStr16(numberLocationManager->GetCountryIso());
        auto str16Number = Str8ToStr16(number);
        auto callManagerClient = DelayedSingleton<CallManagerClient>::GetInstance();
        auto errCode = callManagerClient->FormatPhoneNumberToE164(str16Number, countryCode, formatNumber);
        NUMBER_IDENTITY_LOGI("errorcode: %{public}d", errCode);
        auto noCountryHeadNum = Str16ToStr8(formatNumber);
        if (errCode == TELEPHONY_SUCCESS && StartsWith(noCountryHeadNum, E164_CHINA_MAINLAND_PREFIX2)) {
            auto noPrefixNum = noCountryHeadNum.substr(strlen(E164_CHINA_MAINLAND_PREFIX2));
            if (!RegexMatches(noPrefixNum, HW_CALL_API_FACTORY_MPN_PATTERN) &&
                !(StartsWith(noPrefixNum, "400") || StartsWith(noPrefixNum, "800"))) {
                noCountryHeadNum = LOCAL_MARK + noPrefixNum;
            } else {
                // fix or mobile number
                noCountryHeadNum = noPrefixNum;
            }
            num = noCountryHeadNum;
        }
    }
    if (RegexMatches(num, VALID_1ST_0_PATTERN)) {
        num = num.substr(1);
    }
    auto prefixes = { E164_CHINA_MAINLAND_PREFIX1, E164_CHINA_MAINLAND_PREFIX2, E164_CHINA_MAINLAND_PREFIX3 };
    if (std::any_of(prefixes.begin(), prefixes.end(), [&](auto prefix) { return prefix == num; })) {
        return num;
    }
    if (StartsWith(num, E164_CHINA_MAINLAND_PREFIX1) && IsMobilePhoneNumber(num)) {
        num = num.substr(strlen(E164_CHINA_MAINLAND_PREFIX1));
    } else if (StartsWith(num, E164_CHINA_MAINLAND_PREFIX2)) {
        num = num.substr(strlen(E164_CHINA_MAINLAND_PREFIX2));
    } else if (StartsWith(num, E164_CHINA_MAINLAND_PREFIX3)) {
        num = num.substr(strlen(E164_CHINA_MAINLAND_PREFIX3));
    }
    return num;
}

bool NumberMarkManager::GetFormatNumber(const string &number, string &endString, string &formatNumber)
{
    NUMBER_IDENTITY_LOGI("get format number.");
    if (number.length() > QUERY_NUM_LENGTH_LIMIT) {
        endString = number.substr(number.length() - QUERY_NUM_LENGTH_LIMIT, QUERY_NUM_LENGTH_LIMIT);
    } else {
        endString = number;
    }
    auto numberLocationManager = DelayedSingleton<NumberLocationManager>().GetInstance();
    u16string formatNumber16;
    u16string countryCode = Str8ToStr16(numberLocationManager->GetCountryIso());
    auto number16 = Str8ToStr16(number);
    auto callManagerClient = DelayedSingleton<CallManagerClient>::GetInstance();
    callManagerClient->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
    auto errCode = callManagerClient->FormatPhoneNumberToE164(number16, countryCode, formatNumber16);
    if (errCode == TELEPHONY_SUCCESS) {
        formatNumber = Str16ToStr8(formatNumber16);
        return true;
    }
    NUMBER_IDENTITY_LOGI("errorcode: %{public}d", errCode);
    return false;
}

bool NumberMarkManager::IsMobilePhoneNumber(const string &num)
{
    if (num.empty()) {
        return false;
    }
    string subNum = "";

    // sub last 11 char from num
    if (num.length() > MAX_CHINA_MOBILE_NUMBER_LENGTH) {
        subNum = num.substr(num.length() - MAX_CHINA_MOBILE_NUMBER_LENGTH, num.length());
    }
    if (RegexMatches(num, HW_CALL_API_FACTORY_MPN_PATTERN) || RegexMatches(subNum, HW_CALL_API_FACTORY_MPN_PATTERN)) {
        return true;
    }
    return false;
}

string NumberMarkManager::IpHeadBarber(const string &oriNumber)
{
    string result = oriNumber;
    static set<string> ipHeads = { "17900", "17901", "17908", "17909", "11808", "17950", "17951", "12593", "17931",
        "17910", "17911", "17960", "17968", "17969", "10193", "96435" };

    // prevent the substring out of bound exception
    size_t numberLen = oriNumber.length();
    if (numberLen < IPHEAD_LENGTH) {
        // number length less than ip-prefix length
        return oriNumber;
    }
    string ipHead = oriNumber.substr(0, IPHEAD_LENGTH);
    // whether is ip head is legal
    if (ipHeads.find(ipHead) != ipHeads.end()) {
        result = oriNumber.substr(IPHEAD_LENGTH, numberLen);
    }
    return result;
}

bool NumberMarkManager::IsMaritimeSatelliteNumber(const string &number)
{
    if (number.size() < MIN_LEN_OF_MS_NUMBER || number.size() > MAX_LEN_OF_MS_NUMBER) {
        return false;
    }
    static const vector<string> MARITIME_HEAD = { "00870", "00871", "00872", "00873", "00874" };
    return any_of(MARITIME_HEAD.begin(), MARITIME_HEAD.end(), std::bind(StartsWith, number, placeholders::_1));
}

string NumberMarkManager::ParseFixedPhoneNumber(const string &number)
{
    if (number.empty()) {
        return number;
    }
    string tempNum = RemoveDashesAndBlanksBrackets(number);
    FixPhoneNumber fpnc(tempNum);
    string result = fpnc.SubstringFixedPhoneNumber();
    string resultString = tempNum == result ? number : result;
    return resultString;
}

bool NumberMarkManager::ShouldQueryWeLink(const string &phoneNumber, string &welinkQueryPhoneNumber)
{
    string noAreaNum;
    if (StartsWith(phoneNumber, LOCAL_MARK)) {
        noAreaNum = ParseFixedPhoneNumber(phoneNumber);
    } else {
        noAreaNum = phoneNumber;
    }
    if (noAreaNum.length() == MOBILE_PHONE_LENGTH && StartsWith(noAreaNum, "1")) {
        welinkQueryPhoneNumber = noAreaNum;
        return true;
    }
    return false;
}

} // namespace Telephony
} // namespace OHOS