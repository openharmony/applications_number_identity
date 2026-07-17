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

#include "mobile_phone_number.h"

#include <regex>
#include "number_location_utils.h"
#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"

namespace OHOS {
namespace Telephony {
const int32_t MPN_PATTERN_PREFIX_LEN = 3;
const int32_t MPN_PATTERN_MIN_LEN = 6;
const int32_t MPN_PATTERN_NDC_MIN_LEN = 10;
const int32_t MPN_PATTERN1_PREFIX_LEN = 2;
const int32_t MPN_PATTERN1_MIN_LEN = 5;
const int32_t MPN_PATTERN1_NDC_MIN_LEN = 9;
const int32_t MPN_PATTERN2_PREFIX_LEN = 0;
const int32_t MPN_QUERY_PATTERN2_MIN_LEN = 3;
const int32_t MPN_PATTERN2_NDC_MIN_LEN = 7;
const int32_t MPN_PATTERN3_PREFIX_LEN = 4;
const int32_t MPN_QUERY_PATTERN3_MIN_LEN = 7;
const int32_t MPN_PATTERN3_NDC_MIN_LEN = 11;
const int32_t NDC3_PREFIX_LEN = 3;
const int32_t NDC7_PREFIX_LEN = 7;
const char *MPN_QUERY_PATTERN = "^\\+86\\d{3,11}$";
const char *MPN_QUERY_PATTERN1 = "^86\\d{3,11}$";
const char *MPN_QUERY_PATTERN2 = "^(1\\d{2,10})$";
const char *MPN_QUERY_PATTERN3 = "^0086\\d{3,11}$";
std::map<std::string, std::string> operatorData = {
    {"133", "电信"},
    {"153", "电信"},
    {"180", "电信"},
    {"181", "电信"},
    {"189", "电信"},
    {"173", "电信"},
    {"177", "电信"},
    {"130", "联通"},
    {"131", "联通"},
    {"132", "联通"},
    {"155", "联通"},
    {"156", "联通"},
    {"166", "联通"},
    {"186", "联通"},
    {"145", "联通"},
    {"185", "联通"},
    {"134", "移动"},
    {"135", "移动"},
    {"136", "移动"},
    {"137", "移动"},
    {"138", "移动"},
    {"139", "移动"},
    {"147", "移动"},
    {"150", "移动"},
    {"151", "移动"},
    {"152", "移动"},
    {"157", "移动"},
    {"158", "移动"},
    {"159", "移动"},
    {"182", "移动"},
    {"183", "移动"},
    {"184", "移动"},
    {"187", "移动"},
    {"188", "移动"},
    {"148", "移动"},
    {"192", "广电"}
};

MobilePhoneNumber::MobilePhoneNumber(std::string number): mobilePhoneNumber_(number) {};
MobilePhoneNumber::~MobilePhoneNumber() {};

int32_t MobilePhoneNumber::ParseMobilePhoneNumber()
{
    if (mobilePhoneNumber_.empty()) {
        NUMBER_IDENTITY_LOGE("phone number is nullptr!");
        return NUMBER_IDENTITY_ERROR;
    }
    if (std::regex_match(
        mobilePhoneNumber_, std::regex(MPN_QUERY_PATTERN)) && mobilePhoneNumber_.length() >= MPN_PATTERN_MIN_LEN) {
        ndc3String_ = mobilePhoneNumber_.substr(MPN_PATTERN_PREFIX_LEN, NDC3_PREFIX_LEN);
        if (mobilePhoneNumber_.length() >= MPN_PATTERN_NDC_MIN_LEN) {
            ndc7String_ = mobilePhoneNumber_.substr(MPN_PATTERN_PREFIX_LEN, NDC7_PREFIX_LEN);
            return NUMBER_IDENTITY_ERR_SUCCESS;
        }
    } else if (std::regex_match(
        mobilePhoneNumber_, std::regex(MPN_QUERY_PATTERN1)) && mobilePhoneNumber_.length() >= MPN_PATTERN1_MIN_LEN) {
        ndc3String_ = mobilePhoneNumber_.substr(MPN_PATTERN1_PREFIX_LEN, NDC3_PREFIX_LEN);
        if (mobilePhoneNumber_.length() >= MPN_PATTERN1_NDC_MIN_LEN) {
            ndc7String_ = mobilePhoneNumber_.substr(MPN_PATTERN1_PREFIX_LEN, NDC7_PREFIX_LEN);
            return NUMBER_IDENTITY_ERR_SUCCESS;
        }
    } else if (std::regex_match(mobilePhoneNumber_, std::regex(MPN_QUERY_PATTERN2)) &&
        mobilePhoneNumber_.length() >= MPN_QUERY_PATTERN2_MIN_LEN) {
        ndc3String_ = mobilePhoneNumber_.substr(MPN_PATTERN2_PREFIX_LEN, NDC3_PREFIX_LEN);
        if (mobilePhoneNumber_.length() >= MPN_PATTERN2_NDC_MIN_LEN) {
            ndc7String_ = mobilePhoneNumber_.substr(MPN_PATTERN2_PREFIX_LEN, NDC7_PREFIX_LEN);
            return NUMBER_IDENTITY_ERR_SUCCESS;
        }
    } else if (std::regex_match(mobilePhoneNumber_, std::regex(MPN_QUERY_PATTERN3)) &&
        mobilePhoneNumber_.length() >= MPN_QUERY_PATTERN3_MIN_LEN) {
        ndc3String_ = mobilePhoneNumber_.substr(MPN_PATTERN3_PREFIX_LEN, NDC3_PREFIX_LEN);
        if (mobilePhoneNumber_.length() >= MPN_PATTERN3_NDC_MIN_LEN) {
            ndc7String_ = mobilePhoneNumber_.substr(MPN_PATTERN3_PREFIX_LEN, NDC7_PREFIX_LEN);
            return NUMBER_IDENTITY_ERR_SUCCESS;
        }
    } else {
        NUMBER_IDENTITY_LOGE("phone number not matched!");
    }
    return NUMBER_IDENTITY_ERROR;
}

std::string MobilePhoneNumber::GetMobileParseResult()
{
    NUMBER_IDENTITY_LOGD("GetMobileParseResult: parsing number: %{public}s", mobilePhoneNumber_.c_str());
    
    std::string resString = "";
    if (ParseMobilePhoneNumber() != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("GetMobileParseResult: parse phone number failed!");
        return resString;
    }
    
    NUMBER_IDENTITY_LOGD("GetMobileParseResult: parsed ndc7String_=%{public}s, ndc3String_=%{public}s", 
                         ndc7String_.c_str(), ndc3String_.c_str());
    
    // Try to query location with 7-digit prefix first
    if (!ndc7String_.empty()) {
        NUMBER_IDENTITY_LOGD("GetMobileParseResult: querying location with 7-digit prefix: %{public}s", ndc7String_.c_str());
        resString = DelayedSingleton<NumberLocationUtils>::GetInstance()->QueryUnicodeInformationByPhoneNum(ndc7String_);
        NUMBER_IDENTITY_LOGD("GetMobileParseResult: 7-digit query result: %{public}s", resString.c_str());
    }
    
    // If 7-digit query failed, try 3-digit prefix
    if (resString.empty() && !ndc3String_.empty()) {
        NUMBER_IDENTITY_LOGD("GetMobileParseResult: 7-digit query failed, trying 3-digit prefix: %{public}s", ndc3String_.c_str());
        resString = DelayedSingleton<NumberLocationUtils>::GetInstance()->QueryUnicodeInformationByPhoneNum(ndc3String_);
        NUMBER_IDENTITY_LOGD("GetMobileParseResult: 3-digit query result: %{public}s", resString.c_str());
    }
    
    location_ = resString;
    
    // Query operator - try 7-digit prefix first, then fallback to 3-digit prefix
    if (ndc7String_.length() > 0) {
        NUMBER_IDENTITY_LOGD("GetMobileParseResult: querying operator with 7-digit prefix: %{public}s", ndc7String_.c_str());
        phoneOperator_ =
            DelayedSingleton<NumberLocationUtils>::GetInstance()->QueryUnicodeOPNamebyPhoneNumber(ndc7String_);
        NUMBER_IDENTITY_LOGD("GetMobileParseResult: 7-digit operator query result: %{public}s", phoneOperator_.c_str());
    }
    
    // If 7-digit operator query failed, try 3-digit prefix
    if (phoneOperator_.empty() && ndc3String_.length() > 0) {
        NUMBER_IDENTITY_LOGD("GetMobileParseResult: 7-digit operator query failed, trying 3-digit prefix: %{public}s", ndc3String_.c_str());
        phoneOperator_ =
            DelayedSingleton<NumberLocationUtils>::GetInstance()->QueryUnicodeOPNamebyPhoneNumber(ndc3String_);
        NUMBER_IDENTITY_LOGD("GetMobileParseResult: 3-digit operator query result: %{public}s", phoneOperator_.c_str());
    }
    
    // Fallback to hardcoded operator data as last resort
    if (phoneOperator_.empty() && !ndc3String_.empty()) {
        NUMBER_IDENTITY_LOGD("GetMobileParseResult: operator query failed, trying hardcoded data for prefix: %{public}s", ndc3String_.c_str());
        std::map<std::string, std::string>::iterator iter = operatorData.find(ndc3String_);
        if (iter != operatorData.end()) {
            phoneOperator_ = iter->second;
            NUMBER_IDENTITY_LOGI("GetMobileParseResult: found operator from hardcoded data: %{public}s", phoneOperator_.c_str());
        } else {
            NUMBER_IDENTITY_LOGW("GetMobileParseResult: operator not found in hardcoded data for prefix: %{public}s", ndc3String_.c_str());
        }
    }
    
    // Format final result
    // Priority: location + operator > operator only > location only
    // Important: If location is empty or not found, return operator value
    
    // Check if location is empty or invalid
    bool locationIsEmpty = resString.empty() || resString.length() == 0;
    
    NUMBER_IDENTITY_LOGD("GetMobileParseResult: locationIsEmpty=%{public}d, resString='%{public}s', phoneOperator_='%{public}s'", 
                         locationIsEmpty, resString.c_str(), phoneOperator_.c_str());
    
    if (locationIsEmpty) {
        // Location is empty, return operator value
        if (!phoneOperator_.empty()) {
            resString = phoneOperator_;
            NUMBER_IDENTITY_LOGI("GetMobileParseResult: location is empty, returning operator: %{public}s", resString.c_str());
        } else {
            // Operator is also empty, try hardcoded data as last resort
            NUMBER_IDENTITY_LOGW("GetMobileParseResult: both location and operator are empty, trying hardcoded data");
            if (!ndc3String_.empty()) {
                std::map<std::string, std::string>::iterator iter = operatorData.find(ndc3String_);
                if (iter != operatorData.end()) {
                    resString = iter->second;
                    NUMBER_IDENTITY_LOGI("GetMobileParseResult: using hardcoded operator as last resort: %{public}s", resString.c_str());
                } else {
                    NUMBER_IDENTITY_LOGW("GetMobileParseResult: operator not found in hardcoded data for prefix: %{public}s", ndc3String_.c_str());
                }
            }
        }
    } else {
        // Location is not empty, format result with location and operator
        if (!phoneOperator_.empty()) {
            resString = resString + " " + phoneOperator_;
            NUMBER_IDENTITY_LOGI("GetMobileParseResult: final result (location + operator): %{public}s", resString.c_str());
        } else {
            // Location found but operator not found, return location only
            NUMBER_IDENTITY_LOGI("GetMobileParseResult: final result (location only, operator not found): %{public}s", resString.c_str());
        }
    }
    
    // Final check: if result is still empty, log warning
    if (resString.empty()) {
        NUMBER_IDENTITY_LOGW("GetMobileParseResult: final result is empty for number %{public}s (ndc3String_=%{public}s, ndc7String_=%{public}s)", 
                             mobilePhoneNumber_.c_str(), ndc3String_.c_str(), ndc7String_.c_str());
    }
    
    return resString;
}

std::string MobilePhoneNumber::GetPhoneOperator()
{
    if (!ndc7String_.empty()) {
        phoneOperator_ =
            DelayedSingleton<NumberLocationUtils>::GetInstance()->QueryUnicodeOPNamebyPhoneNumber(ndc7String_);
    } else if (!ndc3String_.empty()) {
        phoneOperator_ =
            DelayedSingleton<NumberLocationUtils>::GetInstance()->QueryUnicodeOPNamebyPhoneNumber(ndc3String_);
    } else {
        NUMBER_IDENTITY_LOGE("not matched!");
    }
    if (phoneOperator_.empty()) {
        std::map<std::string, std::string>::iterator iter = operatorData.find(ndc3String_);
        if (iter != operatorData.end()) {
            phoneOperator_ = iter->second;
        }
    }
    return phoneOperator_;
}

std::string MobilePhoneNumber::GetLocation()
{
    return location_;
}
} // namespace Telephony
} // namespace OHOS