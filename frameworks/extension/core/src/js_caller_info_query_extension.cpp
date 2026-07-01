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


#include <memory>
#include <new>
#include <string>
#include "ability_handler.h"
#include "call_napi_invoker_helper.h"
#include "caller_info_query_extension.h"
#include "caller_info_query_stub.h"
#include "caller_info_query_stub_impl.h"
#include "errors.h"
#include "js_caller_info_query_extension_context.h"
#include "js_native_api_types.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "runtime.h"
#include "caller_info_query_extension_hilog.h"
#include "caller_info_query_extension_callback_data.h"
#include "js_caller_info_query_extension.h"
#include "want.h"
#include "call_napi_async_callback_data.h"

namespace OHOS {
namespace CallerInfoQuery {
constexpr size_t JS_NAPI_ARGC_ONE = 1;
const std::string CONTEXT_MODULE_PATH = "hms.telephony.CallerInfoQueryExtensionContext";
const int PARAM_COUNT = 1;

napi_value PromiseResolve(napi_env env, napi_callback_info info);
napi_value PromiseReject(napi_env env, napi_callback_info info);

napi_value AttachCallerInfoQueryExtensionContext(::napi_env env, void *value, void *hint)
{
    if (value == nullptr) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Invalid parameter, value is null");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<CallerInfoQueryExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Invalid context");
        return nullptr;
    }
    napi_value object = CreateJsCallerInfoQueryExtensionContext(env, ptr);
    auto moduleRef = AbilityRuntime::JsRuntime::LoadSystemModuleByEngine(env,
        CONTEXT_MODULE_PATH, &object, JS_NAPI_ARGC_ONE);
    if (moduleRef == nullptr) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Failed to get moduleRef");
        return nullptr;
    }
    napi_value contextObj = moduleRef->GetNapiValue();
    if (contextObj == nullptr) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Failed to get context native object");
        return nullptr;
    }
    napi_status retStatus = ::napi_coerce_to_native_binding_object(
        env, contextObj,
        AbilityRuntime::DetachCallbackFunc,
        AttachCallerInfoQueryExtensionContext,
        value, nullptr);
    if (retStatus != ::napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Bind native context to js context error");
        return nullptr;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<CallerInfoQueryExtensionContext>(ptr);
    if (workContext == nullptr) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "AttachPushExtensionContext workContext is nulllptr");
        return nullptr;
    }
    retStatus = napi_wrap(
        env, contextObj, workContext,
        [](::napi_env env, void *data, void *) {
            CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Finalizer for extension context called");
            delete static_cast<std::weak_ptr<CallerInfoQueryExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (retStatus != ::napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Napi wrap context error");
        return nullptr;
    }
    return contextObj;
}

JsCallerInfoQueryExtension* JsCallerInfoQueryExtension::Create(
    const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    return new JsCallerInfoQueryExtension(
        static_cast<AbilityRuntime::JsRuntime&>(*runtime));
}

JsCallerInfoQueryExtension::JsCallerInfoQueryExtension(
    AbilityRuntime::JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsCallerInfoQueryExtension::~JsCallerInfoQueryExtension()
{
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
        "Js caller info query extension destructor start.");
    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
        "Js caller info query extension destructor  end.");
}

void JsCallerInfoQueryExtension::Init(
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord>& record,
    const std::shared_ptr<AppExecFwk::OHOSApplication>& application,
    std::shared_ptr<AppExecFwk::AbilityHandler>& handler,
    const sptr<IRemoteObject>& token)
{
    CallerInfoQueryExtension::Init(record, application, handler, token);
    std::string srcPath = GetSrcPath();
    if (srcPath.empty()) {
        CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "JsCallerInfoQueryExtension Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
        "JsCallerInfoQueryExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    AbilityRuntime::HandleScope handleScope(jsRuntime_);

    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        Extension::abilityInfo_->compileMode == AbilityRuntime::CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "JsCallerInfoQueryExtension Failed to get jsObj_");
        return;
    }
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
        "JsCallerInfoQueryExtension::Init ConvertNativeValueTo.");
    JsCallerInfoQueryExtensionContextInit();
}

void JsCallerInfoQueryExtension::JsCallerInfoQueryExtensionContextInit()
{
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
        "JsCallerInfoQueryExtension Init");
    napi_value obj = jsObj_->GetNapiValue();
    auto env = jsRuntime_.GetNapiEnv();
    auto context = GetContext();
    if (context == nullptr) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "JsCallerInfoQueryExtension Failed to get context");
        return;
    }
    napi_value contextObj = CreateJsCallerInfoQueryExtensionContext(env, context);
    auto shellContextRef =
        AbilityRuntime::JsRuntime::LoadSystemModuleByEngine(env,
            CONTEXT_MODULE_PATH, &contextObj, JS_NAPI_ARGC_ONE);
    if (shellContextRef == nullptr) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "shellContextRef nulll");
        return;
    }
    contextObj = shellContextRef->GetNapiValue();
    auto workContext = new (std::nothrow) std::weak_ptr<CallerInfoQueryExtensionContext>(context);
    if (workContext == nullptr) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "workContext null");
        return;
    }
    napi_status retStatus = napi_coerce_to_native_binding_object(
        env, contextObj,
        AbilityRuntime::DetachCallbackFunc,
        AttachCallerInfoQueryExtensionContext,
        workContext,
        nullptr);
    if (retStatus != ::napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Bind native context error");
        return;
    }
    context->Bind(jsRuntime_, shellContextRef.release());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_wrap(env, contextObj, new std::weak_ptr<AbilityRuntime::Context>(context),
        [](napi_env env, void* data, void*) {
            CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
                "Finalizer for weak_ptr service extension context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        }, nullptr, nullptr);

    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION,
        "JsCallerInfoQueryExtension::Init end.");
}

