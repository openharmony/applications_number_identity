/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
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

#ifndef NUMBER_IDENTITY_SERVICE_EXTENSION_CLIENT_H
#define NUMBER_IDENTITY_SERVICE_EXTENSION_CLIENT_H

#include <functional>
#include <mutex>
#include <string>

#include "ability_connect_callback_stub.h"

namespace OHOS {
namespace Telephony {
using AAFwk::Want;
using std::function;
using std::recursive_mutex;
using std::string;
using std::u16string;
using ConnectedCallback = function<void(sptr<IRemoteObject>)>;
constexpr const int32_t DEFAULT_USERID = -1;
class ServiceExtensionConnection : public AAFwk::AbilityConnectionStub {
public:
    explicit ServiceExtensionConnection(const string &tag, const ConnectedCallback &callback);

    virtual ~ServiceExtensionConnection();

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    bool IsAlive();

    sptr<IRemoteObject> GetAbilityProxy();

private:
    string tag_;

    ConnectedCallback connectedCallback_;

    recursive_mutex remoteProxyMutex_;

    sptr<IRemoteObject> remoteObject_;
};

class NumberIdentityServiceExtensionClient {
public:
    NumberIdentityServiceExtensionClient(const string &tag, const Want &want, int32_t userId = DEFAULT_USERID);

    int Connect(ConnectedCallback connectedCallback);

    void Disconnect();

protected:
    /**
     * Mutex for connection.
     */
    recursive_mutex connectionMutex_;
    /**
     * Connection object to the service. If already connected, the connection will be reused.
     */
    sptr<ServiceExtensionConnection> connection_;
    /**
     * Log tag.
     */
    string tag_;
    /**
     * Want and user id is the parameters to connect to the service. They should not be modified.
     * Each connection should have the same connect parameters, othewise the connection cannot be reused.
     */
    Want want_;
    int32_t userId_;
};

inline bool WriteStr16(MessageParcel &parcel, const string &str8)
{
    return parcel.WriteString16(OHOS::Str8ToStr16(str8));
}

inline bool ReadStr16(MessageParcel &parcel, string &str8)
{
    u16string str16;
    auto isOk = parcel.ReadString16(str16);
    str8 = Str16ToStr8(str16);
    return isOk;
}

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_SERVICE_EXTENSION_CLIENT_H */