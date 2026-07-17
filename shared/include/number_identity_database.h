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

#ifndef NUMBER_IDENTITY_DATABASE_H
#define NUMBER_IDENTITY_DATABASE_H

#include <memory>
#include <mutex>
#include <pthread.h>
#include <string>
#include <vector>

#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "hilog_wrapper.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "values_bucket.h"
#include "yellow_page_parser.h"

namespace OHOS {
namespace Telephony {
using std::mutex;
using std::shared_ptr;
using std::string;
using std::vector;
using namespace NativeRdb;
class NumberIdentityDatabase {
  public:
    static const char *const defaultSandboxDbDir;
    static const char *const dbFile;
    static const char *const bundledYellowPageDataPath;
    static void SetDBDirectory(const string &dir);
    static int Insert(RdbStore &store, const string &table, const ValuesBucket &values);
    static int BatchInsert(RdbStore &store, const string &table, const vector<ValuesBucket> &valuesList);
    static int Delete(RdbStore &store, AbsRdbPredicates &predicates);
    static int Update(RdbStore &store, AbsRdbPredicates &predicates, const ValuesBucket &values);
    static shared_ptr<ResultSet> Query(
        RdbStore &store, const AbsRdbPredicates &predicates, const vector<string> &columns);
    /**
     * Static methods are used in `SqliteOpenHelperNumberIdentityCallback`.
     * Works without `NumberIdentityDatabase` instance.
     */
    static int GetProperty(RdbStore &store, const char *propertyKey, string &value);
    static int UpdateProperty(RdbStore &store, const char *propertyKey, const string &value);
    static int InsertProperty(RdbStore &store, const char *propertyKey, const string &value);
    static int PutProperty(RdbStore &store, const char *propertyKey, const string &value);
    /**
     * Import yellow page data by default file path and global RDB store.
     */
    static int ImportYellowPageData();
    /**
     * Import yellow page data by temporary yellowpage.data file path and global RDB store.
     */
    static int ImportYellowPageData(string dataPath);
    /**
     * Import yellow page data on RDB create.
     */
    static int ImportYellowPageData(const string &filePath, RdbStore &store);
    static shared_ptr<NumberIdentityDatabase> GetInstance();
    NumberIdentityDatabase(shared_ptr<RdbStore> store);
    RdbStore &GetStore();
    int GetProperty(const char *propertyKey, string &value);
    int PutProperty(const char *propertyKey, const string &value);
    int Insert(const string &table, const ValuesBucket &values);
    int BatchInsert(const string &table, const vector<ValuesBucket> &valuesList);
    int Delete(AbsRdbPredicates &predicates);
    int Update(AbsRdbPredicates &predicates, const ValuesBucket &values);
    shared_ptr<ResultSet> Query(const AbsRdbPredicates &predicates, const vector<string> &columns);
    int BeginTransaction();
    int Commit();
    int RollBack();

  private:
    static mutex dbMutex_;
    static string databaseDir_;
    static shared_ptr<NumberIdentityDatabase> numberIdentityDatabase_;
    static shared_ptr<NumberIdentityDatabase> Create();
    static int ClearYellowPageData(RdbStore &store);
    static int ImportYellowPageDataWithFileStream(std::ifstream &file, RdbStore &store);
    static int ImportYellowPageRecords(RdbStore &store, vector<YellowPageRecord> &records);
    shared_ptr<RdbStore> store_;
};

class SqliteOpenHelperNumberIdentityCallback : public RdbOpenCallback {
  public:
    int OnCreate(RdbStore &rdbStore) override;
    int OnUpgrade(RdbStore &rdbStore, int oldVersion, int newVersion) override;
    int OnDowngrade(RdbStore &rdbStore, int currentVersion, int targetVersion) override;
};

constexpr int RDB_OBJECT_EMPTY = -1;
constexpr int DATABASE_INIT_VERSION = 1;
constexpr int DATABASE_OPEN_VERSION = 2;

} // namespace Telephony
} // namespace OHOS

#endif // NUMBER_IDENTITY_DATABASE_H
