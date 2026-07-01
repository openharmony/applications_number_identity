/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CALL_NAPI_INVOKER_HELPER_H
#define CALL_NAPI_INVOKER_HELPER_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <functional>
#include "napi/native_api.h"
#include "js_native_api_types.h"

namespace OHOS {
namespace CallerInfoQuery {

class NapiHelper {
public:
    static napi_value CallObjectMethod(
        napi_env env, napi_value object, std::string methdoName, napi_value* argv, size_t argc);
    static void HandlePromise(
        napi_env env, napi_value promiseVal, void* data, napi_callback resolveCb, napi_callback rejectCb);
    static napi_value GetObjectMethod(
        const napi_env& env, napi_value& obj, std::string methodName);
    static bool CheckValueType(const napi_env& env, const napi_value& value, const napi_valuetype type);
    static std::string ToString(const napi_env& env, const napi_value& value);
    static napi_value GetException(const napi_env& env);
};

}
}

#endif
