/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#include "access_token.h"
#include "accesstoken_kit.h"
#include "datashare_ext_ability.h"
#include "datashare_ext_ability_context.h"
#include "datashare_predicates.h"
#include "datashare_values_bucket.h"
#include "nativetoken_kit.h"
#include "number_identity_database.h"
#include "number_identity_datashare_stub_impl.h"
#include "number_identity_ddl.h"
#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"
#include "number_identity_models.h"
#include "number_identity_utils.h"
#include "number_mark_ability.h"
#define private public
#include "number_mark_manager.h"
#undef private
#include "rdb_errno.h"
#include "string_ex.h"
#include "telephony_permission.h"
#include "timer.h"
#include "token_setproc.h"
#include "uri.h"
#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <future>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <map>
#include <memory>
#include <new>
#include <optional>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

using namespace std;

namespace OHOS {
namespace Telephony {
using namespace testing;
using namespace testing::ext;
using namespace Security::AccessToken;
using Security::AccessToken::AccessTokenID;
constexpr const char *MOCK_SUPPLIER_ID = "vendor.mock.gtest";
class AccessToken {
  public:
    uint64_t tokenId;
    vector<const char *> permissions;
    explicit AccessToken(const vector<const char *> &p)
    {
        permissions = p;
        for (auto permission : permissions) {
            NUMBER_IDENTITY_LOGI("Bypass permission(s): %{public}s", permission);
        }
        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0, // Indicates the capsbility list of the sa.
            .permsNum = permissions.size(),
            .aclsNum = 0, // acls is the list of rights that can be escalated.
            .dcaps = nullptr,
            .perms = permissions.data(),
            .acls = nullptr,
            .processName = "BranchTest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        auto result = Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        EXPECT_EQ(result, Security::AccessToken::RET_SUCCESS);
    }
    ~AccessToken() {}
};

class MockNumberMarkVendor : public NumberMarkVendor {
  public:
    int delayMS;
    NumberMarkVendorResult result;
    bool shouldBeInvaliated;
    MockNumberMarkVendor(NumberMarkVendorResult res, int delay = 0, const char *supplierId = MOCK_SUPPLIER_ID,
        bool shouldBeInvaliated = false)
        : NumberMarkVendor(supplierId, NumberMarkVendorType::THIRD_PARTY_APP), delayMS(delay), result(res),
          shouldBeInvaliated(shouldBeInvaliated)
    {
    }
    virtual future<NumberMarkVendorResult> QueryNumberMark(const string &phoneNumber) override
    {
        return async(launch::async, [this]() {
            this_thread::sleep_for(chrono::milliseconds(delayMS));
            if (shouldBeInvaliated) {
                Invalidate("should be invalidated");
            }
            return result;
        });
    }
};

class PermissionBypassNumberMarkAbility : public NumberMarkAbility {
  public:
    PermissionBypassNumberMarkAbility(nullptr_t p): NumberMarkAbility(nullptr) {}
    virtual bool CheckPermissionBypassSelf(const string &permission) override
    {
        return true;
    }
};

class HookedNumberMarkAbility : public NumberMarkAbility {
  public:
    vector<shared_ptr<NumberMarkVendor>> vendors;
    bool switchOn;
    HookedNumberMarkAbility(vector<shared_ptr<NumberMarkVendor>> v, bool switchOn = true)
        : NumberMarkAbility(nullptr), vendors(v), switchOn(switchOn)
    {
    }
    virtual bool IsStrangeNumberIdentitySwitchedOn(sptr<IRemoteObject> token) override
    {
        return switchOn;
    }
    virtual vector<shared_ptr<NumberMarkVendor>> InitializeVendors() override
    {
        return vendors;
    }
};

shared_ptr<NumberMarkVendor> MockVendor(NumberMarkVendorResult res, int delay = 0,
    const char *supplierId = "vendor.mock.gtest", bool shouldBeInvaliated = false)
{
    return make_shared<MockNumberMarkVendor>(res, delay, supplierId, shouldBeInvaliated);
}

using Result = NumberMarkVendorResult;

namespace DBUtils {
void ClearNumberMark()
{
    auto db = NumberIdentityDatabase::GetInstance();
    AbsRdbPredicates predicates(NumberIdentityTables::NUMBER_MARK);
    db->Delete(predicates);
}

void ClearProperties()
{
    auto db = NumberIdentityDatabase::GetInstance();
    AbsRdbPredicates predicates(NumberIdentityTables::PROPERTIES);
    db->Delete(predicates);
}

} // namespace DBUtils

class NumberMarkAbilityGtest : public testing::Test {
  public:
    shared_ptr<NumberMarkAbility> ability;
    shared_ptr<AccessToken> token;
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void NumberMarkAbilityGtest::SetUp()
{
    NUMBER_IDENTITY_LOGI("Setup numbermarkability Gtest");
    NumberIdentityDatabase::SetDBDirectory("/data/test");
    ability = make_shared<NumberMarkAbility>(nullptr);
    // auto permissions = {
    //     Permission::GET_TELEPHONY_STATE,
    //     Permission::SET_TELEPHONY_STATE,
    // };
    // token = make_shared<AccessToken>(permissions);
}

void NumberMarkAbilityGtest::TearDown() {}

void NumberMarkAbilityGtest::SetUpTestCase() {}

void NumberMarkAbilityGtest::TearDownTestCase() {}

HWTEST_F(NumberMarkAbilityGtest, Query_yellow_page_10086, Function | MediumTest | Level1)
{
    vector<string> columns;
    DataShare::DataSharePredicates predicates;
    vector<string> queryNumbers = { "10086", "10000", "10010" };
    predicates.SetWhereArgs(queryNumbers);
    DataShare::DatashareBusinessError businessError;
    Uri uri("datashare:///com.ohos.numbermarkability/number_mark_info");
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_NE(resultSet, nullptr);
 
    resultSet->Close();
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_numbermarkability_Query_View, TestSize.Level0)
{
    Uri uri("datashare:///com.ohos.numbermarkability/yellow_page_view");
    DataSharePredicates predicates;
    predicates.EqualTo(YellowPageViewColumns::NUMBER, "10086");
    vector<string> columns = {};
    DatashareBusinessError businessError;
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_EQ(businessError.GetCode(), 0);
    int count = 0;
    EXPECT_EQ(resultSet->GetRowCount(count), E_OK);
    EXPECT_GT(count, 0);
    EXPECT_EQ(resultSet->GoToFirstRow(), E_OK);
    string yellowPageName;
    int columnIndex;
    EXPECT_EQ(resultSet->GetColumnIndex(YellowPageViewColumns::NAME, columnIndex), E_OK);
    EXPECT_EQ(resultSet->GetString(columnIndex, yellowPageName), E_OK);
    EXPECT_TRUE(yellowPageName.find("中国移动") != string::npos);
    resultSet->Close();
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_numbermarkability_Set_Number_Mark, Function | MediumTest | Level1)
{
    Uri uri("datashare:///com.ohos.numbermarkability/number_mark_info");
    DataSharePredicates predicates;
    DataShareValuesBucket values;
    values.Put(SetNumberMarkParamsFields::phoneNumber, "12345678901");
    values.Put(SetNumberMarkParamsFields::markType, static_cast<int64_t>(MarkType::MARK_TYPE_CUSTOM));
    values.Put(SetNumberMarkParamsFields::customMarkContent, "自定义标记");
    auto errCode = ability->Update(uri, predicates, values);
    EXPECT_GE(errCode, NUMBER_IDENTITY_ERR_SUCCESS);
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_numbermarkability_Query_Group, TestSize.Level0)
{
    Uri uri("datashare:///com.ohos.numbermarkability/yellow_page");
    DataSharePredicates predicates;
    vector<string> columns = {};
    DatashareBusinessError businessError;
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_EQ(businessError.GetCode(), 0);
    int count = 0;
    EXPECT_EQ(resultSet->GetRowCount(count), E_OK);
    EXPECT_GT(count, 0);
    resultSet->Close();
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_numbermarkability_Query_IntelligentDB, Function | MediumTest | Level1)
{
    using Columns = NumberMarkColumns;
    auto db = NumberIdentityDatabase::GetInstance();
    EXPECT_NE(db, nullptr);
    string phoneNumber = "13312345678";
    db->BatchInsert(NumberIdentityTables::NUMBER_MARK,
        {
            ValuesBucket({
                { Columns::NUMBER, ToHexString(*GenerateMD5(StringBytes(phoneNumber))) },
                { Columns::NAME, "智能识别库标记" },
                { Columns::CLASSIFY, "taxi" },
                { Columns::MARKED_COUNT, 20 },
                { Columns::IS_CLOUD, 1 },
                { Columns::SAVE_TIMESTAMP, ToString(GetCurrentTimestamp()) },
                { Columns::SUPPLIER, "电话邦" },
                { Columns::IS_INTELLIGENT_DB, true },
            }),
        });
    NumberMarkInfo markInfo;
    auto errCode = ability->QueryIntelligentDB(phoneNumber, markInfo);
    EXPECT_EQ(errCode, NUMBER_IDENTITY_ERR_SUCCESS);
    EXPECT_EQ(markInfo.markContent, "智能识别库标记");
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.markCount, 20);
    EXPECT_EQ(markInfo.isCloud, true);
    EXPECT_EQ(markInfo.markSource, "电话邦");
    auto numberIdentityDataShareStubImpl = make_unique<DataShare::NumberIdentityDataShareStubImpl>();
    ASSERT_NE(numberIdentityDataShareStubImpl, nullptr);
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(Columns::IS_INTELLIGENT_DB, true);
    Uri uri("datashare:///com.ohos.numbermarkability/number_mark");
    numberIdentityDataShareStubImpl->Delete(uri, predicates);
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_numbermarkability_Query_InvalidTable, TestSize.Level0)
{
    Uri uri("datashare:///com.ohos.numbermarkability/yellow_page_typo");
    DataSharePredicates predicates;
    vector<string> columns = {};
    DatashareBusinessError businessError;
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_NE(businessError.GetCode(), 0);
    EXPECT_EQ(resultSet, nullptr);
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkCloudAPI_GenerateCalleeId, TestSize.Level0)
{
    constexpr const size_t bitLength = 128;
    constexpr const size_t idLength = bitLength / 4;
    auto id1 = NumberMarkCloudAPI::GenerateCalleeId();
    auto id2 = NumberMarkCloudAPI::GenerateCalleeId();
    EXPECT_NE(id1, id2);
    EXPECT_EQ(id1.size(), idLength);
    EXPECT_EQ(id2.size(), idLength);
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkManager_IsMobileNumber, TestSize.Level0)
{
    EXPECT_EQ(NumberMarkManager::IsMobilePhoneNumber(""), false);
    EXPECT_EQ(NumberMarkManager::IsMobilePhoneNumber("10086"), false);
    EXPECT_EQ(NumberMarkManager::IsMobilePhoneNumber("12345678901"), true);
    EXPECT_EQ(NumberMarkManager::IsMobilePhoneNumber("22345678901"), false);
    EXPECT_EQ(NumberMarkManager::IsMobilePhoneNumber("1790012345678901"), true);
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkManager_IpHeadBarber, TestSize.Level0)
{
    EXPECT_EQ(NumberMarkManager::IpHeadBarber(""), "");
    EXPECT_EQ(NumberMarkManager::IpHeadBarber("10086"), "10086");
    EXPECT_EQ(NumberMarkManager::IpHeadBarber("12345678901"), "12345678901");
    EXPECT_EQ(NumberMarkManager::IpHeadBarber("1234567890123456"), "1234567890123456");
    EXPECT_EQ(NumberMarkManager::IpHeadBarber("1790012345678901"), "12345678901");
    EXPECT_EQ(NumberMarkManager::IpHeadBarber("96435"), "");
    EXPECT_EQ(NumberMarkManager::IpHeadBarber("9643512345678901"), "12345678901");
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkManager_StandardizationPhoneNum, Function | MediumTest | Level1)
{
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum(""), "");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("10086"), "10086");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("12345678901"), "12345678901");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("+8612345678901"), "12345678901");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("8612345678901"), "12345678901");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("(+86) 123 4567 8901"), "12345678901");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("(+86)123-4567-8901"), "12345678901");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("1234567890123456"), "1234567890123456");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("1790012345678901"), "12345678901");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("96435"), "");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("017012345678"), "17012345678");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("017912345678"), "017912345678");
    EXPECT_EQ(NumberMarkManager::StandardizationPhoneNum("9643512345678901"), "12345678901");
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_001, Function | MediumTest | Level1)
{
    NumberMarkAbility *abilityPtr = NumberMarkAbility::Create(nullptr);
    EXPECT_NE(abilityPtr, nullptr);
    DataShareValuesBucket values;
    Uri uri("datashare:///com.ohos.numbermarkability/number_mark_info");
    abilityPtr->Insert(uri, values);
    DataSharePredicates predicates;
    abilityPtr->Delete(uri, predicates);
    string table = "table";
    abilityPtr->Put(table, predicates, values);
    abilityPtr->RawDelete(table, predicates);
    abilityPtr->RawUpdate(table, predicates, values);
    NumberMarkQueryContext context;
    context.numberMarkQueryNumber = "10086";
    abilityPtr->NumberMarkCloudQuerySteps(context);
    NumberMarkModel numberMark;
    abilityPtr->PutOrDeleteNumberMark(numberMark);
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_002, TestSize.Level0)
{
    NumberMarkManager numberMarkManager;
    EXPECT_EQ(numberMarkManager.IsMobilePhoneNumber(""), false);
    EXPECT_EQ(numberMarkManager.IsMobilePhoneNumber("10086"), false);
    EXPECT_EQ(numberMarkManager.IsMobilePhoneNumber("12345678901"), true);
    EXPECT_EQ(numberMarkManager.IsMobilePhoneNumber("22345678901"), false);
    EXPECT_EQ(numberMarkManager.IsMobilePhoneNumber("1790012345678901"), true);
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_EqualEmptyStr, TestSize.Level0)
{
    NumberMarkManager numberMarkManager;
    EXPECT_EQ(numberMarkManager.IpHeadBarber(""), "");
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_PhoneNumber, Function | MediumTest | Level1)
{
    NumberMarkAbility ability(nullptr);
    DataSharePredicates predicates;
    DatashareBusinessError businessError;
    ability.QueryNumberMarks(predicates, businessError);
    const string phoneNumber = "1790012345678901";
    NumberMarkInfo markInfo;
    ability.QueryByPhoneNumber(phoneNumber, markInfo, businessError);
    vector<YellowPageViewModel> yellowPages;
    ability.QueryLocalYellowPage(phoneNumber, yellowPages, businessError);
    std::vector<NumberMarkModel> numberMarks;
    ability.QueryLocalNumberMark(phoneNumber, numberMarks, businessError);
    ability.SaveNumberMarkCloudCache(phoneNumber, markInfo);
    DataShareValuesBucket values;
    ability.SetNumberMark(values);
    NumberMarkModel numberMark;
    EXPECT_NE(ability.ParseNumberMark(values, numberMark), NUMBER_IDENTITY_ERR_SUCCESS);
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_TagAndTable, TestSize.Level0)
{
    NumberMarkAbility ability(nullptr);
    DataSharePredicates predicates;
    DatashareBusinessError businessError;
    NumberMarkModel markModel;
    vector<NumberMarkModel> oldMarks;
    DataShareValuesBucket values;
    ability.SetNumberMark(values);
    ability.SetLocalNumberMark(markModel, oldMarks);
    const string tag = "tag";
    ability.RunInTransaction(tag, std::bind([]() { return 0; }));
    string table = "table";
    vector<string> columns;
    ability.RawQuery(table, predicates, columns, businessError);
    EXPECT_NE(ability.RawInsert(table, values), NUMBER_IDENTITY_ERR_SUCCESS);
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_YellowNumber, TestSize.Level0)
{
    NumberMarkManager numberMarkManager;
    EXPECT_EQ(numberMarkManager.IpHeadBarber("10086"), "10086");
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_EqualStr, TestSize.Level0)
{
    NumberMarkManager numberMarkManager;
    EXPECT_EQ(numberMarkManager.IpHeadBarber("12345678901"), "12345678901");
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_EqualLongStr, TestSize.Level0)
{
    NumberMarkManager numberMarkManager;
    EXPECT_EQ(numberMarkManager.IpHeadBarber("1234567890123456"), "1234567890123456");
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_EqualLongEndStr, TestSize.Level0)
{
    NumberMarkManager numberMarkManager;
    EXPECT_EQ(numberMarkManager.IpHeadBarber("1790012345678901"), "12345678901");
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_EmptyStr, TestSize.Level0)
{
    NumberMarkManager numberMarkManager;
    EXPECT_EQ(numberMarkManager.IpHeadBarber("96435"), "");
}

