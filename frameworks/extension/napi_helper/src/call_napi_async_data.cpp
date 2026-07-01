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
#include "call_napi_async_data.h"
#include "call_napi_async_callback_data.h"

namespace OHOS {
namespace CallerInfoQuery {
void AsyncDataBase::HandlePromiseResolve(napi_env env, napi_value content)
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Handle promise resolve begin.");
    std::unique_lock<std::mutex> lck(mtx_);
    auto ret = ParseResolveNapiData(env, content);
    if (!ret) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "Parse resolve napi data fail.");
        retSuccess = false;
        cv_.notify_all();
        return;
    }
    retSuccess = true;
    cv_.notify_all();
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Handle promise resolve end.");
}

void AsyncDataBase::HandlePromiseReject(napi_env env, napi_value content)
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Handle promise reject begin.");
    std::unique_lock<std::mutex> lck(mtx_);
    auto ret = ParseRejectNapiData(env, content);
    if (!ret) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "Parse reject napi data fail.");
        return;
    }
    retSuccess = false;
    cv_.notify_all();
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Handle promise reject end.");
}

void AsyncDataBase::HandleException(napi_env env, napi_value content)
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Handle exception begin.");
    HandlePromiseReject(env, content);
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Handle exception end.");
}

bool AsyncDataBase::Await(int32_t waitTimeMs)
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION,
        "wait for result, time:%{public}d ms.", waitTimeMs);
    std::unique_lock<std::mutex> lck(mtx_);
    auto ret = cv_.wait_for(lck, std::chrono::milliseconds(waitTimeMs));
    if (ret == std::cv_status::timeout) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Wait for result fail, timeout.");
        return false;
    }
    return retSuccess;
}

bool AsyncDataBase::ParseResolveNapiData(napi_env env, napi_value context)
{
    return true;
}

bool AsyncDataBase::ParseRejectNapiData(napi_env env, napi_value context)
{
    return true;
}

AsyncCallBackData* AsyncDataBase::CreateAsyncCallBackData()
{
    return new (std::nothrow) AsyncCallBackData(shared_from_this());
}

}
}