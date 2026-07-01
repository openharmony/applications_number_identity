/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "number_location_utils.h"

#include <regex>
#include "number_location_db_parse.h"
#include "number_identity_log_wrapper.h"

namespace OHOS {
namespace Telephony {

const int32_t GEO_INFO_MAX_ILLEGAL_LENGTH = 3;
const int32_t GEO_INFO_PREFIX_LENGTH = 2;
const int32_t GEO_INFO_DATA_LENGTH_STARTER = 0;
const int32_t GEO_INFO_DATA_LENGTH_ENDER = 2;

NumberLocationUtils::NumberLocationUtils() {}

NumberLocationUtils::~NumberLocationUtils() {}

std::string NumberLocationUtils::QueryUnicodeOPNamebyPhoneNumber(std::string phoneNumber)
{
    std::string opName = "";
    if (phoneNumber.empty()) {
        NUMBER_IDENTITY_LOGE("phone number is empty!");
        return opName;
    }
    
    NUMBER_IDENTITY_LOGD("QueryUnicodeOPNamebyPhoneNumber: querying number: %{public}s", phoneNumber.c_str());
    
    // Query from new data file first
    opName = DelayedSingleton<NumberLocationDbParse>::GetInstance()
        ->QueryOperatorFromDataFile(phoneNumber.c_str());
    if (!opName.empty()) {
        NUMBER_IDENTITY_LOGI("QueryUnicodeOPNamebyPhoneNumber: found in new data file: %{public}s", opName.c_str());
        return opName;
    }
    
    NUMBER_IDENTITY_LOGD("QueryUnicodeOPNamebyPhoneNumber: not found in new data file, trying original logic");
    
    // If not found in new file, use original logic
    std::string opFullName =
        DelayedSingleton<NumberLocationDbParse>::GetInstance()->QueryOpNamebyPhoneNumber(phoneNumber.c_str());
    if (opFullName.empty() || opFullName.length() <= GEO_INFO_MAX_ILLEGAL_LENGTH) {
        NUMBER_IDENTITY_LOGD("QueryUnicodeOPNamebyPhoneNumber: not found in original logic, returning empty (will try hardcoded data)");
        return opName;
    }
    std::string dataSize = opFullName.substr(GEO_INFO_DATA_LENGTH_STARTER, GEO_INFO_DATA_LENGTH_ENDER);
    int32_t actualNum = std::atoi(dataSize.c_str());
    opName = opFullName.substr(GEO_INFO_PREFIX_LENGTH, GEO_INFO_PREFIX_LENGTH + actualNum);
    NUMBER_IDENTITY_LOGI("QueryUnicodeOPNamebyPhoneNumber: found in original logic: %{public}s", opName.c_str());
    return opName;
}

std::string NumberLocationUtils::QueryUnicodeAreaCodeByPhoneNum(std::string phoneNumber)
{
    std::string areaCode = "";
    if (phoneNumber.empty()) {
        NUMBER_IDENTITY_LOGE("phone number is empty!");
        return areaCode;
    }
    areaCode =
        DelayedSingleton<NumberLocationDbParse>::GetInstance()->QueryAreaCodebyPhoneNumber(phoneNumber.c_str());
    if (areaCode.empty() || areaCode.length() <= GEO_INFO_PREFIX_LENGTH) {
        NUMBER_IDENTITY_LOGE("areaCode number is empty or not long enough!");
    }
    return areaCode;
}

std::string NumberLocationUtils::QueryUnicodeInformationByPhoneNum(std::string phoneNumber)
{
    std::string unicodeInformation = "";
    if (phoneNumber.empty()) {
        NUMBER_IDENTITY_LOGE("phone number is empty!");
        return unicodeInformation;
    }
    
    NUMBER_IDENTITY_LOGD("QueryUnicodeInformationByPhoneNum: querying number: %{public}s", phoneNumber.c_str());
    
    // Query from new data file first
    unicodeInformation = DelayedSingleton<NumberLocationDbParse>::GetInstance()
        ->QueryLocationFromDataFile(phoneNumber.c_str());
    if (!unicodeInformation.empty()) {
        NUMBER_IDENTITY_LOGI("QueryUnicodeInformationByPhoneNum: found in new data file: %{public}s", unicodeInformation.c_str());
        return unicodeInformation;
    }
    
    NUMBER_IDENTITY_LOGD("QueryUnicodeInformationByPhoneNum: not found in new data file, trying original logic");
    
    // If not found in new file, use original logic
    std::string phoneLocation =
        DelayedSingleton<NumberLocationDbParse>::GetInstance()->QueryPhoneNumberLocation(phoneNumber.c_str());
    if (phoneLocation.empty() || phoneLocation.length() <= GEO_INFO_MAX_ILLEGAL_LENGTH) {
        NUMBER_IDENTITY_LOGD("QueryUnicodeInformationByPhoneNum: not found in original logic either, returning empty");
        return unicodeInformation;
    }
    std::string dataSize = phoneLocation.substr(GEO_INFO_DATA_LENGTH_STARTER, GEO_INFO_DATA_LENGTH_ENDER);
    int32_t actualNum = std::atoi(dataSize.c_str());
    unicodeInformation = phoneLocation.substr(GEO_INFO_PREFIX_LENGTH, GEO_INFO_PREFIX_LENGTH + actualNum);
    NUMBER_IDENTITY_LOGI("QueryUnicodeInformationByPhoneNum: found in original logic: %{public}s", unicodeInformation.c_str());
    return unicodeInformation;
}

std::string NumberLocationUtils::QueryUnicodeInformationByTelNum(std::string phoneNumber)
{
    std::string unicodeInformation = "";
    if (phoneNumber.empty()) {
        NUMBER_IDENTITY_LOGE("phone number is empty!");
        return unicodeInformation;
    }
    std::string telLocation =
        DelayedSingleton<NumberLocationDbParse>::GetInstance()->QueryTelNumberLocation(phoneNumber.c_str());
    if (telLocation.empty() || telLocation.length() <= GEO_INFO_MAX_ILLEGAL_LENGTH) {
        return unicodeInformation;
    }
    std::string dataSize = telLocation.substr(GEO_INFO_DATA_LENGTH_STARTER, GEO_INFO_DATA_LENGTH_ENDER);
    int32_t actualNum = std::atoi(dataSize.c_str());
    unicodeInformation = telLocation.substr(GEO_INFO_PREFIX_LENGTH, GEO_INFO_PREFIX_LENGTH + actualNum);
    return unicodeInformation;
}
} // namespace Telephony
} // namespace OHOS
