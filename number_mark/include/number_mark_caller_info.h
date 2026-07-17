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

#ifndef NUMBER_MARK_CALLER_INFO_H
#define NUMBER_MARK_CALLER_INFO_H

#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "ability_connect_callback_stub.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "want.h"

#include "number_identity_json_helper.h"

namespace OHOS {
namespace Telephony {
using AAFwk::Want;
using std::enable_shared_from_this;
using std::optional;
using std::promise;
using std::string;
using std::vector;

class CallerInfo {
  public:
    string contactName;
    optional<string> employeeId;
    optional<string> department;
    optional<string> position;
    bool IsEmpty() const;
    string GetMarkContent() const;
    optional<string> GetMarkDetails() const;
};

enum NumberMarkVendorErrCode {
    OTHER_ERROR = -1,
    SUCCESS = 0,
};

class CallerInfoQueryResult {
  public:
    static inline CallerInfoQueryResult Fail(int errCode = NumberMarkVendorErrCode::OTHER_ERROR)
    {
        CallerInfoQueryResult result;
        result.errCode = errCode;
        return result;
    }
    static inline CallerInfoQueryResult Success(const CallerInfo &callerInfo)
    {
        CallerInfoQueryResult result;
        result.errCode = NumberMarkVendorErrCode::SUCCESS;
        result.callerInfo = callerInfo;
        return result;
    }
    int errCode;
    string message;
    CallerInfo callerInfo;
    inline bool IsSuccess() const
    {
        return errCode == NumberMarkVendorErrCode::SUCCESS;
    }
};

struct CallerInfoProvider {
  public:
    static vector<CallerInfoProvider> Parse(const string &json);
    string bundleName;
    string extensionAbilityName;
    optional<bool> switchState;
    bool FromJSONObject(JSONValueType json);
};

class CallerInfoAbilityConnection : public AAFwk::AbilityConnectionStub {
  public:
    CallerInfoAbilityConnection();
    virtual ~CallerInfoAbilityConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;
    sptr<IRemoteObject> GetRemoteSync();

  private:
    promise<sptr<IRemoteObject>> remoteObject_;
};

class CallerInfoAbilityInterface : public IRemoteBroker {
  public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.telephony.call_enhanced.ICallerInfoQueryService");

    virtual CallerInfoQueryResult QueryCallerInfo(const string &phoneNumber) = 0;
};

class CallerInfoAbilityProxy : public IRemoteProxy<CallerInfoAbilityInterface> {
  public:
    explicit CallerInfoAbilityProxy(const sptr<IRemoteObject> &impl);
    virtual ~CallerInfoAbilityProxy();
    virtual CallerInfoQueryResult QueryCallerInfo(const string &phoneNumber) override;
};

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_MAiRK_CALLER_INFO_H */