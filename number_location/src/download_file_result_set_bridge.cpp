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

#include "download_file_result_set_bridge.h"
#include "number_identity_log_wrapper.h"
#include "rdb_errno.h"
#include "rdb_store_config.h"
#include "rdb_utils.h"

namespace OHOS {
namespace NativeRdb {
using namespace DataShare;
using namespace DistributedKv;
using namespace Telephony;

const int32_t FIRST_ROW_COUNT = 1;
const int32_t FIRST_COLUMN = 0;
const int32_t SECOND_COLUMN = 1;
const int32_t INVALID_POSITION = -1;

DownloadFileResultSetBridge::DownloadFileResultSetBridge(std::vector<std::string> propertyKey,
    std::vector<std::string> propertyValue, uint32_t size) : propertyKey_(propertyKey),
    propertyValue_(propertyValue), count_(size) {}

int DownloadFileResultSetBridge::GetRowCount(int32_t &count)
{
    count = static_cast<int32_t>(count_);
    return NativeRdb::E_OK;
}

int DownloadFileResultSetBridge::GetAllColumnNames(std::vector<std::string> &columnsName)
{
    columnsName = { "property_key", "property_value" };
    return NativeRdb::E_OK;
}

bool DownloadFileResultSetBridge::FillBlock(int32_t target, ResultSetBridge::Writer &writer)
{
    DistributedKv::Key key(propertyKey_[target]);
    DistributedKv::Value value(propertyValue_[target]);
    int statusAlloc = writer.AllocRow();
    if (statusAlloc != NativeRdb::E_OK) {
        return false;
    }
    int keyStatus = writer.Write(FIRST_COLUMN, key.ToString().c_str(), key.Size() + FIRST_ROW_COUNT);
    if (keyStatus != NativeRdb::E_OK) {
        return false;
    }
    int valueStatus = writer.Write(SECOND_COLUMN, value.ToString().c_str(), value.Size() + FIRST_ROW_COUNT);
    if (valueStatus != NativeRdb::E_OK) {
        return false;
    }
    return true;
}

int DownloadFileResultSetBridge::OnGo(int32_t start, int32_t target, ResultSetBridge::Writer &writer)
{
    if ((start < 0) || (target < 0) || (start > target)) {
        return INVALID_POSITION;
    }
    return FillBlock(target, writer);
}
} // namespace Media
} // namespace OHOS