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

#ifndef NUMBER_MARK_ABILITY_H
#define NUMBER_MARK_ABILITY_H

#include "number_identity_errors.h"
#include "number_identity_models.h"

#include "ability.h"
#include "ability_lifecycle.h"
#include "abs_shared_result_set.h"
#include "datashare_business_error.h"
#include "datashare_errno.h"
#include "datashare_ext_ability.h"
#include "datashare_ext_ability_context.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "rdb_errno.h"
#include "want.h"

#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {
using DataShare::DatashareBusinessError;
using DataShare::DataShareExtAbility;
using DataShare::DataShareExtAbilityContext;
using DataShare::DataSharePredicates;
using DataShare::DataShareResultSet;
using DataShare::DataShareValuesBucket;
using std::enable_shared_from_this;
using std::function;
using std::future;
using std::recursive_mutex;
using std::set;
using std::shared_ptr;
using std::string;
using std::vector;

class NumberMarkQueryContext {
  public:
    string phoneNumber = "";
    vector<NumberMarkModel> dbMarks;
    vector<YellowPageViewModel> dbYellowPages;
    string numberMarkQueryNumber = "";
    string yellowPageQueryNumber = "";
    set<string> failedVendors = {};
};

enum class CacheStatus {
    NotExist = -1,
    Valid = 0,
    Invalid,
    Legacy,
    Expired,
};

class NumberMarkAbility : public DataShareExtAbility {
  public:
    explicit NumberMarkAbility(shared_ptr<DataShareExtAbilityContext> parentContext);
    virtual ~NumberMarkAbility() override;

    static NumberMarkAbility *Create(shared_ptr<DataShareExtAbilityContext> parentContext);
    virtual void OnStart(const Want &want) override;
    sptr<IRemoteObject> OnConnect(const Want &want) override;

    virtual int Insert(const Uri &uri, const DataShareValuesBucket &value) override;
    virtual int BatchInsert(const Uri &uri, const vector<DataShareValuesBucket> &values) override;
    virtual int Delete(const Uri &uri, const DataSharePredicates &predicates) override;
    virtual int Update(
        const Uri &uri, const DataSharePredicates &predicates, const DataShareValuesBucket &values) override;
    virtual shared_ptr<DataShareResultSet> Query(const Uri &uri, const DataSharePredicates &predicates,
        vector<string> &columns, DatashareBusinessError &businessError) override;

    int Put(const string &table, const DataSharePredicates &predicates, const DataShareValuesBucket &values);

    int RawInsert(const string &table, const DataShareValuesBucket &values);
    int RawBatchInsert(const string &table, const vector<DataShareValuesBucket> &values);
    int RawUpdate(const string &table, const DataSharePredicates &predicates, const DataShareValuesBucket &values);
    int RawDelete(const string &table, const DataSharePredicates &predicates);
    shared_ptr<DataShareResultSet> RawQuery(const string &table, const DataSharePredicates &predicates,
        vector<string> &columns, DatashareBusinessError &businessError);

    virtual bool IsSelfCall();
    virtual bool CheckPermissionBypassSelf(const string &permission);

    shared_ptr<DataShareResultSet> APIQuery(const Uri &uri, const DataSharePredicates &predicates,
        vector<string> &columns, DatashareBusinessError &businessError);
    shared_ptr<DataShareResultSet> ProcessNumber(
        const DataSharePredicates &predicates, vector<string> &columns, DatashareBusinessError &businessError);
    /*
     * Query multiple numbers by where args.
     */
    shared_ptr<DataShareResultSet> QueryNumberMarks(
        const DataSharePredicates &predicates, DatashareBusinessError &businessError);
    int QueryByPhoneNumber(const string &phoneNumber, NumberMarkInfo &markInfo, DatashareBusinessError &businessError);
    int QueryLocalYellowPage(
        const string &phoneNumber, vector<YellowPageViewModel> &yellowPages, DatashareBusinessError &businessError);
    int QueryLocalNumberMark(
        const string &phoneNumber, vector<NumberMarkModel> &numberMarks, DatashareBusinessError &businessError);
    virtual bool IsStrangeNumberIdentitySwitchedOn(sptr<IRemoteObject> token);
    int QueryIntelligentDB(const string &phoneNumber, NumberMarkInfo &markInfo);
    int RunInTransaction(const string &tag, function<int()> transaction);
    int PutOrDeleteNumberMark(const NumberMarkModel &numberMark);
    int SetNumberMark(const DataShareValuesBucket &values);
    int ParseNumberMark(const DataShareValuesBucket &values, NumberMarkModel &numberMark);
    int SetLocalNumberMark(const NumberMarkModel &numberMark, vector<NumberMarkModel> &oldMarks);
    int SetIntelligentDBUpdateTimestamp(const DataShareValuesBucket &values);
    bool isAntifraudSwitchOn(const string &bundleName, const string &appSwitchKey);
    virtual bool ShouldMarkTypeBeReported(MarkType markType);
    virtual int UpdateCallLog(const NumberMarkModel &numberMark);

  private:
    shared_ptr<DataShareExtAbilityContext> parentContext_;
    recursive_mutex tasksMutex_;

    void dealMarkSource(const NumberMarkModel &numberMark, DataShareValuesBucket &values);
};


} // namespace Telephony
} // namespace OHOS

#endif // NUMBER_MARK_ABILITY_H
