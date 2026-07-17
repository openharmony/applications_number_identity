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

#ifndef JS_CALLER_INFO_QUERY_EXTENSION_H
#define JS_CALLER_INFO_QUERY_EXTENSION_H

#include <string>
#include "call_napi_invoker_helper.h"
#include "caller_info_query_extension.h"
#include "caller_info_query_extension_callback_data.h"
#include "js_runtime.h"

namespace OHOS {
namespace CallerInfoQuery {
/**
 * @brief js-level enterprise admin extension.
 */
class JsCallerInfoQueryExtension : public CallerInfoQueryExtension {
public:
    JsCallerInfoQueryExtension(AbilityRuntime::JsRuntime& jsRuntime);

    ~JsCallerInfoQueryExtension() override;

    static JsCallerInfoQueryExtension* Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime);

    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
              const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
              std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
              const sptr<IRemoteObject>& token) override;

    void OnStart(const AAFwk::Want& want) override;

    sptr<IRemoteObject> OnConnect(const AAFwk::Want& want) override;

    void OnDisconnect(const AAFwk::Want& want) override;

    void OnStop() override;

    int OnQueryCallerInfo(std::string phoneNume, QueryCallerInfoResult& result, int32_t waitTimeMs);

private:
    void JsCallerInfoQueryExtensionContextInit();
    std::string GetSrcPath();
    AbilityRuntime::JsRuntime& jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_{nullptr};
};
}
}
#endif