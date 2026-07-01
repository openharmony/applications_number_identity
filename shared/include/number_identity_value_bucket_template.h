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

#ifndef NUMBER_IDENTITY_VALUE_BUCKET_TEMPLATE_H
#define NUMBER_IDENTITY_VALUE_BUCKET_TEMPLATE_H

#include "datashare_values_bucket.h"
#include "number_identity_log_wrapper.h"
#include <optional>
#include <string>
#include <variant>

namespace OHOS {
namespace Telephony {
using DataShare::DataShareValuesBucket;
using std::optional;
using std::string;

template <typename T>
bool GetFromValueBucket(const DataShareValuesBucket &values, const string &columnName, T &target)
{
    bool isValid;
    auto valueObject = values.Get(columnName, isValid);
    if (!isValid) {
        NUMBER_IDENTITY_LOGE("No field %{public}s in value bucket.", columnName.c_str());
        return false;
    }
    T *ptr = std::get_if<T>(&valueObject.value);
    if (ptr == nullptr) {
        NUMBER_IDENTITY_LOGE("Field %{public}s is not of expected type.", columnName.c_str());
        return false;
    }
    target = *ptr;
    return true;
}

template<typename T>
inline void PutOptional(DataShareValuesBucket &values, const string &columnName, const optional<T> &opt)
{
    if (opt.has_value()) {
        values.Put(columnName, *opt);
    } else {
        values.Put(columnName); // Set column to NULL
    }
}

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_VALUE_BUCKET_TEMPLATE_H */