HWTEST_F(NumberMarkAbilityGtest, Telephony_NumberMarkAbility_EqualEndStr, TestSize.Level0)
{
    NumberMarkManager numberMarkManager;
    EXPECT_EQ(numberMarkManager.IpHeadBarber("9643512345678901"), "12345678901");
}

/**
 * @tc.number   APIQuery_ProcessNumber_EmptyInput
 * @tc.name     API query process number empty input
 * @tc.desc     API query process number with empty columns and numbers, expect zero columns and zero result.
 */
HWTEST_F(NumberMarkAbilityGtest, APIQuery_ProcessNumber_EmptyInput, Function | MediumTest | Level1)
{
    Uri uri("datashare:///com.ohos.numbermarkability/api/process_number");
    DataSharePredicates predicates;
    vector<string> numbers = {};
    predicates.In("original_number", numbers);
    DatashareBusinessError businessError;
    vector<string> columns = {};
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_NE(resultSet, nullptr);
    int count;
    vector<string> columnNames;
    EXPECT_EQ(resultSet->GetRowCount(count), E_OK);
    EXPECT_EQ(count, 0);
    EXPECT_EQ(resultSet->GetAllColumnNames(columnNames), E_OK);
    EXPECT_EQ(columnNames.size(), 0);
}

/**
 * @tc.number   APIQuery_ProcessNumber_UnknownColumnName
 * @tc.name     API query process number unknown column name
 * @tc.desc     Process result set cells under unknown column should have null value.
 */
