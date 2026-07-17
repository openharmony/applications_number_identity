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

#ifndef NUMBER_IDENTITY_DDL_H
#define NUMBER_IDENTITY_DDL_H

#include <string>

namespace OHOS {
namespace Telephony {

#pragma region tables
constexpr const char *CREATE_PROPERTIES = R"(
CREATE TABLE IF NOT EXISTS [properties] (
    [property_key] TEXT PRIMARY KEY,
    [property_value] TEXT
);
)";

constexpr const char *CREATE_YELLOW_PAGE = R"(
CREATE TABLE IF NOT EXISTS [yellow_page](
    [_ID] INTEGER PRIMARY KEY AUTOINCREMENT,
    [name] TEXT,
    [group_name] TEXT,
    [data] TEXT,
    [photo] TEXT
);
)";
constexpr const char *CREATE_YELLOW_PAGE_PHONE = R"(
CREATE TABLE IF NOT EXISTS [yellow_page_phone] (
    [_ID] INTEGER PRIMARY KEY AUTOINCREMENT,
    [name] TEXT,
    [number] TEXT,
    [hot_points] NUMERIC,
    [dial_map] TEXT,
    [ypid] INTEGER,
    [pinyin] TEXT,
    [pinyin_abbreviation] TEXT,
    [pinyin_sub_string] TEXT,
    [match_pattern] TEXT,
    [device_type] INTEGER,
    [alias_name] TEXT,
    FOREIGN KEY (ypid) REFERENCES yellow_page (_ID) ON DELETE CASCADE
);
)";

constexpr const char *CREATE_YELLOW_PAGE_VIEW = R"(
CREATE VIEW IF NOT EXISTS [yellow_page_view] AS SELECT
    [yellow_page_phone].[_ID],
    [yellow_page].[photo],
    [yellow_page_phone].[name],
    [yellow_page_phone].[number],
    [yellow_page_phone].[hot_points],
    [yellow_page_phone].[dial_map],
    [yellow_page_phone].[ypid],
    [yellow_page_phone].[pinyin],
    [yellow_page_phone].[pinyin_abbreviation],
    [yellow_page_phone].[pinyin_sub_string],
    [yellow_page_phone].[match_pattern],
    [yellow_page_phone].[device_type],
    [yellow_page_phone].[alias_name]
FROM [yellow_page_phone], [yellow_page]
WHERE [yellow_page_phone].[ypid] = [yellow_page].[_ID];
)";

constexpr const char *CREATE_NUMBER_MARK = R"(
CREATE TABLE IF NOT EXISTS [number_mark] (
    [_ID] INTEGER PRIMARY KEY AUTOINCREMENT,
    [NUMBER] Text,
    [NAME] Text,
    [CLASSIFY] Text,
    [MARKED_COUNT] int DEFAULT 0,
    [IS_CLOUD] int default 0,
    [DESCRIPTION] Text,
    [SAVE_TIMESTAMP] Text,
    [SUPPLIER] Text,
    [SUPPLIER_ID] Text,
    [IS_INTELLIGENT_DB] int default 0
);
)";

constexpr const char *CREATE_NUMBER_MARK_EXTRAS = R"(
CREATE TABLE IF NOT EXISTS [number_mark_extras] (
  [_ID] INTEGER PRIMARY KEY AUTOINCREMENT,
  [NUMBER] TEXT,
  [TITLE] TEXT,
  [CONTENT] TEXT,
  [TYPE] TEXT,
  [ICON] TEXT,
  [INTERNAL_LINK] TEXT,
  [EXTERNAL_LINK] TEXT,
  [LONGITUDE] TEXT,
  [LATITUDE] TEXT,
  [TIMESTAMP] TEXT
);
)";

#pragma endregion

#pragma region columns

class NumberIdentityTables {
  public:
    static constexpr const char *PROPERTIES = "properties";
    static constexpr const char *YELLOW_PAGE = "yellow_page";
    static constexpr const char *YELLOW_PAGE_PHONE = "yellow_page_phone";
    static constexpr const char *YELLOW_PAGE_VIEW = "yellow_page_view";
    static constexpr const char *NUMBER_MARK = "number_mark";
    static constexpr const char *NUMBER_MARK_EXTRAS = "number_mark_extras";
};

class PropertiesColumns {
  public:
    static constexpr const char *PROPERTY_KEY = "property_key";
    static constexpr const char *PROPERTY_VALUE = "property_value";
};

