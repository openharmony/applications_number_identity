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

#ifndef NUMBER_IDENTITY_RESULT_SET_BRIDGE_TEMPLATE_H
#define NUMBER_IDENTITY_RESULT_SET_BRIDGE_TEMPLATE_H

#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"
#include "number_identity_utils.h"

#include "result_set_bridge.h"
#include "types.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {
using DataShare::ResultSetBridge;
using Writer = DataShare::ResultSetBridge::Writer;
using std::function;
using std::string;
using std::vector;

template <typename T> class NumberIdentityResultSetBridge : public ResultSetBridge {
  public:
    using WriteFunc = function<int(Writer &, const T &, uint32_t)>;
    struct ColumnDefinition {
        string name;
        WriteFunc write;
    };
    NumberIdentityResultSetBridge(const vector<T> &data);
    ~NumberIdentityResultSetBridge() = default;
    int GetRowCount(int32_t &count) override;
    int GetAllColumnNames(vector<string> &columnNames) override;
    int OnGo(int32_t startRowIndex, int32_t targetRowIndex, Writer &writer) override;
    int FillBlocks(int32_t startRowIndex, int32_t targetRowIndex, Writer &writer);
    vector<ColumnDefinition> columns;

  private:
    /**
     * Should be a copy of data, not reference.
     */
    vector<T> data_;
};

inline int WriteString(Writer &writer, uint32_t columnIndex, const string &value)
{
    DistributedKv::Value insertValue(value);
    auto converted = insertValue.ToString();
    auto chars = converted.c_str();
    auto size = insertValue.Size() + 1;
    auto errCode = writer.Write(columnIndex, chars, size);
    LOG_IF_ERR(errCode, "WriteString");
    return errCode;
}

inline int WriteOptionalString(Writer &writer, uint32_t columnIndex, const optional<string> &opt)
{
    return opt.has_value() ? WriteString(writer, columnIndex, opt.value()) : ERR_OK;
}

inline int WriteInt64(Writer &writer, uint32_t columnIndex, int64_t value)
{
    auto errCode = writer.Write(columnIndex, value);
    LOG_IF_ERR(errCode, "WriteInt64");
    return errCode;
}

template <typename T> static inline int WriteOptionalInt64(Writer &writer, uint32_t columnIndex, const optional<T> &opt)
{
    return opt.has_value() ? WriteInt64(writer, columnIndex, static_cast<int64_t>(opt.value())) : ERR_OK;
}

inline int WriteDouble64(Writer &writer, uint32_t columnIndex, double_t value)
{
    auto errCode = writer.Write(columnIndex, value);
    LOG_IF_ERR(errCode, "WriteInt64");
    return errCode;
}

inline int WriteBlob(Writer &writer, uint32_t columnIndex, const vector<uint8_t> &value)
{
    auto errCode = writer.Write(columnIndex, value.data(), value.size());
    LOG_IF_ERR(errCode, "WriteInt64");
    return errCode;
}

template <typename T>
NumberIdentityResultSetBridge<T>::NumberIdentityResultSetBridge(const vector<T> &data) : ResultSetBridge(), data_(data)
{
    NUMBER_IDENTITY_LOGD("Create NumberIdentityResultSetBridge");
}

template <typename T> int NumberIdentityResultSetBridge<T>::GetRowCount(int32_t &count)
{
    count = static_cast<int32_t>(this->data_.size());
    return ERR_OK;
}

template <typename T> int NumberIdentityResultSetBridge<T>::GetAllColumnNames(vector<string> &columnNames)
{
    MapVector<ColumnDefinition, string>(columns, columnNames, [](const ColumnDefinition &def) { return def.name; });
    return ERR_OK;
}

template <typename T>
int NumberIdentityResultSetBridge<T>::OnGo(int32_t startRowIndex, int32_t targetRowIndex, Writer &writer)
{
    if ((startRowIndex < 0) || (targetRowIndex < 0) || (startRowIndex > targetRowIndex) ||
        (targetRowIndex >= static_cast<int32_t>(data_.size()))) {
        // When failed to move, we should return -1.
        return -1;
    }
    // When called DataShareResultSet.GoToRow(index),
    // DataShareResultSet will try to call `OnGo(index, rowCount - 1, writer)`.
    // If we decide not to fill all the data at one `move`,
    // `OnGo` should return the final row index of actual filled blocks.
    // Here we fill all the data in range [index, target].
    return FillBlocks(startRowIndex, targetRowIndex, writer);
}

template <typename T>
int NumberIdentityResultSetBridge<T>::FillBlocks(int32_t startRowIndex, int32_t targetRowIndex, Writer &writer)
{
    int allocatedRows = 0;
    for (int32_t rowIndex = startRowIndex; rowIndex <= targetRowIndex; ++rowIndex) {
        auto errCode = writer.AllocRow();
        HANDLE_ERR("AllocRow", errCode, return rowIndex - 1);
        ++allocatedRows;
        auto &markInfo = data_[rowIndex];
        for (uint32_t columnIndex = 0, size = columns.size(); columnIndex < size; ++columnIndex) {
            const auto &write = columns.at(columnIndex).write;
            errCode = write(writer, markInfo, columnIndex);
            HANDLE_ERR("Write", errCode, return rowIndex);
        }
    }
    return targetRowIndex;
}

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_RESULT_SET_BRIDGE_TEMPLATE_H */