HWTEST_F(NumberMarkAbilityGtest, APIQuery_ProcessNumber_UnknownColumnName, TestSize.Level0)
{
    Uri uri("datashare:///com.ohos.numbermarkability/api/process_number");
    DataSharePredicates predicates;
    vector<string> numbers = { "10086" };
    predicates.In("original_number", numbers);
    DatashareBusinessError businessError;
    vector<string> columns = { "unknown" };
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_NE(resultSet, nullptr);
    int count, columnIndex = -1;
    string stringValue;
    vector<string> columnNames;
    bool isNull;
    EXPECT_EQ(resultSet->GetRowCount(count), E_OK);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(resultSet->GetAllColumnNames(columnNames), E_OK);
    EXPECT_EQ(columnNames, columns);

    EXPECT_EQ(resultSet->GoToRow(0), E_OK);
    EXPECT_EQ(resultSet->GetColumnIndex(columns[0], columnIndex), E_OK);
    EXPECT_EQ(resultSet->IsColumnNull(columnIndex, isNull), E_OK);
    EXPECT_TRUE(isNull);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "");
}

/**
 * @tc.number   APIQuery_ProcessNumber_SingleYellowPageNormalized
 * @tc.name     API query process number single yellow page normalized
 * @tc.desc     Numbers under column `yellow_page_normalized` should be normalized (area code removed).
 */
