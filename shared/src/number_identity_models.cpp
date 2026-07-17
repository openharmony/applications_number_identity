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

#include "number_identity_models.h"
#include "number_identity_ddl.h"
#include "number_identity_rdb_helper.h"
#include "number_identity_value_bucket_template.h"
#include <initializer_list>
#include <map>
#include <optional>

namespace OHOS {
namespace Telephony {
using namespace NativeRdb;
using namespace std;
int YellowPageViewModel::CreateFromResultSet(ResultSet &resultSet)
{
    using Columns = YellowPageViewColumns;
    int errCode = E_OK;
    int columnIndex;
    bool isNull;
    int64_t longValue;
    string stringValue;
    FETCH_FIELD(Columns::ID, Long, longValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(id, longValue, isNull);
    FETCH_FIELD(Columns::NAME, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(name, stringValue, isNull);
    FETCH_FIELD(Columns::NUMBER, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(number, stringValue, isNull);
    FETCH_FIELD(Columns::PHOTO, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(photo, stringValue, isNull);
finally:
    return errCode;
}

int NumberMarkModel::CreateFromResultSet(ResultSet &resultSet)
{
    using Columns = NumberMarkColumns;
    int errCode = E_OK;
    int columnIndex;
    bool isNull;
    string stringValue;
    int64_t longValue;
    FETCH_FIELD(Columns::ID, Long, longValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(id, longValue, isNull)
    FETCH_FIELD(Columns::NUMBER, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(number, stringValue, isNull)
    FETCH_FIELD(Columns::NAME, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(name, stringValue, isNull)
    FETCH_FIELD(Columns::CLASSIFY, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(classify, stringValue, isNull)
    FETCH_FIELD(Columns::MARKED_COUNT, Long, longValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(marked_count, longValue, isNull)
    FETCH_FIELD(Columns::IS_CLOUD, Long, longValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(is_cloud, longValue, isNull)
    FETCH_FIELD(Columns::DESCRIPTION, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(description, stringValue, isNull)
    FETCH_FIELD(Columns::SAVE_TIMESTAMP, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(save_timestamp, stringValue, isNull)
    FETCH_FIELD(Columns::SUPPLIER, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(supplier, stringValue, isNull)
    FETCH_FIELD(Columns::SUPPLIER_ID, String, stringValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(supplier_id, stringValue, isNull);
    FETCH_FIELD(Columns::IS_INTELLIGENT_DB, Long, longValue, isNull, resultSet, columnIndex, errCode, goto finally);
    ASSIGN_IF_NOT_NULL(is_intelligent_db, longValue, isNull);
finally:
    return errCode;
}

MarkType NumberMarkModel::MarkType() const
{
    return GetMarkType(classify);
}

DataShareValuesBucket NumberMarkModel::ToDataShareValuesBucket() const
{
    DataShareValuesBucket values;
    using Columns = NumberMarkColumns;
    values.Put(Columns::NUMBER, number);
    PutOptional(values, Columns::NAME, name);
    values.Put(Columns::CLASSIFY, classify);
    PutOptional(values, Columns::MARKED_COUNT, marked_count);
    values.Put(Columns::IS_CLOUD, is_cloud);
    PutOptional(values, Columns::DESCRIPTION, description);
    values.Put(Columns::SAVE_TIMESTAMP, ToString(GetCurrentTimestamp()));
    PutOptional(values, Columns::SUPPLIER, supplier);
    PutOptional(values, Columns::SUPPLIER_ID, supplier_id);
    values.Put(Columns::IS_INTELLIGENT_DB, is_intelligent_db);
    return values;
}

DataSharePredicates NumberMarkModel::ToIdentityDataSharePredicates() const
{
    DataSharePredicates predicates;
    predicates.EqualTo(NumberMarkColumns::NUMBER, this->number)
        ->And()
        ->EqualTo(NumberMarkColumns::IS_INTELLIGENT_DB, this->is_intelligent_db)
        ->And()
        ->EqualTo(NumberMarkColumns::IS_CLOUD, this->is_cloud);
    return predicates;
}

static initializer_list<pair<const string, MarkType>> numberMarkTypeMapping = {
    { "none", MarkType::MARK_TYPE_NONE },
    { "crank", MarkType::MARK_TYPE_CRANK },
    { "fraud", MarkType::MARK_TYPE_FRAUD },
    { "express", MarkType::MARK_TYPE_EXPRESS },
    { "promote_sales", MarkType::MARK_TYPE_PROMOTE_SALES },
    { "house_agent", MarkType::MARK_TYPE_HOUSE_AGENT },
    { "insurance", MarkType::MARK_TYPE_INSURANCE },
    { "taxi", MarkType::MARK_TYPE_TAXI },
    { "custom", MarkType::MARK_TYPE_CUSTOM },
    { "others", MarkType::MARK_TYPE_OTHERS },
    { "yellow_page", MarkType::MARK_TYPE_YELLOW_PAGE },
    { "enterprise", MarkType::MARK_TYPE_ENTERPRISE },
};

MarkType GetMarkType(const string &text)
{
    static map<string, MarkType> numberMarkTypeNames = numberMarkTypeMapping;
    auto entry = numberMarkTypeNames.find(text);
    if (entry == numberMarkTypeNames.end()) {
        NUMBER_IDENTITY_LOGD("Unknown mark type: %{public}s", text.c_str());
        return MarkType::MARK_TYPE_NONE;
    }
    NUMBER_IDENTITY_LOGD("Mapped %{public}s to type %{public}d", text.c_str(), entry->second);
    return entry->second;
}

string GetClassify(MarkType markType)
{
    for (auto &entry : numberMarkTypeMapping) {
        auto &[key, value] = entry;
        if (value == markType) {
            return key;
        }
    }
    return "none";
}

void NumberMarkInfo::FromYellowPage(const YellowPageViewModel &yellowPage)
{
    markType = MarkType::MARK_TYPE_YELLOW_PAGE;
    markContent = yellowPage.name;
    markSource = nullopt;
    markCount = nullopt;
    isCloud = nullopt;
    markDetails = nullopt;
}

void NumberMarkInfo::FromNumberMark(const NumberMarkModel &numberMark)
{
    isCloud = numberMark.is_cloud == 1;
    markSource = numberMark.supplier;
    markContent = numberMark.name;
    markCount = numberMark.marked_count;
    markType = numberMark.MarkType();
    markDetails = numberMark.description;
}

} // namespace Telephony
} // namespace OHOS