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

#ifndef CALLER_INFO_QUERY_STUB_H
#define CALLER_INFO_QUERY_STUB_H

#include <iremote_stub.h>
#include <map>
#include "icaller_info_query_service.h"

namespace OHOS {
namespace CallerInfoQuery {
class CallerInfoQueryStub : public IRemoteStub<ICallerInfoQueryService> {
public:
    CallerInfoQueryStub();

    virtual ~CallerInfoQueryStub();

    /**
     * @brief The OnRemoteRequest callback.
     *
     * @param code The code.
     * @param data The data.
     * @param reply The reply.
     * @param option The option.
     * @return ERR_OK on success, others on failure.
     */
    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;

private:
    int OnQueryCallerInfoInner(MessageParcel& data, MessageParcel& reply);
};
}
} // namespace OHOS
#endif

