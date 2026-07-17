/*
 * Copyright (C) 2024 Huawei Device Co., Ltd. rights reserved.
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
#include <openssl/sha.h>
#include <unistd.h>
#include <string>
#include "download_file.h"
#include "number_identity_log_wrapper.h"
#include "number_identity_errors.h"
#include "http_client.h"
#include "http_client_constant.h"
#include "net_conn_client.h"
#include "download_file_rdb.h"
#include "workscheduler_srv_client.h"
#include "ability_context_impl.h"
#include "number_identity_ddl.h"
#include "number_identity_database.h"
#include "parse_number_identity_config.h"
#include "parameters.h"

namespace OHOS {
namespace Telephony {
using namespace NetStack::HttpClient;
using namespace NetManagerStandard;
using namespace OHOS::WorkScheduler;
constexpr static const int32_t WAIT_TIME_SECOND = 30;
constexpr static const int32_t SLEEP_TIME_SECOND = 3;
constexpr static const unsigned int HTTP_TIME_MICRO_SECOND = WAIT_TIME_SECOND * 1000;
constexpr static const uint64_t TIMER_7DAY_MSECOND = 7 * 24 * 60 * 60 * 1000;
constexpr static const uint64_t TIMER_6HOUR_MSECOND = 6 * 60 * 60 * 1000;
constexpr static const int32_t RETRY_NUM = 3;
constexpr static const int32_t NUM_TWO = 2;
std::string g_numberLocationDirName = "/data/storage/el2/base/files/numberlocation.dat";
std::string g_yellowPageDirName = "/data/storage/el2/base/files/yellowpage.data";

const char *DOWNLOAD_URL = "downloadUrl";
const char *VERSION = "ver";
const char *SIGNATURE = "signature";

DownloadFile& DownloadFile::GetInstance()
{
    static auto instance = new DownloadFile();
    return *instance;
}

DownloadFile::DownloadFile()
{
    NUMBER_IDENTITY_LOGI("construct DownloadFile.");
}

DownloadFile::~DownloadFile()
{
    NUMBER_IDENTITY_LOGI("destruct DownloadFile.");
}

bool DownloadFile::IsString(const cJSON *jsonObj, const std::string &key)
{
    if (jsonObj == nullptr || !cJSON_IsObject(jsonObj)) {
        NUMBER_IDENTITY_LOGE("JSON parameter is invalid.");
        return false;
    }
    cJSON *paramValue = cJSON_GetObjectItemCaseSensitive(jsonObj, key.c_str());
    if (paramValue == nullptr) {
        NUMBER_IDENTITY_LOGE("paramValue is null");
        return false;
    }

    if (cJSON_IsString(paramValue)) {
        return true;
    }
    return false;
}

bool DownloadFile::CJsonParamCheck(const cJSON *jsonObj, const std::initializer_list<std::string> &keys)
{
    if (jsonObj == nullptr || !cJSON_IsObject(jsonObj)) {
        NUMBER_IDENTITY_LOGE("JSON parameter is invalid.");
        return false;
    }

    for (auto it = keys.begin(); it != keys.end(); it++) {
        cJSON *paramValue = cJSON_GetObjectItemCaseSensitive(jsonObj, (*it).c_str());
        if (paramValue == nullptr) {
            NUMBER_IDENTITY_LOGE("JSON parameter does not contain key: %s", (*it).c_str());
            return false;
        }
        bool res = IsString(jsonObj, *it);
        if (!res) {
            NUMBER_IDENTITY_LOGE("The key %s value format in JSON is illegal.", (*it).c_str());
            return false;
        }
    }
    return true;
}

int32_t DownloadFile::retry(const std::string &url)
{
    int32_t retryNum = 0;
    int32_t result = -1;
    while (retryNum < RETRY_NUM) {
        retryNum ++;
        NUMBER_IDENTITY_LOGI("retryNum: %{public}d.", retryNum);
        sleep(SLEEP_TIME_SECOND * retryNum);
        if (url == numberLocationUrl_) {
            result = DownloadNumberLocationFile();
        } else {
            result = DownloadYellowPageFile();
        }
        if (result == NUMBER_IDENTITY_ERR_SUCCESS) {
            break;
        }
    }
    return result;
}

int32_t DownloadFile::CheckNetworkAndDownloadFiles(sptr<IRemoteObject> token)
{
    NUMBER_IDENTITY_LOGI("check network and download files.");
    if (OHOS::system::GetParameter("const.global.region", "CN") != "CN") {
        NUMBER_IDENTITY_LOGW("oversea version.");
        return NUMBER_IDENTITY_ERR_SUCCESS;
    }
    int32_t result = CheckNetwork(token);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("check network failed.");
        bool startResult = UpdateTimer(TIMER_6HOUR_MSECOND);
        if (!startResult) {
            NUMBER_IDENTITY_LOGE("UpdateTimer failed.");
        }
        return NUMBER_IDENTITY_ERROR;
    }
    result = DownloadFiles();
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("download files failed, re-download after 6 hours.");
        bool startResult = UpdateTimer(TIMER_6HOUR_MSECOND);
        if (!startResult) {
            NUMBER_IDENTITY_LOGE("UpdateTimer failed.");
        }
        return NUMBER_IDENTITY_ERROR;
    }

    NUMBER_IDENTITY_LOGI("download files seccess, update after 7 days.");
    bool startResult = UpdateTimer(TIMER_7DAY_MSECOND);
    if (!startResult) {
        NUMBER_IDENTITY_LOGE("UpdateTimer failed.");
        return NUMBER_IDENTITY_ERROR;
    }
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

int32_t DownloadFile::CheckNetwork(sptr<IRemoteObject> token)
{
    NUMBER_IDENTITY_LOGI("check network.");
    NetHandle handle;
    int32_t result = NetConnClient::GetInstance().GetDefaultNet(handle);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("GetDefaultNet error, result: %{public}d", result);
        return result;
    }
    NetAllCapabilities netAllCap;
    result = NetConnClient::GetInstance().GetNetCapabilities(handle, netAllCap);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        if (result == NET_CONN_ERR_INVALID_NETWORK) {
            NUMBER_IDENTITY_LOGW("no network, result: %{public}d.", result);
        } else {
            NUMBER_IDENTITY_LOGE("GetNetCapabilities failed, result: %{public}d.", result);
        }
        return result;
    }
    int32_t networkType = *(netAllCap.bearerTypes_).begin();
    if (networkType == NetBearType::BEARER_CELLULAR) {
        NUMBER_IDENTITY_LOGI("networkType:BEARER_CELLULAR.");
    } else if (networkType == NetBearType::BEARER_WIFI) {
        NUMBER_IDENTITY_LOGI("networkType:BEARER_WIFI.");
    } else {
        NUMBER_IDENTITY_LOGI("networkType: %{public}d.", networkType);
    }
    std::string queryValue = DownloadFileRdb::GetInstance().QueryNetworkType(token);
    NUMBER_IDENTITY_LOGI("networkType: %{public}d, queryValue: %{public}s.", networkType, queryValue.c_str());
    if ((queryValue == PropertyValues::WLAN_ONLY && networkType == NetBearType::BEARER_WIFI) ||
        (queryValue == PropertyValues::ALL_NETWORK && networkType == NetBearType::BEARER_WIFI) ||
        (queryValue == PropertyValues::ALL_NETWORK && networkType == NetBearType::BEARER_CELLULAR)) {
        return NUMBER_IDENTITY_ERR_SUCCESS;
    } else {
        NUMBER_IDENTITY_LOGI("networkType not match.");
        return NUMBER_IDENTITY_ERROR;
    }
}

int32_t DownloadFile::DownloadFiles()
{
    bool numberLocationDownloadSuccess = false;
    bool yellowPageDownloadSuccess = false;
    ParseStringConfig::GetInstance().LoadConfig();
    numberLocationUrl_ = ParseStringConfig::GetInstance().GetNumberLocationUrl();
    yellowPageUrl_ = ParseStringConfig::GetInstance().GetYellowPageUrl();
    currentTimeStamp_ = ParseStringConfig::GetInstance().GetCurrentTimeStamp();
    int32_t result = DownloadNumberLocationFile();
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        result = retry(numberLocationUrl_);
        if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
            NUMBER_IDENTITY_LOGE("download numberLocation.bat failed.");
        } else {
            numberLocationDownloadSuccess = true;
        }
    } else {
        numberLocationDownloadSuccess = true;
    }

    result = DownloadYellowPageFile();
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        result = retry(yellowPageUrl_);
        if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
            NUMBER_IDENTITY_LOGE("download yellowpage.bat failed.");
        } else {
            yellowPageDownloadSuccess = true;
        }
    } else {
        yellowPageDownloadSuccess = true;
    }

    if (numberLocationDownloadSuccess && yellowPageDownloadSuccess) {
        return NUMBER_IDENTITY_ERR_SUCCESS;
    } else {
        return NUMBER_IDENTITY_ERROR;
    }
}

int32_t DownloadFile::DownloadNumberLocationFile()
{
    NUMBER_IDENTITY_LOGI("DownloadNumberLocationFile start.");
    /* get download url */
    int32_t result = CreateHttpRequest(numberLocationUrl_);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("CreateHttpRequest error");
        return result;
    }
    std::string downloadURL;
    std::string version;
    result = CheckVersionAndParseDowloadURL(PropertyKeys::NUMBER_LOCATION_VERSION_TIME_STAMP, downloadURL, version);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        if (result == NUMBER_IDENTITY_ERR_ALREADY_NEW_VERSION) {
            return NUMBER_IDENTITY_ERR_SUCCESS;
        }
        NUMBER_IDENTITY_LOGE("CheckVersionAndParseDowloadURL error");
        return result;
    }
    std::string responseData = responseData_;
    /* download file */
    result = CreateHttpRequest(downloadURL);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("CreateHttpRequest error");
        return result;
    }
    /* check file signature */
    result = CheckFileSignature(responseData, version);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("CheckFileSignature error");
        return result;
    }
    /* save file */
    result = UpdateDataToFile(g_numberLocationDirName);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("UpdateDataToFile error");
        return result;
    }
    result = DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::NUMBER_LOCATION_VERSION_TIME_STAMP, version);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("UpdateOrInsert error");
        return result;
    }
    NUMBER_IDENTITY_LOGI("DownloadNumberLocationFile success.");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

