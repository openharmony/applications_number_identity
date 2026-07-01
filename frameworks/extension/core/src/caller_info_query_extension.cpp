/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "caller_info_query_extension.h"
#include "js_caller_info_query_extension.h"
#include "caller_info_query_extension_context.h"
#include "runtime.h"

namespace OHOS {
namespace CallerInfoQuery {
using namespace OHOS::AppExecFwk;

CallerInfoQueryExtension* CallerInfoQueryExtension::Create(
    const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    if (!runtime) {
        return new CallerInfoQueryExtension();
    }
    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return JsCallerInfoQueryExtension::Create(runtime);
        default:
            return new CallerInfoQueryExtension();
    }
}

void CallerInfoQueryExtension::Init(
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
    const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
    std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
    const sptr<IRemoteObject>& token)
{
    ExtensionBase<CallerInfoQueryExtensionContext>::Init(
        record, application, handler, token);
}

std::shared_ptr<CallerInfoQueryExtensionContext> CallerInfoQueryExtension::CreateAndInitContext(
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
    const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
    std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
    const sptr<IRemoteObject>& token)
{
    std::shared_ptr<CallerInfoQueryExtensionContext> context =
        ExtensionBase<CallerInfoQueryExtensionContext>::CreateAndInitContext(
            record, application, handler, token);
    return context;
}
} // namespace CallerInfoQuery
} // namespace OHOS