HWTEST_F(NumberMarkAbilityGtest, APIQuery_ProcessNumber_YellowPageNormalized, TestSize.Level0)
{
    Uri uri("datashare:///com.ohos.numbermarkability/api/process_number");
    DataSharePredicates predicates;
    string number = { "021956011" };
    predicates.EqualTo("original_number", number);
    DatashareBusinessError businessError;
    vector<string> columns = { "original_number", "yellow_page_normalized" };
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_NE(resultSet, nullptr);
    int count, columnIndex = -1;
    string stringValue;
    vector<string> columnNames;
    EXPECT_EQ(resultSet->GetRowCount(count), E_OK);
    EXPECT_EQ(count, 1);
    EXPECT_EQ(resultSet->GetAllColumnNames(columnNames), E_OK);
    EXPECT_EQ(columnNames, columns);

    EXPECT_EQ(resultSet->GoToRow(0), E_OK);
    EXPECT_EQ(resultSet->GetColumnIndex("original_number", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 0);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "021956011");
    EXPECT_EQ(resultSet->GetColumnIndex("yellow_page_normalized", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 1);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "956011");
}

/**
 * @tc.number   APIQuery_ProcessNumber_BatchYellowPageNormalized
 * @tc.name     API query process number batch yellow page normalized
 * @tc.desc     Numbers under column `yellow_page_normalized` should be normalized (area code removed).
 */
HWTEST_F(NumberMarkAbilityGtest, APIQuery_ProcessNumber_BatchYellowPageNormalized, TestSize.Level0)
{
    Uri uri("datashare:///com.ohos.numbermarkability/api/process_number");
    DataSharePredicates predicates;
    vector<string> numbers = { "956011", "021956011" };
    predicates.In("original_number", numbers);
    DatashareBusinessError businessError;
    vector<string> columns = { "original_number", "yellow_page_normalized" };
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_NE(resultSet, nullptr);
    int count, columnIndex = -1;
    string stringValue;
    vector<string> columnNames;
    EXPECT_EQ(resultSet->GetRowCount(count), E_OK);
    EXPECT_EQ(count, numbers.size());
    EXPECT_EQ(resultSet->GetAllColumnNames(columnNames), E_OK);
    EXPECT_EQ(columnNames, columns);

    EXPECT_EQ(resultSet->GoToRow(0), E_OK);
    EXPECT_EQ(resultSet->GetColumnIndex("original_number", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 0);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "956011");
    EXPECT_EQ(resultSet->GetColumnIndex("yellow_page_normalized", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 1);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "956011");

    EXPECT_EQ(resultSet->GoToRow(1), E_OK);
    EXPECT_EQ(resultSet->GetColumnIndex("original_number", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 0);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "021956011");
    EXPECT_EQ(resultSet->GetColumnIndex("yellow_page_normalized", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 1);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "956011");
}

/**
 * @tc.number   APIQuery_ProcessNumber_BatchNumberMarkNormalized
 * @tc.name     API query process number batch number mark normalized
 * @tc.desc     Numbers under column `number_mark_normalized` should be normalized (ip header & country code removed).
 */
HWTEST_F(NumberMarkAbilityGtest, APIQuery_ProcessNumber_BatchNumberMarkNormalized, TestSize.Level0)
{
    Uri uri("datashare:///com.ohos.numbermarkability/api/process_number");
    DataSharePredicates predicates;
    vector<string> numbers = { "+8613512345678", "008613512345678", "13512345678" };
    predicates.In("original_number", numbers);
    DatashareBusinessError businessError;
    vector<string> columns = { "original_number", "number_mark_normalized" };
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_NE(resultSet, nullptr);
    int count, columnIndex = -1;
    string stringValue;
    vector<string> columnNames;
    EXPECT_EQ(resultSet->GetRowCount(count), E_OK);
    EXPECT_EQ(count, numbers.size());
    EXPECT_EQ(resultSet->GetAllColumnNames(columnNames), E_OK);
    EXPECT_EQ(columnNames, columns);

    EXPECT_EQ(resultSet->GoToRow(0), E_OK);
    EXPECT_EQ(resultSet->GetColumnIndex("original_number", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 0);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "+8613512345678");
    EXPECT_EQ(resultSet->GetColumnIndex("number_mark_normalized", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 1);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "13512345678");

    EXPECT_EQ(resultSet->GoToRow(1), E_OK);
    EXPECT_EQ(resultSet->GetColumnIndex("original_number", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 0);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "008613512345678");
    EXPECT_EQ(resultSet->GetColumnIndex("number_mark_normalized", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 1);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "13512345678");

    EXPECT_EQ(resultSet->GoToRow(2), E_OK);
    EXPECT_EQ(resultSet->GetColumnIndex("original_number", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 0);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "13512345678");
    EXPECT_EQ(resultSet->GetColumnIndex("number_mark_normalized", columnIndex), E_OK);
    EXPECT_EQ(columnIndex, 1);
    EXPECT_EQ(resultSet->GetString(columnIndex, stringValue), E_OK);
    EXPECT_EQ(stringValue, "13512345678");
}

