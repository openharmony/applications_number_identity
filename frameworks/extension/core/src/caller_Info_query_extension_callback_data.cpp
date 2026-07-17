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


#include "caller_info_query_extension_callback_data.h"
#include "call_napi_invoker_helper.h"
#include "caller_info_query_extension_hilog.h"
#include "ipc_debug.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include <chrono>
#include <condition_variable>
#include <mutex>

namespace OHOS::CallerInfoQuery {

    const static std::unordered_map<int, std::string> ErrMsgMap = {
        {ERR_TIME_OUT, "time out"},
        {ERR_QUERY_SUCCESS, "query success"},
        {ERR_PARSE_RESOLVE, "resolve parse err"},
        {ERR_QUERY_FAIL, "query fail"},
    };

    bool CallerInfoAsyncData::ParseResolveNapiData(napi_env env, napi_value context)
    {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "start parse caller info");
        napi_status ret = napi_status::napi_ok;
        napi_value result;
        ret = napi_get_named_property(env, context, "contactName", &result);
        if (ret != napi_status::napi_ok) {
            result_.code_ = ERR_PARSE_RESOLVE;
            CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
                "parse contact name fail, ret:%{public}d", ret);
            return false;
        }

        result_.info_.contactName_ = NapiHelper::ToString(env, result);

        ret = napi_get_named_property(env, context, "employeeId", &result);
        if (ret == napi_status::napi_ok) {
            result_.info_.employeeId_ = NapiHelper::ToString(env, result);
        }

        ret = napi_get_named_property(env, context, "department", &result);
        if (ret == napi_status::napi_ok) {
            result_.info_.department_ = NapiHelper::ToString(env, result);
        }

        ret = napi_get_named_property(env, context, "position", &result);
        if (ret == napi_status::napi_ok) {
            result_.info_.position_ = NapiHelper::ToString(env, result);
        }
        result_.code_ = ERR_QUERY_SUCCESS;
        return true;
    }

    bool CallerInfoAsyncData::ParseRejectNapiData(napi_env env, napi_value context)
    {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "parse reject napi data");
        napi_status ret = napi_status::napi_ok;
        napi_valuetype valueType;
        result_.code_ = ERR_EXTENAL;
        ret = napi_typeof(env, context, &valueType);
        if (valueType == napi_string) {
            result_.errorMsg_= NapiHelper::ToString(env, context);
            return true;
        }

        if (valueType == napi_object) {
            bool result = false;
            ret = napi_has_named_property(env, context, "message", &result);
            if (ret != napi_status::napi_ok || !result) {
                return true;
            }
            napi_value msgVal;
            ret = napi_get_named_property(env, context, "message", &msgVal);
            if (ret != napi_status::napi_ok) {
                return true;
            }
            napi_valuetype msgValType;
            ret = napi_typeof(env, msgVal, &msgValType);
            if (msgValType != napi_string) {
                return true;
            }
            result_.errorMsg_ = NapiHelper::ToString(env, msgVal);
            return true;
        }
        result_.code_ = ERR_QUERY_FAIL;
        return true;
    }

    QueryCallerInfoResult CallerInfoAsyncData::GetQueryCallerInfoResult()
    {
        if (ErrMsgMap.count(result_.code_)) {
            result_.errorMsg_ = ErrMsgMap.at(result_.code_);
        }
        return result_;
    }
}

