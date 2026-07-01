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

#ifndef CODE_YELLOW_PAGE_PARSER_H
#define CODE_YELLOW_PAGE_PARSER_H
#include "number_identity_json_helper.h"
#include <fstream>
#include <iostream>
#include <istream>
#include <optional>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {
using std::istream;
using std::optional;
using std::string;
using std::vector;

class YellowPagePhone {
  public:
    string name;
    string phone;
    optional<string> pinyin;
    int32_t hot_points;
    string dial_map;
    optional<string> match_pattern;
    optional<int32_t> device_type;
    optional<string> alias_name;
};

class YellowPageRecord {
  public:
    string rawData;
    string name;
    string group;
    optional<string> photo;
    vector<YellowPagePhone> phone;
};

class YellowPageDataSet {
  public:
    string version;
    vector<YellowPageRecord> records;
};

class YellowPageParser {
  public:
    bool Parse(istream &is, YellowPageDataSet &result);
    bool ParseVersion(const string &line, string &result);
    bool ParseRecord(const string &line, YellowPageRecord &result);

  private:
    bool ParseJSON(const string &text, JSONValueType &result);
};

} // namespace Telephony
} // namespace OHOS

#endif /* CODE_YELLOW_PAGE_PARSER_H */
