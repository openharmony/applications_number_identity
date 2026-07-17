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

#ifndef CALLER_INFO_QUERY_EXTENSION_CALLBACK_DATA_H
#define CALLER_INFO_QUERY_EXTENSION_CALLBACK_DATA_H

#include "call_napi_async_data.h"
#include <unordered_map>

namespace OHOS::CallerInfoQuery {

struct CallerInfo {
    std::string contactName_;
    std::string employeeId_;
    std::string department_;
    std::string position_;
};

enum ErrCode {
    ERR_TIME_OUT,
    ERR_QUERY_SUCCESS,
    ERR_PARSE_RESOLVE,
    ERR_QUERY_FAIL,
    ERR_EXTENAL,
};

struct QueryCallerInfoResult {
    ErrCode code_ = ERR_TIME_OUT;
    std::string errorMsg_;
    CallerInfo info_;
};

class CallerInfoAsyncData : public AsyncDataBase {
public:
    virtual ~CallerInfoAsyncData() {};
    QueryCallerInfoResult GetQueryCallerInfoResult();
protected:
    bool ParseResolveNapiData(napi_env env, napi_value context) override;
    bool ParseRejectNapiData(napi_env env, napi_value context) override;
    QueryCallerInfoResult result_;
};

}

#endif