/**
 * @tc.number   APIQuery_ProcessNumber_BatchBothNormalized
 * @tc.name     API query process number batch both normalized
 * @tc.desc     Combination of yellow page normalized and number mark normalized in one query.
 */
HWTEST_F(NumberMarkAbilityGtest, APIQuery_ProcessNumber_BatchBothNormalized, Function | MediumTest | Level1)
{
    Uri uri("datashare:///com.ohos.numbermarkability/api/process_number");
    DataSharePredicates predicates;
    vector<string> numbers = { "956011", "021956011", "13512345678", "008613512345678" };
    predicates.In("original_number", numbers);
    DatashareBusinessError businessError;
    vector<string> columns = { "original_number", "yellow_page_normalized", "number_mark_normalized" };
    auto resultSet = ability->Query(uri, predicates, columns, businessError);
    EXPECT_NE(resultSet, nullptr);
    int count;
    string stringValue;
    vector<string> columnNames;
    EXPECT_EQ(resultSet->GetRowCount(count), E_OK);
    EXPECT_EQ(count, numbers.size());
    EXPECT_EQ(resultSet->GetAllColumnNames(columnNames), E_OK);
    EXPECT_EQ(columnNames, columns);

    EXPECT_EQ(resultSet->GoToRow(0), E_OK);
    EXPECT_EQ(resultSet->GetString(0, stringValue), E_OK);
    EXPECT_EQ(stringValue, "956011");
    EXPECT_EQ(resultSet->GetString(1, stringValue), E_OK);
    EXPECT_EQ(stringValue, "956011");
    EXPECT_EQ(resultSet->GetString(2, stringValue), E_OK);
    EXPECT_EQ(stringValue, "956011");

    EXPECT_EQ(resultSet->GoToRow(1), E_OK);
    EXPECT_EQ(resultSet->GetString(0, stringValue), E_OK);
    EXPECT_EQ(stringValue, "021956011");
    EXPECT_EQ(resultSet->GetString(1, stringValue), E_OK);
    EXPECT_EQ(stringValue, "956011");
    EXPECT_EQ(resultSet->GetString(2, stringValue), E_OK);
    EXPECT_EQ(stringValue, "021956011");

    EXPECT_EQ(resultSet->GoToRow(2), E_OK);
    EXPECT_EQ(resultSet->GetString(0, stringValue), E_OK);
    EXPECT_EQ(stringValue, "13512345678");
    EXPECT_EQ(resultSet->GetString(1, stringValue), E_OK);
    EXPECT_EQ(stringValue, "13512345678");
    EXPECT_EQ(resultSet->GetString(2, stringValue), E_OK);
    EXPECT_EQ(stringValue, "13512345678");

    EXPECT_EQ(resultSet->GoToRow(3), E_OK);
    EXPECT_EQ(resultSet->GetString(0, stringValue), E_OK);
    EXPECT_EQ(stringValue, "008613512345678");
    EXPECT_EQ(resultSet->GetString(1, stringValue), E_OK);
    EXPECT_EQ(stringValue, "008613512345678");
    EXPECT_EQ(resultSet->GetString(2, stringValue), E_OK);
    EXPECT_EQ(stringValue, "13512345678");
}

