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

#include "download_file_ability.h"
#include "parse_work_id_int.h"

#include "ability_context.h"
#include "ability_loader.h"
#include "datashare_ext_ability.h"
#include "datashare_predicates.h"
#include "new"
#include "number_identity_datashare_stub_impl.h"
#include "download_file_result_set_bridge.h"
#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"
#include "rdb_errno.h"
#include "rdb_utils.h"
#include "uri.h"
#include "utility"
#include "download_file.h"
#include "download_file_rdb.h"
#include "number_identity_ddl.h"

namespace OHOS {
using AbilityRuntime::Extension;
using namespace NativeRdb;
namespace Telephony {

DownloadFileAbility::DownloadFileAbility(std::shared_ptr<DataShareExtAbilityContext> parentContext)
    : DataShareExtAbility()
{
    if (parentContext == nullptr) {
        NUMBER_IDENTITY_LOGF("parent context is null!");
    }
    parentContext_ = parentContext;
}

DownloadFileAbility::~DownloadFileAbility() {}

std::shared_ptr<DownloadFileAbility> DownloadFileAbility::Create(
    std::shared_ptr<DataShareExtAbilityContext> parentContext)
{
    NUMBER_IDENTITY_LOGI("DownloadFileAbility::Create begin.");
    return std::make_shared<DownloadFileAbility>(parentContext);
}

sptr<IRemoteObject> DownloadFileAbility::OnConnect(const AAFwk::Want &want)
{
    NUMBER_IDENTITY_LOGI("DownloadFileAbility::OnConnect");
    Extension::OnConnect(want);
    sptr<DataShare::NumberIdentityDataShareStubImpl> remoteObject =
        new (std::nothrow) DataShare::NumberIdentityDataShareStubImpl();
    if (remoteObject == nullptr) {
        NUMBER_IDENTITY_LOGE("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    remoteObject->SetDownloadFileAbility(std::static_pointer_cast<DownloadFileAbility>(shared_from_this()));
    NUMBER_IDENTITY_LOGI("DownloadFileAbility %{public}s end.", __func__);
    return remoteObject->AsObject();
}

void DownloadFileAbility::OnStart(const AppExecFwk::Want &want)
{
    NUMBER_IDENTITY_LOGI("DownloadFileAbility::OnStart");
    Extension::OnStart(want);
}

int DownloadFileAbility::Insert(const Uri &uri, const DataShare::DataShareValuesBucket &value)
{
    return NUMBER_IDENTITY_ERROR;
}

std::shared_ptr<DataShare::DataShareResultSet> DownloadFileAbility::Query(
    const Uri &uri, const DataShare::DataSharePredicates &predicates, std::vector<std::string> &columns,
    DataShare::DatashareBusinessError &businessError)
{
    NUMBER_IDENTITY_LOGI("Query.");
    std::shared_ptr<DataShare::DataShareResultSet> sharedPtrResult = nullptr;
    std::vector<std::string> propertyKeys;
    std::vector<std::string> propertyValues;
    std::string timeWorkerStartTime =
        DownloadFileRdb::GetInstance().Query(PropertyKeys::TIME_WORKER_START_TIME_SECOND, "");
    auto token = parentContext_ != nullptr ? parentContext_->GetToken() : nullptr;
    std::string networkType = DownloadFileRdb::GetInstance().QueryNetworkType(token);
    if (timeWorkerStartTime.empty() && networkType == PropertyValues::WLAN_ONLY) {
        DownloadFile::GetInstance().StartTimer();
    }
    propertyKeys.push_back(PropertyKeys::NETWORK_TYPE);
    propertyValues.push_back(networkType);
    uint32_t listSize = propertyValues.size();
    std::shared_ptr<DataShare::ResultSetBridge> resultSet =
        std::make_shared<DownloadFileResultSetBridge>(propertyKeys, propertyValues, listSize);
    sharedPtrResult = std::make_shared<DataShare::DataShareResultSet>(resultSet);
    NUMBER_IDENTITY_LOGI("Query success.");
    return sharedPtrResult;
}

void DownloadFileAbility::DealTaskTypeOfWorkSheduler(const DataShare::DataShareValuesBucket &value)
{
    bool isValid = false;
    std::string networkType;
    std::string forceUpdated;
    DataShare::DataShareValueObject object = value.Get("work_id", isValid);
    std::string workIdStr = object;
    int32_t workId = 0;
    if (!ParseWorkIdInt(workIdStr, workId)) {
        NUMBER_IDENTITY_LOGE("invalid work_id: %{public}s", workIdStr.c_str());
        return;
    }
    NUMBER_IDENTITY_LOGI("workId: %{public}d", workId);
    auto token = parentContext_ != nullptr ? parentContext_->GetToken() : nullptr;
    networkType = DownloadFileRdb::GetInstance().QueryNetworkType(token);
    forceUpdated = DownloadFileRdb::GetInstance().Query(PropertyKeys::FORCE_UPDATED, "0");
    if (workId == BOOT_WORK_ID && forceUpdated == "0") {
        if (!DownloadFile::GetInstance().CheckNetworkAndDownloadFiles(token)) {
            DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::FORCE_UPDATED, "1");
        }
        if (networkType == PropertyValues::CLOSE_UPDATE) {
            DownloadFile::GetInstance().StopTimer();
        }
        return;
    }
    if (workId == BOOT_WORK_ID && networkType != PropertyValues::CLOSE_UPDATE) {
        DownloadFile::GetInstance().StartTimer();
    } else if (workId == COMMON_WORK_ID) {
        DownloadFile::GetInstance().CheckNetworkAndDownloadFiles(token);
    }
}

int DownloadFileAbility::Update(const Uri &uri, const DataShare::DataSharePredicates &predicates,
    const DataShare::DataShareValuesBucket &value)
{
    bool isValid = false;
    std::string queryValue;
    DataShare::DataShareValueObject object = value.Get("task_type", isValid);
    std::string task_type = object;
    NUMBER_IDENTITY_LOGI("task_type: %{public}s", task_type.c_str());
    auto token = parentContext_ != nullptr ? parentContext_->GetToken() : nullptr;
    if (task_type == PropertyValues::WORK_SHEDULER) {
        DealTaskTypeOfWorkSheduler(value);
    } else if (task_type == PropertyValues::WLAN_ONLY) {
        queryValue = DownloadFileRdb::GetInstance().QueryNetworkType(token);
        if (queryValue == PropertyValues::WLAN_ONLY) {
            return NUMBER_IDENTITY_ERR_SUCCESS;
        }
        DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::WLAN_ONLY);
        if (queryValue == PropertyValues::CLOSE_UPDATE) {
            DownloadFile::GetInstance().CheckNetworkAndDownloadFiles(token);
        }
    } else if (task_type == PropertyValues::ALL_NETWORK) {
        queryValue = DownloadFileRdb::GetInstance().QueryNetworkType(token);
        if (queryValue == PropertyValues::ALL_NETWORK) {
            return NUMBER_IDENTITY_ERR_SUCCESS;
        }
        DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::ALL_NETWORK);
        if (queryValue == PropertyValues::CLOSE_UPDATE) {
            DownloadFile::GetInstance().CheckNetworkAndDownloadFiles(token);
        }
    } else if (task_type == PropertyValues::CLOSE_UPDATE) {
        queryValue = DownloadFileRdb::GetInstance().QueryNetworkType(token);
        if (queryValue == PropertyValues::CLOSE_UPDATE) {
            return NUMBER_IDENTITY_ERR_SUCCESS;
        }
        DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NETWORK_TYPE, PropertyValues::CLOSE_UPDATE);
        DownloadFile::GetInstance().StopTimer();
    }
    NUMBER_IDENTITY_LOGI("Update success.");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

int DownloadFileAbility::Delete(const Uri &uri, const DataShare::DataSharePredicates &predicates)
{
    return NUMBER_IDENTITY_ERROR;
}
} // namespace Telephony
} // namespace OHOS