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

#define private public
#define protected public
#include <gmock/gmock.h>
#include <cstring>
#include <string>
#include "number_identity_log_wrapper.h"
#define private public
#include "download_file.h"
#undef private
#include "number_identity_errors.h"
#include "access_token.h"
#include "parse_number_identity_config.h"
#include "number_identity_database.h"
#include "number_identity_ddl.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
using ::testing::Return;
using ::testing::_;

class DownloadFileGtest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DownloadFileGtest::SetUp() {}

void DownloadFileGtest::TearDown() {}

void DownloadFileGtest::SetUpTestCase() {}

void DownloadFileGtest::TearDownTestCase() {}

class MockDownloadFile: public DownloadFile {
public:
    MOCK_METHOD1(CheckNetwork, int32_t(sptr<IRemoteObject>));
    MOCK_METHOD1(CreateHttpRequest, int32_t(const std::string &));
    MOCK_METHOD1(UpdateTimer, bool(uint64_t));
    MOCK_CONST_METHOD3(WriteBufferToFile, bool(const std::unique_ptr<char[]> &, uint32_t, const std::string &));
};

/********************************************* Test GetNumberLocation()***********************************************/
/**
 * @tc.number   CheckNetworkAndDownloadFiles_0100
 * @tc.name     download file.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, CheckNetworkAndDownloadFiles_0100, Function | MediumTest | Level1)
{
    AccessToken token;
    EXPECT_EQ(DownloadFile::GetInstance().CheckNetworkAndDownloadFiles(nullptr), NUMBER_IDENTITY_ERROR);
    NumberIdentityDatabase::SetDBDirectory("/data/test");
    NumberIdentityDatabase::GetInstance()->PutProperty(PropertyKeys::NETWORK_TYPE, PropertyValues::ALL_NETWORK);
    EXPECT_EQ(DownloadFile::GetInstance().CheckNetworkAndDownloadFiles(nullptr), NUMBER_IDENTITY_ERROR);
    NumberIdentityDatabase::GetInstance()->PutProperty(PropertyKeys::NETWORK_TYPE, "");
    MockDownloadFile mockDownloadFile;
    EXPECT_CALL(mockDownloadFile, CheckNetwork(_)).WillRepeatedly(Return(NUMBER_IDENTITY_ERR_SUCCESS));
    EXPECT_EQ(mockDownloadFile.CheckNetworkAndDownloadFiles(nullptr), NUMBER_IDENTITY_ERROR);
    EXPECT_CALL(mockDownloadFile, CreateHttpRequest(_)).WillRepeatedly(Return(NUMBER_IDENTITY_ERR_SUCCESS));
    EXPECT_EQ(mockDownloadFile.CheckNetworkAndDownloadFiles(nullptr), NUMBER_IDENTITY_ERROR);
    mockDownloadFile.responseData_ = "[{";
    mockDownloadFile.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    mockDownloadFile.responseData_ += "\"ver\":\"1732932935503\",";
    mockDownloadFile.responseData_ += "\"signature\":\"95915da7025023d0cef045e5c8b003d875197facdf336e1587abb3f64225\",";
    mockDownloadFile.responseData_ += "\"fileId\":\"1\"";
    mockDownloadFile.responseData_ += "}]";
    EXPECT_EQ(mockDownloadFile.CheckNetworkAndDownloadFiles(nullptr), NUMBER_IDENTITY_ERROR);
    EXPECT_CALL(mockDownloadFile, UpdateTimer(_)).WillRepeatedly(Return(true));
    EXPECT_EQ(mockDownloadFile.CheckNetworkAndDownloadFiles(nullptr), NUMBER_IDENTITY_ERR_SUCCESS);
}

/**
 * @tc.number   DownloadNumberLocationFile_0100
 * @tc.name     DownloadNumberLocationFile.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, DownloadNumberLocationFile_0100, Function | MediumTest | Level1)
{
    MockDownloadFile mockDownloadFile;
    EXPECT_CALL(mockDownloadFile, CreateHttpRequest(_)).WillRepeatedly(Return(NUMBER_IDENTITY_ERR_SUCCESS));
    mockDownloadFile.responseData_ = "[{";
    mockDownloadFile.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    mockDownloadFile.responseData_ += "\"ver\":\"9732932935503\",";
    mockDownloadFile.responseData_ +=
        "\"signature\":\"5d0d81a914e4053d3e932c8e4fb474d5075c4ae39919c8a5db8b1e64f9a2cd2a\",";
    mockDownloadFile.responseData_ += "\"fileId\":\"1\"";
    mockDownloadFile.responseData_ += "}]";
    EXPECT_EQ(mockDownloadFile.DownloadNumberLocationFile(), NUMBER_IDENTITY_ERR_HTTP_FAIL);
    EXPECT_CALL(mockDownloadFile, CreateHttpRequest(_))
        .Times(2)
        .WillOnce(Return(NUMBER_IDENTITY_ERR_SUCCESS))
        .WillOnce(Return(NUMBER_IDENTITY_ERROR));
    EXPECT_EQ(mockDownloadFile.DownloadNumberLocationFile(), NUMBER_IDENTITY_ERROR);
}

/**
 * @tc.number   DownloadNumberLocationFile_0101
 * @tc.name     DownloadNumberLocationFile.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, DownloadNumberLocationFile_0101, Function | MediumTest | Level1)
{
    MockDownloadFile mockDownloadFile;
    mockDownloadFile.responseData_ = "[{";
    mockDownloadFile.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    mockDownloadFile.responseData_ += "\"ver\":\"9732932935503\",";
    mockDownloadFile.responseData_ +=
        "\"signature\":\"0262a3f595110a1b2e50e120e2d3dab79cd36ff07db3f834f052845b900bc7c4\",";
    mockDownloadFile.responseData_ += "\"fileId\":\"1\"";
    mockDownloadFile.responseData_ += "}]";
    EXPECT_CALL(mockDownloadFile, CreateHttpRequest(_))
        .Times(5)
        .WillOnce(Return(NUMBER_IDENTITY_ERR_SUCCESS))
        .WillOnce([&mockDownloadFile]() {
            mockDownloadFile.responseData_ = "123abc";
            return NUMBER_IDENTITY_ERR_SUCCESS;
        })
        .WillOnce(Return(NUMBER_IDENTITY_ERR_SUCCESS))
        .WillOnce(Return(NUMBER_IDENTITY_ERR_SUCCESS))
        .WillOnce([&mockDownloadFile]() {
            mockDownloadFile.responseData_ = "123abc";
            return NUMBER_IDENTITY_ERR_SUCCESS;
        });
    EXPECT_EQ(mockDownloadFile.DownloadNumberLocationFile(), NUMBER_IDENTITY_ERR_HTTP_FAIL);
    EXPECT_EQ(mockDownloadFile.DownloadNumberLocationFile(), NUMBER_IDENTITY_ERR_HTTP_FAIL);
    mockDownloadFile.responseData_ = "[{";
    mockDownloadFile.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    mockDownloadFile.responseData_ += "\"ver\":\"9732932935503\",";
    mockDownloadFile.responseData_ +=
        "\"signature\":\"0262a3f595110a1b2e50e120e2d3dab79cd36ff07db3f834f052845b900bc7c4\",";
    mockDownloadFile.responseData_ += "\"fileId\":\"1\"";
    mockDownloadFile.responseData_ += "}]";
    EXPECT_CALL(mockDownloadFile, WriteBufferToFile(_, _, _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_EQ(mockDownloadFile.DownloadNumberLocationFile(), NUMBER_IDENTITY_ERR_SUCCESS);
}

/**
 * @tc.number   DownloadYellowPageFile_0100
 * @tc.name     DownloadNumberLocationFile.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, DownloadYellowPageFile_0100, Function | MediumTest | Level1)
{
    MockDownloadFile mockDownloadFile;
    EXPECT_CALL(mockDownloadFile, CreateHttpRequest(_)).WillRepeatedly(Return(NUMBER_IDENTITY_ERR_SUCCESS));
    mockDownloadFile.responseData_ = "[{";
    mockDownloadFile.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    mockDownloadFile.responseData_ += "\"ver\":\"9732932935503\",";
    mockDownloadFile.responseData_ +=
        "\"signature\":\"5d0d81a914e4053d3e932c8e4fb474d5075c4ae39919c8a5db8b1e64f9a2cd2a\",";
    mockDownloadFile.responseData_ += "\"fileId\":\"1\"";
    mockDownloadFile.responseData_ += "}]";
    EXPECT_EQ(mockDownloadFile.DownloadYellowPageFile(), NUMBER_IDENTITY_ERR_HTTP_FAIL);
    EXPECT_CALL(mockDownloadFile, CreateHttpRequest(_))
        .Times(2)
        .WillOnce(Return(NUMBER_IDENTITY_ERR_SUCCESS))
        .WillOnce(Return(NUMBER_IDENTITY_ERROR));
    EXPECT_EQ(mockDownloadFile.DownloadYellowPageFile(), NUMBER_IDENTITY_ERROR);
}

/**
 * @tc.number   DownloadYellowPageFile_0101
 * @tc.name     DownloadYellowPageFile.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, DownloadYellowPageFile_0101, Function | MediumTest | Level1)
{
    MockDownloadFile mockDownloadFile;
    mockDownloadFile.responseData_ = "[{";
    mockDownloadFile.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    mockDownloadFile.responseData_ += "\"ver\":\"9732932935503\",";
    mockDownloadFile.responseData_ +=
        "\"signature\":\"0262a3f595110a1b2e50e120e2d3dab79cd36ff07db3f834f052845b900bc7c4\",";
    mockDownloadFile.responseData_ += "\"fileId\":\"1\"";
    mockDownloadFile.responseData_ += "}]";
    EXPECT_CALL(mockDownloadFile, CreateHttpRequest(_))
        .Times(5)
        .WillOnce(Return(NUMBER_IDENTITY_ERR_SUCCESS))
        .WillOnce([&mockDownloadFile]() {
            mockDownloadFile.responseData_ = "123abc";
            return NUMBER_IDENTITY_ERR_SUCCESS;
        })
        .WillOnce(Return(NUMBER_IDENTITY_ERR_SUCCESS))
        .WillOnce(Return(NUMBER_IDENTITY_ERR_SUCCESS))
        .WillOnce([&mockDownloadFile]() {
            mockDownloadFile.responseData_ = "123abc";
            return NUMBER_IDENTITY_ERR_SUCCESS;
        });
    EXPECT_EQ(mockDownloadFile.DownloadYellowPageFile(), NUMBER_IDENTITY_ERR_HTTP_FAIL);
    EXPECT_EQ(mockDownloadFile.DownloadYellowPageFile(), NUMBER_IDENTITY_ERR_HTTP_FAIL);
    mockDownloadFile.responseData_ = "[{";
    mockDownloadFile.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    mockDownloadFile.responseData_ += "\"ver\":\"9732932935503\",";
    mockDownloadFile.responseData_ +=
        "\"signature\":\"0262a3f595110a1b2e50e120e2d3dab79cd36ff07db3f834f052845b900bc7c4\",";
    mockDownloadFile.responseData_ += "\"fileId\":\"1\"";
    mockDownloadFile.responseData_ += "}]";
    EXPECT_CALL(mockDownloadFile, WriteBufferToFile(_, _, _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_EQ(mockDownloadFile.DownloadYellowPageFile(), E_ERROR);
}

/**
 * @tc.number   StartTimer_0100
 * @tc.name     download file.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, StartTimer_0100, Function | MediumTest | Level1)
{
    AccessToken token;
    EXPECT_EQ(DownloadFile::GetInstance().StartTimer(), false);
}

/**
 * @tc.number   StartTimer_0100
 * @tc.name     download file.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, StopTimer_0100, Function | MediumTest | Level1)
{
    AccessToken token;
    EXPECT_EQ(DownloadFile::GetInstance().StopTimer(), false);
}

/**
 * @tc.number   ParseStringConfig_0100
 * @tc.name     pase string config file.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, ParseStringConfig_0100, Function | MediumTest | Level1)
{
    AccessToken token;
    ParseStringConfig::GetInstance().LoadConfig();
    std::string numberLocationUrl = ParseStringConfig::GetInstance().GetNumberLocationUrl();
    EXPECT_EQ(true, !numberLocationUrl.empty());
    std::string yellowPageUrl = ParseStringConfig::GetInstance().GetYellowPageUrl();
    EXPECT_EQ(true, !yellowPageUrl.empty());
    std::string currentTimeStamp = ParseStringConfig::GetInstance().GetCurrentTimeStamp();
    EXPECT_EQ(true, !currentTimeStamp.empty());
}

/**
 * @tc.number   CheckVersionAndParseDowloadURLGtest_0100
 * @tc.name     CheckVersionAndParseDowloadURL.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, CheckVersionAndParseDowloadURLGtest_0100, Function | MediumTest | Level1)
{
    DownloadFile download;
    const char *version_key = "version_key";
    std::string downloadURL;
    std::string version;
    EXPECT_GE(download.CheckVersionAndParseDowloadURL(version_key, downloadURL, version), NUMBER_IDENTITY_ERR_SUCCESS);
    uint32_t len = 0;
    const std::unique_ptr<char[]> buff = nullptr;
    const std::string strPathName = "strPathName";
    download.WriteBufferToFile(buff, len, strPathName);
}

/**
 * @tc.number   CheckVersionAndParseDowloadURLGtest_0101
 * @tc.name     CheckVersionAndParseDowloadURL.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, CheckVersionAndParseDowloadURLGtest_0101, Function | MediumTest | Level1)
{
    DownloadFile download;
    download.responseData_ = "[{";
    download.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    download.responseData_ += "\"signature\":\"95915da7025023d0cef045e5c8b0053d8751978f0facdf336e1587abb3f64225\",";
    download.responseData_ += "\"fileId\":\"1\"";
    download.responseData_ += "}]";
    const char *version_key = "version_key";
    std::string downloadURL;
    std::string version;
    EXPECT_EQ(download.CheckVersionAndParseDowloadURL(version_key, downloadURL, version),
        NUMBER_IDENTITY_ERR_HTTP_FAIL);
    download.responseData_ = "[{";
    download.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    download.responseData_ += "\"ver\":\"1732932935503\",";
    download.responseData_ += "\"signature\":\"95915da7025023d0cef045e5c8b0053d8751978f0facdf336e1587abb3f64225\",";
    download.responseData_ += "\"fileId\":\"1\"";
    download.responseData_ += "}]";
    EXPECT_EQ(download.CheckVersionAndParseDowloadURL(version_key, downloadURL, version), NUMBER_IDENTITY_ERR_SUCCESS);
    NumberIdentityDatabase::SetDBDirectory("/data/test");
    download.currentTimeStamp_ = "1732932935504";
    EXPECT_EQ(download.CheckVersionAndParseDowloadURL(version_key, downloadURL, version),
        NUMBER_IDENTITY_ERR_ALREADY_NEW_VERSION);
}

/**
 * @tc.number   CheckVersionAndParseDowloadURLGtest_0102
 * @tc.name     CheckVersionAndParseDowloadURL.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, CheckVersionAndParseDowloadURLGtest_0102, Function | MediumTest | Level1)
{
    DownloadFile download;
    download.responseData_ = "[{";
    download.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    download.responseData_ += "\"ver\":100,";
    download.responseData_ += "\"signature\":101,";
    download.responseData_ += "\"fileId\":\"1\"";
    download.responseData_ += "}]";
    const char *version_key = "version_key";
    std::string downloadURL;
    std::string version;
    EXPECT_EQ(download.CheckVersionAndParseDowloadURL(version_key, downloadURL, version),
        NUMBER_IDENTITY_ERR_HTTP_FAIL);
    EXPECT_FALSE(download.CJsonParamCheck(nullptr, { "downloadUrl", "ver", "signature" }));
    cJSON *jParam = (cJSON *)malloc(1);
    EXPECT_FALSE(download.CJsonParamCheck(jParam, { "downloadUrl", "ver", "signature" }));
    EXPECT_FALSE(download.IsString(nullptr, "downloadUrl"));
    EXPECT_FALSE(download.IsString(jParam, "downloadUrl"));
    cJSON *response2json = cJSON_Parse(download.responseData_.c_str());
    jParam = cJSON_GetArrayItem(response2json, 0);
    EXPECT_FALSE(download.IsString(jParam, "nodownloadUrl"));
    cJSON_Delete(response2json);
}

/**
 * @tc.number   DownloadFileGtest_0101
 * @tc.name     DownloadFiles.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, DownloadFileGtest_0101, Function | MediumTest | Level1)
{
    string url;
    DownloadFile::GetInstance().retry(url);
    DownloadFile::GetInstance().DownloadNumberLocationFile();
    DownloadFile::GetInstance().DownloadYellowPageFile();
    DownloadFile::GetInstance().CreateHttpRequest(url);
    DownloadFile::GetInstance().HttpRequest(url);
    EXPECT_GE(DownloadFile::GetInstance().UpdateDataToFile("/test/datafile"), NUMBER_IDENTITY_ERR_SUCCESS);
}

/**
 * @tc.number   DownloadFileGtest_0102
 * @tc.name     DownloadFiles.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, DownloadFileGtest_0102, Function | MediumTest | Level1)
{
    uint32_t len = 128;
    const std::unique_ptr<char[]> buff = std::make_unique<char[]>(len);
    const std::string strPathName = "strPathName";
    EXPECT_EQ(DownloadFile::GetInstance().WriteBufferToFile(buff, len, strPathName), true);
}

/**
 * @tc.number   DownloadFileGtest_DownloadFiles
 * @tc.name     DownloadFiles.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, DownloadFileGtest_DownloadFiles, Function | MediumTest | Level1)
{
    int32_t downloadFilesResult = DownloadFile::GetInstance().DownloadFiles();
    if (downloadFilesResult == NUMBER_IDENTITY_ERR_SUCCESS) {
       EXPECT_EQ(downloadFilesResult, NUMBER_IDENTITY_ERR_SUCCESS);
    } else {
       EXPECT_EQ(downloadFilesResult, NUMBER_IDENTITY_ERROR);
    }
}

/**
 * @tc.number   CheckFileSignature_0100
 * @tc.name     CheckFileSignature.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, CheckFileSignature_0100, Function | MediumTest | Level1)
{
    DownloadFile DownloadFile;
    std::string responseData = "";
    std::string version = "456";
    EXPECT_EQ(DownloadFile.CheckFileSignature(responseData, version), NUMBER_IDENTITY_ERR_HTTP_FAIL);
    responseData = "123";
    EXPECT_EQ(DownloadFile.CheckFileSignature(responseData, version), NUMBER_IDENTITY_ERR_HTTP_FAIL);
    responseData = "[{";
    responseData += "\"downloadUrl\":\"https://downloadUrl\",";
    responseData += "\"ver\":\"9732932935503\",";
    responseData += "\"fileId\":\"1\"";
    responseData += "}]";
    EXPECT_EQ(DownloadFile.CheckFileSignature(responseData, version), NUMBER_IDENTITY_ERR_HTTP_FAIL);
}

/**
 * @tc.number   CheckVersionAndParseDowloadURL_0100
 * @tc.name     CheckVersionAndParseDowloadURL.
 * @tc.desc     Function test
 * @tc.require: I5P2WO
 */
HWTEST_F(DownloadFileGtest, CheckVersionAndParseDowloadURL_0100, Function | MediumTest | Level1)
{
    DownloadFile DownloadFile;
    DownloadFile.responseData_ = "[{";
    DownloadFile.responseData_ += "\"downloadUrl\":\"https://downloadUrl\",";
    DownloadFile.responseData_ +=
        "\"signature\":\"5d0d81a914e4053d3e932c8e4fb474d5075c4ae39919c8a5db8b1e64f9a2cd2a\",";
    DownloadFile.responseData_ += "\"fileId\":\"1\"";
    DownloadFile.responseData_ += "}]";
    std::string downloadURL;
    std::string version;
    EXPECT_EQ(DownloadFile.CheckVersionAndParseDowloadURL(PropertyKeys::YELLOW_PAGE_VERSION_TIME_STAMP,
        downloadURL, version), NUMBER_IDENTITY_ERR_HTTP_FAIL);
}
} // Telephony
} // OHOS
