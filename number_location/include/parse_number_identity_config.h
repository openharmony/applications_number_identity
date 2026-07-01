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

#ifndef PARSE_NUMBER_IDENTITY_CONFIG_H
#define PARSE_NUMBER_IDENTITY_CONFIG_H
#include <map>
#include "file_ex.h"
#include <filesystem>
#include <cJSON.h>
namespace OHOS {
namespace Telephony {
class ParseStringConfig {
public:
    static ParseStringConfig& GetInstance();
    void LoadConfig();
    std::string GetNumberLocationUrl();
    std::string GetYellowPageUrl();
    std::string GetCurrentTimeStamp();
private:
    ParseStringConfig();
    ~ParseStringConfig();
    std::map<std::string, std::string> GetJsonFromPath(const std::string &path);
    std::string ReadFileIntoString(const std::string &path);
    std::string numberLocationUrl_ = "";
    std::string yellowPageUrl_ = "";
    std::string currentTimeStamp_ = "";
};
} // Telephony
} // OHOS
#endif // PARSE_NUMBER_IDENTITY_CONFIG_H