std::string Sha256(const std::string &str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(NUM_TWO) << std::setfill('0') << (int)hash[i];
    }
    return ss.str();
}

int32_t DownloadFile::CheckFileSignature(std::string &responseData, std::string &version)
{
    std::string stringToCheck;
    cJSON *response2json = cJSON_Parse(responseData.c_str());
    if (response2json == nullptr) {
        NUMBER_IDENTITY_LOGE("Failed to parse JSON: %{public}s", cJSON_GetErrorPtr());
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    cJSON *jParam = cJSON_GetArrayItem(response2json, 0);
    if (jParam == nullptr) {
        NUMBER_IDENTITY_LOGE("Failed to get item: %{public}s", cJSON_GetErrorPtr());
        cJSON_Delete(response2json);
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    cJSON *dhIdItem = cJSON_GetObjectItem(jParam, SIGNATURE);
    if (dhIdItem == nullptr || !cJSON_IsString(dhIdItem)) {
        NUMBER_IDENTITY_LOGE("Not found the value of the key : %{public}s.", SIGNATURE);
        cJSON_Delete(response2json);
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    stringToCheck = responseData_ + version;
    std::string checkResult = Sha256(stringToCheck);
    if (checkResult != dhIdItem->valuestring) {
        cJSON_Delete(response2json);
        NUMBER_IDENTITY_LOGE("CheckFileSignature err");
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    cJSON_Delete(response2json);
    NUMBER_IDENTITY_LOGI("CheckFileSignature success");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

int32_t DownloadFile::DownloadYellowPageFile()
{
    NUMBER_IDENTITY_LOGI("DownloadYellowPageFile start.");
    /* get download url */
    int32_t result = CreateHttpRequest(yellowPageUrl_);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("CreateHttpRequest error");
        return result;
    }
    std::string downloadURL;
    std::string version;
    result = CheckVersionAndParseDowloadURL(PropertyKeys::YELLOW_PAGE_VERSION_TIME_STAMP, downloadURL, version);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        if (result == NUMBER_IDENTITY_ERR_ALREADY_NEW_VERSION) {
            return NUMBER_IDENTITY_ERR_SUCCESS;
        }
        NUMBER_IDENTITY_LOGE("CheckVersionAndParseDowloadURL error");
        return result;
    }
    std::string responseData = responseData_;
    /* download file */
    result = CreateHttpRequest(downloadURL);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("CreateHttpRequest error");
        return result;
    }
    /* check file signature */
    result = CheckFileSignature(responseData, version);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("CheckFileSignature error");
        return result;
    }
    /* save file */
    result = UpdateDataToFile(g_yellowPageDirName);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        return result;
    }
    result = NumberIdentityDatabase::ImportYellowPageData(g_yellowPageDirName);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("ImportYellowPageData error");
        return result;
    }
    NUMBER_IDENTITY_LOGI("ImportYellowPageData success.");
    result = DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::YELLOW_PAGE_VERSION_TIME_STAMP, version);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("UpdateOrInsert error");
        return result;
    }
    NUMBER_IDENTITY_LOGI("DownloadYellowPageFile success.");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

