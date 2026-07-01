/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <future>
#include <mutex>
#include <new>
#include <string>
#include <vector>

#include "errors.h"
#include "extension_manager_client.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"
#include "number_identity_utils.h"
#include "string_ex.h"
#include "want.h"

#include "number_identity_hsdr_helper.h"

namespace OHOS {
namespace Telephony {
using namespace std;
using Client = AAFwk::ExtensionManagerClient;

TimeLogger hsdrConnectLogger;

int HsdrHelper::RequestHsdrAsync(HsdrRequest &payload, OnConnect onConnect, OnResponse onResponse, OnError onError)
{
    NUMBER_IDENTITY_LOGI("HsdrHelper::RequestHsdrAsync begin.");
    int errCode = NUMBER_IDENTITY_ERR_SUCCESS;
    hsdrConnectLogger.TimeStart();
    auto requestId = Subscribe({ onResponse, onError });
    payload.requestId = requestId;
    OnResponse wrapOnResponse = [this, requestId](const auto &response) { HandleResponse(requestId, response); };
    OnError wrapOnError = [this, requestId](int errCode) { HandleError(requestId, errCode); };
    errCode = ConnectHsdr([payload, onConnect, wrapOnError, wrapOnResponse](sptr<IRemoteObject> remoteObject) {
        if (onConnect != nullptr) {
            onConnect();
        }
        auto connectTag = "HSDR Connect for " + payload.requestId;
        hsdrConnectLogger.TimeEnd(connectTag.c_str());
        FAIL_IF_NULL(remoteObject, {
            wrapOnError(NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
            return;
        });
        HsdrProxy service(remoteObject);
        if (payload.hasRemoteCallback) {
            sptr<HsdrCallbackStub> callbackStub =
                new (std::nothrow) HsdrCallbackStub(payload.requestId, wrapOnResponse, wrapOnError);
            FAIL_IF_NULL(callbackStub, {
                wrapOnError(NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
                return;
            });
            int errCode = service.InvokeHsdrAsync(callbackStub, payload);
            HANDLE_ERR("service.InvokeHsdrAsync", errCode, {
                wrapOnError(errCode);
                return;
            });
        } else {
            int errCode = service.InvokeHsdrSync(wrapOnResponse, payload);
            HANDLE_ERR("service.InvokeHsdrSync", errCode, {
                wrapOnError(errCode);
                return;
            });
        }
    });
    LOG_IF_ERR(errCode, "ConnectHsdr");
    return errCode;
}

void HsdrConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    lock_guard lock(remoteProxyMutex_);
    NUMBER_IDENTITY_LOGI("HsdrConnection::OnAbilityConnectDone begin.");
    FAIL_IF_NULL(remoteObject, return);
    remoteObject_ = remoteObject;
    FAIL_IF_NULL(connectedCallback_, return);
    NUMBER_IDENTITY_LOGI("HsdrConnection::OnAbilityConnectDone callback begin.");
    connectedCallback_(remoteObject_);
    NUMBER_IDENTITY_LOGI("HsdrConnection::OnAbilityConnectDone callback done.");
    NUMBER_IDENTITY_LOGI("HsdrConnection::OnAbilityConnectDone end.");
}

void HsdrConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    lock_guard lock(remoteProxyMutex_);
    NUMBER_IDENTITY_LOGI("HsdrConnection::OnAbilityDisconnectDone");
    remoteObject_ = nullptr;
}

bool HsdrConnection::IsAlive()
{
    lock_guard lock(remoteProxyMutex_);
    return remoteObject_ != nullptr && !remoteObject_->IsObjectDead();
}

sptr<IRemoteObject> HsdrConnection::GetAbilityProxy()
{
    lock_guard lock(remoteProxyMutex_);
    return remoteObject_;
}

int HsdrHelper::ConnectHsdr(ConnectedCallback connectedCallback)
{
    lock_guard lock(connectionMutex_);
    NUMBER_IDENTITY_LOGI("ConnectHsdr begin");
    if (connection_ != nullptr && connection_->IsAlive()) {
        NUMBER_IDENTITY_LOGI("Already connected, use old connection.");
        connectedCallback(connection_->GetAbilityProxy());
        return OHOS::ERR_OK;
    }
    AAFwk::Want want;
    want.SetElementName(HSDR_BUNDLE_NAME, HSDR_ABILITY_NAME);
    want.SetAction("NO_PUSH");
    connection_ = new (nothrow) HsdrConnection(connectedCallback);
    FAIL_IF_NULL(connection_, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    auto errCode = Client::GetInstance().ConnectServiceExtensionAbility(want, connection_, HSDR_USERID);
    HANDLE_ERR("ConnectServiceExtensionAbility", errCode, {
        connection_ = nullptr;
        return errCode;
    });
    NUMBER_IDENTITY_LOGI("ConnectHsdr done.");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

void HsdrHelper::DisconnectHsdr()
{
    lock_guard connectionLock(connectionMutex_);
    lock_guard subscriptionLock(subscriptionMutex_);
    NUMBER_IDENTITY_LOGI("HsdrHelper::DisconnectHsdr begin.");
    subscriptions_.clear();
    FAIL_IF_NULL(connection_, return);
    auto errCode = Client::GetInstance().DisconnectAbility(connection_);
    connection_ = nullptr;
    HANDLE_ERR("ExtensionManagerClient::DisconnectAbility", errCode, return);
    NUMBER_IDENTITY_LOGI("HsdrHelper::DisconnectHsdr done.");
}

void HsdrHelper::HandleResponse(const string &requestId, const HsdrResponse &response)
{
    lock_guard lock(subscriptionMutex_);
    NUMBER_IDENTITY_LOGI("HandleResponse %{public}s => %{public}s", requestId.c_str(), response.requestId.c_str());
    if (auto it = subscriptions_.find(requestId); it != subscriptions_.end()) {
        auto &[_requestId, callbacks] = *it;
        auto &[onResponse, onError] = callbacks;
        onResponse(response);
    }
    Unsubscribe(requestId);
}

void HsdrHelper::HandleError(const string &requestId, int errCode)
{
    lock_guard lock(subscriptionMutex_);
    NUMBER_IDENTITY_LOGI("HandleError requestId = %{public}s, errCode = %{public}d", requestId.c_str(), errCode);
    if (auto it = subscriptions_.find(requestId); it != subscriptions_.end()) {
        auto &[_requestId, callbacks] = *it;
        auto &[_onResponse, onError] = callbacks;
        onError(errCode);
    }
    Unsubscribe(requestId);
}

string HsdrHelper::Subscribe(RequestCallbacks callbacks)
{
    lock_guard lock(subscriptionMutex_);
    // Generate request ID.
    while (true) {
        string requestId = ToString(GenerateRandomLong());
        if (subscriptions_.find(requestId) == subscriptions_.end()) {
            subscriptions_.insert({ requestId, callbacks });
            NUMBER_IDENTITY_LOGI("IpcSubscription: created %{public}s", requestId.c_str());
            return requestId;
        }
        NUMBER_IDENTITY_LOGW("Good luck! Duplicated requestId: %{public}s", requestId.c_str());
    }
}

void HsdrHelper::Unsubscribe(const string &requestId)
{
    lock_guard lock(subscriptionMutex_);
    if (auto it = subscriptions_.find(requestId); it != subscriptions_.end()) {
        subscriptions_.erase(it);
        NUMBER_IDENTITY_LOGI("IpcSubscription: removed %{public}s", requestId.c_str());
    }
}

int HsdrProxy::InvokeHsdrSync(OnResponse onResponse, const HsdrRequest &payload)
{
    NUMBER_IDENTITY_LOGI("InvokeHsdrSync begin.");
    int errCode = NUMBER_IDENTITY_ERR_SUCCESS;
    MessageParcel data;
    BOOL_CHECK(data.WriteInterfaceToken(GetDescriptor()), return NUMBER_IDENTITY_ERR_WRITE_DATA_FAIL);
    BOOL_CHECK(WriteStr16(data, payload.serviceName), return NUMBER_IDENTITY_ERR_WRITE_DATA_FAIL);
    BOOL_CHECK(WriteStr16(data, payload.requestId), return NUMBER_IDENTITY_ERR_WRITE_DATA_FAIL);
    vector<u16string> request = { OHOS::Str8ToStr16(payload.body) };
    BOOL_CHECK(data.WriteString16Vector(request), return NUMBER_IDENTITY_ERR_WRITE_DATA_FAIL);
    MessageParcel reply;
    MessageOption option;
    auto remote = Remote();
    FAIL_IF_NULL(remote, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    errCode = remote->SendRequest(static_cast<uint32_t>(payload.command), data, reply, option);
    HANDLE_ERR("remote->SendRequest()", errCode, return errCode);
    int replyCode = reply.ReadInt32();
    LOGI_EXPR(replyCode);
    vector<u16string> responseBuffer;
    BOOL_CHECK(reply.ReadString16Vector(&responseBuffer), return NUMBER_IDENTITY_ERR_READ_DATA_FAIL);
    HsdrResponse response;
    for (auto &&part : responseBuffer) {
        response.body += OHOS::Str16ToStr8(part);
    }
    onResponse(response);
    NUMBER_IDENTITY_LOGI("InvokeHsdrSync done.");
    return errCode;
}

int HsdrProxy::InvokeHsdrAsync(sptr<IRemoteObject> callbackStub, const HsdrRequest &payload)
{
    NUMBER_IDENTITY_LOGI("HsdrProxy::InvokeHsdrAsync begin.");
    int errCode = NUMBER_IDENTITY_ERR_SUCCESS;
    MessageParcel data;
    auto command = static_cast<uint32_t>(payload.command);
    LOGI_EXPR(command);
    BOOL_CHECK(data.WriteInterfaceToken(GetDescriptor()), return NUMBER_IDENTITY_ERR_WRITE_DATA_FAIL);
    BOOL_CHECK(WriteStr16(data, payload.serviceName), return NUMBER_IDENTITY_ERR_WRITE_DATA_FAIL);
    BOOL_CHECK(WriteStr16(data, payload.requestId), return NUMBER_IDENTITY_ERR_WRITE_DATA_FAIL);
    // A request can be devided into an array. Here we do not perform the partition optimization because it's small.
    vector<u16string> request = { OHOS::Str8ToStr16(payload.body) };
    BOOL_CHECK(data.WriteString16Vector(request), return NUMBER_IDENTITY_ERR_WRITE_DATA_FAIL);
    BOOL_CHECK(data.WriteRemoteObject(callbackStub), return NUMBER_IDENTITY_ERR_WRITE_DATA_FAIL);
    MessageParcel reply;
    MessageOption option;
    auto remote = Remote();
    FAIL_IF_NULL(remote, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    errCode = remote->SendRequest(command, data, reply, option);
    HANDLE_ERR("remote->SendRequest()", errCode, return errCode);
    auto replyCode = reply.ReadInt32();
    LOGI_EXPR(replyCode);
    HANDLE_ERR("reply.ReadInt32()", errCode, return errCode);
    NUMBER_IDENTITY_LOGI("HsdrProxy::InvokeHsdrAsync done.");
    return errCode;
}

int32_t HsdrCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    NUMBER_IDENTITY_LOGI("HsdrCallbackStub::OnRemoteRequest begin. code = %{public}u", code);
    int errCode = NUMBER_IDENTITY_ERR_SUCCESS;
    auto descripter = GetDescriptor();
    auto remoteDescripter = data.ReadInterfaceToken();
    BOOL_CHECK(descripter == remoteDescripter, {
        onError_(NUMBER_IDENTITY_ERR_PERMISSION_ERR);
        return NUMBER_IDENTITY_ERR_PERMISSION_ERR;
    });
    HsdrResponse response;
    BOOL_CHECK(ReadStr16(data, response.requestId), {
        onError_(NUMBER_IDENTITY_ERR_READ_DATA_FAIL);
        return NUMBER_IDENTITY_ERR_READ_DATA_FAIL;
    });
    if (response.requestId != requestId_) {
        NUMBER_IDENTITY_LOGW("expect %{public}s, received %{public}s", requestId_.c_str(), response.requestId.c_str());
    }
    int cloudErrCode = data.ReadInt32();
    LOGI_EXPR(cloudErrCode);
    vector<u16string> body;
    BOOL_CHECK(data.ReadString16Vector(&body), {
        onError_(NUMBER_IDENTITY_ERR_READ_DATA_FAIL);
        return NUMBER_IDENTITY_ERR_READ_DATA_FAIL;
    });
    NUMBER_IDENTITY_LOGI("body vector length %{public}ld", body.size());
    for (auto &str : body) {
        response.body += OHOS::Str16ToStr8(str);
    }
    NUMBER_IDENTITY_LOGI("body total length %{public}ld", response.body.size());
    onResponse_(response);
    NUMBER_IDENTITY_LOGI("HsdrCallbackStub::OnRemoteRequest done.");
    return errCode;
}

HsdrConnection::HsdrConnection(ConnectedCallback connectedCallback) : connectedCallback_(connectedCallback)
{
    NUMBER_IDENTITY_LOGI("Create HsdrConnection");
}

HsdrConnection::~HsdrConnection()
{
    NUMBER_IDENTITY_LOGI("Descroy HsdrConnection");
}

HsdrHelper::HsdrHelper()
{
    NUMBER_IDENTITY_LOGI("Create HsdrHelper");
}

HsdrCallbackStub::HsdrCallbackStub(const string &requestId, OnResponse onResponse, OnError onError)
    : requestId_(requestId), onResponse_(onResponse), onError_(onError)
{
    NUMBER_IDENTITY_LOGI("Create HsdrCallbackStub");
}

HsdrCallbackStub::~HsdrCallbackStub()
{
    NUMBER_IDENTITY_LOGI("Destroy HsdrCallbackStub");
}

HsdrProxy::HsdrProxy(const sptr<IRemoteObject> &remote) : IRemoteProxy<HsdrInterface>(remote)
{
    NUMBER_IDENTITY_LOGI("Create HsdrProxy");
}

HsdrProxy::~HsdrProxy()
{
    NUMBER_IDENTITY_LOGI("Desctroy HsdrProxy");
}

} // namespace Telephony
} // namespace OHOS