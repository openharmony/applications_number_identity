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

#include "caller_info_query_extension_hilog.h"
#include "call_napi_invoker_helper.h"
#include "js_native_api.h"

namespace OHOS {
namespace CallerInfoQuery {
const int STRING_VALUE_MAX_SIZE = 4096;
const int ARGV_COUNT_ONE = 1;

napi_value NapiHelper::CallObjectMethod(
    napi_env env, napi_value object, std::string methodName, napi_value* argv, size_t argc)
{
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
        "CallObjectMethod(%{public}s), begin", methodName.c_str());
    napi_value method = nullptr;
    napi_get_named_property(env, object, methodName.c_str(), &method);
    if (method == nullptr) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "Failed to get '%{public}s'", methodName.c_str());
        return nullptr;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, method, &valueType);
    if (valueType != napi_function) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "'%{public}s' is not function", methodName.c_str());
        return nullptr;
    }

    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
        "JsCallerInfoQueryExtension::CallFunction(%{public}s), success", methodName.c_str());
    napi_value result = nullptr;
    napi_status status = napi_call_function(env, object, method, argc, argv, &result);
    if (status != napi_ok) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "Failed to call function");
    }
    return result;
}

napi_value NapiHelper::GetObjectMethod(const napi_env& env, napi_value& obj, std::string methodName)
{
    napi_value func;
    auto ret = napi_get_named_property(env, obj, methodName.c_str(), &func);
    if (ret != napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "get method %{public}s fail.", methodName.c_str());
        return nullptr;
    }

    if (!CheckValueType(env, func, napi_valuetype::napi_function)) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "method %{public}s is not function", methodName.c_str());
        return nullptr;
    }
    return func;
}

void NapiHelper::HandlePromise(
    napi_env env, napi_value promiseVal, void* data, napi_callback resolveCb, napi_callback rejectCb)
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Handle Promise begin");
    napi_value thenFunc = NapiHelper::GetObjectMethod(env, promiseVal, "then");
    if (thenFunc == nullptr) {
        return;
    }
    napi_value catchFunc = NapiHelper::GetObjectMethod(env, promiseVal, "catch");
    if (catchFunc == nullptr) {
        return;
    }

    napi_value resolve;
    napi_status retStatus = napi_create_function(env,
        "resolveCb", NAPI_AUTO_LENGTH, resolveCb,  data, &resolve);
    if (retStatus != ::napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "create promise Resolve fail.");
        return;
    }

    napi_value reject;
    retStatus = napi_create_function(env,
        "rejectCb", NAPI_AUTO_LENGTH, rejectCb, data, &reject);
    if (retStatus != napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "create promise Reject fail.");
        return;
    }

    napi_value argvPromiseResolve[] = { resolve };
    napi_value thenResult;
    retStatus = napi_call_function(env,
        promiseVal, thenFunc, ARGV_COUNT_ONE, argvPromiseResolve, &thenResult);
    if (retStatus != napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "register then function fail: %d", retStatus);
        return;
    }

    napi_value argvPromiseReject[] = { reject };
    napi_value catchResult;
    retStatus = napi_call_function(env,
        promiseVal, catchFunc, ARGV_COUNT_ONE, argvPromiseReject, &catchResult);
    if (retStatus != napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "register catch function fail: %d", retStatus);
    }
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Handle Promise end");
}

bool NapiHelper::CheckValueType(
    const napi_env& env, const napi_value& value, const napi_valuetype type)
{
    if (value == nullptr) {
        return false;
    }
    napi_valuetype valuetype;
    auto ret = napi_typeof(env, value, &valuetype);
    if (ret != napi_ok) {
        return false;
    }
    return valuetype == type;
}

napi_value NapiHelper::GetException(const napi_env& env)
{
    napi_value exception = nullptr;
    napi_get_and_clear_last_exception(env, &exception);
    return exception;
}

std::string NapiHelper::ToString(const napi_env& env, const napi_value& value)
{
    if (!CheckValueType(env, value, napi_string)) {
        return std::string();
    }

    std::unique_ptr<char[]> valueBuf = std::make_unique<char[]>(STRING_VALUE_MAX_SIZE);
    size_t strSize = 0;
    auto ret = napi_get_value_string_utf8(env,
        value, valueBuf.get(), STRING_VALUE_MAX_SIZE, &strSize);
    if (ret != napi_ok) {
        return std::string();
    }
    std::string result = std::string(valueBuf.get(), strSize);
    return result;
}

}
}