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

#ifndef CALL_NAPI_ASYNC_DATA_H
#define CALL_NAPI_ASYNC_DATA_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <functional>
#include "call_napi_invoker_helper.h"
#include "napi/native_api.h"
#include "js_native_api_types.h"

namespace OHOS {
namespace CallerInfoQuery {
class AsyncCallBackData;
class AsyncDataBase : public std::enable_shared_from_this<AsyncDataBase> {
public:
    virtual ~AsyncDataBase(){};
    virtual void HandlePromiseResolve(napi_env env, napi_value context);
    virtual void HandlePromiseReject(napi_env env, napi_value context);
    virtual void HandleException(napi_env env, napi_value context);
    AsyncCallBackData* CreateAsyncCallBackData();
    virtual bool Await(int32_t waitTimeMs);
    virtual bool ParseResolveNapiData(napi_env env, napi_value context);
    virtual bool ParseRejectNapiData(napi_env env, napi_value context);
protected:
    uint32_t type = 0;
    bool retSuccess = false;
    std::condition_variable cv_;
    std::mutex mtx_;
};

}
}
#endif