/**
 * @tc.number   VendorQuery_NoVendor
 * @tc.name     Vendor Query No Vendor
 * @tc.desc     Vendor query no vendor should return none result.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_NoneResult, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({});
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_NONE);
}

/**
 * @tc.number   VendorQuery_Single_NoneResult
 * @tc.name     Vendor Query Single None Result
 * @tc.desc     When single vendor returns none result, the ability should return none result.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Single_NoneResult, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_NONE,
        })),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_NONE);
}

/**
 * @tc.number   VendorQuery_Single_ExactResult
 * @tc.name     Vendor Query Single Exact Result
 * @tc.desc     When single vendor returns exact result, the ability should return exact result.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Single_ExactResult, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .markContent = "出租车",
            .isCloud = true,
        })),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.markContent, "出租车");
    EXPECT_EQ(markInfo.isCloud, true);
}

/**
 * @tc.number   VendorQuery_Single_CacheResult
 * @tc.name     Vendor Query Single Cache Result
 * @tc.desc     When single vendor query failed, the ability should return its cached result.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Single_CacheResult, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .markContent = "出租车",
            .isCloud = true,
        })),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.markContent, "出租车");
    EXPECT_EQ(markInfo.isCloud, true);
    NumberMarkInfo markInfo2;
    // next query
    auto ability2 = HookedNumberMarkAbility({
        MockVendor(Result::Fail(), 0, MOCK_SUPPLIER_ID, false),
    });
    auto errCode2 = ability2.QueryByPhoneNumber("13312345678", markInfo2, businessError);
    EXPECT_EQ(errCode2, E_OK);
    // should return cache when failed
    EXPECT_EQ(markInfo2.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo2.markContent, "出租车");
    EXPECT_EQ(markInfo2.isCloud, true);
}

/**
 * @tc.number   VendorQuery_Single_Updated
 * @tc.name     Vendor Query Single Updated
 * @tc.desc     When vendor query result updated, the returned result should be updated.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Single_Updated, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .markContent = "出租车",
            .isCloud = true,
        })),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.markContent, "出租车");
    EXPECT_EQ(markInfo.isCloud, true);
    NumberMarkInfo markInfo2;
    // next query
    auto ability2 = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_EXPRESS,
            .markContent = "快递送餐",
            .isCloud = true,
        }), 100),
    });
    auto errCode2 = ability2.QueryByPhoneNumber("13312345678", markInfo2, businessError);
    EXPECT_EQ(errCode2, E_OK);
    // should return updated
    EXPECT_EQ(markInfo2.markType, MarkType::MARK_TYPE_EXPRESS);
    EXPECT_EQ(markInfo2.markContent, "快递送餐");
    EXPECT_EQ(markInfo2.isCloud, true);
}

/**
 * @tc.number   VendorQuery_Single_Timeout
 * @tc.name     Vendor Query Single Timeout
 * @tc.desc     When vendor query timeout, the cache should be used.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Multiple_Timeout, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .markContent = "出租车",
            .isCloud = true,
        })),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.markContent, "出租车");
    EXPECT_EQ(markInfo.isCloud, true);
    NumberMarkInfo markInfo2;
    // next query
    auto ability2 = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_EXPRESS,
            .markContent = "快递送餐",
            .isCloud = true,
        }), 2500), // timeout
    });
    auto errCode2 = ability2.QueryByPhoneNumber("13312345678", markInfo2, businessError);
    EXPECT_EQ(errCode2, E_OK);
    // should return cache
    EXPECT_EQ(markInfo2.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo2.markContent, "出租车");
    EXPECT_EQ(markInfo2.isCloud, true);
}


/**
 * @tc.number   VendorQuery_Single_NotSetNoCacheResult
 * @tc.name     Vendor Query Single Uninstalled No Cache Result
 * @tc.desc     When vendor is invalidated (settings outdated, or ability connect failed), should not return its cache.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Single_NotSetNoCacheResult, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .markContent = "出租车",
            .isCloud = true,
        })),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.markContent, "出租车");
    EXPECT_EQ(markInfo.isCloud, true);
    NumberMarkInfo markInfo2;
    // next query
    auto ability2 = HookedNumberMarkAbility({
        MockVendor(Result::Fail(), 0, MOCK_SUPPLIER_ID, true),
    });
    auto errCode2 = ability2.QueryByPhoneNumber("13312345678", markInfo2, businessError);
    EXPECT_EQ(errCode2, E_OK);
    // should not return cache when failed because invalidated
    EXPECT_EQ(markInfo2.markType, MarkType::MARK_TYPE_NONE);
}

/**
 * @tc.number   VendorQuery_Single_UninstalledNoCacheResult
 * @tc.name     Vendor Query Single Uninstalled No Cache Result
 * @tc.desc     When single vendor is not present, the ability should not return its cache result.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Single_UninstalledNoCacheResult, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .markContent = "出租车",
            .isCloud = true,
        })),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.markContent, "出租车");
    EXPECT_EQ(markInfo.isCloud, true);
    NumberMarkInfo markInfo2;
    // next query
    auto ability2 = HookedNumberMarkAbility({
        MockVendor(Result::Fail(), 0, "yet.another.vendor"), // only other vendor exist
    });
    auto errCode2 = ability2.QueryByPhoneNumber("13312345678", markInfo2, businessError);
    EXPECT_EQ(errCode2, E_OK);
    // should not return cache when failed because vendor of cache does not exist/uninstalled
    EXPECT_EQ(markInfo2.markType, MarkType::MARK_TYPE_NONE);
}

/**
 * @tc.number   VendorQuery_Multiple_NoRacing_01
 * @tc.name     Vendor Query Multiple No Racing 01
 * @tc.desc     When multiple vendors are present, the result depends on the order, not the speed.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Multiple_NoRacing_01, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto now = GetCurrentTimestamp();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_EXPRESS,
            .markContent = "快递送餐",
            .isCloud = true,
            .markSource = "mock1",
        }), 200, "mock.supplier.no1"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .markContent = "出租车",
            .isCloud = true,
            .markSource = "mock2",
        }), 100, "mock.supplier.no2"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_HOUSE_AGENT,
            .markContent = "房产中介",
            .isCloud = true,
            .markSource = "mock3",
        }), 50, "mock.supplier.no3"),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_GE(GetCurrentTimestamp() - now, 200);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_EXPRESS);
    EXPECT_EQ(markInfo.markContent, "快递送餐");
    EXPECT_EQ(markInfo.isCloud, true);
    EXPECT_EQ(markInfo.markSource, "mock1");
}

/**
 * @tc.number   VendorQuery_Multiple_NoRacing_02
 * @tc.name     Vendor Query Multiple No Racing 02
 * @tc.desc     When multiple vendors are present, the first not none result is accepted.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Multiple_NoRacing_02, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto now = GetCurrentTimestamp();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_NONE,
            .isCloud = true,
            .markSource = "mock1",
        }), 100, "mock.supplier.no1"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_EXPRESS,
            .markContent = "快递送餐",
            .isCloud = true,
            .markSource = "mock2",
        }), 150, "mock.supplier.no2"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .markContent = "出租车",
            .isCloud = true,
            .markSource = "mock3",
        }), 100, "mock.supplier.no3"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_HOUSE_AGENT,
            .markContent = "房产中介",
            .isCloud = true,
            .markSource = "mock4",
        }), 50, "mock.supplier.no4"),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_GE(GetCurrentTimestamp() - now, 150);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_EXPRESS);
    EXPECT_EQ(markInfo.markContent, "快递送餐");
    EXPECT_EQ(markInfo.isCloud, true);
    EXPECT_EQ(markInfo.markSource, "mock2");
}

/**
 * @tc.number   VendorQuery_Multiple_FirstResult
 * @tc.name     Vendor Query Multiple First Result
 * @tc.desc     When multiple vendors are present, the first not none result is accepted.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Multiple_FirstResult, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto now = GetCurrentTimestamp();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_NONE,
            .isCloud = true,
        }), 10, "mock.supplier.no1"),
        MockVendor(Result::Fail(), 10, "mock.supplier.no2"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .markContent = "出租车",
            .isCloud = true,
            .markSource = "mock3",
        }), 10, "mock.supplier.no3"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_EXPRESS,
            .markContent = "快递送餐",
            .isCloud = true,
            .markSource = "mock4",
        }), 10, "mock.supplier.no4"),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_GE(GetCurrentTimestamp() - now, 10);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.markContent, "出租车");
    EXPECT_EQ(markInfo.isCloud, true);
    EXPECT_EQ(markInfo.markSource, "mock3");
}

/**
 * @tc.number   VendorQuery_Multiple_WithFailure_UseFirstNotCache
 * @tc.name     Vendor Query Multiple With Failure Use First Not Cache
 * @tc.desc     When multiple vendors are present, the first not none result is accepted, and failed cache is not used.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Multiple_WithFailure_UseFirstNotCache, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .isCloud = true,
        }), 0, "mock.supplier.no1"),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.isCloud, true);

    NumberMarkInfo markInfo2;
    auto ability2 = HookedNumberMarkAbility({
        MockVendor(Result::Fail(), 10, "mock.supplier.no1"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_EXPRESS,
            .isCloud = true,
        }), 10, "mock.supplier.no2"),
    });
    auto errCode2 = ability2.QueryByPhoneNumber("13312345678", markInfo2, businessError);
    EXPECT_EQ(errCode2, E_OK);
    // should not return cache because no2 succeded
    EXPECT_EQ(markInfo2.markType, MarkType::MARK_TYPE_EXPRESS);
    EXPECT_EQ(markInfo2.isCloud, true);
}

/**
 * @tc.number   VendorQuery_Multiple_AllFailed_UseCache
 * @tc.name     Vendor Query Multiple All Failed Use Cache
 * @tc.desc     When all vendors failed or timeout, if the cache belongs to a valid vendor, will be used.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Multiple_AllFailed_UseCache, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .isCloud = true,
        }), 0, "mock.supplier.no1"),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.isCloud, true);

    NumberMarkInfo markInfo2;
    auto ability2 = HookedNumberMarkAbility({
        MockVendor(Result::Fail(), 10, "mock.supplier.no1"),
        MockVendor(Result::Fail(), 10, "mock.supplier.no2"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_NONE,
        }), 3000, "mock.supplier.no3"),
    });
    auto errCode2 = ability2.QueryByPhoneNumber("13312345678", markInfo2, businessError);
    EXPECT_EQ(errCode2, E_OK);
    // should return cache because all of them failed or timedout
    EXPECT_EQ(markInfo2.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo2.isCloud, true);
}

/**
 * @tc.number   VendorQuery_Multiple_AllFailed_NotUseInvalidCache
 * @tc.name     Vendor Query Multiple All Failed Not Use Invalid Cache
 * @tc.desc     When all vendors failed or timeout, if the cache belongs to an invalid vendor, it won't be used.
 */
