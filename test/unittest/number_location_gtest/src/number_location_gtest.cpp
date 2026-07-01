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
#include "number_location_gtest.h"

#include <gtest/gtest.h>
#include <cstring>
#include <string>
#include "datashare_predicates.h"
#include "number_location_ability.h"
#include "number_identity_log_wrapper.h"
#include "uri.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
#ifndef TEL_TEST_UNSUPPORT

void NumberLocationGtest::SetUp() {}

void NumberLocationGtest::TearDown() {}

void NumberLocationGtest::SetUpTestCase() {}

void NumberLocationGtest::TearDownTestCase() {}

std::string NumberLocationGtest::GeneratingNumber(int32_t number)
{
    std::string phoneNumber;
    for (int i = 0; i < number; i++) {
        phoneNumber += std::to_string(rand() % DECIMAL_NUMBER);
    }
    return phoneNumber;
}

/********************************************* Test GetNumberLocation()***********************************************/
/**
 * @tc.number   Telephony_NumberLocation_GetNumberLocation_0100
 * @tc.name     get the number location of the mobile phone number.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(NumberLocationGtest, Telephony_NumberLocation_GetNumberLocation_0100, Function | MediumTest | Level1)
{
    int number = 11;
    std::string phoneNumber = GeneratingNumber(number);
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    std::string isExactMatchStr = "true";
    columns.push_back(isExactMatchStr);
    std::vector<std::string> phoneNumbers;
    phoneNumbers.push_back(phoneNumber);
    predicates.SetWhereArgs(phoneNumbers);
    DataShare::DatashareBusinessError businessError;
    std::shared_ptr<NumberLocationAbility> numberLocationAbility = std::make_shared<NumberLocationAbility>();
    OHOS::Uri uri("datashare:///com.ohos.numberlocationability");
    auto resultSet = numberLocationAbility->Query(uri, predicates, columns, businessError);
    if (resultSet != nullptr) {
        NUMBER_IDENTITY_LOGE("resultSet is not nullptr!");
        resultSet->GoToFirstRow();
        int columnIndex = 0;
        resultSet->GetColumnIndex(COLUMN_NUMBER_LOCATION, columnIndex);
        std::string numberLocation = "";
        resultSet->GetString(columnIndex, numberLocation);
        resultSet->Close();
        EXPECT_TRUE(numberLocation.empty());
    }
}

/**
 * @tc.number   Telephony_NumberLocation_GetNumberLocation_0200
 * @tc.name     get the number location of the prefix mobile phone number
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(NumberLocationGtest, Telephony_NumberLocation_GetNumberLocation_0200, Function | MediumTest | Level1)
{
    std::string phoneNumber = "156";
    DataShare::DataSharePredicates predicates;
    std::vector<std::string> columns;
    std::string isExactMatchStr = "false";
    columns.push_back(isExactMatchStr);
    std::vector<std::string> phoneNumbers;
    phoneNumbers.push_back(phoneNumber);
    predicates.SetWhereArgs(phoneNumbers);
    DataShare::DatashareBusinessError businessError;
    std::shared_ptr<NumberLocationAbility> numberLocationAbility = std::make_shared<NumberLocationAbility>();
    OHOS::Uri uri("datashare:///com.ohos.numberlocationability");
    auto resultSet = numberLocationAbility->Query(uri, predicates, columns, businessError);
    if (resultSet != nullptr) {
        NUMBER_IDENTITY_LOGE("resultSet is not nullptr!");
        resultSet->GoToFirstRow();
        int columnIndex = 0;
        resultSet->GetColumnIndex(COLUMN_NUMBER_LOCATION, columnIndex);
        std::string numberLocation = "";
        resultSet->GetString(columnIndex, numberLocation);
        resultSet->Close();
        EXPECT_FALSE(numberLocation.empty());
    }
}

/**
 * @tc.number   Telephony_NumberLocation_GetNumberLocation_0300
 * @tc.name     get the geo number location of the fix phone number.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(NumberLocationGtest, Telephony_NumberLocation_GetNumberLocation_0300, Function | MediumTest | Level1)
{
    int number = 8;
    std::string phoneNumber = GeneratingNumber(number);
    DataShare::DataSharePredicates predicates;
    std::string isExactMatchStr = "true";
    std::vector<std::string> columns;
    columns.push_back(isExactMatchStr);
    std::vector<std::string> phoneNumbers;
    phoneNumbers.push_back(phoneNumber);
    predicates.SetWhereArgs(phoneNumbers);
    DataShare::DatashareBusinessError businessError;
    std::shared_ptr<NumberLocationAbility> numberLocationAbility = std::make_shared<NumberLocationAbility>();
    OHOS::Uri uri("datashare:///com.ohos.numberlocationability");
    auto resultSet = numberLocationAbility->Query(uri, predicates, columns, businessError);
    if (resultSet != nullptr) {
        NUMBER_IDENTITY_LOGE("resultSet is not nullptr!");
        resultSet->GoToFirstRow();
        int columnIndex = 0;
        resultSet->GetColumnIndex(COLUMN_NUMBER_LOCATION, columnIndex);
        std::string numberLocation = "";
        resultSet->GetString(columnIndex, numberLocation);
        resultSet->Close();
        EXPECT_TRUE(numberLocation.empty());
    }
}

/**
 * @tc.number   Telephony_NumberLocation_GetNumberLocation_0400
 * @tc.name     get the number location of the prefix mobile phone number.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(NumberLocationGtest, Telephony_NumberLocation_GetNumberLocation_0400, Function | MediumTest | Level1)
{
    std::string phoneNumber1 = "188";
    std::string phoneNumber2 = "1881";
    DataShare::DataSharePredicates predicates;
    std::string isExactMatchStr = "false";
    std::vector<std::string> columns;
    columns.push_back(isExactMatchStr);
    std::vector<std::string> phoneNumbers;
    phoneNumbers.push_back(phoneNumber1);
    phoneNumbers.push_back(phoneNumber2);
    predicates.SetWhereArgs(phoneNumbers);
    DataShare::DatashareBusinessError businessError;
    std::shared_ptr<NumberLocationAbility> numberLocationAbility = std::make_shared<NumberLocationAbility>();
    OHOS::Uri uri("datashare:///com.ohos.numberlocationability");
    auto resultSet = numberLocationAbility->Query(uri, predicates, columns, businessError);
    if (resultSet != nullptr) {
        NUMBER_IDENTITY_LOGE("resultSet is not nullptr!");
        resultSet->GoToFirstRow();
        int columnIndex = 0;
        resultSet->GetColumnIndex(COLUMN_NUMBER_LOCATION, columnIndex);
        std::string numberLocation1 = "";
        resultSet->GetString(columnIndex, numberLocation1);
        resultSet->OnGo(0, 1);
        resultSet->GetColumnIndex(COLUMN_NUMBER_LOCATION, columnIndex);
        std::string numberLocation2 = "";
        resultSet->GetString(columnIndex, numberLocation2);
        resultSet->Close();
        EXPECT_FALSE(numberLocation1.empty());
        EXPECT_FALSE(numberLocation2.empty());
        // phoneNumber1 and phoneNumber2 both only have operator
        EXPECT_TRUE(std::strcmp(numberLocation1.c_str(), numberLocation2.c_str()) == 0);
    }
}

/**
 * @tc.number   Telephony_NumberLocation_GetNumberLocation_0500
 * @tc.name     get the number location of the prefix mobile phone number.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(NumberLocationGtest, Telephony_NumberLocation_GetNumberLocation_0500, Function | MediumTest | Level1)
{
    std::string phoneNumber1 = "1881144";
    std::string phoneNumber2 = "18811445";
    DataShare::DataSharePredicates predicates;
    std::string isExactMatchStr = "false";
    std::vector<std::string> columns;
    columns.push_back(isExactMatchStr);
    std::vector<std::string> phoneNumbers;
    phoneNumbers.push_back(phoneNumber1);
    phoneNumbers.push_back(phoneNumber2);
    predicates.SetWhereArgs(phoneNumbers);
    DataShare::DatashareBusinessError businessError;
    std::shared_ptr<NumberLocationAbility> numberLocationAbility = std::make_shared<NumberLocationAbility>();
    OHOS::Uri uri("datashare:///com.ohos.numberlocationability");
    auto resultSet = numberLocationAbility->Query(uri, predicates, columns, businessError);
    if (resultSet != nullptr) {
        NUMBER_IDENTITY_LOGE("resultSet is not nullptr!");
        resultSet->GoToFirstRow();
        int columnIndex = 0;
        resultSet->GetColumnIndex(COLUMN_NUMBER_LOCATION, columnIndex);
        std::string numberLocation1 = "";
        resultSet->GetString(columnIndex, numberLocation1);
        resultSet->OnGo(0, 1);
        resultSet->GetColumnIndex(COLUMN_NUMBER_LOCATION, columnIndex);
        std::string numberLocation2 = "";
        resultSet->GetString(columnIndex, numberLocation2);
        resultSet->Close();
        EXPECT_FALSE(numberLocation1.empty());
        EXPECT_FALSE(numberLocation2.empty());
        // phoneNumber1 and phoneNumber2 both have operator and location
        EXPECT_TRUE(std::strcmp(numberLocation1.c_str(), numberLocation2.c_str()) == 0);
    }
}

/**
 * @tc.number   Telephony_NumberLocation_GetNumberLocation_0600
 * @tc.name     get the number location of the prefix mobile phone number.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(NumberLocationGtest, Telephony_NumberLocation_GetNumberLocation_0600, Function | MediumTest | Level1)
{
    std::string phoneNumber1 = "188114";
    std::string phoneNumber2 = "1881144";
    DataShare::DataSharePredicates predicates;
    std::string isExactMatchStr = "false";
    std::vector<std::string> columns;
    columns.push_back(isExactMatchStr);
    std::vector<std::string> phoneNumbers;
    phoneNumbers.push_back(phoneNumber1);
    phoneNumbers.push_back(phoneNumber2);
    predicates.SetWhereArgs(phoneNumbers);
    DataShare::DatashareBusinessError businessError;
    std::shared_ptr<NumberLocationAbility> numberLocationAbility = std::make_shared<NumberLocationAbility>();
    OHOS::Uri uri("datashare:///com.ohos.numberlocationability");
    auto resultSet = numberLocationAbility->Query(uri, predicates, columns, businessError);
    if (resultSet != nullptr) {
        NUMBER_IDENTITY_LOGE("resultSet is not nullptr!");
        resultSet->GoToFirstRow();
        int columnIndex = 0;
        resultSet->GetColumnIndex(COLUMN_NUMBER_LOCATION, columnIndex);
        std::string numberLocation1 = "";
        resultSet->GetString(columnIndex, numberLocation1);
        resultSet->GoToNextRow();
        resultSet->GetColumnIndex(COLUMN_NUMBER_LOCATION, columnIndex);
        std::string numberLocation2 = "";
        resultSet->GetString(columnIndex, numberLocation2);
        resultSet->Close();
        EXPECT_FALSE(numberLocation1.empty());
        EXPECT_FALSE(numberLocation2.empty());
        // phoneNumber1 only has operator, phoneNumber2 has operator and location
        EXPECT_EQ(numberLocation1, "移动");
        EXPECT_EQ(numberLocation2, "北京 移动");
    }
}
}
}
#endif // TEL_TEST_UNSUPPORT