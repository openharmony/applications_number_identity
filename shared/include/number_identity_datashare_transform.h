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

#ifndef NUMBER_IDENTITY_DATASHARE_TRANSFORM_H
#define NUMBER_IDENTITY_DATASHARE_TRANSFORM_H

#include "number_identity_utils.h"

#include "datashare_predicates.h"
#include "datashare_predicates_def.h"
#include "datashare_result_set.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace OHOS {
namespace Telephony {
using DataShare::DataSharePredicates;
using DataShare::DataShareResultSet;
using DataShare::OperationItem;
using std::function;
using std::map;
using std::monostate;
using std::nullopt;
using std::optional;
using std::shared_ptr;
using std::string;
using std::variant;
using std::vector;

using OperationTransform = function<vector<OperationItem>(const OperationItem &)>;
using StringTransform = function<string(const string &)>;
/**
 * Query wrapper function.
 * The proxy only makes proxy on the predicates and the result set,
 * so any other relevant query parameter should be wrapped in the query function,
 * and predicates is the only input and result set is the only output.
 */
using NativeQuery = function<shared_ptr<DataShareResultSet>(const DataSharePredicates &)>;
/**
 * The union of native result set data types.
 * Relation of OHOS::DataShare::DataType enum:
 * TYPE_NULL -> std::monostate
 * TYPE_INTEGER -> int64_t
 * TYPE_FLOAT -> double_t
 * TYPE_STRING -> std::string
 * TYPE_BLOB -> std::vector<uint8_t>
 */
using NativeData = variant<monostate, int64_t, double_t, string, vector<uint8_t>>;
/**
 * The representation of record of native result set.
 * A native record is a row of `NativeData`.
 */
using NativeRecord = vector<NativeData>;
struct NativeDataSet {
  public:
    vector<string> columnNames;
    /**
     * All data rows of result set.
     */
    vector<NativeRecord> records;
};
struct StringFieldProxy {
  public:
    /**
     * The mapping maps original value to new value. By default, its an identity mapping.
     */
    using Mapping = map<string, string>;
    using TransformMapping = optional<Mapping>;
    string field;
    StringTransform transform;
    NativeQuery query;
    shared_ptr<DataShareResultSet> ProxyQuery(const DataSharePredicates &predicates);

  private:
    vector<OperationItem> TransformEqualTo(const OperationItem &item, TransformMapping &mapping);
    vector<OperationItem> TransformSqlIn(const OperationItem &item, TransformMapping &mapping);
    shared_ptr<DataShareResultSet> MapResult(DataShareResultSet &resultSet, const Mapping &replaceMapping);
};

template <typename T> optional<T> GetEqualToFieldValue(const string &field, const OperationItem &item)
{
    auto it = item.singleParams.cbegin();
    BOOL_CHECK(it != item.singleParams.cend(), return nullopt);
    auto fieldPtr = std::get_if<string>(&*it);
    FAIL_IF_NULL(fieldPtr, return nullopt);
    BOOL_CHECK(*fieldPtr == field, return nullopt);
    ++it;
    BOOL_CHECK(it != item.singleParams.cend(), return nullopt);
    auto valuePtr = std::get_if<T>(&*it);
    FAIL_IF_NULL(valuePtr, return nullopt);
    return std::move(optional<T>(*valuePtr));
}

template <typename T> optional<vector<T>> GetSqlInFieldValues(const string &field, const OperationItem &item)
{
    auto fieldIt = item.singleParams.cbegin();
    BOOL_CHECK(fieldIt != item.singleParams.cend(), return nullopt);
    auto fieldPtr = std::get_if<string>(&*fieldIt);
    FAIL_IF_NULL(fieldPtr, return nullopt);
    BOOL_CHECK(*fieldPtr == field, return nullopt);
    auto it = item.multiParams.cbegin();
    BOOL_CHECK(it != item.multiParams.cend(), return nullopt);
    auto valueSetPtr = std::get_if<vector<T>>(&*it);
    FAIL_IF_NULL(valueSetPtr, return nullopt);
    return std::move(optional<vector<T>>(*valueSetPtr));
}

/**
 * Transform datashare predicates (query condition) with custom rules.
 */
DataSharePredicates TransformOperation(const DataSharePredicates &predicates, OperationTransform transform);

inline vector<OperationItem> NoTransform(const OperationItem &item)
{
    return { item };
}

int GetNativeData(DataShareResultSet &resultSet, int columnIndex, NativeData &data);

int ToNativeDataSet(DataShareResultSet &resultSet, NativeDataSet &dataSet);

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_DATASHARE_TRANSFORM_H */