HWTEST_F(NumberMarkAbilityGtest, VendorQuery_Multiple_AllFailed_NotUseInvalidCache, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    auto ability = HookedNumberMarkAbility({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_TAXI,
            .isCloud = true,
        }), 0, "mock.supplier.no1"),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = ability.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_TAXI);
    EXPECT_EQ(markInfo.isCloud, true);

    NumberMarkInfo markInfo2;
    auto ability2 = HookedNumberMarkAbility({
        MockVendor(Result::Fail(), 10, "mock.supplier.no1", true),
        MockVendor(Result::Fail(), 10, "mock.supplier.no2"),
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_NONE,
        }), 2500, "mock.supplier.no3"),
    });
    auto errCode2 = ability2.QueryByPhoneNumber("13312345678", markInfo2, businessError);
    EXPECT_EQ(errCode2, E_OK);
    // should return cache because all of them failed or timedout
    EXPECT_EQ(markInfo2.markType, MarkType::MARK_TYPE_NONE);
    EXPECT_EQ(markInfo2.isCloud, true);
}

class CallLogUpdateTestNumberMarkAbility : public NumberMarkAbility {
  public:
    CallLogUpdateTestNumberMarkAbility() : NumberMarkAbility(nullptr) {}
    MOCK_METHOD1(UpdateCallLog, int(const NumberMarkModel &));
    virtual bool ShouldMarkTypeBeReported(MarkType markType) override
    {
        return false;
    }
};

bool operator==(const NumberMarkModel &a, const NumberMarkModel &b)
{
    return a.number == b.number && a.name == b.name && a.classify == b.classify && a.marked_count == b.marked_count &&
           a.is_cloud == b.is_cloud && a.description == b.description && a.save_timestamp == b.save_timestamp &&
           a.supplier == b.supplier && a.supplier_id == b.supplier_id && a.is_intelligent_db == b.is_intelligent_db &&
           true; // for trailing `&&`
}

/**
 * @tc.number   SetNumberMark_CallLogUpdate_Override
 * @tc.name     Set Number Mark Call Log Update Override
 * @tc.desc     When setting local number mark for a number with cloud mark, the call log should be overriden.
 */
HWTEST_F(NumberMarkAbilityGtest, SetNumberMark_CallLogUpdate_Override, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    // Cache
    HookedNumberMarkAbility cacheSteps({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_OTHERS,
            .markContent = "云查结果",
            .markCount = 10,
            .markSource = "mock1",
            .isCloud = true,
        })),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = cacheSteps.QueryByPhoneNumber("13312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_OTHERS);

    // Set local mark
    CallLogUpdateTestNumberMarkAbility setSteps;
    EXPECT_CALL(setSteps, UpdateCallLog(Eq(NumberMarkModel{
        .number = "13312345678",
        .classify = "express",
        .is_cloud = 0,
    })))
        .Times(1)
        .WillOnce(Return(0));
    auto values = DataShare::DataShareValuesBucket({
        { SetNumberMarkParamsFields::phoneNumber, "13312345678" },
        { SetNumberMarkParamsFields::markType, static_cast<long>(MarkType::MARK_TYPE_EXPRESS) },
    });
    errCode = setSteps.SetNumberMark(values);
    EXPECT_EQ(errCode, E_OK);
    vector<NumberMarkModel> localMarks;
    errCode = setSteps.QueryLocalNumberMark("13312345678", localMarks, businessError);
    EXPECT_EQ(errCode, E_OK);
    auto localMark = localMarks[0];
    EXPECT_EQ(localMark.number, "13312345678");
    EXPECT_EQ(localMark.MarkType(), MarkType::MARK_TYPE_OTHERS);
    EXPECT_EQ(localMark.name, "云查结果");
    EXPECT_EQ(localMark.marked_count, 10);
    EXPECT_EQ(localMark.supplier, "mock1");
    EXPECT_EQ(localMark.is_cloud, true);

    // Remove local mark
    CallLogUpdateTestNumberMarkAbility removeSteps;
    EXPECT_CALL(removeSteps, UpdateCallLog(Eq(localMark)))
        .Times(1)
        .WillOnce(Return(0));
    auto values2 = DataShare::DataShareValuesBucket({
        { SetNumberMarkParamsFields::phoneNumber, "13312345678" },
        { SetNumberMarkParamsFields::markType, static_cast<long>(MarkType::MARK_TYPE_NONE) },
    });
    errCode = removeSteps.SetNumberMark(values2);
    EXPECT_EQ(errCode, E_OK);
}