class PropertyKeys {
  public:
    static constexpr const char *YELLOW_PAGE_VERSION = "yellow_page_version";
    static constexpr const char *CLOUD_QUERY_CALLEE_ID = "cloud_query_callee_id";
    static constexpr const char *CLOUD_QUERY_CALLEE_ID_TIMESTAMP = "cloud_query_callee_id_timestamp";
    static constexpr const char *NETWORK_TYPE = "network_type";
    static constexpr const char *NUMBER_LOCATION_VERSION_TIME_STAMP = "number_location_version_time_stamp";
    static constexpr const char *YELLOW_PAGE_VERSION_TIME_STAMP = "yellow_page_version_time_stamp";
    static constexpr const char *TIME_WORKER_START_TIME_SECOND = "time_worker_start_time_second";
    static constexpr const char *INTELLIGENT_DB_UPDATE_TIMESTAMP = "intelligent_db_update_timestamp";
    static constexpr const char *FORCE_UPDATED = "force_updated";
};

class PropertyValues {
  public:
    static constexpr const char *CLOSE_UPDATE = "close_update";
    static constexpr const char *WLAN_ONLY = "wlan_only";
    static constexpr const char *ALL_NETWORK = "all_network";
    static constexpr const char *WORK_SHEDULER = "work_sheduler";
};

class YellowPageColumns {
  public:
    static constexpr const char *ID = "_ID";
    static constexpr const char *NAME = "name";
    static constexpr const char *GROUP_NAME = "group_name";
    static constexpr const char *DATA = "data";
    static constexpr const char *PHOTO = "photo";
};

class YellowPagePhoneColumns {
  public:
    static constexpr const char *ID = "_ID";
    static constexpr const char *NAME = "name";
    static constexpr const char *NUMBER = "number";
    static constexpr const char *HOT_POINTS = "hot_points";
    static constexpr const char *DIAL_MAP = "dial_map";
    static constexpr const char *YELLOW_PAGE_ID = "ypid";
    static constexpr const char *PINYIN = "pinyin";
    static constexpr const char *PINYIN_ABBREVIATION = "pinyin_abbreviation";
    static constexpr const char *PINYIN_SUB_STRING = "pinyin_sub_string";
    static constexpr const char *MATCH_PATTERN = "match_pattern";
    static constexpr const char *DEVICE_TYPE = "device_type";
    static constexpr const char *ALIAS_NAME = "alias_name";
};

class YellowPageViewColumns {
  public:
    static constexpr const char *ID = "_ID";
    static constexpr const char *PHOTO = "photo";
    static constexpr const char *NAME = "name";
    static constexpr const char *NUMBER = "number";
    static constexpr const char *HOT_POINTS = "hot_points";
    static constexpr const char *DIAL_MAP = "dial_map";
    static constexpr const char *YELLOW_PAGE_ID = "ypid";
    static constexpr const char *PINYIN = "pinyin";
    static constexpr const char *PINYIN_ABBREVIATION = "pinyin_abbreviation";
    static constexpr const char *PINYIN_SUB_STRING = "pinyin_sub_string";
    static constexpr const char *MATCH_PATTERN = "match_pattern";
    static constexpr const char *DEVICE_TYPE = "device_type";
    static constexpr const char *ALIAS_NAME = "alias_name";
};

class NumberMarkColumns {
  public:
    static constexpr const char *ID = "_ID";
    static constexpr const char *NUMBER = "NUMBER";
    static constexpr const char *NAME = "NAME";
    static constexpr const char *CLASSIFY = "CLASSIFY";
    static constexpr const char *MARKED_COUNT = "MARKED_COUNT";
    static constexpr const char *IS_CLOUD = "IS_CLOUD";
    static constexpr const char *DESCRIPTION = "DESCRIPTION";
    static constexpr const char *SAVE_TIMESTAMP = "SAVE_TIMESTAMP";
    static constexpr const char *SUPPLIER = "SUPPLIER";
    static constexpr const char *SUPPLIER_ID = "SUPPLIER_ID";
    static constexpr const char *IS_INTELLIGENT_DB = "IS_INTELLIGENT_DB";
};

class NumberMarkExtrasColumns {
  public:
    static constexpr const char *ID = "_ID";
    static constexpr const char *NUMBER = "NUMBER";
    static constexpr const char *TITLE = "TITLE";
    static constexpr const char *CONTENT = "CONTENT";
    static constexpr const char *TYPE = "TYPE";
    static constexpr const char *ICON = "ICON";
    static constexpr const char *INTERNAL_LINK = "INTERNAL_LINK";
    static constexpr const char *EXTERNAL_LINK = "EXTERNAL_LINK";
    static constexpr const char *LONGITUDE = "LONGITUDE";
    static constexpr const char *LATITUDE = "LATITUDE";
    static constexpr const char *TIMESTAMP = "TIMESTAMP";
};

#pragma endregion

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_DDL_H */
