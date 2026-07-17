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
#include "number_identity_settings.h"
#include "application_context.h"
#include "core_service_client.h"
#include "datashare_helper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "number_identity_errors.h"
#include "number_identity_rdb_helper.h"
#include "number_identity_utils.h"

#include "net_conn_client.h"
#include "parameter.h"
#include "system_ability_definition.h"
#include "sysversion.h"
#include "telephony_errors.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {

constexpr const char *SETTINGS_URI = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATA_URI = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA";

NetworkInfo::NetworkInfo() : hasNetWork(false), ip("")
{
    NUMBER_IDENTITY_LOGD("NetworkInfo created.");
}

NetworkInfo::~NetworkInfo()
{
    NUMBER_IDENTITY_LOGD("NetworkInfo destroyed.");
}

int NetworkInfo::Refresh()
{
    using namespace NetManagerStandard;
    int errCode = ERR_OK;
    auto &net = NetConnClient::GetInstance();
    NetHandle netHandle;
    errCode = net.GetDefaultNet(netHandle);
    HANDLE_ERR("net.GetDefaultNet", errCode, return errCode);
    NetAllCapabilities netAllCap;
    errCode = net.GetNetCapabilities(netHandle, netAllCap);
    auto bearerTypesCount = netAllCap.bearerTypes_.size();
    NUMBER_IDENTITY_LOGI("capabilities size: %{public}ld, errCode %{public}d ", bearerTypesCount, errCode);
    if (ERR_OK != errCode || bearerTypesCount == 0) {
        NUMBER_IDENTITY_LOGI("No network.");
        Clear();
        return ERR_OK;
    }
    NetLinkInfo info;
    errCode = net.GetConnectionProperties(netHandle, info);
    HANDLE_ERR("GetConnectionProperties", errCode, {
        Clear();
        return errCode;
    });
    const auto &ips = info.netAddrList_;
    NUMBER_IDENTITY_LOGD("Got netAddrList size = %{public}ld", ips.size());
    if (ips.begin() == ips.end()) {
        NUMBER_IDENTITY_LOGW("No ip address found.");
        return errCode;
    }
    SetIp(ips.begin()->address_);
    return errCode;
}

void NetworkInfo::SetIp(const string &ipAddr)
{
    ip = ipAddr;
    hasNetWork = true;
}

void NetworkInfo::Clear()
{
    ip = "";
    hasNetWork = false;
}

string GetDeviceType()
{
    static auto cstr = GetProductModel();
    return cstr;
}

string GetOsVersion()
{
    auto osFullName = GetOSFullName();
    FAIL_IF_NULL(osFullName, return "");
    stringstream buf;
    buf << osFullName << "_" << GetMajorVersion() << "_" << GetSeniorVersion();
    return buf.str();
}

int GetSettingsData(const string &key, string &value, sptr<IRemoteObject> token)
{
    using namespace DataShare;
    using namespace std;
    int errCode = NUMBER_IDENTITY_ERR_READ_DATA_FAIL;
    FAIL_IF_NULL(token, return errCode);
    auto helper = DataShare::DataShareHelper::Creator(token, SETTINGS_URI);
    FAIL_IF_NULL(helper, return errCode);
    DataSharePredicates predicates;
    predicates.EqualTo("KEYWORD", key);
    Uri uri(string(SETTINGS_DATA_URI) + "?Proxy=true&key=" + key);
    vector<string> columns;
    auto resultSet = helper->Query(uri, predicates, columns);
    FAIL_IF_NULL(resultSet, goto releaseHelper);
    bool isNull;
    int count;
    int columnIndex;
    errCode = resultSet->GetRowCount(count);
    HANDLE_ERR("resultSet->GetRowCount", errCode, goto finally);
    if (count == 0) {
        NUMBER_IDENTITY_LOGI("Record of settings not found.");
        errCode = NUMBER_IDENTITY_ERR_READ_DATA_FAIL;
        goto finally;
    }
    errCode = resultSet->GoToFirstRow();
    HANDLE_ERR("resultSet->GoToFirstRow()", errCode, goto finally);
    FETCH_FIELD("VALUE", String, value, isNull, *resultSet, columnIndex, errCode, goto finally);
    if (isNull) {
        NUMBER_IDENTITY_LOGE("Value of settings %{public}s is NULL", key.c_str());
    } else {
        errCode = NUMBER_IDENTITY_ERR_SUCCESS;
    }
finally:
    resultSet->Close();
releaseHelper:
    helper->Release();
    return errCode;
}

