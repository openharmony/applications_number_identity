/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef NUMBER_ACCESS_TOKEN_H
#define NUMBER_ACCESS_TOKEN_H

#include "accesstoken_kit.h"
#include "token_setproc.h"
#include "number_identity_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;

inline HapInfoParams testInfoParams = {
    .bundleName = "com.ohos.numberidentity",
    .userID = 1,
    .instIndex = 0,
    .appIDDesc = "test",
    //.apiVersion = 8,
    .isSystemApp = true,
};

inline PermissionDef testPermPlaceCallDef = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .bundleName = "com.ohos.numberidentity",
    .grantMode = 1, // SYSTEM_GRANT
    .availableLevel = APL_SYSTEM_BASIC,
    .label = "label",
    .labelId = 1,
    .description = "Test download file",
    .descriptionId = 1,
};

inline PermissionStateFull testPlaceCallState = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .isGeneral = true,
    .resDeviceID = { "local" },
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .grantFlags = { 2 }, // PERMISSION_USER_SET
};

inline HapPolicyParams testPolicyParams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = { testPermPlaceCallDef },
    .permStateList = { testPlaceCallState },
};

class AccessToken {
public:
    AccessToken()
    {
        currentID_ = GetSelfTokenID();
        NUMBER_IDENTITY_LOGI("currentID_ %{public}d.", currentID_);
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testInfoParams, testPolicyParams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        NUMBER_IDENTITY_LOGI("accessID_ %{public}d.", accessID_);
        SetSelfTokenID(tokenIdEx.tokenIDEx);
        NUMBER_IDENTITY_LOGI("tokenIdEx.tokenIDEx %{public}llu.", tokenIdEx.tokenIDEx);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }
private:
    AccessTokenID currentID_ = 0;
    AccessTokenID accessID_ = 0;
};
} // namespace Telephony
} // namespace OHOS

#endif //NUMBER_ACCESS_TOKEN_H
