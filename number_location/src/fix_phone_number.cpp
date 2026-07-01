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

#include "fix_phone_number.h"

#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"
#include "number_location_utils.h"
#include <regex>

namespace OHOS {
namespace Telephony {
const char *G_FIXED_NUMBER_TO_P2_TOKE_N1 = "01";
const char *G_FIXED_NUMBER_TO_P2_TOKE_N2 = "02";
const int32_t AREA_CODE_LENGTH = 5;
const int32_t AREA_CODE_TOP_LENGTH = 3;
const int32_t AREA_CODE_TOP_VALID_LENGTH = 2;
const int32_t AREA_CODE_NORMAL_VALID_LENGTH = 4;

FixPhoneNumber::FixPhoneNumber(std::string number): fixPhoneNumber_(number) {};
FixPhoneNumber::~FixPhoneNumber() {};

int32_t FixPhoneNumber::ParseFixPhoneNumber()
{
    if (fixPhoneNumber_.empty() || fixPhoneNumber_.length() < AREA_CODE_LENGTH) {
        NUMBER_IDENTITY_LOGE("phone number is not match!");
        return NUMBER_IDENTITY_ERROR;
    }
    std::string topNumber = fixPhoneNumber_.substr(0, AREA_CODE_TOP_VALID_LENGTH);
    if (std::strcmp(topNumber.c_str(), G_FIXED_NUMBER_TO_P2_TOKE_N1) == 0 ||
        std::strcmp(topNumber.c_str(), G_FIXED_NUMBER_TO_P2_TOKE_N2) == 0) {
        areaCode_ = fixPhoneNumber_.substr(0, AREA_CODE_TOP_LENGTH);
        return NUMBER_IDENTITY_ERR_SUCCESS;
    } else {
        areaCode_ = fixPhoneNumber_.substr(0, AREA_CODE_NORMAL_VALID_LENGTH);
        return NUMBER_IDENTITY_ERR_SUCCESS;
    }
}

std::string FixPhoneNumber::GetFixParseResult()
{
    std::string resString = "";
    if (ParseFixPhoneNumber() != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("phone number failed!");
        return resString;
    }
    resString = DelayedSingleton<NumberLocationUtils>::GetInstance()->QueryUnicodeInformationByTelNum(areaCode_);
    return resString;
}

std::string FixPhoneNumber::SubstringFixedPhoneNumber()
{
    if (ParseFixPhoneNumber() != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGI("No need to process number.");
        return fixPhoneNumber_;
    }
    std::string resultString = fixPhoneNumber_;
    std::string temp = GetFixParseResult();
    if (!temp.empty() && !areaCode_.empty()) {
        NUMBER_IDENTITY_LOGI("Deleted area code, length = %{public}ld.", areaCode_.size());
        resultString = fixPhoneNumber_.substr(areaCode_.size());
    }
    return resultString;
}

} // namespace Telephony
} // namespace OHOS