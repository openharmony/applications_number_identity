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

#include "number_location_manager.h"

#include <regex>

#include "call_manager_client.h"
#include "core_service_client.h"
#include "fix_phone_number.h"
#include "locale_config.h"
#include "locator.h"
#include "mobile_phone_number.h"
#include "number_location_utils.h"
#include "number_identity_log_wrapper.h"
#include "phone_number_format.h"
#include "parameters.h"

namespace OHOS {
namespace Telephony {

const int32_t TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID = 4005;
const char *CHINA_NETWORK_COUNTRYISO = "CN";
const char *MACAO_NETWORK_COUNTRYISO = "MO";
const char *ZERO_MOBILE_PATTERN = "0(13[0-9]|14[5-9]|15[012356789]|16[2567]|17[2-8]|18[0-9]|19[012356789])[0-9]{8}";
const char *LOCAL_PATTERN = "^(1)\\d{10}$";
const char *MPN_QUERY_PATTERN_PREFIX = "^((\\+86)|(86)|(0086))?(1)\\d{2,10}$";
const char *FIX_QUERY_PATTERN = "^(0\\d{2,9})|(0\\d{2,10})|(0\\d{2,11})$";
const char *MPN_PATTERN = "^((\\+86)|(86)|(0086))?(1)[1-9]\\d{9}$";
const char *NAT_PATTERN = "^(\\+[1-9]([0-9]{0,14}))$|^(00[1-9]([0-9]{0,13}))$";
const char *FIX_PATTERN = "(0\\d{9,11})|(0\\d{2,3}(?:100\\d{2}|95\\d{3,4}))";
const char *CHINA_AREACODE = "0";
const int32_t MPN_MIN_MATCH_LENGTH = 7;
const int32_t MIN_MATCH_SIZE_NOT_CN = 7;
const int32_t MPN_NUMBER_PATTERN1_LENGTH = 3;
const int32_t MPN_NUMBER_PATTERN2_LENGTH = 4;
const int32_t FIX_MIN_MATCH_LENGTH = 3;
const int32_t FIX_TOP2_STRING_LENGTH = 2;
const int32_t FIX_NORMAL_MATCH_LENGTH = 4;
const int32_t MIN_MATCH_SIZE_CN = 8;
const int32_t IPHEAD_LENGTH = 5;
const int32_t SECOND_NUMBER_INDEX = 1;
std::map<std::string, std::list<std::string>> internationPrefix = {
    {"KR", {"00700", "00365", "001", "005", "006", "00727", "00766", "008", "00794", "002"}},
    {"AU", {"0014", "0015", "0016", "0019", "0011"}},
    {"CU", {"119"}},
    {"FI", {"990", "994", "999", "00"}},
    {"BR", {"0014", "0015", "0021", "0023", "0031"}},
    {"CA", {"011"}},
    {"MN", {"001"}},
    {"NG", {"009"}},
    {"CO", {"005", "007", "009"}},
    {"KP", {"99"}},
    {"KH", {"001"}},
    {"KZ", {"810"}},
    {"TH", {"001"}},
    {"UZ", {"810"}},
    {"HK", {"001"}},
    {"TW", {"002", "005", "006", "007", "009", "9002", "9005", "9006", "9007", "9009"}},
    {"SG", {"000", "001", "002", "008"}},
    {"RU", {"810", "858", "856", "857", "859", "826", "827", "828"}},
    {"JP", {"001", "0033", "0036", "0037", "0039", "0041",
            "0053", "0056", "0061", "0066", "0071", "0077", "0080", "0081", "0088", "0089", "0091"}},
    {"ID", {"01018", "01019", "001", "008", "01016", "007", "01017", "009", "01010", "01012"}},
    {"PY", {"002"}},
    {"US", {"011"}}
};

NumberLocationManager::NumberLocationManager() {}

NumberLocationManager::~NumberLocationManager() {}

std::string NumberLocationManager::GetAttributionInfo(std::string number, std::string countryIso, bool isExactMatch)
{
    std::string phoneNumber = number;
    if (std::regex_match(phoneNumber, std::regex(ZERO_MOBILE_PATTERN))) {
        phoneNumber = phoneNumber.substr(SECOND_NUMBER_INDEX);
    }
    std::string resString = "";
    if (std::regex_match(phoneNumber, std::regex(MPN_PATTERN))) {
        std::unique_ptr<MobilePhoneNumber> mobilePhoneNumber = std::make_unique<MobilePhoneNumber>(phoneNumber);
        resString = mobilePhoneNumber->GetMobileParseResult();
    } else if (std::regex_match(phoneNumber, std::regex(NAT_PATTERN))) {
        resString = GetNatAttributionInfo(phoneNumber, countryIso);
    } else if (std::regex_match(phoneNumber, std::regex(FIX_PATTERN))) {
        std::unique_ptr<FixPhoneNumber> fixPhoneNumber = std::make_unique<FixPhoneNumber>(phoneNumber);
        resString = fixPhoneNumber->GetFixParseResult();
    } else if (!isExactMatch && std::regex_match(phoneNumber, std::regex(MPN_QUERY_PATTERN_PREFIX))) {
        std::unique_ptr<MobilePhoneNumber> mobilePhoneNumber = std::make_unique<MobilePhoneNumber>(phoneNumber);
        mobilePhoneNumber->GetMobileParseResult();
        std::string showLocation = mobilePhoneNumber->GetLocation();
        std::string showOperator = mobilePhoneNumber->GetPhoneOperator();
        if (showLocation.empty()) {
            resString = showOperator;
        } else if (showOperator.empty()) {
            resString = showLocation;
        } else {
            resString = showLocation + " " + showOperator;
        }
    } else if (!isExactMatch && std::regex_match(phoneNumber, std::regex(FIX_QUERY_PATTERN))) {
        std::unique_ptr<FixPhoneNumber> fixPhoneNumber = std::make_unique<FixPhoneNumber>(phoneNumber);
        resString = fixPhoneNumber->GetFixParseResult();
    }
    return resString;
}

std::string NumberLocationManager::GetNatAttributionInfo(std::string number, std::string countryIso)
{
    std::string fixNumber = number;
    bool isChinaFixNumber = false;
    if (std::strncmp(fixNumber.c_str(), "+86", MPN_NUMBER_PATTERN1_LENGTH) == 0) {
        fixNumber = fixNumber.substr(MPN_NUMBER_PATTERN1_LENGTH);
        isChinaFixNumber = true;
    } else if (std::strncmp(fixNumber.c_str(), "0086", MPN_NUMBER_PATTERN2_LENGTH) == 0) {
        fixNumber = fixNumber.substr(MPN_NUMBER_PATTERN2_LENGTH);
        isChinaFixNumber = true;
    }
    std::string res = "";
    if (isChinaFixNumber) {
        if (std::strncmp(fixNumber.c_str(), CHINA_AREACODE, SECOND_NUMBER_INDEX) != 0) {
            fixNumber = CHINA_AREACODE + fixNumber;
        }
        if (std::regex_match(fixNumber, std::regex(FIX_PATTERN))) {
            std::unique_ptr<FixPhoneNumber> fixPhoneNumber = std::make_unique<FixPhoneNumber>(fixNumber);
            res = fixPhoneNumber->GetFixParseResult();
            return res;
        } else {
            return res;
        }
    } else {
        res = GetDescription(number, countryIso);
    }
    return res;
}

std::string NumberLocationManager::GetNumberLocationDefault(
    std::string number, std::string countryIso, bool isExactMatch)
{
    std::u16string tmpCountryIso = Str8ToStr16(countryIso);
    Trim(number);
    std::u16string formatNumber = Str8ToStr16(number);
    std::u16string phoneNumber = Str8ToStr16(number);
    if (std::strcmp(countryIso.c_str(), CHINA_NETWORK_COUNTRYISO) == 0) {
        if (number.length() > MIN_MATCH_SIZE_CN && (number.at(0) != '0')) {
            DelayedSingleton<CallManagerClient>::GetInstance()->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
            DelayedSingleton<CallManagerClient>::GetInstance()->FormatPhoneNumberToE164(
                phoneNumber, tmpCountryIso, formatNumber);
            if (formatNumber.empty()) {
                formatNumber = Str8ToStr16(number);
            }
        }
    }
    std::string resString = GetAttributionInfo(Str16ToStr8(formatNumber), countryIso, isExactMatch);
    return resString;
}

std::string NumberLocationManager::GetNumberLocationNotCn(
    std::string number, std::string countryIso, bool isExactMatch)
{
    NUMBER_IDENTITY_LOGI("get number location not cn.");
    std::string res = "";
    if (number.length() < MIN_MATCH_SIZE_NOT_CN) {
        return res;
    }
    res = DeleteInternationalPrefix(number, countryIso);
    std::string tempNumber = "";
    if (std::strncmp(res.c_str(), "+86", MPN_NUMBER_PATTERN1_LENGTH) &&
        std::strncmp(res.c_str(), "0086", MPN_NUMBER_PATTERN2_LENGTH)) {
        if (std::regex_match(res, std::regex(NAT_PATTERN))) {
            tempNumber = GetAttributionInfo(res, countryIso, isExactMatch);
        } else {
            tempNumber = GetDescription(res, countryIso);
        }
    } else {
        tempNumber = GetAttributionInfo(res, countryIso, isExactMatch);
    }
    return tempNumber;
}

std::string NumberLocationManager::GetNumberLocation(std::string number, bool isExactMatch,
    const std::string &currentIso)
{
    std::string res = "";
    if (number.empty()) {
        NUMBER_IDENTITY_LOGE("number is empty!");
        return res;
    }

    if (OHOS::system::GetParameter("const.global.region", "CN") != "CN") {
        std::string res = "";
        if (number.length() < MIN_MATCH_SIZE_NOT_CN) {
            NUMBER_IDENTITY_LOGE("number length is not long enough.");
            return res;
        }
        res = DeleteInternationalPrefix(number, currentIso);
        return GetDescription(res, currentIso);
    }

    std::string numberPattern = IpHeadBarber(number);
    if (numberPattern.empty()) {
        NUMBER_IDENTITY_LOGE("numberPattern is empty!");
        return res;
    }
    if (strcmp(currentIso.c_str(), CHINA_NETWORK_COUNTRYISO) == 0) {
        res = GetNumberLocationDefault(numberPattern, currentIso, isExactMatch);
    } else {
        NUMBER_IDENTITY_LOGI("Current ISO is not CN.");
        res = GetNumberLocationNotCn(numberPattern, currentIso, isExactMatch);
    }
    return res;
}

std::string NumberLocationManager::DeleteInternationalPrefix(std::string number, std::string countryIso)
{
    NUMBER_IDENTITY_LOGI("delete international prefix.");
    std::string res = number;
    if (number.empty() || countryIso.empty()) {
        NUMBER_IDENTITY_LOGE("number or countryIso is empty!");
        return res;
    }
    std::list<std::string> natPrefixNumber;
    auto iter = internationPrefix.find(countryIso);
    if (iter != internationPrefix.end()) {
        natPrefixNumber = iter->second;
    }
    if (natPrefixNumber.empty()) {
        NUMBER_IDENTITY_LOGE("natPrefixNumber is empty!");
        return res;
    }
    std::string initernationCallPrefix = "";
    for (std::string tempNum : natPrefixNumber) {
        if (std::strncmp(number.c_str(), tempNum.c_str(), tempNum.length()) == 0) {
            initernationCallPrefix = tempNum;
            break;
        }
    }
    if (!initernationCallPrefix.empty()) {
        res = number.substr(initernationCallPrefix.length());
        res = "+" + res;
    }
    NUMBER_IDENTITY_LOGI("delete international prefix end.");
    return res;
}

std::string NumberLocationManager::IpHeadBarber(std::string number)
{
    std::string resString = number;
    std::vector<std::string> ipHeadList = {"17900", "17901", "17908", "17909", "11808", "17950", "17951",
        "12593", "17931", "17910", "17911", "17960", "17968", "17969", "10193", "96435"};
    uint32_t numberLen = resString.length();
    if (numberLen < IPHEAD_LENGTH) {
        return resString;
    }
    std::string ipHeadString = resString.substr(0, IPHEAD_LENGTH);
    std::vector<std::string>::iterator head = std::find(ipHeadList.begin(), ipHeadList.end(), ipHeadString);
    if (head != ipHeadList.end()) {
        resString = number.substr(IPHEAD_LENGTH, numberLen);
    }
    return resString;
}

std::string NumberLocationManager::GetAreaCode(std::string number)
{
    NUMBER_IDENTITY_LOGI("get area code.");
    std::string resString = "";
    if (number.empty()) {
        NUMBER_IDENTITY_LOGE("number is empty!");
        return resString;
    }
    std::string phoneNumber = IpHeadBarber(number);
    if (phoneNumber.empty()) {
        NUMBER_IDENTITY_LOGE("number is empty!");
        return resString;
    }
    if (std::regex_match(phoneNumber, std::regex(MPN_PATTERN))) {
        phoneNumber = phoneNumber.substr(0, MPN_MIN_MATCH_LENGTH);
        resString = DelayedSingleton<NumberLocationUtils>::GetInstance()->QueryUnicodeAreaCodeByPhoneNum(phoneNumber);
    } else if (std::regex_match(
        phoneNumber, std::regex(FIX_PATTERN)) && phoneNumber.length() >= FIX_MIN_MATCH_LENGTH) {
        std::string fixedNumberTop2Token1 = "01";
        std::string fixedNumberTop2Token2 = "02";
        std::string top2Str = phoneNumber.substr(0, FIX_TOP2_STRING_LENGTH);
        if (std::strcmp(top2Str.c_str(), fixedNumberTop2Token1.c_str()) == 0 ||
            std::strcmp(top2Str.c_str(), fixedNumberTop2Token2.c_str()) == 0) {
            resString = phoneNumber.substr(0, FIX_MIN_MATCH_LENGTH);
        } else if (phoneNumber.length() >= FIX_NORMAL_MATCH_LENGTH) {
            resString = phoneNumber.substr(0, FIX_NORMAL_MATCH_LENGTH);
        } else {
            NUMBER_IDENTITY_LOGE("not match!");
        }
    } else {
        NUMBER_IDENTITY_LOGE("not match!");
    }
    return resString;
}

std::string NumberLocationManager::GetDescription(std::string number, std::string countryIso)
{
    std::string locale = Global::I18n::LocaleConfig::GetSystemLocale();
    std::string formatType = "";
    std::map<std::string, std::string> options = {{"type", "INTERNATIONAL"}};
    std::string geoDiscription = "";
    std::unique_ptr<Global::I18n::PhoneNumberFormat> formatter =
        Global::I18n::PhoneNumberFormat::CreateInstance(countryIso, options);
    if (formatter == nullptr) {
        NUMBER_IDENTITY_LOGE("formatter is nullptr!");
        return geoDiscription;
    }
    if (!formatter->isValidPhoneNumber(number)) {
        NUMBER_IDENTITY_LOGE("phoneNum is is inValid!");
        return geoDiscription;
    }
    geoDiscription = formatter->getLocationName(number, locale);
    return geoDiscription;
}

void NumberLocationManager::Trim(std::string &number)
{
    ulong index = 0;
    if (!number.empty()) {
        while ((index = number.find(' ', index)) != std::string::npos) {
            number.erase(index, SECOND_NUMBER_INDEX);
        }
    }
}

std::string NumberLocationManager::GetCountryIso()
{
    std::string countryIso = "";
    countryIso = GetCountryIsoFromNetwork();
    if (countryIso.empty()) {
        NUMBER_IDENTITY_LOGW("GetCountryIsoFromNetwork is empty");
        countryIso = GetCountryIsoFromSim();
    }
    if (countryIso.empty()) {
        NUMBER_IDENTITY_LOGW("GetCountryIsoFromSim is empty");
        countryIso = GetCountryIsoFromLocal();
    }
    transform(countryIso.begin(), countryIso.end(), countryIso.begin(), ::toupper);
    return countryIso;
}

std::string NumberLocationManager::GetCountryIsoFromNetwork()
{
    int32_t slotId = 0;
    CoreServiceClient::GetInstance().GetPrimarySlotId(slotId);
    std::u16string countryIso = u"";
    CoreServiceClient::GetInstance().GetIsoCountryCodeForNetwork(slotId, countryIso);
    return Str16ToStr8(countryIso);
}

std::string NumberLocationManager::GetCountryIsoFromSim()
{
    int32_t slotId = 0;
    CoreServiceClient::GetInstance().GetPrimarySlotId(slotId);
    std::u16string countryIso = u"";
    CoreServiceClient::GetInstance().GetISOCountryCodeForSim(slotId, countryIso);
    return Str16ToStr8(countryIso);
}

std::string NumberLocationManager::GetCountryIsoFromLocal()
{
    std::string currentIso = Global::I18n::LocaleConfig::GetSystemRegion();
    return currentIso;
}

} // namespace Telephony
} // namespace OHOS
