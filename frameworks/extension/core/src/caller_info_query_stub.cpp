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

#include "caller_info_query_stub.h"
#include "caller_info_query_extension_callback_data.h"
#include "caller_info_query_extension_hilog.h"
#include <accesstoken_kit.h>
#include <ipc_skeleton.h>

namespace OHOS {
namespace CallerInfoQuery {

CallerInfoQueryStub::CallerInfoQueryStub()
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION,  "create caller info query stub");
}

CallerInfoQueryStub::~CallerInfoQueryStub()
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION,  "distory caller info query stub");
}

int CallerInfoQueryStub::OnQueryCallerInfoInner(MessageParcel& data, MessageParcel& reply)
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION,  "OnQueryCallerInfoInner");
    std::string phoneNumber = data.ReadString();
    int32_t waitTimeMs = data.ReadInt32();
    QueryCallerInfoResult result;
    auto ret = OnQueryCallerInfo(phoneNumber, result, waitTimeMs);
    reply.WriteInt32(ret);
    reply.WriteString(result.errorMsg_);
    reply.WriteString(result.info_.contactName_);
    reply.WriteString(result.info_.employeeId_);
    reply.WriteString(result.info_.department_);
    reply.WriteString(result.info_.position_);
    return ret;
}

int CallerInfoQueryStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption &option)
{
    CALLER_INFO_QUERY_HILOGW(CALLER_INFO_QUERY_MODULE_EXTENSION,  "onRemoteRequest code = %{public}u", code);
    std::u16string localDescripter = CallerInfoQueryStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (localDescripter != remoteDescripter) {
        CALLER_INFO_QUERY_HILOGE(CALLER_INFO_QUERY_MODULE_EXTENSION, "end descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    switch (code) {
        case static_cast<uint32_t>(static_cast<uint32_t>(CallerInfoQueryCode::QUERY_CALLER_INFO)):
            return OnQueryCallerInfoInner(data, reply);
        default:
            CALLER_INFO_QUERY_HILOGW(
                CALLER_INFO_QUERY_MODULE_EXTENSION, "receive unknown code, code = %{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return ERR_OK;
}

}
} // namespace OHOS
