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

#ifndef NUMBER_MARK_RESULT_SET_BRIDGE_H
#define NUMBER_MARK_RESULT_SET_BRIDGE_H

#include "number_identity_datashare_transform.h"
#include "number_identity_models.h"
#include "number_identity_result_set_bridge_template.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {
using std::string;
using std::vector;

class NumberMarkResultSetBridge : public NumberIdentityResultSetBridge<NumberMarkInfo> {
  public:
    explicit NumberMarkResultSetBridge(const vector<NumberMarkInfo> &data);
};

class NativeDataResultSetBridge : public NumberIdentityResultSetBridge<NativeRecord> {
  public:
    explicit NativeDataResultSetBridge(const NativeDataSet &dataSet);

  private:
    void DefineColumns(const vector<string> &dynamicColumns);
};

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_MARK_RESULT_SET_BRIDGE_H */