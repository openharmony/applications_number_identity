/*
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

#ifndef NUMBER_IDENTITY_SETTINGS_H
#define NUMBER_IDENTITY_SETTINGS_H
#include "context.h"
#include "iremote_object.h"
#include "singleton.h"
#include <string>

namespace OHOS {
namespace Telephony {
using std::string;

class NetworkInfo {
    DECLARE_DELAYED_REF_SINGLETON(NetworkInfo);

  public:
    bool hasNetWork;
    string ip;
    int Refresh();
    void SetIp(const string &ipAddr);
    void Clear();
};

string GetDeviceType();

string GetOsVersion();

int GetSettingsData(const string &key, string &value, sptr<IRemoteObject> token);

int InsertSettingsData(const string &key, string &value, sptr<IRemoteObject> token);

int UpdateSettingsData(const string &key, string &value, sptr<IRemoteObject> token);

bool IsSlotNetworkRoaming(int32_t slotId, sptr<IRemoteObject> token);

bool IsNetworkRoaming();

int QueryAppName(const string &bundleName, string &appName);

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_SETTINGS_H */