int InsertSettingsData(const string &key, string &value, sptr<IRemoteObject> token)
{
    using namespace DataShare;
    using namespace std;
    int errCode = NUMBER_IDENTITY_ERR_READ_DATA_FAIL;
    FAIL_IF_NULL(token, return errCode);
    auto helper = DataShare::DataShareHelper::Creator(token, SETTINGS_URI);
    FAIL_IF_NULL(helper, return errCode);
    DataShare::DataShareValuesBucket valuesBucket;
    valuesBucket.Put("KEYWORD", key);
    valuesBucket.Put("VALUE", value);
    Uri uri(string(SETTINGS_DATA_URI) + "?Proxy=true&key=" + key);
    int result = helper->Insert(uri, valuesBucket);
    NUMBER_IDENTITY_LOGI("insert result: %{public}d.", result);
    helper->Release();
    if (result > 0) {
        return NUMBER_IDENTITY_ERR_SUCCESS;
    } else {
        NUMBER_IDENTITY_LOGE("insert error!");
        return NUMBER_IDENTITY_ERROR;
    }
}

int UpdateSettingsData(const string &key, string &value, sptr<IRemoteObject> token)
{
    using namespace DataShare;
    using namespace std;
    int errCode = NUMBER_IDENTITY_ERR_READ_DATA_FAIL;
    FAIL_IF_NULL(token, return errCode);
    auto helper = DataShare::DataShareHelper::Creator(token, SETTINGS_URI);
    FAIL_IF_NULL(helper, return errCode);
    DataSharePredicates predicates;
    predicates.EqualTo("KEYWORD", key);
    DataShare::DataShareValuesBucket valuesBucket;
    valuesBucket.Put("VALUE", value);
    Uri uri(string(SETTINGS_DATA_URI) + "?Proxy=true&key=" + key);
    int result = helper->Update(uri, predicates, valuesBucket);
    NUMBER_IDENTITY_LOGI("update result: %{public}d.", result);
    helper->Release();
    if (result > 0) {
        return NUMBER_IDENTITY_ERR_SUCCESS;
    } else {
        NUMBER_IDENTITY_LOGE("update error!");
        return NUMBER_IDENTITY_ERROR;
    }
}

bool IsSlotNetworkRoaming(int32_t slotId)
{
    auto &client = CoreServiceClient::GetInstance();
    sptr<NetworkState> networkState;
    auto errCode = client.GetNetworkState(slotId, networkState);
    if (errCode != TELEPHONY_ERR_SUCCESS) {
        return false;
    }
    return networkState != nullptr && networkState->IsRoaming();
}

bool IsNetworkRoaming()
{
    auto &client = CoreServiceClient::GetInstance();
    auto simCount = client.GetMaxSimCount();
    if (simCount == 1) {
        return IsSlotNetworkRoaming(0);
    } else {
        return IsSlotNetworkRoaming(0) || IsSlotNetworkRoaming(1);
    }
}

int QueryAppName(const string &bundleName, string &appName)
{
    using namespace AppExecFwk;
    int errCode = NUMBER_IDENTITY_ERR_SUCCESS;
    auto systemAbilityManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    FAIL_IF_NULL(systemAbilityManager, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    auto bundleMgrSa = systemAbilityManager->GetSystemAbility(OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    FAIL_IF_NULL(bundleMgrSa, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    auto bundleMgr = OHOS::iface_cast<IBundleMgr>(bundleMgrSa);
    FAIL_IF_NULL(bundleMgr, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    auto bundleResourceMgr = bundleMgr->GetBundleResourceProxy();
    FAIL_IF_NULL(bundleResourceMgr, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    BundleResourceInfo bundleResourceInfo;
    auto flags = static_cast<int>(ResourceFlag::GET_RESOURCE_INFO_ALL);
    errCode = bundleResourceMgr->GetBundleResourceInfo(bundleName, flags, bundleResourceInfo);
    HANDLE_ERR("GetApplicationInfo", errCode, return errCode);
    appName = bundleResourceInfo.label;
    NUMBER_IDENTITY_LOGI("appName: %{public}s.", appName.c_str());
    return errCode;
}

} // namespace Telephony
} // namespace OHOS