void JsCallerInfoQueryExtension::OnStart(const AAFwk::Want& want)
{
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "begin");
    AbilityRuntime::Extension::OnStart(want);
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "end");
}

void JsCallerInfoQueryExtension::OnStop()
{
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "begin.");
    AbilityRuntime::Extension::OnStop();
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "end.");
}

sptr<IRemoteObject> JsCallerInfoQueryExtension::OnConnect(const AAFwk::Want& want)
{
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "begin");
    AbilityRuntime::Extension::OnConnect(want);
    sptr<CallerInfoQueryStubImpl> remoteObject = new (std::nothrow) CallerInfoQueryStubImpl(
        std::static_pointer_cast<JsCallerInfoQueryExtension>(shared_from_this()));

    if (remoteObject == nullptr) {
        CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "OnConnect get null");
        return remoteObject;
    }
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "end");
    return remoteObject->AsObject();
}

void JsCallerInfoQueryExtension::OnDisconnect(const AAFwk::Want& want)
{
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "begin.");
    AbilityRuntime::Extension::OnDisconnect(want);
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "end.");
}

napi_value PromiseResolve(napi_env env, napi_callback_info info)
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Promise resolve");
    size_t argc = PARAM_COUNT;
    napi_value args[] = { nullptr };
    void* data = nullptr;
    napi_status retVal = napi_get_cb_info(env,
        info, &argc, args, nullptr, &data);
    if (retVal != napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "Failed to get data function");
        return nullptr;
    }
    AsyncCallBackData* cbData = static_cast<AsyncCallBackData*>(data);
    auto asyncData = cbData->GetData().lock();
    if (asyncData == nullptr) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "Napi js callback data has been released");
        return nullptr;
    }
    asyncData->HandlePromiseResolve(env, args[0]);
    return nullptr;
}

napi_value PromiseReject(napi_env env, napi_callback_info info)
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION, "Promise reject");
    size_t argc = PARAM_COUNT;
    napi_value args[] = { nullptr };
    void* data = nullptr;
    napi_status retVal = napi_get_cb_info(env,
        info, &argc, args, nullptr, &data);
    if (retVal != napi_status::napi_ok) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "Failed to get data function");
        return nullptr;
    }
    AsyncCallBackData* cbData = static_cast<AsyncCallBackData*>(data);
    auto asyncData = cbData->GetData().lock();
    if (asyncData == nullptr) {
        CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "Napi js callback data has been released");
        return nullptr;
    }
    asyncData->HandlePromiseReject(env, args[0]);
    return nullptr;
}

int JsCallerInfoQueryExtension::OnQueryCallerInfo(
    std::string phoneNumber, QueryCallerInfoResult& result, int32_t waitTimeMs)
{
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "begin");
    std::shared_ptr<CallerInfoAsyncData> asyncData = std::make_shared<CallerInfoAsyncData>();
    auto task = [phoneNumber, this, asyncData]() {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "query task begin.");
        AbilityRuntime::HandleScope handleScope(jsRuntime_);
        auto env = jsRuntime_.GetNapiEnv();
        napi_value argv[] = { AbilityRuntime::CreateJsValue(env, phoneNumber) };
        if (!jsObj_) {
            CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "jsObj is null.");
            return;
        }

        napi_value value = jsObj_->GetNapiValue();
        if (value == nullptr) {
            CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
                "Failed to get CallerInfoQueryExtension object");
            return;
        }
        auto result = NapiHelper::CallObjectMethod(env,
            value, "onQueryCallerInfo", argv, JS_NAPI_ARGC_ONE);

        if (auto exception = NapiHelper::GetException(env); exception != nullptr) {
            asyncData->HandleException(env, exception);
            CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
                "onQueryCallerInfo handle exception.");
            return;
        }
        auto data = asyncData->CreateAsyncCallBackData();
        if (data == nullptr) {
            CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
                "Failed to create async callback data");
            return;
        }
        NapiHelper::HandlePromise(env,
            result, data, PromiseResolve, PromiseReject);
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION,
            "onQueryCallerInfo task end.");
    };
    int ret = CallerInfoQueryStub::RESULT_SUCCESS;
    handler_->PostTask(task);
    if (!asyncData->Await(waitTimeMs)) {
        result = asyncData->GetQueryCallerInfoResult();
        return CallerInfoQueryStub::RESULT_FAIL;
    }
    result = asyncData->GetQueryCallerInfoResult();
    CALLER_INFO_QUERY_HILOGI(CALLER_INFO_QUERY_MODULE_EXTENSION, "end");
    return ret;
}

std::string JsCallerInfoQueryExtension::GetSrcPath()
{
    std::string srcPath;
    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
        return srcPath;
    }
    return std::string();
}
}
}