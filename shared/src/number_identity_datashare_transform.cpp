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

#include "number_identity_datashare_transform.h"
#include "number_identity_errors.h"
#include "number_identity_utils.h"
#include "number_mark_result_set_bridge.h"

#include "basic/result_set.h"
#include "datashare_errno.h"
#include "datashare_predicates.h"
#include "datashare_predicates_def.h"
#include "datashare_result_set.h"
#include "errors.h"
#include "result_set_bridge.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace OHOS {
namespace Telephony {
using namespace std;
using DataShare::E_OK;

DataSharePredicates TransformOperation(const DataSharePredicates &predicates, OperationTransform transform)
{
    vector<OperationItem> operationList;
    for (const auto &operation : predicates.GetOperationList()) {
        auto transformed = transform(operation);
        operationList.insert(operationList.end(), transformed.begin(), transformed.end());
    }
    DataSharePredicates result(operationList);
    result.SetSettingMode(predicates.GetSettingMode());
    result.SetWhereArgs(predicates.GetWhereArgs());
    result.SetWhereClause(predicates.GetWhereClause());
    result.SetOrder(predicates.GetOrder());
    return result;
}

int GetNativeData(DataShareResultSet &resultSet, int columnIndex, NativeData &data)
{
    int errCode = E_OK;
    DataShare::DataType dataType;
    errCode = resultSet.GetDataType(columnIndex, dataType);
    NUMBER_IDENTITY_LOGI("dataType %{public}d.", dataType);
    HANDLE_ERR("GetDataType", errCode, return errCode);
    switch (dataType) {
        case DataShare::DataType::TYPE_INTEGER:
            {
                int64_t value;
                errCode = resultSet.GetLong(columnIndex, value);
                HANDLE_ERR("GetLong", errCode, return errCode);
                data = value;
                break;
            }
        case DataShare::DataType::TYPE_FLOAT:
            {
                double_t value;
                errCode = resultSet.GetDouble(columnIndex, value);
                HANDLE_ERR("GetDouble", errCode, return errCode);
                data = value;
                break;
            }
        case DataShare::DataType::TYPE_STRING:
            {
                string value;
                errCode = resultSet.GetString(columnIndex, value);
                HANDLE_ERR("GetString", errCode, return errCode);
                data = value;
                break;
            }
        case DataShare::DataType::TYPE_BLOB:
            {
                vector<uint8_t> blob;
                errCode = resultSet.GetBlob(columnIndex, blob);
                HANDLE_ERR("GetBlob", errCode, return errCode);
                data = blob;
                break;
            }
        case DataShare::DataType::TYPE_NULL:
        default:
            data = monostate{};
            break;
    }
    return errCode;
}

int ToNativeDataSet(DataShareResultSet &resultSet, NativeDataSet &dataSet)
{
    int errCode = E_OK;
    int rowCount;
    errCode = resultSet.GetAllColumnNames(dataSet.columnNames);
    const auto &columnNames = dataSet.columnNames;
    HANDLE_ERR("GetAllColumnNames", errCode, return errCode);
    LOGD_EXPR(columnNames);
    vector<int> columnIndexes(columnNames.size());
    int columnIndex;
    for (size_t i = 0; i < columnNames.size(); ++i) {
        errCode = resultSet.GetColumnIndex(columnNames[i], columnIndex);
        HANDLE_ERR("GetColumnIndex", errCode, return errCode);
        columnIndexes[i] = columnIndex; // Assert the comlun index will not change.
    }
    errCode = resultSet.GetRowCount(rowCount);
    HANDLE_ERR("GetRowCount", errCode, return errCode);
    LOGD_EXPR(rowCount);
    dataSet.records = {};
    dataSet.records.resize(rowCount);
    for (int i = 0; i < rowCount; ++i) {
        auto &record = dataSet.records[i];
        record.resize(columnNames.size());
        errCode = resultSet.GoToRow(i);
        HANDLE_ERR("GoToRow", errCode, return errCode);
        for (size_t j = 0; j < columnNames.size(); ++j) {
            columnIndex = columnIndexes[j];
            NativeData data;
            errCode = GetNativeData(resultSet, columnIndex, data);
            HANDLE_ERR("GetNativeData", errCode, return errCode);
            record[columnIndex] = data;
        }
    }
    return errCode;
}

vector<OperationItem> StringFieldProxy::TransformEqualTo(const OperationItem &item, TransformMapping &transformMapping)
{
    NUMBER_IDENTITY_LOGI("TransformEqualTo begin.");
    auto it = item.singleParams.cbegin();
    BOOL_CHECK(it != item.singleParams.cend(), return NoTransform(item));
    auto field = std::get_if<string>(&*it);
    FAIL_IF_NULL(field, return NoTransform(item));
    if (*field != this->field) {
        NUMBER_IDENTITY_LOGI("Skipped field %{public}s.", field->c_str());
        return NoTransform(item);
    }
    ++it;
    BOOL_CHECK(it != item.singleParams.cend(), return NoTransform(item));
    auto value = std::get_if<string>(&*it);
    FAIL_IF_NULL(value, return NoTransform(item));
    auto transformed = this->transform(*value);
    if (transformed == *value) {
        NUMBER_IDENTITY_LOGI("No need to transform.");
        return NoTransform(item);
    }
    DataSharePredicates predicates;
    predicates.In(*field, vector<string>({ *value, transformed }));
    transformMapping = Mapping({ { *value, transformed } });
    return predicates.GetOperationList();
}

vector<OperationItem> StringFieldProxy::TransformSqlIn(const OperationItem &item, TransformMapping &trabsformMapping)
{
    NUMBER_IDENTITY_LOGI("TransformSqlIn begin.");
    auto fieldIt = item.singleParams.cbegin();
    if (fieldIt == item.singleParams.cend()) {
        NUMBER_IDENTITY_LOGI("no data.");
        return NoTransform(item);
    }
    auto field = std::get_if<string>(&*fieldIt);
    FAIL_IF_NULL(field, return NoTransform(item));
    if (*field != this->field) {
        NUMBER_IDENTITY_LOGI("Skipped field %{public}s.", field->c_str());
        return NoTransform(item);
    }
    auto valueSetIt = item.multiParams.cbegin();
    BOOL_CHECK(valueSetIt != item.multiParams.cend(), return NoTransform(item));
    auto valueSetPtr = std::get_if<vector<string>>(&*valueSetIt);
    FAIL_IF_NULL(valueSetPtr, return NoTransform(item));
    set<string> values;
    Mapping mapping;
    for (size_t i = 0; i < valueSetPtr->size(); ++i) {
        const auto &value = valueSetPtr->at(i);
        if (auto iter = mapping.find(value); iter != mapping.end()) {
            NUMBER_IDENTITY_LOGI("duplicated in condition for %{public}s, index = %{public}lu", field->c_str(), i);
        } else {
            auto transformed = this->transform(value);
            mapping[value] = transformed;
            values.insert(value); // SQL in with original value.
            if (transformed != value) {
                values.insert(transformed); // SQL in with transformed value.
            }
        }
    }
    vector<string> transformedValueSet;
    std::copy(values.begin(), values.end(), std::back_inserter(transformedValueSet));
    DataSharePredicates predicates;
    predicates.In(*field, transformedValueSet);
    trabsformMapping = mapping;
    return predicates.GetOperationList();
}

shared_ptr<DataShareResultSet> StringFieldProxy::MapResult(DataShareResultSet &resultSet, const Mapping &replaceMapping)
{
    NUMBER_IDENTITY_LOGI("MapResult begin.");
    int errCode = E_OK;
    vector<string> columnNames;
    errCode = resultSet.GetAllColumnNames(columnNames);
    HANDLE_ERR("GetAllColumnNames", errCode, return nullptr);
    auto iter = std::find(columnNames.cbegin(), columnNames.cend(), field);
    BOOL_CHECK(iter != columnNames.cend(), return nullptr);
    auto replaceIndex = iter - columnNames.cbegin();
    NativeDataSet dataSet;
    errCode = ToNativeDataSet(resultSet, dataSet);
    HANDLE_ERR("ToNativeDataSet", errCode, return nullptr);
    map<string, NativeRecord> recordMapping;
    for (const auto &record : dataSet.records) {
        auto stringData = &record[replaceIndex];
        if (auto ptr = std::get_if<string>(stringData); ptr != nullptr) {
            recordMapping[*ptr] = record;
        } else {
            NUMBER_IDENTITY_LOGE("Field type of %{public}s is not string!", field.c_str());
            return nullptr;
        }
    }
    NativeDataSet resultDataSet;
    resultDataSet.columnNames = columnNames;
    for (const auto &entry : replaceMapping) {
        const auto &[originValue, replaceValue] = entry;
        if (auto recordEntry = recordMapping.find(originValue); recordEntry != recordMapping.end()) {
            auto [_key, record] = *recordEntry; // record copy
            resultDataSet.records.emplace_back(record);
        } else if (auto recordEntry = recordMapping.find(replaceValue); recordEntry != recordMapping.end()) {
            auto [_key, record] = *recordEntry; // record copy
            record[replaceIndex] = originValue; // Replacement only takes effect when exact match is not present.
            resultDataSet.records.emplace_back(record);
        }
    }
    shared_ptr<ResultSetBridge> bridge = make_shared<NativeDataResultSetBridge>(resultDataSet);
    FAIL_IF_NULL(bridge, return nullptr);
    return make_shared<DataShareResultSet>(bridge);
}

shared_ptr<DataShareResultSet> StringFieldProxy::ProxyQuery(const DataSharePredicates &predicates)
{
    NUMBER_IDENTITY_LOGI("ProxyQuery begin.");
    TransformMapping mapping;
    DataSharePredicates transformedPredicates = TransformOperation(predicates, [&](const OperationItem &item) {
        NUMBER_IDENTITY_LOGI("transformedPredicates.");
        if (item.operation == DataShare::EQUAL_TO) {
            return this->TransformEqualTo(item, mapping);
        }
        if (item.operation == DataShare::SQL_IN) {
            return this->TransformSqlIn(item, mapping);
        }
        return NoTransform(item);
    });
    auto resultSet = this->query(transformedPredicates);
    FAIL_IF_NULL(resultSet, return nullptr);
    if (mapping.has_value()) {
        auto mapped = this->MapResult(*resultSet, *mapping);
        FAIL_IF_NULL(mapped, return resultSet);
        resultSet = mapped;
    }
    return resultSet;
}

} // namespace Telephony
} // namespace OHOS