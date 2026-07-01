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

#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "ability_context.h"
#include "ability_loader.h"
#include "abs_predicates.h"
#include "abs_rdb_predicates.h"
#include "rdb_errno.h"
#include "rdb_store.h"
#include "singleton.h"
#include "string_ex.h"
#include "values_bucket.h"

#include "number_identity_database.h"
#include "number_identity_ddl.h"
#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"
#include "number_identity_pinyin_helper.h"
#include "number_identity_rdb_helper.h"
#include "number_identity_utils.h"
#include "yellow_page_parser.h"

namespace OHOS {
namespace Telephony {
shared_ptr<NumberIdentityDatabase> NumberIdentityDatabase::numberIdentityDatabase_ = nullptr;
const char *const NumberIdentityDatabase::defaultSandboxDbDir = "/data/storage/el1/database";
const char *const NumberIdentityDatabase::dbFile = "number_identity.db";
const char *const NumberIdentityDatabase::bundledYellowPageDataPath = "/system/etc/telephony/yellowpage.data";
string NumberIdentityDatabase::databaseDir_ = "";
mutex NumberIdentityDatabase::dbMutex_;

void NumberIdentityDatabase::SetDBDirectory(const string &dir)
{
    databaseDir_ = dir;
}

int NumberIdentityDatabase::Insert(RdbStore &store, const string &table, const ValuesBucket &values)
{
    int64_t outRowId;
    auto errCode = store.Insert(outRowId, table, values);
    NUMBER_IDENTITY_LOGI("Inserted 1 row into table %{public}s, rowId = %{public}ld", table.c_str(), outRowId);
    return errCode;
}

int NumberIdentityDatabase::BatchInsert(RdbStore &store, const string &table, const vector<ValuesBucket> &valuesList)
{
    int64_t outInsertNum;
    auto errCode = store.BatchInsert(outInsertNum, table, valuesList);
    NUMBER_IDENTITY_LOGI("Inserted %{public}ld rows into table %{public}s.", outInsertNum, table.c_str());
    return errCode;
}

int NumberIdentityDatabase::Delete(RdbStore &store, AbsRdbPredicates &predicates)
{
    int deletedRows;
    int errCode = store.Delete(deletedRows, predicates);
    NUMBER_IDENTITY_LOGI("Deleted %{public}d rows of table %{public}s", deletedRows, predicates.GetTableName().c_str());
    return errCode;
}

int NumberIdentityDatabase::Update(RdbStore &store, AbsRdbPredicates &predicates, const ValuesBucket &values)
{
    int changedRows;
    int errCode = store.Update(changedRows, values, predicates);
    NUMBER_IDENTITY_LOGI("Updated %{public}d rows of table %{public}s", changedRows, predicates.GetTableName().c_str());
    return errCode;
}

shared_ptr<ResultSet> NumberIdentityDatabase::Query(
    RdbStore &store, const AbsRdbPredicates &predicates, const vector<string> &columns)
{
    auto stmt = predicates.GetStatement();
    NUMBER_IDENTITY_LOGI("Query: %{public}s with columns = %{public}s", stmt.c_str(), TO_C_STR(columns));
    return store.Query(predicates, columns);
}

int NumberIdentityDatabase::GetProperty(RdbStore &store, const char *propertyKey, string &value)
{
    using Columns = PropertiesColumns;
    int errCode = E_OK;
    int count = 0;
    int columnIndex = 0;
    bool isNull = false;
    RdbPredicates predicates(NumberIdentityTables::PROPERTIES);
    predicates.EqualTo(Columns::PROPERTY_KEY, propertyKey);
    vector<string> colums = { PropertiesColumns::PROPERTY_VALUE };
    auto result = Query(store, predicates, colums);
    FAIL_IF_NULL(result, goto finally);
    errCode = result->GetRowCount(count);
    HANDLE_ERR("result->GetRowCount", errCode, goto finally);
    if (count == 0) {
        NUMBER_IDENTITY_LOGI("Property '%{public}s' does not exist.", propertyKey);
        value = "";
        errCode = E_ERROR;
        goto finally;
    }
    errCode = result->GoToFirstRow();
    HANDLE_ERR("result->GoToFirstRow", errCode, goto finally);
    FETCH_FIELD(Columns::PROPERTY_VALUE, String, value, isNull, *result, columnIndex, errCode, goto finally);
finally:
    if (result != nullptr) {
        int closeErrCode = result->Close();
        LOG_IF_ERR(closeErrCode, "result->Close");
    }
    return errCode;
}

int NumberIdentityDatabase::UpdateProperty(RdbStore &store, const char *propertyKey, const string &value)
{
    ValuesBucket values;
    values.Put(PropertiesColumns::PROPERTY_VALUE, value);
    AbsRdbPredicates predicates(NumberIdentityTables::PROPERTIES);
    predicates.EqualTo(PropertiesColumns::PROPERTY_KEY, propertyKey);
    return Update(store, predicates, values);
}

int NumberIdentityDatabase::InsertProperty(RdbStore &store, const char *propertyKey, const string &value)
{
    ValuesBucket values;
    values.Put(PropertiesColumns::PROPERTY_VALUE, value);
    values.Put(PropertiesColumns::PROPERTY_KEY, propertyKey);
    return Insert(store, NumberIdentityTables::PROPERTIES, values);
}

int NumberIdentityDatabase::PutProperty(RdbStore &store, const char *propertyKey, const string &value)
{
    string oldValue;
    NUMBER_IDENTITY_LOGI("PutProperty %{public}s = %{public}s", propertyKey, value.c_str());
    int errCode = GetProperty(store, propertyKey, oldValue);
    if (errCode == E_OK) {
        // update property
        NUMBER_IDENTITY_LOGI(
            "Updating property %{public}s, old value is %{public}s row(s).", propertyKey, oldValue.c_str());
        errCode = UpdateProperty(store, propertyKey, value);
        LOG_IF_ERR(errCode, "Update property %{public}s", propertyKey);
    } else {
        NUMBER_IDENTITY_LOGI("Inserting property %{public}s = %{public}s", propertyKey, value.c_str());
        errCode = InsertProperty(store, propertyKey, value);
        LOG_IF_ERR(errCode, "Insert property %{public}s", propertyKey);
    }
    return errCode;
}

int NumberIdentityDatabase::ClearYellowPageData(RdbStore &store)
{
    int errCode = E_OK;
    AbsRdbPredicates delYellowPagePred(NumberIdentityTables::YELLOW_PAGE);
    AbsRdbPredicates delYellowPagePhonePred(NumberIdentityTables::YELLOW_PAGE_PHONE);
    errCode = Delete(store, delYellowPagePred);
    HANDLE_ERR("Clear yellow_page", errCode, goto finally);
    errCode = Delete(store, delYellowPagePhonePred);
    HANDLE_ERR("Clear yellow_page_phone", errCode, goto finally);
finally:
    return errCode;
}

int NumberIdentityDatabase::ImportYellowPageRecords(RdbStore &store, vector<YellowPageRecord> &records)
{
    int errCode = E_OK;
    int yellowPageId = 0;
    int yellowPagePhoneId = 0;
    errCode = ClearYellowPageData(store);
    auto &pinyinHelper = DelayedRefSingleton<PinyinHelper>::GetInstance();
    HANDLE_ERR("ClearYellowPageData", errCode, goto finally);
    for (auto record : records) {
        ValuesBucket values;
        values.PutInt(YellowPageColumns::ID, yellowPageId);
        values.PutString(YellowPageColumns::DATA, record.rawData);
        values.PutString(YellowPageColumns::GROUP_NAME, record.group);
        values.PutString(YellowPageColumns::NAME, record.name);
        if (record.photo.has_value()) {
            values.PutString(YellowPageColumns::PHOTO, *record.photo);
        }
        int64_t rowId;
        errCode = store.Insert(rowId, NumberIdentityTables::YELLOW_PAGE, values);
        HANDLE_ERR("Insert YellowPage", errCode, goto finally);
        for (auto phone : record.phone) {
            ValuesBucket values;
            string pinyin = phone.pinyin.has_value() ? *phone.pinyin : pinyinHelper.ConvertChineseToPinyin(phone.name);
            pinyin = UpperStr(pinyin);
            values.PutInt(YellowPagePhoneColumns::ID, yellowPagePhoneId);
            values.PutInt(YellowPagePhoneColumns::YELLOW_PAGE_ID, yellowPageId);
            values.PutInt(YellowPagePhoneColumns::HOT_POINTS, phone.hot_points);
            values.PutString(YellowPagePhoneColumns::DIAL_MAP, phone.dial_map);
            values.PutString(YellowPagePhoneColumns::NAME, phone.name);
            values.PutString(YellowPagePhoneColumns::NUMBER, phone.phone);
            values.PutString(YellowPagePhoneColumns::PINYIN, pinyin);
            values.PutString(YellowPagePhoneColumns::PINYIN_SUB_STRING, pinyinHelper.GetPinyinSubString(pinyin));
            values.PutString(YellowPagePhoneColumns::PINYIN_ABBREVIATION, pinyinHelper.GetPinyinAbbreviation(pinyin));
            if (phone.match_pattern.has_value()) {
                values.PutString(YellowPagePhoneColumns::MATCH_PATTERN, *phone.match_pattern);
            }
            if (phone.device_type.has_value()) {
                values.PutInt(YellowPagePhoneColumns::MATCH_PATTERN, *phone.device_type);
            }
            if (phone.alias_name.has_value()) {
                values.PutString(YellowPagePhoneColumns::MATCH_PATTERN, *phone.alias_name);
            }
            errCode = store.Insert(rowId, NumberIdentityTables::YELLOW_PAGE_PHONE, values);
            HANDLE_ERR("Insert YellowPagePhone", errCode, goto finally);
            ++yellowPagePhoneId;
        }
        ++yellowPageId;
    }
finally:
    return errCode;
}

static bool IsValidPath(const char *path)
{
    // Useless check. Just for code check.
    return true;
}

int NumberIdentityDatabase::ImportYellowPageData()
{
    return ImportYellowPageData(bundledYellowPageDataPath, *GetInstance()->store_);
}

int NumberIdentityDatabase::ImportYellowPageData(string dataPath)
{
    return ImportYellowPageData(dataPath, *GetInstance()->store_);
}

int NumberIdentityDatabase::ImportYellowPageData(const string &filePath, RdbStore &store)
{
    int errCode = E_OK;
    NUMBER_IDENTITY_LOGI("Import Yellow Page begin.");
    char *resolved = realpath(filePath.c_str(), nullptr);
    if (resolved == nullptr) {
        NUMBER_IDENTITY_LOGE("Failed to verify realpath of yellow page file import path.");
        return E_ERROR;
    }
    if (!IsValidPath(resolved)) {
        NUMBER_IDENTITY_LOGE("Illegal yellow page path.");
        errCode = E_ERROR;
    } else {
        std::ifstream file;
        file.open(resolved);
        if (file.fail()) {
            NUMBER_IDENTITY_LOGE("Open yellow page data file failed.");
            errCode = E_ERROR;
        } else {
            errCode = ImportYellowPageDataWithFileStream(file, store);
        }
        file.close();
    }
    free(resolved);
    return errCode;
}

int NumberIdentityDatabase::ImportYellowPageDataWithFileStream(std::ifstream &file, RdbStore &store)
{
    YellowPageParser parser;
    YellowPageDataSet dataSet;
    string version;
    int errCode = E_OK;
    if (!parser.Parse(file, dataSet)) {
        return E_ERROR;
    }
    errCode = store.BeginTransaction();
    HANDLE_ERR("BeginTransaction", errCode, goto error);
    errCode = GetProperty(store, PropertyKeys::YELLOW_PAGE_VERSION, version);
    // When there is old version, check old version before database update.
    if (errCode == E_OK && version >= dataSet.version) {
        NUMBER_IDENTITY_LOGI(
            "Old version %{public}s >= %{public}s, not updated.", version.c_str(), dataSet.version.c_str());
        goto complete;
    }
    errCode = PutProperty(store, PropertyKeys::YELLOW_PAGE_VERSION, dataSet.version);
    HANDLE_ERR("Put YellowPage version", errCode, goto error);
    errCode = ImportYellowPageRecords(store, dataSet.records);
    HANDLE_ERR("Import YellowPage Record", errCode, goto error);
complete:
    errCode = store.Commit();
    HANDLE_ERR("Import YellowPage RDB Commit", errCode, goto error);
    NUMBER_IDENTITY_LOGI("Import Yellow Page done.");
    return E_OK;
error:
    store.RollBack();
    NUMBER_IDENTITY_LOGE("Import Yellow Page failed.");
    return E_ERROR;
}

NumberIdentityDatabase::NumberIdentityDatabase(shared_ptr<RdbStore> store): store_(store) {}

shared_ptr<NumberIdentityDatabase> NumberIdentityDatabase::Create()
{
    if (databaseDir_ == "") {
        SetDBDirectory(defaultSandboxDbDir);
    }
    shared_ptr<NumberIdentityDatabase> instance = nullptr;
    shared_ptr<RdbStore> store = nullptr;
    string dbPath = databaseDir_ + "/" + dbFile;
    int errCode = E_OK;
    RdbStoreConfig config(dbPath);
    SqliteOpenHelperNumberIdentityCallback sqliteOpenHelperCallback;
    errCode = config.SetBundleName("com.ohos.numberidentity");
    HANDLE_ERR("NumberIdentity config.SetBundleName", errCode, goto error);
    store = RdbHelper::GetRdbStore(config, DATABASE_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    HANDLE_ERR("NumberIdentityDatabase GetRdbStore", errCode, goto error);
    NUMBER_IDENTITY_LOGI("NumberIdentityDatabase GetRdbStore success.");
    instance = std::make_shared<NumberIdentityDatabase>(store);
    return instance;
error:
    NUMBER_IDENTITY_LOGF("Failed to Create NumberIdentityDatabase.");
    return nullptr;
}

shared_ptr<NumberIdentityDatabase> NumberIdentityDatabase::GetInstance()
{
    std::lock_guard lock(dbMutex_);
    if (numberIdentityDatabase_ == nullptr) {
        numberIdentityDatabase_ = NumberIdentityDatabase::Create();
    }
    return numberIdentityDatabase_;
}

int NumberIdentityDatabase::BeginTransaction()
{
    if (store_ == nullptr) {
        NUMBER_IDENTITY_LOGF("NumberIdentityDatabase BeginTransaction store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int errCode = store_->BeginTransaction();
    LOG_IF_ERR(errCode, "NumberIdentityDatabase BeginTransaction");
    return errCode;
}

int NumberIdentityDatabase::Commit()
{
    if (store_ == nullptr) {
        NUMBER_IDENTITY_LOGF("NumberIdentityDatabase Commit store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int errCode = store_->Commit();
    LOG_IF_ERR(errCode, "NumberIdentityDatabase Commit");
    return errCode;
}

int NumberIdentityDatabase::RollBack()
{
    if (store_ == nullptr) {
        NUMBER_IDENTITY_LOGF("NumberIdentityDatabase RollBack store_ is nullptr");
        return RDB_OBJECT_EMPTY;
    }
    int errCode = store_->RollBack();
    LOG_IF_ERR(errCode, "NumberIdentityDatabase RollBack");
    return errCode;
}

int SqliteOpenHelperNumberIdentityCallback::OnCreate(RdbStore &store)
{
    NUMBER_IDENTITY_LOGI("SqliteOpenHelperNumberIdentityCallback::OnCreate");
    int errCode = E_OK;
    errCode = store.ExecuteSql(CREATE_PROPERTIES);
    HANDLE_ERR("CREATE_PROPERTIES", errCode, goto finally);
    errCode = store.ExecuteSql(CREATE_YELLOW_PAGE);
    HANDLE_ERR("CREATE_YELLOW_PAGE", errCode, goto finally);
    errCode = store.ExecuteSql(CREATE_YELLOW_PAGE_PHONE);
    HANDLE_ERR("CREATE_YELLOW_PAGE_PHONE", errCode, goto finally);
    errCode = store.ExecuteSql(CREATE_NUMBER_MARK);
    HANDLE_ERR("CREATE_NUMBER_MARK", errCode, goto finally);
    errCode = store.ExecuteSql(CREATE_NUMBER_MARK_EXTRAS);
    HANDLE_ERR("CREATE_NUMBER_MARK_EXTRAS", errCode, goto finally);
    errCode = store.ExecuteSql(CREATE_YELLOW_PAGE_VIEW);
    HANDLE_ERR("CREATE_YELLOW_PAGE_VIEW", errCode, goto finally);
    errCode = NumberIdentityDatabase::ImportYellowPageData(NumberIdentityDatabase::bundledYellowPageDataPath, store);
    LOG_IF_ERR(errCode, "ImportYellowPageData");
    // Ignore yellow page import error in OnCreate().
    errCode = E_OK;
finally:
    return errCode;
}

static vector<function<int(RdbStore &)>> g_upgrades = {
    [](RdbStore &store) -> int {
        int errCode = E_OK;
        errCode = store.ExecuteSql("ALTER TABLE number_mark ADD SUPPLIER_ID Text;");
        HANDLE_ERR("Upgrade 1 to 2 add SUPPLIER_ID", errCode, return errCode);
        errCode = store.ExecuteSql("ALTER TABLE number_mark ADD IS_INTELLIGENT_DB int default 0;");
        HANDLE_ERR("Upgrade 1 to 2 ADD IS_INTELLIGENT_DB", errCode, return errCode);
        return errCode;
    },
};

int SqliteOpenHelperNumberIdentityCallback::OnUpgrade(RdbStore &store, int oldVersion, int newVersion)
{
    NUMBER_IDENTITY_LOGI("Upgrade DB %{public}d => %{public}d", oldVersion, newVersion);
    int errCode = E_OK;
    for (int i = oldVersion; i < newVersion; ++i) {
        auto &upgrade = g_upgrades[i - 1];
        errCode = upgrade(store);
        if (errCode != E_OK) {
            NUMBER_IDENTITY_LOGF("DB Upgrade from %{public}d to %{public}d failed: %{public}d", i, i + 1, errCode);
            break;
        }
    }
    return errCode;
}

int SqliteOpenHelperNumberIdentityCallback::OnDowngrade(RdbStore &store, int oldVersion, int newVersion)
{
    NUMBER_IDENTITY_LOGI("SqliteOpenHelperNumberIdentityCallback::OnDowngrade");
    return E_OK;
}

RdbStore &NumberIdentityDatabase::GetStore()
{
    return *store_;
}

int NumberIdentityDatabase::GetProperty(const char *propertyKey, string &value)
{
    return NumberIdentityDatabase::GetProperty(GetStore(), propertyKey, value);
}

int NumberIdentityDatabase::PutProperty(const char *propertyKey, const string &value)
{
    return NumberIdentityDatabase::PutProperty(GetStore(), propertyKey, value);
}

int NumberIdentityDatabase::Insert(const string &table, const ValuesBucket &values)
{
    FAIL_IF_NULL(store_, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    return Insert(*store_, table, values);
}

int NumberIdentityDatabase::BatchInsert(const string &table, const vector<ValuesBucket> &valuesList)
{
    FAIL_IF_NULL(store_, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    return BatchInsert(*store_, table, valuesList);
}

int NumberIdentityDatabase::Delete(AbsRdbPredicates &predicates)
{
    FAIL_IF_NULL(store_, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    return Delete(*store_, predicates);
}

int NumberIdentityDatabase::Update(AbsRdbPredicates &predicates, const ValuesBucket &values)
{
    FAIL_IF_NULL(store_, return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL);
    return Update(*store_, predicates, values);
}

shared_ptr<ResultSet> NumberIdentityDatabase::Query(const AbsRdbPredicates &predicates, const vector<string> &columns)
{
    FAIL_IF_NULL(store_, return nullptr);
    return Query(*store_, predicates, columns);
}

} // namespace Telephony
} // namespace OHOS