/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef NATIVE_COMMON_TYPE_H
#define NATIVE_COMMON_TYPE_H

#include "ability_context.h"
#include "base_context.h"
#include "context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "number_identity_models.h"
#include "pac_map.h"
#include <memory>
#include <string>

namespace OHOS {
namespace Telephony {
const int32_t ARGS_ONE = 1;
const int32_t kMaxNumberLen = 255;
const int16_t BOOL_VALUE_IS_TRUE = 1;
const int16_t NATIVE_VERSION = 1;
const int16_t NATIVE_FLAGS = 0;
const int16_t ZERO_VALUE = 0;
const int16_t ONLY_ONE_VALUE = 1;
const int16_t TWO_VALUE_LIMIT = 2;
const int16_t VALUE_MAXIMUM_LIMIT = 3;
const int16_t THREE_VALUE_MAXIMUM_LIMIT = 3;
const int16_t FOUR_VALUE_MAXIMUM_LIMIT = 4;
const int16_t FIVE_VALUE_MAXIMUM_LIMIT = 5;
const int16_t ARRAY_INDEX_FIRST = 0;
const int16_t ARRAY_INDEX_SECOND = 1;
const int16_t ARRAY_INDEX_THIRD = 2;
const int16_t ARRAY_INDEX_FOURTH = 3;
const int16_t DATA_LENGTH_ONE = 1;
const int16_t DATA_LENGTH_TWO = 2;
const int16_t DTMF_DEFAULT_OFF = 10;
const int16_t PHONE_NUMBER_MAXIMUM_LIMIT = 255;
const int16_t MESSAGE_CONTENT_MAXIMUM_LIMIT = 160;
const int16_t NAPI_MAX_TIMEOUT_SECOND = 10;
const int16_t UNKNOWN_EVENT = 0;

struct AsyncContext : BaseContext {
    char number[kMaxNumberLen + 1] = { 0 };
    size_t numberLen = 0;
    napi_value value[VALUE_MAXIMUM_LIMIT] = { 0 };
    size_t valueLen = 0;
    bool isExactMatch = true;
    std::shared_ptr<OHOS::AbilityRuntime::Context> context = nullptr;
};

struct NumberLocationAsyncContext : AsyncContext {
    std::string numberLocation = "";
};

struct NumberLocationsAsyncContext : AsyncContext {
    std::vector<std::string> phoneNumbers{};
    std::vector<std::string> numberLocations{};
};

struct GetNumberMarkInfoAsyncContext : BaseContext {
    std::string phoneNumber = "";
    NumberMarkInfo markInfo;
    std::shared_ptr<AbilityRuntime::Context> context = nullptr;
};

struct SetNumberMarkInfoAsyncContext : BaseContext {
    std::string phoneNumber = "";
    NumberMarkInfo markInfo;
    std::shared_ptr<AbilityRuntime::Context> context = nullptr;
};

} // namespace Telephony
} // namespace OHOS

#endif // NAPI_CALL_MANAGER_TYPES_H
