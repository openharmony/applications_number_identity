/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef NAPI_NUMBER_IDENTITY_H
#define NAPI_NUMBER_IDENTITY_H

#include "datashare_helper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_number_identity_types.h"
#include "pac_map.h"
#include "string_ex.h"

namespace OHOS {
namespace Telephony {
#define GET_PARAMS(env, info, num) \
    size_t argc = num;             \
    napi_value argv[num] = { 0 };    \
    napi_value thisVar = nullptr;  \
    void *data;                    \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

/**
 * NapiNumberIdentity is responsible for NAPI initialization and JavaScript data parsing.
 */
class NapiNumberIdentity {
public:
    static napi_value DeclareBasisInterface(napi_env env, napi_value exports);
    static napi_value GetNumberLocation(napi_env env, napi_callback_info info);
    static napi_value GetNumberLocations(napi_env env, napi_callback_info info);
    static napi_value GetNumberMarkInfo(napi_env env, napi_callback_info info);
    static napi_value SetNumberMarkInfo(napi_env env, napi_callback_info info);
    static napi_value RegisterNumberIdentityFunc(napi_env env, napi_value exports);

private:
    static void NativeGetNumberLocation(napi_env env, void *data);
    static void NativeGetNumberLocations(napi_env env, void *data);
    static void NativeGetNumberMarkInfo(napi_env env, void *data);
    static void NativeSetNumberMarkInfo(napi_env env, void *data);
    static bool MatchObjectAndStringParameter(
        napi_env env, const napi_value parameters[], const size_t parameterCount);
    static bool MatchTwoObjectParameter(
        napi_env env, const napi_value parameters[], const size_t parameterCount);
    static void GetNumberLocationCallback(napi_env env, napi_status status, void *data);
    static void GetNumberLocationsCallback(napi_env env, napi_status status, void *data);
    static void GetNumberMarkInfoCallback(napi_env env, napi_status status, void *data);
    static void SetNumberMarkInfoCallback(napi_env env, napi_status status, void *data);
};
} // namespace Telephony
} // namespace OHOS

#endif // NAPI_NUMBER_IDENTITY_H