int32_t DownloadFile::CheckVersionAndParseDowloadURL(const char *version_key,
    std::string &downloadURL, std::string &version)
{
    cJSON *response2json = cJSON_Parse(responseData_.c_str());
    if (response2json == nullptr) {
        NUMBER_IDENTITY_LOGE("Failed to parse JSON: %{public}s", cJSON_GetErrorPtr());
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    cJSON *jParam = cJSON_GetArrayItem(response2json, 0);
    if (jParam == nullptr) {
        NUMBER_IDENTITY_LOGE("Failed to get item: %{public}s", cJSON_GetErrorPtr());
        cJSON_Delete(response2json);
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    if (!CJsonParamCheck(jParam, { DOWNLOAD_URL, VERSION, SIGNATURE })) {
        NUMBER_IDENTITY_LOGE("CJsonParamCheck failed.");
        cJSON_Delete(response2json);
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    cJSON *dhIdItem = cJSON_GetObjectItem(jParam, VERSION);
    if (dhIdItem == nullptr || !cJSON_IsString(dhIdItem)) {
        NUMBER_IDENTITY_LOGE("Not found the value of the key : %{public}s.", VERSION);
        cJSON_Delete(response2json);
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    std::string queryValue = DownloadFileRdb::GetInstance().Query(version_key, currentTimeStamp_);
    if (queryValue < dhIdItem->valuestring) {
        NUMBER_IDENTITY_LOGI("new version exist, go to uptate, new version: %{public}s.", dhIdItem->valuestring);
    } else {
        NUMBER_IDENTITY_LOGW("already new version, do not update, the version: %{public}s.", dhIdItem->valuestring);
        cJSON_Delete(response2json);
        return NUMBER_IDENTITY_ERR_ALREADY_NEW_VERSION;
    }
    version = dhIdItem->valuestring;
    dhIdItem = cJSON_GetObjectItem(jParam, DOWNLOAD_URL);
    if (dhIdItem == nullptr || !cJSON_IsString(dhIdItem)) {
        NUMBER_IDENTITY_LOGE("Not found the value of the key : %{public}s.", DOWNLOAD_URL);
        cJSON_Delete(response2json);
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    downloadURL = dhIdItem->valuestring;
    cJSON_Delete(response2json);
    NUMBER_IDENTITY_LOGI("CheckVersionAndParseDowloadURL success.");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

int32_t DownloadFile::CreateHttpRequest(const std::string &url)
{
    if (HttpRequest(url) != NUMBER_IDENTITY_ERR_SUCCESS) {
        NUMBER_IDENTITY_LOGE("http fail error");
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    {
        std::unique_lock<std::mutex> lck(httpSuccessMutex_);
        httpSuccess_ = false;
    }
    {
        std::unique_lock<std::mutex> lck(clientCts_);
        if (clientCv_.wait_for(lck, std::chrono::seconds(WAIT_TIME_SECOND)) == std::cv_status::timeout) {
            NUMBER_IDENTITY_LOGE("wait(), timeout");
            return NUMBER_IDENTITY_ERR_HTTP_FAIL;
        }
    }
    std::unique_lock<std::mutex> lck(httpSuccessMutex_);
    if (!httpSuccess_) {
        NUMBER_IDENTITY_LOGE("CreateHttpRequest fail.");
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    NUMBER_IDENTITY_LOGI("CreateHttpRequest success.");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

int32_t DownloadFile::HttpRequest(const std::string &url)
{
    NUMBER_IDENTITY_LOGI("HttpRequest begin");
    HttpClientRequest httpReq;
    httpReq.SetURL(url);
    httpReq.SetConnectTimeout(HTTP_TIME_MICRO_SECOND);
    httpReq.SetTimeout(HTTP_TIME_MICRO_SECOND);
    httpReq.SetMethod(HttpConstant::HTTP_METHOD_GET);
    HttpSession &session = HttpSession::GetInstance();
    auto task = session.CreateTask(httpReq);
    if (task == nullptr) {
        return NUMBER_IDENTITY_ERR_LOCAL_PTR_NULL;
    }
    responseData_ = "";
    ProcesslTaskCb(task);
    task->Start();
    NUMBER_IDENTITY_LOGI("HttpRequest success.");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

int32_t DownloadFile::UpdateDataToFile(const std::string &storeDirName)
{
    uint32_t len = responseData_.size();

    std::unique_ptr<char[]> resultResponse = std::make_unique<char[]>(len);
    if (memset_s(resultResponse.get(), len, 0x00, len) != EOK) {
        NUMBER_IDENTITY_LOGE("memset_s error");
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    if (memcpy_s(resultResponse.get(), len, &responseData_[0], len) != EOK) {
        NUMBER_IDENTITY_LOGE("memcpy_s error");
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    if (!WriteBufferToFile(std::move(resultResponse), len, storeDirName)) {
        NUMBER_IDENTITY_LOGE("write to file error");
        return NUMBER_IDENTITY_ERR_HTTP_FAIL;
    }
    NUMBER_IDENTITY_LOGI("update file success.");
    return NUMBER_IDENTITY_ERR_SUCCESS;
}

void DownloadFile::ProcesslTaskCb(std::shared_ptr<HttpClientTask> task)
{
    task->OnSuccess([task, this](const HttpClientRequest &request, const HttpClientResponse &response) {
        NUMBER_IDENTITY_LOGI("OnSuccess");
        {
            std::unique_lock<std::mutex> lck(httpSuccessMutex_);
            httpSuccess_ = true;
        }
        std::unique_lock<std::mutex> lck(clientCts_);
        clientCv_.notify_one();
    });
    task->OnCancel([this](const HttpClientRequest &request, const HttpClientResponse &response) {
        NUMBER_IDENTITY_LOGI("OnCancel");
        std::unique_lock<std::mutex> lck(clientCts_);
        clientCv_.notify_one();
    });
    task->OnFail(
        [this](const HttpClientRequest &request, const HttpClientResponse &response, const HttpClientError &error) {
            NUMBER_IDENTITY_LOGE("OnFailed, responseCode:%{public}d", response.GetResponseCode());
            std::unique_lock<std::mutex> lck(clientCts_);
            clientCv_.notify_one();
        });
    task->OnDataReceive([this](const HttpClientRequest &request, const uint8_t *data, size_t length) {
        if (data == nullptr || length == 0) {
            NUMBER_IDENTITY_LOGI("OnDataReceive");
            return;
        }
        responseData_.insert(responseData_.size(), reinterpret_cast<const char *>(data), length);
    });
    task->OnProgress(
        [](const HttpClientRequest &request, u_long dltotal, u_long dlnow, u_long ultotal, u_long ulnow) {
        });
}

bool DownloadFile::WriteBufferToFile(
    const std::unique_ptr<char[]> &buff, uint32_t len, const std::string &strPathName) const
{
    if (buff == nullptr) {
        NUMBER_IDENTITY_LOGE("buff nullptr");
        return false;
    }
    FILE *pFile = nullptr;
    pFile = fopen(strPathName.c_str(), "wb+");
    if (!pFile) {
        NUMBER_IDENTITY_LOGE("open file fail, errno: %{public}d: %{public}s", errno, strerror(errno));
        return false;
    }
    uint32_t fileLen = fwrite(buff.get(), len, 1, pFile);
    if (fileLen == 0) {
        NUMBER_IDENTITY_LOGE("write buffer to file error");
        (void)fclose(pFile);
        return false;
    }
    (void)fclose(pFile);
    return true;
}

bool DownloadFile::StartTimer()
{
    if (OHOS::system::GetParameter("const.global.region", "CN") != "CN") {
        NUMBER_IDENTITY_LOGW("oversea version.");
        return true;
    }
    NUMBER_IDENTITY_LOGI("StartTimer, default 7 days");
    WorkInfo workInfo = WorkInfo();
    workInfo.SetElement("com.ohos.numberidentity", "DownloadFileWorkSheduler");
    workInfo.RequestRepeatCycle(TIMER_7DAY_MSECOND, 1);
    workInfo.RequestPersisted(true);
    workInfo.SetWorkId(COMMON_WORK_ID);
    int32_t result = WorkSchedulerSrvClient::GetInstance().StartWork(workInfo);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        if (result != NUMBER_IDENTITY_ERR_ADD_REPEAT_WORK) {
            NUMBER_IDENTITY_LOGE("StartTimer err, result: %{public}d", result);
            return false;
        } else {
            NUMBER_IDENTITY_LOGW("work already exist.");
            return true;
        }
    }
    struct timeval timeOfDay {};
    gettimeofday(&timeOfDay, NULL);
    int64_t currentTime = timeOfDay.tv_sec;
    std::string currentTimeString = std::to_string(currentTime);
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::TIME_WORKER_START_TIME_SECOND, currentTimeString);
    NUMBER_IDENTITY_LOGI("StartTimer ok, currentTimeString: %{public}s", currentTimeString.c_str());
    return true;
}

bool DownloadFile::UpdateTimer(uint64_t duration)
{
    NUMBER_IDENTITY_LOGI("UpdateTimer start");
    WorkInfo workInfo = WorkInfo();
    workInfo.SetElement("com.ohos.numberidentity", "DownloadFileWorkSheduler");
    workInfo.SetWorkId(COMMON_WORK_ID);
    int32_t result = WorkSchedulerSrvClient::GetInstance().StopAndCancelWork(workInfo);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        if (result != NUMBER_IDENTITY_ERR_WORK_NOT_EXIST) {
            NUMBER_IDENTITY_LOGE("StopTimer err, result: %{public}d", result);
        } else {
            NUMBER_IDENTITY_LOGW("work not exist.");
        }
    }

    workInfo.RequestRepeatCycle(duration, 1);
    workInfo.RequestPersisted(true);
    result = WorkSchedulerSrvClient::GetInstance().StartWork(workInfo);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        if (result != NUMBER_IDENTITY_ERR_ADD_REPEAT_WORK) {
            DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::TIME_WORKER_START_TIME_SECOND, "");
            NUMBER_IDENTITY_LOGE("StartTimer err, result: %{public}d", result);
            return false;
        } else {
            NUMBER_IDENTITY_LOGW("work already exist.");
            return true;
        }
    }
    struct timeval timeOfDay {};
    gettimeofday(&timeOfDay, NULL);
    int64_t currentTime = timeOfDay.tv_sec;
    std::string currentTimeString = std::to_string(currentTime);
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::TIME_WORKER_START_TIME_SECOND, currentTimeString);
    NUMBER_IDENTITY_LOGI("StartTimer ok, currentTimeString: %{public}s", currentTimeString.c_str());
    return true;
}

bool DownloadFile::StopTimer()
{
    if (OHOS::system::GetParameter("const.global.region", "CN") != "CN") {
        NUMBER_IDENTITY_LOGW("oversea version.");
        return true;
    }
    WorkInfo workInfo = WorkInfo();
    workInfo.SetElement("com.ohos.numberidentity", "DownloadFileWorkSheduler");
    workInfo.SetWorkId(COMMON_WORK_ID);
    int32_t result = WorkSchedulerSrvClient::GetInstance().StopAndCancelWork(workInfo);
    if (result != NUMBER_IDENTITY_ERR_SUCCESS) {
        if (result != NUMBER_IDENTITY_ERR_WORK_NOT_EXIST) {
            NUMBER_IDENTITY_LOGE("StopTimer err, result: %{public}d", result);
            return false;
        }
        NUMBER_IDENTITY_LOGW("work not exist.");
    }
    DownloadFileRdb::GetInstance().UpdateOrInsert(PropertyKeys::TIME_WORKER_START_TIME_SECOND, "");
    NUMBER_IDENTITY_LOGI("StopTimer success");
    return true;
}
} // Telephony
} // OHOS