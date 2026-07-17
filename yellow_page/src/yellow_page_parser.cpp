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

#include "yellow_page_parser.h"
#include "cJSON.h"
#include "errors.h"
#include "number_identity_json_helper.h"
#include "number_identity_log_wrapper.h"

#include <regex>

namespace OHOS {
namespace Telephony {

bool YellowPageParser::ParseJSON(const string &text, JSONValueType &result)
{
    result = cJSON_Parse(text.c_str());
    return result != nullptr;
}

bool YellowPageParser::ParseVersion(const string &line, string &result)
{
    NUMBER_IDENTITY_LOGD("ParseVersion begin.");
    JSONValueType json;
    JSONValueType cursor;
    bool isOk = true;
    if (!this->ParseJSON(line, json)) {
        NUMBER_IDENTITY_LOGE("Invalid JSON format.");
        isOk = false;
        goto finally;
    }
    JSON_ASSERT_TYPE(json, Object, isOk, goto finally, "Expect root to be object.");
    JSON_GET_REQUIRED_MEMBER_VALUE(json, "version", cursor, String, result, isOk, goto finally);
finally:
    cJSON_Delete(json);
    return isOk;
}

bool YellowPageParser::ParseRecord(const string &line, YellowPageRecord &result)
{
    JSONValueType json;
    JSONValueType cursor;
    bool isOk = true;
    bool hasSpecialExtraFields = false;
    if (!this->ParseJSON(line, json)) {
        NUMBER_IDENTITY_LOGI("Invalid JSON format. Trying to fix with regex");
        std::regex pattern(R"(,(\w+)=)");
        string replaced = std::regex_replace(line, pattern, R"(,"$1":)");
        if (!this->ParseJSON(replaced, json)) {
            NUMBER_IDENTITY_LOGE("Failed to fix invalid JSON with regex.");
            goto finally;
        }
        hasSpecialExtraFields = true;
    }
    result.rawData = line;
    JSON_ASSERT_TYPE(json, Object, isOk, goto finally, "Expect JSON root to be type of Object.");
    JSON_GET_REQUIRED_MEMBER_VALUE(json, "group", cursor, String, result.group, isOk, goto finally);
    JSON_GET_REQUIRED_MEMBER_VALUE(json, "name", cursor, String, result.name, isOk, goto finally);
    JSON_GET_OPTIONAL_MEMBER_VALUE(json, "photo", cursor, String, result.photo, isOk, goto finally);
    JSONValueType phone;
    JSON_GET_MEMBER(json, "phone", phone, isOk, goto finally, "Failed to get #root['phone'].");
    JSON_ASSERT_TYPE(phone, Array, isOk, goto finally, "Expect #root['phone'] to be type of Array.");
    JSONValueType item;
    JSON_ARRAY_FOREACH(item, phone)
    {
        YellowPagePhone phone;
        JSON_GET_REQUIRED_MEMBER_VALUE(item, "dial_map", cursor, String, phone.dial_map, isOk, goto finally);
        JSON_GET_REQUIRED_MEMBER_VALUE(item, "hot_points", cursor, Number, phone.hot_points, isOk, goto finally);
        JSON_GET_REQUIRED_MEMBER_VALUE(item, "name", cursor, String, phone.name, isOk, goto finally);
        JSON_GET_REQUIRED_MEMBER_VALUE(item, "phone", cursor, String, phone.phone, isOk, goto finally);
        JSON_GET_OPTIONAL_MEMBER_VALUE(item, "pinyin", cursor, String, phone.pinyin, isOk, goto finally);
        JSON_GET_OPTIONAL_MEMBER_VALUE(item, "match_pattern", cursor, String, phone.match_pattern, isOk, goto finally);
        JSON_GET_OPTIONAL_MEMBER_VALUE(item, "device_type", cursor, Number, phone.device_type, isOk, goto finally);
        JSON_GET_OPTIONAL_MEMBER_VALUE(item, "alias_name", cursor, String, phone.alias_name, isOk, goto finally);
        result.phone.push_back(phone);
    }
finally:
    cJSON_Delete(json);
    return isOk;
}

bool YellowPageParser::Parse(istream &is, YellowPageDataSet &result)
{
    NUMBER_IDENTITY_LOGD("YellowPage::Parse begin.");
    string line;
    getline(is, line);

    if (!this->ParseVersion(line, result.version)) {
        NUMBER_IDENTITY_LOGE("YellowPage version Parse failed. first line is %{public}s", line.c_str());
        return false;
    }
    size_t lineNo = 1;
    while (!is.eof()) {
        getline(is, line);
        ++lineNo;
        YellowPageRecord record;
        if (this->ParseRecord(line, record)) {
            result.records.emplace_back(record);
        } else {
            NUMBER_IDENTITY_LOGE("ParseRecord failed at line %zu.", lineNo);
        }
    }
    NUMBER_IDENTITY_LOGI("YellowPage::parse Done.");
    return true;
}
} // namespace Telephony
} // namespace OHOS