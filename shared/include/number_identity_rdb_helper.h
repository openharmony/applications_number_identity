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

#ifndef NUMBER_IDENTITY_RDB_HELPER_H
#define NUMBER_IDENTITY_RDB_HELPER_H

#include "number_identity_log_wrapper.h"
#include "number_identity_utils.h"

#include "rdb_errno.h"

namespace OHOS {
namespace Telephony {

constexpr const int NO_ROW_AFFECTED = 0;

#define GET_COLUMN_INDEX_AND_CHECK_NULL(field, isNull, resultSet, columnIndex, errCode, finally)                       \
    do {                                                                                                               \
        (errCode) = (resultSet).GetColumnIndex((field), (columnIndex));                                                \
        HANDLE_ERR("GetColumnIndex(" #field ")", errCode, finally);                                                    \
        (errCode) = (resultSet).IsColumnNull(columnIndex, isNull);                                                     \
        HANDLE_ERR("IsColumnNull(" #field ")", errCode, finally);                                                      \
    } while (0)

#define FETCH_FIELD(field, type, target, isNull, resultSet, columnIndex, errCode, finally)                             \
    do {                                                                                                               \
        GET_COLUMN_INDEX_AND_CHECK_NULL(field, isNull, resultSet, columnIndex, errCode, finally);                      \
        if (!(isNull)) {                                                                                               \
            (errCode) = (resultSet).Get##type(columnIndex, target);                                                    \
            HANDLE_ERR("Get" #type, errCode, finally);                                                                 \
        }                                                                                                              \
    } while (0)

#define ASSIGN_IF_NOT_NULL(target, value, isNull)                                                                      \
    if (!(isNull)) {                                                                                                   \
        (target) = (value);                                                                                            \
    }

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_RDB_HELPER_H */