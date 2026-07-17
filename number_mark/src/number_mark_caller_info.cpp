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

#include <future>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "ability_manager_client.h"
#include "cJSON.h"
#include "icaller_info_query_service.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

#include "number_identity_errors.h"
#include "number_identity_json_helper.h"
#include "number_identity_models.h"
#include "number_identity_service_extension_client.h"
#include "number_identity_settings.h"
#include "number_identity_utils.h"
#include "number_mark_caller_info.h"
#include "number_mark_manager.h"

namespace OHOS {
namespace Telephony {
constexpr int CALLER_INFO_QUERY_WAIT_TIME = 2000;
using namespace std;

bool CallerInfo::IsEmpty() const
{
    return IsEmptyStr(contactName) && IsEmptyStr(employeeId) && IsEmptyStr(department) && IsEmptyStr(position);
}

string CallerInfo::GetMarkContent() const
{
    return Join({ contactName, employeeId.value_or("") });
}

optional<string> CallerInfo::GetMarkDetails() const
{
    return Join({ department.value_or(""), position.value_or("") });
}

vector<CallerInfoProvider> CallerInfoProvider::Parse(const string &json)
{
    vector<CallerInfoProvider> providers;
    JSONValueType element = nullptr;
    auto root = cJSON_Parse(json.c_str());
    bool isOk;
    FAIL_IF_NULL(root, goto finally);
    JSON_ASSERT_TYPE(root, Array, isOk, goto finally, "expect array type");
    cJSON_ArrayForEach(element, root)
    {
        CallerInfoProvider provider;
        if (!provider.FromJSONObject(element)) {
            continue;
        }
        providers.emplace_back(provider);
    }
finally:
    cJSON_Delete(root);
    return providers;
}

bool CallerInfoProvider::FromJSONObject(JSONValueType json)
{
    bool isOk = true;
    JSONValueType cursor = nullptr;
    JSON_ASSERT_TYPE(json, Object, isOk, goto finally, "expect object type");
    JSON_READ_REQUIRED_MEMBER(json, bundleName, String, cursor, isOk, goto finally);
    JSON_READ_REQUIRED_MEMBER(json, extensionAbilityName, String, cursor, isOk, goto finally);
    JSON_READ_OPTIONAL_MEMBER(json, switchState, Bool, cursor, isOk, goto finally);
finally:
    return isOk;
}

CallerInfoAbilityConnection::CallerInfoAbilityConnection() : AAFwk::AbilityConnectionStub()
{
    NUMBER_IDENTITY_LOGI("CallerInfoAbilityConnection created.");
}

CallerInfoAbilityConnection::~CallerInfoAbilityConnection()
{
    NUMBER_IDENTITY_LOGI("CallerInfoAbilityConnection destroyed.");
}

void CallerInfoAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    NUMBER_IDENTITY_LOGI(
        "connected to %{public}s, resultCode: %{public}d", element.GetAbilityName().c_str(), resultCode);
    FAIL_IF_NULL(remoteObject, return);
    remoteObject_.set_value(remoteObject);
}

void CallerInfoAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    NUMBER_IDENTITY_LOGI(
        "disconnected to %{public}s, resultCode: %{public}d", element.GetAbilityName().c_str(), resultCode);
}

sptr<IRemoteObject> CallerInfoAbilityConnection::GetRemoteSync()
{
    return remoteObject_.get_future().get();
}

CallerInfoAbilityProxy::CallerInfoAbilityProxy(const sptr<IRemoteObject> &remoteObject)
    : IRemoteProxy<CallerInfoAbilityInterface>(remoteObject)
{
    NUMBER_IDENTITY_LOGI("CallerInfoAbilityProxy created.");
}

CallerInfoAbilityProxy::~CallerInfoAbilityProxy()
{
    NUMBER_IDENTITY_LOGI("CallerInfoAbilityProxy destroyed.");
}

CallerInfoQueryResult CallerInfoAbilityProxy::QueryCallerInfo(const string &phoneNumber)
{
    CallerInfoQueryResult result;
    result.errCode = NUMBER_IDENTITY_ERR_FAIL;
    auto remote = Remote();
    FAIL_IF_NULL(remote, return result);
    MessageParcel data;
    BOOL_CHECK(data.WriteInterfaceToken(GetDescriptor()), return result);
    data.WriteString(phoneNumber);
    data.WriteInt32(CALLER_INFO_QUERY_WAIT_TIME);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    int errCode = remote->SendRequest(
        CallerInfoQuery::ICallerInfoQueryService::CallerInfoQueryCode::QUERY_CALLER_INFO, data, reply, option);
    HANDLE_ERR("SendRequest", errCode, return result);
    BOOL_CHECK(reply.ReadInt32(result.errCode), return result);
    auto &callerInfo = result.callerInfo;
    BOOL_CHECK(reply.ReadString(result.message), return result);
    string replyString;
    BOOL_CHECK(reply.ReadString(replyString), return result);
    callerInfo.contactName = replyString;
    BOOL_CHECK(reply.ReadString(replyString), return result);
    callerInfo.employeeId = replyString;
    BOOL_CHECK(reply.ReadString(replyString), return result);
    callerInfo.department = replyString;
    BOOL_CHECK(reply.ReadString(replyString), return result);
    callerInfo.position = replyString;
    return result;
}

} // namespace Telephony
} // namespace OHOS