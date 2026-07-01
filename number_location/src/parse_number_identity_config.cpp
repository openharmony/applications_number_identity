/*
 * Copyright (C) 2024 Huawei Device Co., Ltd. rights reserved.
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

#include "parse_number_identity_config.h"
#include "number_identity_log_wrapper.h"
 
namespace OHOS::Telephony {
const std::string NUMBER_IDENTITY_CONFIG_FILE = "/system/etc/telephony/number_identity_config.json";
const std::string NUMBER_LOCATION_URL = "number_location_url";
const std::string YELLOWPAGE_URL = "yellow_page_url";
const std::string CURRENT_TIME_STAMP = "current_time_stamp";
 
ParseStringConfig& ParseStringConfig::GetInstance()
{
    static ParseStringConfig config;
    return config;
}

ParseStringConfig::ParseStringConfig()
{
    NUMBER_IDENTITY_LOGI("construct ParseStringConfig.");
}

ParseStringConfig::~ParseStringConfig()
{
    NUMBER_IDENTITY_LOGI("destruct ParseStringConfig.");
}

void ParseStringConfig::LoadConfig()
{
    NUMBER_IDENTITY_LOGI("LoadConfig.");
    std::filesystem::path path(NUMBER_IDENTITY_CONFIG_FILE);
    auto absPath = std::filesystem::absolute(path);
    auto valMap = GetJsonFromPath(absPath.string());
    if (valMap.empty()) {
        NUMBER_IDENTITY_LOGE("LoadConfig empty");
        return;
    }
    if (valMap.count(NUMBER_LOCATION_URL)) {
        numberLocationUrl_ = valMap[NUMBER_LOCATION_URL];
    }
    if (valMap.count(YELLOWPAGE_URL)) {
        yellowPageUrl_ = valMap[YELLOWPAGE_URL];
    }
    if (valMap.count(CURRENT_TIME_STAMP)) {
        currentTimeStamp_ = valMap[CURRENT_TIME_STAMP];
    }
}
 
std::string ParseStringConfig::GetNumberLocationUrl()
{
    return numberLocationUrl_;
}
 
std::string ParseStringConfig::GetYellowPageUrl()
{
    return yellowPageUrl_;
}

std::string ParseStringConfig::GetCurrentTimeStamp()
{
    return currentTimeStamp_;
}
 
std::map<std::string, std::string> ParseStringConfig::GetJsonFromPath(const std::string &path)
{
    std::map<std::string, std::string> jsonValueMap;
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) {
        NUMBER_IDENTITY_LOGE("file not exists!");
        return jsonValueMap;
    }
    std::string fileData = ReadFileIntoString(path);
    if (fileData.empty()) {
        NUMBER_IDENTITY_LOGE("file is empty!");
        return jsonValueMap;
    }
    cJSON *root = cJSON_Parse(fileData.c_str());
    if (root != nullptr) {
        int arraySize = cJSON_GetArraySize(root);
        NUMBER_IDENTITY_LOGI("arraySize: %{public}d", arraySize);
        for (auto i = 0; i < arraySize; i++) {
            cJSON *childNode = cJSON_GetArrayItem(root, i);
            if (childNode == nullptr || childNode->type != cJSON_String) {
                continue;
            }
            auto key = childNode->string;
            auto val = childNode->valuestring;
            jsonValueMap.emplace(key, val);
        }
    }
    cJSON_Delete(root);
    return jsonValueMap;
}
 
std::string ParseStringConfig::ReadFileIntoString(const std::string &path)
{
    std::string content;
    OHOS::LoadStringFromFile(path, content);
    return content;
}
}