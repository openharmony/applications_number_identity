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

#include "number_identity_json_helper.h"
#include "cJSON.h"
#include "number_identity_utils.h"

namespace OHOS {
namespace Telephony {

bool cJSON_GetBoolValue(const cJSON *item)
{
    return cJSON_IsTrue(item);
}

const char *cJSONHelper_GetJSONType(JSONValueType json)
{
    // Check type in order of frequency.
    if (cJSON_IsString(json)) {
        return "String";
    }
    if (cJSON_IsNumber(json)) {
        return "Number";
    }
    if (cJSON_IsObject(json)) {
        return "Object";
    }
    if (cJSON_IsArray(json)) {
        return "Array";
    }
    if (cJSON_IsBool(json)) {
        return "Boolean";
    }
    if (cJSON_IsNull(json)) {
        return "Null";
    }
    return "Unknown";
}

bool cJSONHelper_Stringify(JSONValueType json, string &result)
{
    bool isOk = true;
    auto jsonStr = cJSON_PrintUnformatted(json);
    FAIL_IF_NULL(jsonStr, goto finally);
    result = jsonStr;
finally:
    cJSON_free(jsonStr);
    return isOk;
}

bool cJSONHelper_UnwrapJSONString(const string &wrapped, string &result)
{
    bool isOk = true;
    char *jsonStr = nullptr;
    auto json = cJSON_Parse(wrapped.c_str());
    FAIL_IF_NULL(json, goto finally);
    jsonStr = cJSON_GetStringValue(json);
    FAIL_IF_NULL(jsonStr, goto finally);
    result = jsonStr;
finally:
    cJSON_Delete(json);
    // No need to free `jsonStr`. When the json root is deleted, the string member is also deleted.
    return isOk;
}

} // namespace Telephony
} // namespace OHOS
