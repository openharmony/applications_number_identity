/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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
#define private public
#define protected public

#include <gtest/gtest.h>

#include "event_handler.h"
#include "fix_phone_number.h"
#include "mobile_phone_number.h"
#include "number_identity_datashare_stub_impl.h"
#include "number_identity_errors.h"
#include "number_identity_log_wrapper.h"
#include "number_identity_rdb_helper.h"
#include "number_location_ability.h"
#include "number_location_db_parse.h"
#include "number_location_manager.h"
#include "number_location_result_set_bridge.h"
#include "number_location_utils.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;

const int32_t MOBILE_OPERATOR_LENGTH = 50;
const int32_t MAXCITYNAME_UNICODE_BUF = 22;
const int32_t MAXCITYNAME_UTF8_BUF = 32;
const int32_t MAX_MOBILE_OP_NAME_LEN = 40;
constexpr int32_t BUFFER_LENGTH = 1024 * 2;
constexpr int8_t NUMBER_0 = 0;
constexpr int8_t NUMBER_1 = 1;
constexpr int8_t NUMBER_2 = 2;
constexpr int8_t NUMBER_9 = 9;
const char *g_path = "/system/etc/telephony/numberlocation.dat";
class DemoHandler : public AppExecFwk::EventHandler {
  public:
    explicit DemoHandler(std::shared_ptr<AppExecFwk::EventRunner> &eventRunner) : AppExecFwk::EventHandler(eventRunner)
    {
    }
    virtual ~DemoHandler() {}
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) {}
};

class BridgeWriter final : public DataShare::ResultSetBridge::Writer {
  public:
    int AllocRow() override;
    int FreeLastRow() override;
    int Write(uint32_t column) override;
    int Write(uint32_t column, int64_t value) override;
    int Write(uint32_t column, double value) override;
    int Write(uint32_t column, const uint8_t *value, size_t size) override;
    int Write(uint32_t column, const char *value, size_t size) override;
    void SetAllocRowStatue(int status);
    DistributedKv::Key GetKey() const;
    DistributedKv::Value GetValue() const;

  private:
    int allocStatus_ = DataShare::E_OK;
    std::vector<uint8_t> key_;
    std::vector<uint8_t> value_;
};

void BridgeWriter::SetAllocRowStatue(int status)
{
    allocStatus_ = status;
}

DistributedKv::Key BridgeWriter::GetKey() const
{
    return key_;
}

DistributedKv::Value BridgeWriter::GetValue() const
{
    return value_;
}

int BridgeWriter::AllocRow()
{
    return allocStatus_;
}

int BridgeWriter::FreeLastRow()
{
    return DataShare::E_OK;
}

int BridgeWriter::Write(uint32_t column)
{
    return DataShare::E_OK;
}

int BridgeWriter::Write(uint32_t column, int64_t value)
{
    return DataShare::E_OK;
}

int BridgeWriter::Write(uint32_t column, double value)
{
    return DataShare::E_OK;
}

int BridgeWriter::Write(uint32_t column, const uint8_t *value, size_t size)
{
    return DataShare::E_OK;
}

int BridgeWriter::Write(uint32_t column, const char *value, size_t size)
{
    if (column < 0 || column > 1 || value == nullptr) {
        return DataShare::E_ERROR;
    }
    auto vec = std::vector<uint8_t>(value, value + size - 1);
    if (column == 0) {
        key_.insert(key_.end(), vec.begin(), vec.end());
    } else {
        value_.insert(value_.end(), vec.begin(), vec.end());
    }
    return DataShare::E_OK;
}

