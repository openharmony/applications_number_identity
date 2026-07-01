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

#ifndef NUMBER_IDENTITY_HSDR_HELPER_H
#define NUMBER_IDENTITY_HSDR_HELPER_H

#include <functional>
#include <map>
#include <mutex>
#include <tuple>

#include "ability_connect_callback_stub.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "singleton.h"

#include "number_identity_service_extension_client.h"
#include "number_identity_utils.h"

namespace OHOS {
namespace Telephony {
using std::function;
using std::map;
using std::recursive_mutex;
using std::string;
using std::tuple;

constexpr const char *HSDR_BUNDLE_NAME = "com.ohos.hsdr";
constexpr const char *HSDR_ABILITY_NAME = "HSDRService";
constexpr const int HSDR_USERID = 100;

enum class HsdrCommands {
    COMMAND_QUERY_CURRENT_INFO = 1,
    COMMAND_CLOUD_CONNECT = 2,
    COMMAND_UCS_REQUEST = 3,
    COMMAND_FILE_REQUEST = 4,
};

class HsdrRequest {
  public:
    string requestId;
    string serviceName;
    HsdrCommands command;
    string body;
    bool hasRemoteCallback;
};

class HsdrResponse {
  public:
    string requestId;
    string body;
};

using ConnectedCallback = function<void(sptr<IRemoteObject>)>;
using OnConnect = function<void()>;
using OnResponse = function<void(const HsdrResponse &)>;
using OnError = function<void(int)>;
using RequestCallbacks = tuple<OnResponse, OnError>;

class HsdrCallBack : public IRemoteBroker {
  public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Security.CloudConnectCallback");
};

class HsdrConnection : public AAFwk::AbilityConnectionStub {
  public:
    explicit HsdrConnection(ConnectedCallback connectedCallback);

    ~HsdrConnection();

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    bool IsAlive();

    sptr<IRemoteObject> GetAbilityProxy();

  private:
    ConnectedCallback connectedCallback_;

    recursive_mutex remoteProxyMutex_;

    sptr<IRemoteObject> remoteObject_;
};

class HsdrHelper {
    DECLARE_DELAYED_REF_SINGLETON(HsdrHelper);

  public:
    int RequestHsdrAsync(HsdrRequest &payload, OnConnect onConnect, OnResponse onResponse, OnError onError);

    int ConnectHsdr(ConnectedCallback connectedCallback);

    void DisconnectHsdr();
    /**
     * Invoke OnResponse callback via request id, and remove it.
     */
    void HandleResponse(const string &requestId, const HsdrResponse &response);
    /**
     * Invoke OnError callback via request id, and remove it.
     */
    void HandleError(const string &requestId, int errCode);
    /**
     * Register subscribe callbacks, returns request id.
     */
    string Subscribe(RequestCallbacks callbacks);
    /**
     * Unsubscribe callbacks with request ID.
     */
    void Unsubscribe(const string &requestId);

  private:
    /**
     * Mutex for subscriptions.
     */
    recursive_mutex subscriptionMutex_;

    /**
     * The key is request ID.
     */
    map<string, RequestCallbacks> subscriptions_;

    /**
     * Mutex for connection.
     */
    recursive_mutex connectionMutex_;

    sptr<HsdrConnection> connection_;
};

class HsdrCallbackStub : public IRemoteStub<HsdrCallBack> {
  public:
    HsdrCallbackStub(const string &requestId, OnResponse onResponse, OnError onError);

    ~HsdrCallbackStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

  private:
    string requestId_;
    OnResponse onResponse_;
    OnError onError_;
};

class HsdrInterface : public IRemoteBroker {
  public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Security.HSDR");

    virtual int InvokeHsdrSync(OnResponse onResponse, const HsdrRequest &payload) = 0;

    virtual int InvokeHsdrAsync(sptr<IRemoteObject> callbackStub, const HsdrRequest &payload) = 0;
};

class HsdrProxy : IRemoteProxy<HsdrInterface> {
  public:
    explicit HsdrProxy(const sptr<IRemoteObject> &remote);

    virtual ~HsdrProxy();

    virtual int InvokeHsdrSync(OnResponse onResponse, const HsdrRequest &payload) override;

    virtual int InvokeHsdrAsync(sptr<IRemoteObject> callbackStub, const HsdrRequest &payload) override;

  private:
    static constexpr int COMMAND_REQUEST_HSDR = OHOS::MIN_TRANSACTION_ID + 1;
};

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_HSDR_HELPER_H */
