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

#ifndef CODE_NUMBER_LOCATION_PARSER_H
#define CODE_NUMBER_LOCATION_PARSER_H
#include "number_identity_json_helper.h"
#include <fstream>
#include <iostream>
#include <istream>
#include <string>
#include <map>

namespace OHOS {
namespace Telephony {
using std::istream;
using std::string;
using std::map;

class NumberLocationRecord {
  public:
    string prefix;      // Phone number prefix (3 or 7 digits)
    string province;    // Province name
    string city;        // City name
    string operator_;   // Carrier operator
    // Compatible with old format: use location field if province and city are empty
    string location;   // Location (compatibility field, prefer province+city)
};

class NumberLocationDataSet {
  public:
    string version;
    map<string, NumberLocationRecord> records;  // key is prefix, value is record
};

class NumberLocationParser {
  public:
    bool Parse(istream &is, NumberLocationDataSet &result);
    bool ParseVersion(const string &line, string &result);
    bool ParseRecord(const string &line, NumberLocationRecord &result);

  private:
    bool ParseJSON(const string &text, JSONValueType &result);
};

} // namespace Telephony
} // namespace OHOS

#endif /* CODE_NUMBER_LOCATION_PARSER_H */

