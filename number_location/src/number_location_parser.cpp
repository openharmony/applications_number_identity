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

#include "number_location_parser.h"
#include "cJSON.h"
#include "errors.h"
#include "number_identity_json_helper.h"
#include "number_identity_log_wrapper.h"

namespace OHOS {
namespace Telephony {

bool NumberLocationParser::ParseJSON(const string &text, JSONValueType &result)
{
    result = cJSON_Parse(text.c_str());
    return result != nullptr;
}

bool NumberLocationParser::ParseVersion(const string &line, string &result)
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

bool NumberLocationParser::ParseRecord(const string &line, NumberLocationRecord &result)
{
    JSONValueType json = nullptr;
    JSONValueType cursor;
    bool isOk = true;
    if (!this->ParseJSON(line, json)) {
        NUMBER_IDENTITY_LOGE("Invalid JSON format.");
        isOk = false;
        goto finally;
    }
    JSON_ASSERT_TYPE(json, Object, isOk, goto finally, "Expect JSON root to be type of Object.");
    JSON_GET_REQUIRED_MEMBER_VALUE(json, "prefix", cursor, String, result.prefix, isOk, goto finally);
    JSON_GET_REQUIRED_MEMBER_VALUE(json, "operator", cursor, String, result.operator_, isOk, goto finally);
    
    JSON_GET_OPTIONAL_MEMBER_VALUE(json, "province", cursor, String, result.province, isOk, goto finally);
    JSON_GET_OPTIONAL_MEMBER_VALUE(json, "city", cursor, String, result.city, isOk, goto finally);
    
    JSON_GET_OPTIONAL_MEMBER_VALUE(json, "location", cursor, String, result.location, isOk, goto finally);
    
    if (result.province.empty() && result.city.empty() && !result.location.empty()) {
        size_t spacePos = result.location.find(' ');
        if (spacePos != string::npos) {
            result.province = result.location.substr(0, spacePos);
            result.city = result.location.substr(spacePos + 1);
        } else {
            result.province = result.location;
            result.city = result.location;
        }
    }
finally:
    cJSON_Delete(json);
    return isOk;
}

bool NumberLocationParser::Parse(istream &is, NumberLocationDataSet &result)
{
    NUMBER_IDENTITY_LOGD("NumberLocation::Parse begin.");
    string line;
    getline(is, line);

    if (!this->ParseVersion(line, result.version)) {
        NUMBER_IDENTITY_LOGE("NumberLocation version Parse failed. first line is %{public}s", line.c_str());
        return false;
    }
    NUMBER_IDENTITY_LOGI("NumberLocation::Parse version: %{public}s", result.version.c_str());
    
    size_t lineNo = 1;
    size_t successCount = 0;
    size_t failCount = 0;
    map<string, size_t> duplicatePrefixes;  // Track duplicate prefixes
    
    while (!is.eof()) {
        getline(is, line);
        ++lineNo;
        if (line.empty()) {
            continue;
        }
        NumberLocationRecord record;
        if (this->ParseRecord(line, record)) {
            // Check for duplicate prefix
            if (result.records.find(record.prefix) != result.records.end()) {
                duplicatePrefixes[record.prefix]++;
                  
            }
            result.records[record.prefix] = record;
            successCount++;
        } else {
            NUMBER_IDENTITY_LOGE("ParseRecord failed at line %zu: %{public}s", lineNo, line.c_str());
            failCount++;
        }
    }
    
    NUMBER_IDENTITY_LOGI("NumberLocation::Parse Done. Total lines: %zu, Success: %zu, Failed: %zu, Unique prefixes: %zu",
                          lineNo, successCount, failCount, result.records.size());
    
    if (!duplicatePrefixes.empty()) {
        NUMBER_IDENTITY_LOGW("NumberLocation::Parse: found %zu duplicate prefixes (later entries overwrite earlier ones):",
                              duplicatePrefixes.size());
        for (const auto &pair : duplicatePrefixes) {
            NUMBER_IDENTITY_LOGW("  prefix %{public}s appears %zu times", pair.first.c_str(), pair.second + 1);
        }
    }
    
    return true;
}

} // namespace Telephony
} // namespace OHOS

