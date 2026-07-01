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

#ifndef NUMBER_LOCATION_RESULT_SET_BRIDGE_H
#define NUMBER_LOCATION_RESULT_SET_BRIDGE_H

#include "number_identity_result_set_bridge_template.h"

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

#include "kvstore_result_set.h"
#include "result_set_bridge.h"

namespace OHOS {
namespace Telephony {

using std::string;
using std::vector;

class NumberLocationResultSetBridge : public NumberIdentityResultSetBridge<string> {
  public:
    explicit NumberLocationResultSetBridge(const vector<string> &data);
};
} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_LOCATION_RESULT_SET_BRIDGE_H */