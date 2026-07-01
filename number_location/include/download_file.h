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

#ifndef DOWNLOAD_FILE_H
#define DOWNLOAD_FILE_H
#include <cinttypes>
#include <string>
#include <mutex>
#include "http_client_task.h"
#include "ability.h"
#include "cJSON.h"

namespace OHOS {
namespace Telephony {
enum {
    BOOT_WORK_ID = 1,
    COMMON_WORK_ID
};
class DownloadFile {
public:
    static DownloadFile& GetInstance();
    int32_t CheckNetworkAndDownloadFiles(sptr<IRemoteObject> token);
    bool StartTimer();
    virtual bool UpdateTimer(uint64_t duration);
    bool StopTimer();
private:
    int32_t HttpRequest(const std::string &url);
    int32_t UpdateDataToFile(const std::string &storeDirName);
    void ProcesslTaskCb(std::shared_ptr<NetStack::HttpClient::HttpClientTask> task);
    virtual bool WriteBufferToFile(const std::unique_ptr<char[]> &buff, uint32_t len,
        const std::string &strPathName) const;
    virtual int32_t CreateHttpRequest(const std::string &url);
    virtual int32_t CheckNetwork(sptr<IRemoteObject> token);
    int32_t DownloadFiles();
    int32_t DownloadNumberLocationFile();
    int32_t CheckFileSignature(std::string &responseData, std::string &version);
    int32_t DownloadYellowPageFile();
    int32_t retry(const std::string &url);
    int32_t CheckVersionAndParseDowloadURL(const char *version_key, std::string &downloadURL, std::string &version);
    bool IsString(const cJSON *jsonObj, const std::string &key);
    bool CJsonParamCheck(const cJSON *jsonObj, const std::initializer_list<std::string> &keys);
    DownloadFile();
    ~DownloadFile();
private:
    std::mutex httpSuccessMutex_;
    bool httpSuccess_ = false;
    std::mutex clientCts_;
    std::condition_variable clientCv_;
    std::string responseData_ = "";
    std::string numberLocationUrl_ = "";
    std::string yellowPageUrl_ = "";
    std::string currentTimeStamp_ = "";
};
} // Telephony
} // OHOS
#endif // DOWNLOAD_FILE_H