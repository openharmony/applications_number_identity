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

#ifndef DOWNLOAD_FILE_RESULT_SET_BRIDGE_H
#define DOWNLOAD_FILE_RESULT_SET_BRIDGE_H

#include <condition_variable>
#include <mutex>

#include "kvstore_result_set.h"
#include "result_set_bridge.h"

namespace OHOS {
namespace NativeRdb {
class DownloadFileResultSetBridge : public DataShare::ResultSetBridge {
public:
    DownloadFileResultSetBridge(std::vector<std::string> propertyKey,
        std::vector<std::string> propertyValue, uint32_t size);
    ~DownloadFileResultSetBridge() = default;
    int GetRowCount(int32_t &count) override;
    int GetAllColumnNames(std::vector<std::string> &columnNames) override;
    int OnGo(int32_t startRowIndex, int32_t targetRowIndex, DataShare::ResultSetBridge::Writer &writer) override;
    bool FillBlock(int32_t target, DataShare::ResultSetBridge::Writer &Writer);

private:
    std::vector<std::string> propertyKey_ {};
    std::vector<std::string> propertyValue_ {};
    uint32_t count_ = 0;
};
} // namespace Media
} // namespace OHOS
#endif // DOWNLOAD_FILE_RESULT_SET_BRIDGE_H