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

#ifndef NUMBER_IDENTITY_CJSON_UTILS_H
#define NUMBER_IDENTITY_CJSON_UTILS_H
#include "cJSON.h"
#include "number_identity_log_wrapper.h"
#include "number_identity_utils.h"
#include <optional>
#include <string>

// General macro parameter design:
// isOk: variable name checkes success
// finally: label name to goto when operations cannot go forward
// json: variable name for a json value
// jsonType: type name for a json value
// out: assign target
// cursor: temporary variable name for a json value
// member: class member name, as well as key of json

#define FAIL_WITH_MESSAGE(isOk, finally, fmt, ...)                                                                     \
    do {                                                                                                               \
        NUMBER_IDENTITY_LOGE(fmt, ##__VA_ARGS__);                                                                      \
        (isOk) = false;                                                                                                \
        finally;                                                                                                       \
    } while (0)

#define JSON_ASSERT_TYPE(json, jsonType, isOk, finally, fmt, ...)                                                      \
    do {                                                                                                               \
        if (!cJSON_Is##jsonType(json)) {                                                                               \
            FAIL_WITH_MESSAGE(isOk, finally, "Expected JSON type '%{public}s', actual type: '%{public}s'. " fmt,       \
                #jsonType, cJSONHelper_GetJSONType(json), ##__VA_ARGS__);                                              \
        }                                                                                                              \
    } while (0)

#define JSON_GET_MEMBER(json, key, out, isOk, finally, fmt, ...)                                                       \
    do {                                                                                                               \
        (out) = cJSON_GetObjectItemCaseSensitive(json, key);                                                           \
        if ((out) == nullptr) {                                                                                        \
            FAIL_WITH_MESSAGE(isOk, finally, "JSON object missing member %{public}s. " fmt, key, ##__VA_ARGS__);       \
        }                                                                                                              \
    } while (0)

#define JSON_GET_OPTIONAL_MEMBER_VALUE(json, key, cursor, jsonType, out, isOk, finally)                                \
    do {                                                                                                               \
        (cursor) = cJSON_GetObjectItemCaseSensitive(json, key);                                                        \
        if (cJSON_Is##jsonType(cursor)) {                                                                              \
            (out) = cJSON_Get##jsonType##Value(cursor);                                                                \
        } else if ((cursor) != nullptr) {                                                                              \
            NUMBER_IDENTITY_LOGW("type of member '%{public}s' is not '%{public}s'", key, #jsonType);                   \
        }                                                                                                              \
    } while (0)

#define JSON_GET_REQUIRED_MEMBER_VALUE(json, key, cursor, jsonType, out, isOk, finally)                                \
    do {                                                                                                               \
        JSON_GET_MEMBER(json, key, cursor, isOk, finally, "Failed to get member value.");                              \
        JSON_ASSERT_TYPE(cursor, jsonType, isOk, finally, "Type of member %{public}s is invalid.", key);               \
        (out) = cJSON_Get##jsonType##Value(cursor);                                                                    \
    } while (0)

#define JSON_ARRAY_FOREACH(item, array) cJSON_ArrayForEach(item, array)

#define JSON_READ_OPTIONAL_MEMBER(json, member, jsonType, cursor, isOk, finally)                                       \
    JSON_GET_OPTIONAL_MEMBER_VALUE(json, #member, cursor, jsonType, member, isOk, finally)

#define JSON_READ_REQUIRED_MEMBER(json, member, jsonType, cursor, isOk, finally)                                       \
    JSON_GET_REQUIRED_MEMBER_VALUE(json, #member, cursor, jsonType, member, isOk, finally)

#define JSON_READ_BOOL_MEMBER(json, member, cursor, isOk, finally)                                                     \
    do {                                                                                                               \
        (cursor) = cJSON_GetObjectItemCaseSensitive(json, #member);                                                    \
        (member) = cJSON_IsTrue(cursor);                                                                               \
    } while (0)

#define JSON_WRITE_REQUIRED_STRING_MEMBER(json, member, cursor, finally)                                               \
    do {                                                                                                               \
        (cursor) = cJSON_AddStringToObject(json, #member, (member).c_str());                                           \
        if ((cursor) == nullptr) {                                                                                     \
            NUMBER_IDENTITY_LOGE("cJSON write string member %{public}s failed", #member);                              \
            finally;                                                                                                   \
        }                                                                                                              \
    } while (0)

#define JSON_WRITE_OPTIONAL_STRING_MEMBER(json, member, cursor, finally)                                               \
    do {                                                                                                               \
        if ((member).has_value()) {                                                                                    \
            (cursor) = cJSON_AddStringToObject(json, #member, (member)->c_str());                                      \
            if ((cursor) == nullptr) {                                                                                 \
                NUMBER_IDENTITY_LOGE("cJSON write string member %{public}s failed", #member);                          \
                finally;                                                                                               \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)

#define JSON_WRITE_REQUIRED_NUMBER_MEMBER(json, member, cursor, finally)                                               \
    do {                                                                                                               \
        cursor = cJSON_AddNumberToObject(json, #member, static_cast<double>(member));                                  \
        if ((cursor) == nullptr) {                                                                                     \
            NUMBER_IDENTITY_LOGE("cJSON write number member %{public}s failed", #member);                              \
            finally;                                                                                                   \
        }                                                                                                              \
    } while (0)

#define JSON_WRITE_OPTIONAL_NUMBER_MEMBER(json, member, cursor, finally)                                               \
    do {                                                                                                               \
        if ((member).has_value()) {                                                                                    \
            (cursor) = cJSON_AddNumberToObject(json, #member, static_cast<double>(*(member)));                         \
            if ((cursor) == nullptr) {                                                                                 \
                NUMBER_IDENTITY_LOGE("cJSON write number member %{public}s failed", #member);                          \
                finally;                                                                                               \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)

#define JSON_WRITE_MEMBER(json, member, jsonItem, finally)                                                             \
    do {                                                                                                               \
        if (!cJSON_AddItemToObject(json, #member, jsonItem)) {                                                         \
            NUMBER_IDENTITY_LOGE("Add member %{public}s failed.", #member);                                            \
            finally;                                                                                                   \
        }                                                                                                              \
    } while (0)

namespace OHOS {
namespace Telephony {
using JSONValueType = cJSON *;
using std::string;

bool cJSON_GetBoolValue(const cJSON *item);

const char *cJSONHelper_GetJSONType(JSONValueType json);

bool cJSONHelper_Stringify(JSONValueType json, string &result);

/**
 * Unwrap JSON string as string.
 */
bool cJSONHelper_UnwrapJSONString(const string &wrapped, string &result);

template <typename T> bool ToJSON(const T &entity, string &result)
{
    bool isOk = true;
    auto json = entity.ToJSONObject();
    if (json == nullptr) {
        goto finally;
    };
    isOk = cJSONHelper_Stringify(json, result);
finally:
    cJSON_Delete(json);
    return isOk;
}

template <typename T> bool FromJSON(const string &jsonString, T &entity)
{
    bool isOk = true;
    JSONValueType json = cJSON_Parse(jsonString.c_str());
    if (json == nullptr) {
        goto finally;
    }
    entity = T(); // Assign all members with default values.
    isOk = entity.FromJSONObject(json);
finally:
    cJSON_Delete(json);
    return isOk;
}

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_CJSON_UTILS_H */