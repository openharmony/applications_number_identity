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

#ifndef CALLER_INFO_QUERY_STUB_IMPL_H
#define CALLER_INFO_QUERY_STUB_IMPL_H

#include <memory>
#include "caller_info_query_stub.h"
#include "js_caller_info_query_extension.h"

namespace OHOS {
namespace CallerInfoQuery {
class CallerInfoQueryStubImpl : public CallerInfoQueryStub {
public:
    explicit CallerInfoQueryStubImpl(const std::shared_ptr<JsCallerInfoQueryExtension>& extension)
        : extension_(extension) {}

    virtual ~CallerInfoQueryStubImpl() {}

    int OnQueryCallerInfo(std::string phoneNumber,  QueryCallerInfoResult& result, int32_t waitTimeMs) override;

private:
    std::weak_ptr<JsCallerInfoQueryExtension> extension_;
};
}
}
#endif