class BranchTest : public testing::Test {
  public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void BranchTest::SetUp() {}

void BranchTest::TearDown() {}

void BranchTest::SetUpTestCase() {}

void BranchTest::TearDownTestCase() {}

/**
 * @tc.number   Telephony_NumberIdentityResultSetBridge_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberIdentityResultSetBridge_001, TestSize.Level0)
{
    std::vector<std::string> numberLocations;
    std::shared_ptr<NumberLocationResultSetBridge> resultSet =
        std::make_shared<NumberLocationResultSetBridge>(numberLocations);
    int32_t start = -1;
    int32_t target = 0;
    BridgeWriter writer;
    EXPECT_EQ(resultSet->OnGo(start, target, writer), -1);
    start = 0;
    target = -1;
    EXPECT_EQ(resultSet->OnGo(start, target, writer), -1);
    start = 1;
    target = 0;
    EXPECT_EQ(resultSet->OnGo(start, target, writer), -1);
}

/**
 * @tc.number   Telephony_NumberLocationManager_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationManager_001, TestSize.Level0)
{
    auto numberLocationManager = DelayedSingleton<NumberLocationManager>::GetInstance();
    std::string phoneNumber = "";
    std::string currentIso = "";
    EXPECT_TRUE(numberLocationManager->GetNumberLocationDefault(phoneNumber, currentIso, false).empty());
    currentIso = numberLocationManager->GetCountryIso();
    EXPECT_TRUE(numberLocationManager->GetNumberLocation(phoneNumber, false, currentIso).empty());
    phoneNumber = "13219730029";
    EXPECT_FALSE(numberLocationManager->GetNumberLocation(phoneNumber, false, currentIso).empty());
    phoneNumber = "07712193061";
    EXPECT_FALSE(numberLocationManager->GetNumberLocation(phoneNumber, false, currentIso).empty());
    phoneNumber = "";
    EXPECT_TRUE(numberLocationManager->GetNatAttributionInfo(phoneNumber, currentIso).empty());
    EXPECT_TRUE(numberLocationManager->IpHeadBarber(phoneNumber).empty());
    EXPECT_TRUE(numberLocationManager->GetAreaCode(phoneNumber).empty());
    EXPECT_FALSE(numberLocationManager->GetCountryIso().empty());
    numberLocationManager->GetCountryIsoFromSim();
    numberLocationManager->GetCountryIsoFromLocal();
    numberLocationManager->DeleteInternationalPrefix(phoneNumber, currentIso);
    phoneNumber = "12345";
    numberLocationManager->DeleteInternationalPrefix(phoneNumber, currentIso);
    numberLocationManager->GetNumberLocationNotCn(phoneNumber, currentIso, false);
    phoneNumber = "123456789";
    numberLocationManager->GetNumberLocationNotCn(phoneNumber, currentIso, false);
    phoneNumber = "+861234567";
    numberLocationManager->GetNumberLocationNotCn(phoneNumber, currentIso, false);
    numberLocationManager->GetAreaCode(phoneNumber);
    phoneNumber = "00861234567";
    numberLocationManager->GetNumberLocationNotCn(phoneNumber, currentIso, false);
    currentIso = "CN";
    numberLocationManager->DeleteInternationalPrefix(phoneNumber, currentIso);
    currentIso = "HK";
    numberLocationManager->DeleteInternationalPrefix(phoneNumber, currentIso);
    phoneNumber = "0011234567";
    numberLocationManager->DeleteInternationalPrefix(phoneNumber, currentIso);
    numberLocationManager->GetAreaCode(phoneNumber);
    phoneNumber = "17900";
    numberLocationManager->GetAreaCode(phoneNumber);
    phoneNumber = "1790012345";
    numberLocationManager->GetAreaCode(phoneNumber);
    phoneNumber = "+8600";
    EXPECT_TRUE(numberLocationManager->GetNatAttributionInfo(phoneNumber, currentIso).empty());
}

/**
 * @tc.number   Telephony_NumberLocationUtils_ThreeNumber
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationUtils_ThreeNumber, TestSize.Level0)
{
    std::unique_ptr<NumberLocationUtils> numberLocationUtils = std::make_unique<NumberLocationUtils>();
    std::string phoneNumber = "010";
    EXPECT_FALSE(numberLocationUtils->QueryUnicodeInformationByTelNum(phoneNumber).empty());
}

/**
 * @tc.number   Telephony_NumberLocationUtils_EmptyNumber
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationUtils_EmptyNumber, TestSize.Level0)
{
    std::unique_ptr<NumberLocationUtils> numberLocationUtils = std::make_unique<NumberLocationUtils>();
    std::string phoneNumber = "";
    EXPECT_TRUE(numberLocationUtils->QueryUnicodeOPNamebyPhoneNumber(phoneNumber).empty());
    EXPECT_TRUE(numberLocationUtils->QueryUnicodeAreaCodeByPhoneNum(phoneNumber).empty());
    EXPECT_TRUE(numberLocationUtils->QueryUnicodeInformationByPhoneNum(phoneNumber).empty());
    EXPECT_TRUE(numberLocationUtils->QueryUnicodeInformationByTelNum(phoneNumber).empty());
}

/**
 * @tc.number   Telephony_NumberLocationUtils_ContinuumNumber
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationUtils_ContinuumNumber, TestSize.Level0)
{
    std::unique_ptr<NumberLocationUtils> numberLocationUtils = std::make_unique<NumberLocationUtils>();
    std::string phoneNumber = "123456";
    EXPECT_TRUE(numberLocationUtils->QueryUnicodeOPNamebyPhoneNumber(phoneNumber).empty());
    EXPECT_TRUE(numberLocationUtils->QueryUnicodeAreaCodeByPhoneNum(phoneNumber).empty());
    EXPECT_TRUE(numberLocationUtils->QueryUnicodeInformationByPhoneNum(phoneNumber).empty());
    EXPECT_TRUE(numberLocationUtils->QueryUnicodeInformationByTelNum(phoneNumber).empty());
}


/**
 * @tc.number   Telephony_NumberLocationUtils_RandomNumber
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationUtils_RandomNumber, TestSize.Level0)
{
    std::unique_ptr<NumberLocationUtils> numberLocationUtils = std::make_unique<NumberLocationUtils>();
    std::string phoneNumber = "1701234";
    EXPECT_FALSE(numberLocationUtils->QueryUnicodeOPNamebyPhoneNumber(phoneNumber).empty());
    EXPECT_FALSE(numberLocationUtils->QueryUnicodeInformationByPhoneNum(phoneNumber).empty());
}

/**
 * @tc.number   Telephony_NumberLocationUtils_LongRandomNumber
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationUtils_LongRandomNumber, TestSize.Level0)
{
    std::unique_ptr<NumberLocationUtils> numberLocationUtils = std::make_unique<NumberLocationUtils>();
    std::string phoneNumber = "131123456";
    EXPECT_FALSE(numberLocationUtils->QueryUnicodeAreaCodeByPhoneNum(phoneNumber).empty());
}

/**
 * @tc.number   Telephony_NumberLocationAbility_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationAbility_001, TestSize.Level0)
{
    std::shared_ptr<NumberLocationAbility> numberLocationAbility = std::make_shared<NumberLocationAbility>();
    OHOS::Uri uri("datashare:///com.ohos.numberlocationability");
    AppExecFwk::Want want;
    DataShare::DataSharePredicates predicates;
    DataShare::DataShareValuesBucket valuesBucket;
    EXPECT_EQ(numberLocationAbility->Delete(uri, predicates), NUMBER_IDENTITY_ERROR);
    EXPECT_EQ(numberLocationAbility->Insert(uri, valuesBucket), NUMBER_IDENTITY_ERROR);
    EXPECT_EQ(numberLocationAbility->Update(uri, predicates, valuesBucket), NUMBER_IDENTITY_ERROR);
}

/**
 * @tc.number   Telephony_NumberLocationDbParse_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationDbParse_001, TestSize.Level0)
{
    std::unique_ptr<NumberLocationDbParse> numberLocationDbParse = std::make_unique<NumberLocationDbParse>();
    FILE *fp = nullptr;
    if ((fp = fopen(g_path, "rb")) != nullptr) {
        char blockBuffer[BUFFER_LENGTH];
        EXPECT_EQ(numberLocationDbParse->ReadBlockConfigToBuffer(fp, blockBuffer, -1), NUMBER_IDENTITY_ERR_MEMSET_FAIL);
        EXPECT_NE(
            numberLocationDbParse->ReadBlockConfigToBuffer(fp, blockBuffer, MOBILE_OPERATOR_LENGTH), NUMBER_IDENTITY_ERROR);
        numberLocationDbParse->ReadBlockById(nullptr, 0, nullptr);
        std::string number = "";
        char mobileOpname[MOBILE_OPERATOR_LENGTH];
        numberLocationDbParse->GetMobileOperator(fp, number.c_str(), mobileOpname);
        numberLocationDbParse->GetMobileOperator(fp, "321", mobileOpname);
        numberLocationDbParse->GetMobileOperator(fp, "123", mobileOpname);
        numberLocationDbParse->GetMobileOperator(fp, "123456", mobileOpname);
        int index = 0;
        EXPECT_NE(numberLocationDbParse->GetPrefixIndex(fp, blockBuffer, "131123456", index), NUMBER_IDENTITY_ERROR);
        EXPECT_EQ(numberLocationDbParse->GetPrefixIndex(fp, blockBuffer, "123456", index), NUMBER_IDENTITY_ERROR);
        EXPECT_EQ(numberLocationDbParse->GetPrefixIndex(fp, blockBuffer, "", index), NUMBER_IDENTITY_ERROR);
        fclose(fp);
        EXPECT_EQ(numberLocationDbParse->GetPrefixIndex(fp, blockBuffer, nullptr, index), NUMBER_IDENTITY_ERROR);
        int32_t prefixIndex = -1;
        int32_t phonNumIndex = 6;
        short cityIndex;
        char outData[NUMBER_9];
        numberLocationDbParse->GetCityName(fp, blockBuffer, prefixIndex, (unsigned char *)outData);
        numberLocationDbParse->GetCityIndexByPhoneNum(fp, blockBuffer, phonNumIndex, prefixIndex, &cityIndex);
        numberLocationDbParse->ReadPhoneNumberPrefixList(fp, blockBuffer, NUMBER_9, outData);
        numberLocationDbParse->ReadMobileOpFlagPos(fp);
        numberLocationDbParse->ReadMobileOpIndexPos(fp);
        numberLocationDbParse->ReadMobileOpDataPos(fp);
        numberLocationDbParse->ReadMobileOpNamesPos(fp);
        char tempOpName[MAX_MOBILE_OP_NAME_LEN];
        int32_t pos = 0;
        short mobileOpNameIndex = 10000;
        numberLocationDbParse->ReadMaxQuHaoNum(fp, blockBuffer, BUFFER_LENGTH);
        EXPECT_EQ(numberLocationDbParse->ReadOpIndexAndOpNamePos(fp, tempOpName, "123456", pos, mobileOpNameIndex),
            NUMBER_IDENTITY_ERROR);
        EXPECT_EQ(numberLocationDbParse->ReadOpIndexAndOpNamePos(fp, tempOpName, "1234567", pos, mobileOpNameIndex),
            NUMBER_IDENTITY_ERR_SUCCESS);
        char quHaoData[] = "123456";
        int32_t readSize = 1;
        EXPECT_EQ(numberLocationDbParse->GetCityIndexByQuHao(
                    fp, blockBuffer, (unsigned char *)quHaoData, pos, readSize, (unsigned char *)outData),
            NUMBER_IDENTITY_ERROR);
    }
}

/**
 * @tc.number   Telephony_NumberLocationDbParse_002
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationDbParse_002, TestSize.Level0)
{
    auto numberLocationDbParse = std::make_unique<NumberLocationDbParse>();
    if (numberLocationDbParse != nullptr) {
        wchar_t wText[NUMBER_9] = { NUMBER_0 };
        numberLocationDbParse->GetEndNum(wText, NUMBER_9);
        numberLocationDbParse->GetActualNum(wText, NUMBER_1);
        wText[NUMBER_9 - 1] = NUMBER_1;
        numberLocationDbParse->GetEndNum(wText, NUMBER_9);
        wText[0] = NUMBER_1;
        numberLocationDbParse->GetActualNum(wText, NUMBER_1);
        char data[MOBILE_OPERATOR_LENGTH];
        BlockMapStr blockMapStr;
        numberLocationDbParse->ReadBlockById(data, 0, &blockMapStr);
        numberLocationDbParse->GetMobileOperator(nullptr, nullptr, nullptr);
        char mobileOpName[MOBILE_OPERATOR_LENGTH];
        numberLocationDbParse->GetMobileOperator(nullptr, nullptr, mobileOpName);
        char phoneNumber[NUMBER_2];
        phoneNumber[NUMBER_0] = '\0';
        numberLocationDbParse->GetMobileOperator(nullptr, phoneNumber, mobileOpName);
        phoneNumber[NUMBER_0] = '2';
        phoneNumber[NUMBER_1] = '\0';
        numberLocationDbParse->GetMobileOperator(nullptr, phoneNumber, mobileOpName);
        phoneNumber[NUMBER_0] = '1';
        phoneNumber[NUMBER_1] = '\0';
        numberLocationDbParse->GetMobileOperator(nullptr, phoneNumber, mobileOpName);
        char phoneNumber2[NUMBER_9] = { '1' };
        phoneNumber2[NUMBER_9 - 1] = '\0';
        numberLocationDbParse->GetMobileOperator(nullptr, phoneNumber2, mobileOpName);
        EXPECT_TRUE(numberLocationDbParse->QueryAreaCodebyPhoneNumber("1234567").empty());
    }
}

/**
 * @tc.number   Telephony_NumberLocationDbParse_003
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberLocationDbParse_003, TestSize.Level0)
{
    auto numberLocationDbParse = std::make_unique<NumberLocationDbParse>();
    if (numberLocationDbParse != nullptr) {
            char pOut[NUMBER_9];
            wchar_t wText[NUMBER_9] = { NUMBER_0 };
            numberLocationDbParse->UnicodeToUTF_8(pOut, wText);
            char cityNameUnicode[MAXCITYNAME_UNICODE_BUF];
            char cityNameUTF8[MAXCITYNAME_UTF8_BUF];
            numberLocationDbParse->TransformUnicodeToUTF8((char *)cityNameUnicode, (char *)cityNameUTF8, MAXCITYNAME_UTF8_BUF);
            int index = 0;
            int32_t readSize = 1;
            char ptTmpEq[] = "123456";
            char quHaoData[] = "123456";
            int findFlag = 0;
            char ptQuHaoList[] = "123456";
            numberLocationDbParse->GetCityIndexInner(
                index, readSize, (char *)ptTmpEq, (unsigned char *)quHaoData, findFlag, (char *)ptQuHaoList);
            char ptTmpNe[] = "122222";
            numberLocationDbParse->GetCityIndexInner(
                index, readSize, (char *)ptTmpNe, (unsigned char *)quHaoData, findFlag, (char *)ptQuHaoList);
            char quHaoDataEnd[] = "123";
            numberLocationDbParse->GetCityIndexInner(
                index, readSize, (char *)ptTmpNe, (unsigned char *)quHaoDataEnd, findFlag, (char *)ptQuHaoList);
            EXPECT_NE(numberLocationDbParse->QueryPhoneNumberLocation("13456"), "");
            EXPECT_EQ(numberLocationDbParse->QueryPhoneNumberLocation("123456"), "");
            EXPECT_EQ(numberLocationDbParse->QueryPhoneNumberLocation("1234567654321"), "");
            EXPECT_NE(numberLocationDbParse->QueryOpNamebyPhoneNumber("123456"), "");
            EXPECT_EQ(numberLocationDbParse->QueryTelNumberLocation("123456"), "");
            EXPECT_EQ(numberLocationDbParse->QueryTelNumberLocation("12"), "");
            EXPECT_NE(numberLocationDbParse->QueryTelNumberLocation("010"), "");
    }
}

/**
 * @tc.number   Telephony_FixPhoneNumber_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_FixPhoneNumber_001, TestSize.Level0)
{
    auto fixPhoneNumber = std::make_unique<FixPhoneNumber>("");
    if (fixPhoneNumber != nullptr) {
        EXPECT_EQ(fixPhoneNumber->GetFixParseResult(), "");
        EXPECT_EQ(fixPhoneNumber->ParseFixPhoneNumber(), NUMBER_IDENTITY_ERROR);
        fixPhoneNumber->fixPhoneNumber_ = "123";
        EXPECT_EQ(fixPhoneNumber->ParseFixPhoneNumber(), NUMBER_IDENTITY_ERROR);
        fixPhoneNumber->fixPhoneNumber_ = "123456";
        EXPECT_EQ(fixPhoneNumber->GetFixParseResult(), "");
        EXPECT_EQ(fixPhoneNumber->ParseFixPhoneNumber(), NUMBER_IDENTITY_ERR_SUCCESS);
        fixPhoneNumber->fixPhoneNumber_ = "012345";
        EXPECT_EQ(fixPhoneNumber->ParseFixPhoneNumber(), NUMBER_IDENTITY_ERR_SUCCESS);
        fixPhoneNumber->fixPhoneNumber_ = "023456";
        EXPECT_EQ(fixPhoneNumber->ParseFixPhoneNumber(), NUMBER_IDENTITY_ERR_SUCCESS);
    }
}

/**
 * @tc.number   Telephony_NumberIdentityDataShareStubImpl_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_NumberIdentityDataShareStubImpl_001, TestSize.Level0)
{
    auto numberIdentityDataShareStubImpl = std::make_unique<DataShare::NumberIdentityDataShareStubImpl>();
    if (numberIdentityDataShareStubImpl != nullptr) {
        Uri uri("\nullptr");
        DataShare::DataSharePredicates predicates;
        std::vector<std::string> columns;
        DataShare::DatashareBusinessError businessError;
        numberIdentityDataShareStubImpl->Query(uri, predicates, columns, businessError);
        EXPECT_TRUE(numberIdentityDataShareStubImpl->GetOwner(uri) == nullptr);
        DataShare::DataShareValuesBucket valuesBucket;
        EXPECT_EQ(numberIdentityDataShareStubImpl->Insert(uri, valuesBucket), NUMBER_IDENTITY_ERROR);
        EXPECT_EQ(numberIdentityDataShareStubImpl->Update(uri, predicates, valuesBucket), NUMBER_IDENTITY_ERROR);
        EXPECT_EQ(numberIdentityDataShareStubImpl->Delete(uri, predicates), NO_ROW_AFFECTED);
        std::vector<DataShare::DataShareValuesBucket> values;
        EXPECT_EQ(numberIdentityDataShareStubImpl->BatchInsert(uri, values), NUMBER_IDENTITY_ERR_SUCCESS);
        std::shared_ptr<DataShare::DataShareExtAbility> extension = std::make_shared<DataShare::DataShareExtAbility>();
        numberIdentityDataShareStubImpl->SetNumberLocationAbility(extension);
        EXPECT_NE(numberIdentityDataShareStubImpl->GetNumberLocationAbility(), nullptr);
        numberIdentityDataShareStubImpl->GetFileTypes(uri, "");
        EXPECT_EQ(numberIdentityDataShareStubImpl->OpenFile(uri, ""), -1);
        EXPECT_EQ(numberIdentityDataShareStubImpl->OpenRawFile(uri, ""), -1);
        EXPECT_EQ(numberIdentityDataShareStubImpl->GetType(uri), "");
        EXPECT_FALSE(numberIdentityDataShareStubImpl->NotifyChange(uri));
        EXPECT_EQ(numberIdentityDataShareStubImpl->NormalizeUri(uri), uri);
        EXPECT_EQ(numberIdentityDataShareStubImpl->DenormalizeUri(uri), uri);
        numberIdentityDataShareStubImpl->SetDownloadFileAbility(extension);
        numberIdentityDataShareStubImpl->GetDownloadFileAbility();
        numberIdentityDataShareStubImpl->SetNumberMarkAbility(extension);
        numberIdentityDataShareStubImpl->GetNumberMarkAbility();
    }
}

/**
 * @tc.number   Telephony_MobilePhoneNumber_001
 * @tc.name     test error branch
 * @tc.desc     Function test
 */
HWTEST_F(BranchTest, Telephony_MobilePhoneNumber_001, TestSize.Level0)
{
    auto mobilePhoneNumber = std::make_unique<MobilePhoneNumber>("");
    if (mobilePhoneNumber != nullptr) {
        mobilePhoneNumber->GetPhoneOperator();
        EXPECT_EQ(mobilePhoneNumber->GetMobileParseResult(), "");
        EXPECT_EQ(mobilePhoneNumber->ParseMobilePhoneNumber(), NUMBER_IDENTITY_ERROR);
        mobilePhoneNumber->GetLocation();
        mobilePhoneNumber->ndc3String_ = "133";
        mobilePhoneNumber->GetPhoneOperator();
        mobilePhoneNumber->ndc3String_ = "170";
        mobilePhoneNumber->ndc7String_ = "123456";
        mobilePhoneNumber->GetPhoneOperator();
        mobilePhoneNumber->mobilePhoneNumber_ = "1234567";
        mobilePhoneNumber->GetMobileParseResult();
        mobilePhoneNumber->mobilePhoneNumber_ = "1701234";
        mobilePhoneNumber->GetMobileParseResult();
        mobilePhoneNumber->mobilePhoneNumber_ = "008612";
        EXPECT_EQ(mobilePhoneNumber->ParseMobilePhoneNumber(), NUMBER_IDENTITY_ERROR);
        mobilePhoneNumber->mobilePhoneNumber_ = "00861234567";
        EXPECT_EQ(mobilePhoneNumber->ParseMobilePhoneNumber(), NUMBER_IDENTITY_ERR_SUCCESS);
        mobilePhoneNumber->mobilePhoneNumber_ = "1234567";
        EXPECT_EQ(mobilePhoneNumber->ParseMobilePhoneNumber(), NUMBER_IDENTITY_ERR_SUCCESS);
        mobilePhoneNumber->mobilePhoneNumber_ = "8612345678";
        EXPECT_EQ(mobilePhoneNumber->ParseMobilePhoneNumber(), NUMBER_IDENTITY_ERR_SUCCESS);
        mobilePhoneNumber->mobilePhoneNumber_ = "+861234567";
        EXPECT_EQ(mobilePhoneNumber->ParseMobilePhoneNumber(), NUMBER_IDENTITY_ERR_SUCCESS);
    }
}
} // namespace Telephony
} // namespace OHOS
