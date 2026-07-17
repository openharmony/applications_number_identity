/*
 * Copyright (C) 2024 Huawei Device Co., Ltd. rights reserved.
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

#include <unistd.h>
#include "number_identity_log_wrapper.h"
#include "number_identity_errors.h"
#include "download_file_rdb.h"
#include "number_identity_database.h"
#include "number_identity_settings.h"
#include "number_identity_ddl.h"

namespace OHOS {
namespace Telephony {
static constexpr const char *STRANGE_NUMBER_IDENTITY_SETTINGS_KEY = "settings.telephony.number_identity_switch_oobe";
constexpr const char *SETTING_IS_OFF = "0";
DownloadFileRdb& DownloadFileRdb::GetInstance()
{
    static auto instance = new DownloadFileRdb();
    return *instance;
}

DownloadFileRdb::DownloadFileRdb()
{
    NUMBER_IDENTITY_LOGI("construct DownloadFileRdb.");
}

DownloadFileRdb::~DownloadFileRdb()
{
    NUMBER_IDENTITY_LOGI("destruct DownloadFileRdb.");
}

void DownloadFileRdb::SetDBDirectory(const string &dir)
{
    NumberIdentityDatabase::GetInstance()->SetDBDirectory(dir);
}

string DownloadFileRdb::Query(const char * key, const std::string defaultValue)
{
    NUMBER_IDENTITY_LOGI("Query begin.");
    string value;
    shared_ptr<NumberIdentityDatabase> numberIdentityDatabase = NumberIdentityDatabase::GetInstance();
    if (numberIdentityDatabase == nullptr) {
        NUMBER_IDENTITY_LOGE("numberIdentityDatabase is nullptr!");
        return "";
    }
    int result = numberIdentityDatabase->GetProperty(key, value);
    if (!result) {
        NUMBER_IDENTITY_LOGI("query %{public}s success, value: %{public}s", key, value.c_str());
        return value;
    } else {
        NUMBER_IDENTITY_LOGI("return %{public}s defaultValue: %{public}s", key, defaultValue.c_str());
        return defaultValue;
    }
}

string DownloadFileRdb::QueryNetworkType(sptr<IRemoteObject> token)
{
    NUMBER_IDENTITY_LOGI("Query begin.");
    string value;
    shared_ptr<NumberIdentityDatabase> numberIdentityDatabase = NumberIdentityDatabase::GetInstance();
    if (numberIdentityDatabase == nullptr) {
        NUMBER_IDENTITY_LOGE("numberIdentityDatabase is nullptr!");
        return "";
    }
    int result = numberIdentityDatabase->GetProperty(PropertyKeys::NETWORK_TYPE, value);
    if (!result) {
        NUMBER_IDENTITY_LOGI("query network_type success, value: %{public}s", value.c_str());
        return value;
    } else {
        GetSettingsData(STRANGE_NUMBER_IDENTITY_SETTINGS_KEY, value, token);
        NUMBER_IDENTITY_LOGI("value: %{public}s", value.c_str());
        if (value == SETTING_IS_OFF) {
            NUMBER_IDENTITY_LOGI("oobe setting is off");
            return PropertyValues::CLOSE_UPDATE;
        } else {
            NUMBER_IDENTITY_LOGI("return network_type defaultValue: %{public}s", PropertyValues::WLAN_ONLY);
            return PropertyValues::WLAN_ONLY;
        }
    }
}

int32_t DownloadFileRdb::UpdateOrInsert(const char *key, std::string value)
{
    NUMBER_IDENTITY_LOGI("UpdateOrInsert key:%{public}s, value:%{public}s.", key, value.c_str());
    shared_ptr<NumberIdentityDatabase> numberIdentityDatabase = NumberIdentityDatabase::GetInstance();
    if (numberIdentityDatabase == nullptr) {
        NUMBER_IDENTITY_LOGE("numberIdentityDatabase is nullptr!");
        return NUMBER_IDENTITY_ERROR;
    }
    int result = numberIdentityDatabase->PutProperty(key, value);
    if (result) {
        NUMBER_IDENTITY_LOGI("update fail, result: %{public}d", result);
        return NUMBER_IDENTITY_ERROR;
    }
    NUMBER_IDENTITY_LOGI("update success.");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}
} // Telephony
} // OHOS