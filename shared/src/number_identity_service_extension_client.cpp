/**
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <mutex>

#include "extension_manager_client.h"
#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"
#include "number_identity_service_extension_client.h"
#include "number_identity_utils.h"

namespace OHOS {
namespace Telephony {
using namespace std;
using Client = AAFwk::ExtensionManagerClient;

ServiceExtensionConnection::ServiceExtensionConnection(const string &tag, const ConnectedCallback &callback)
    : tag_(tag), connectedCallback_(callback)
{
    NUMBER_IDENTITY_LOGI("create %{public}s ServiceExtensionConnection", tag_.c_str());
}

ServiceExtensionConnection::~ServiceExtensionConnection()
{
    NUMBER_IDENTITY_LOGI("destroy %{public}s ServiceExtensionConnection", tag_.c_str());
}

void ServiceExtensionConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    lock_guard lock(remoteProxyMutex_);
    NUMBER_IDENTITY_LOGI("%{public}s OnAbilityConnectDone begin.", tag_.c_str());
    FAIL_IF_NULL(remoteObject, return);
    remoteObject_ = remoteObject;
    FAIL_IF_NULL(connectedCallback_, return);
    connectedCallback_(remoteObject_);
    NUMBER_IDENTITY_LOGI("%{public}s OnAbilityConnectDone end.", tag_.c_str());
}

void ServiceExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    lock_guard lock(remoteProxyMutex_);
    NUMBER_IDENTITY_LOGI("%{public}s OnAbilityDisconnectDone", tag_.c_str());
    remoteObject_ = nullptr;
}

bool ServiceExtensionConnection::IsAlive()
{
    lock_guard lock(remoteProxyMutex_);
    return remoteObject_ != nullptr && !remoteObject_->IsObjectDead();
}

sptr<IRemoteObject> ServiceExtensionConnection::GetAbilityProxy()
{
    lock_guard lock(remoteProxyMutex_);
    return remoteObject_;
}

NumberIdentityServiceExtensionClient::NumberIdentityServiceExtensionClient(
    const string &tag, const Want &want, int32_t userId)
    : tag_(tag), want_(want), userId_(userId)
{
    NUMBER_IDENTITY_LOGI("create %{public}s NumberIdentityServiceExtensionClient", tag_.c_str());
}

int NumberIdentityServiceExtensionClient::Connect(ConnectedCallback connectedCallback)
{
    lock_guard lock(connectionMutex_);
    NUMBER_IDENTITY_LOGI("NumberIdentityServiceExtensionClient::Connect begin");
    if (connection_ != nullptr && connection_->IsAlive()) {
        NUMBER_IDENTITY_LOGI("Already connected to %{public}s, use old connection.", want_.GetBundle().c_str());
        connectedCallback(connection_->GetAbilityProxy());
        return OHOS::ERR_OK;
    }
    connection_ = new (nothrow) ServiceExtensionConnection(tag_.c_str(), connectedCallback);
    FAIL_IF_NULL(connection_, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    auto errCode = Client::GetInstance().ConnectServiceExtensionAbility(want_, connection_, userId_);
    HANDLE_ERR("ConnectServiceExtensionAbility", errCode, {
        connection_ = nullptr;
        return errCode;
    });
    NUMBER_IDENTITY_LOGI("%{public}s::Connect done.", tag_.c_str());
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

void NumberIdentityServiceExtensionClient::Disconnect()
{
    lock_guard connectionLock(connectionMutex_);
    NUMBER_IDENTITY_LOGI("%{public}s Disconnect begin.", tag_.c_str());
    FAIL_IF_NULL(connection_, return);
    auto errCode = Client::GetInstance().DisconnectAbility(connection_);
    connection_ = nullptr;
    HANDLE_ERR("ExtensionManagerClient::DisconnectAbility", errCode, return);
    NUMBER_IDENTITY_LOGI("%{public}s Disconnect done.", tag_.c_str());
}

} // namespace Telephony
} // namespace OHOS