/**
 * @tc.number   SetNumberMark_CallLogUpdate_Override_WithPrefix
 * @tc.name     Set Number Mark Call Log Update Override With Prefix
 * @tc.desc     When setting local number mark for a prefixed number with cloud mark, the test case alse works.
 */
HWTEST_F(NumberMarkAbilityGtest, SetNumberMark_CallLogUpdate_Override_WithPrefix, Function | MediumTest | Level1)
{
    DBUtils::ClearNumberMark();
    // Cache
    HookedNumberMarkAbility cacheSteps({
        MockVendor(Result::Success({
            .markType = MarkType::MARK_TYPE_OTHERS,
            .markContent = "云查结果",
            .markCount = 10,
            .markSource = "mock1",
            .isCloud = true,
        })),
    });
    NumberMarkInfo markInfo;
    DatashareBusinessError businessError;
    auto errCode = cacheSteps.QueryByPhoneNumber("+8613312345678", markInfo, businessError);
    EXPECT_EQ(errCode, E_OK);
    EXPECT_EQ(markInfo.markType, MarkType::MARK_TYPE_OTHERS);

    // Set local mark
    CallLogUpdateTestNumberMarkAbility setSteps;
    EXPECT_CALL(setSteps, UpdateCallLog(Eq(NumberMarkModel{
        .number = "+8613312345678",
        .classify = "express",
        .is_cloud = 0,
    })))
        .Times(1)
        .WillOnce(Return(0));
    auto values = DataShare::DataShareValuesBucket({
        { SetNumberMarkParamsFields::phoneNumber, "+8613312345678" },
        { SetNumberMarkParamsFields::markType, static_cast<long>(MarkType::MARK_TYPE_EXPRESS) },
    });
    errCode = setSteps.SetNumberMark(values);
    EXPECT_EQ(errCode, E_OK);
    vector<NumberMarkModel> localMarks;
    errCode = setSteps.QueryLocalNumberMark("13312345678", localMarks, businessError);
    EXPECT_EQ(errCode, E_OK);
    auto localMark = localMarks[0];
    EXPECT_EQ(localMark.number, "13312345678");
    EXPECT_EQ(localMark.MarkType(), MarkType::MARK_TYPE_OTHERS);
    EXPECT_EQ(localMark.name, "云查结果");
    EXPECT_EQ(localMark.marked_count, 10);
    EXPECT_EQ(localMark.supplier, "mock1");
    EXPECT_EQ(localMark.is_cloud, true);

    // Remove local mark
    CallLogUpdateTestNumberMarkAbility removeSteps;
    auto originalMark = localMark;
    originalMark.number = "+8613312345678";
    EXPECT_CALL(removeSteps, UpdateCallLog(Eq(originalMark)))
        .Times(1)
        .WillOnce(Return(0));
    auto values2 = DataShare::DataShareValuesBucket({
        { SetNumberMarkParamsFields::phoneNumber, "+8613312345678" },
        { SetNumberMarkParamsFields::markType, static_cast<long>(MarkType::MARK_TYPE_NONE) },
    });
    errCode = removeSteps.SetNumberMark(values2);
    EXPECT_EQ(errCode, E_OK);
}

/**
 * @tc.number   Update_Intelligent_DB_Update_Timestamp_Long
 * @tc.name     Update Intelligent DB Update Timestamp Long
 * @tc.desc     When importing intelligent db file, the timestamp in properties should be updated. Using long input.
 */
HWTEST_F(NumberMarkAbilityGtest, Update_Intelligent_DB_Update_Timestamp_Long, Function | MediumTest | Level1)
{
    DBUtils::ClearProperties();
    Uri uri("datashare:///com.ohos.numbermarkability/intelligent_db_update_timestamp");
    DataSharePredicates predicates;
    auto values = DataShareValuesBucket({
        { "update_timestamp", 1732107653534L },
    });
    ability->Update(uri, predicates, values);
    auto db = NumberIdentityDatabase::GetInstance();
    string value;
    db->GetProperty("intelligent_db_update_timestamp", value);
    EXPECT_EQ(value, "1732107653534");
}

/**
 * @tc.number   Update_Intelligent_DB_Update_Timestamp_Double
 * @tc.name     Update Intelligent DB Update Timestamp Double
 * @tc.desc     When importing intelligent db file, the timestamp in properties should be updated. Using double input.
 */
HWTEST_F(NumberMarkAbilityGtest, Update_Intelligent_DB_Update_Timestamp_Double, Function | MediumTest | Level1)
{
    DBUtils::ClearProperties();
    Uri uri("datashare:///com.ohos.numbermarkability/intelligent_db_update_timestamp");
    DataSharePredicates predicates;
    auto values = DataShareValuesBucket({
        { "update_timestamp", 1732107653534.9 },
    });
    ability->Update(uri, predicates, values);
    auto db = NumberIdentityDatabase::GetInstance();
    string value;
    db->GetProperty("intelligent_db_update_timestamp", value);
    EXPECT_EQ(value, "1732107653534");
}

} // namespace Telephony
} // namespace OHOS
