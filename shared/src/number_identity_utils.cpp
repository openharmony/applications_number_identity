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

#include "number_identity_utils.h"

#include "blob.h"
#include "datetime_ex.h"
#include "errors.h"
#include "md.h"
#include "base/security/crypto_framework/common/inc/memory.h"
#include "object_base.h"
#include "result.h"
#include "securec.h"
#include "string_ex.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <limits>
#include <mutex>
#include <optional>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {
using namespace std;

string Join(const vector<string> &strs, const string &delimiter)
{
    stringstream buf;
    bool inserted = false;
    for (size_t i = 0; i < strs.size(); ++i) {
        const auto &str = strs[i];
        if (str.empty()) {
            continue;
        }
        if (inserted) {
            buf << delimiter;
        }
        buf << str;
        inserted = true;
    }
    return buf.str();
}

string GetUriPathName(const Uri &uri)
{
    auto uriString = uri.ToString();
    regex pattern(R"([a-zA-Z0-9]+:///?[a-zA-Z0-9\.]+/(.+))");
    smatch match;
    if (regex_search(uriString, match, pattern)) {
        return match[1];
    }
    return "";
}

static string g_base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefghijklmnopqrstuvwxyz"
                              "0123456789+/";

string EncodeBase64(const char *bytes, unsigned int size)
{
    string result;
    int i = 0;
    int j = 0;
    uint8_t charArray3[3] = { 0 }; // store 3 byte of bytes_to_encode
    uint8_t charArray4[4] = { 0 }; // store encoded character to 4 bytes

    while (size--) {
        charArray3[i++] = *(bytes++); // get three bytes (24 bits)
        if (i == 3) {
            // eg. we have 3 bytes as ( 0100 1101, 0110 0001, 0110 1110) --> (010011, 010110, 000101, 101110)
            charArray4[0] = (charArray3[0] & 0xfc) >> 2; // get first 6 bits of first byte,
            charArray4[1] =
                ((charArray3[0] & 0x03) << 4) +
                ((charArray3[1] & 0xf0) >> 4); // get last 2 bits of first byte and first 4 bit of second byte
            charArray4[2] =
                ((charArray3[1] & 0x0f) << 2) +
                ((charArray3[2] & 0xc0) >> 6);    // get last 4 bits of second byte and first 2 bits of third byte
            charArray4[3] = charArray3[2] & 0x3f; // get last 6 bits of third byte

            for (i = 0; (i < 4); i++) {
                result += g_base64Chars[charArray4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            charArray3[j] = '\0';
        }

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++) {
            result += g_base64Chars[charArray4[j]];
        }

        while ((i++ < 3)) {
            result += '=';
        }
    }

    return result;
}

static inline bool IsBase64(uint8_t c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

string DecodeBase64(const string &encoded)
{
    size_t size = encoded.size();
    int i = 0;
    int j = 0;
    int index = 0;
    unsigned char charArray4[4] = { 0 };
    unsigned char charArray3[3] = { 0 };
    string result;

    while (size-- && (encoded[index] != '=') && IsBase64(encoded[index])) {
        charArray4[i++] = encoded[index];
        index++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                charArray4[i] = g_base64Chars.find(charArray4[i]) & 0xff;
            }

            charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
            charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
            charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];

            for (i = 0; (i < 3); i++) {
                result += charArray3[i];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++) {
            charArray4[j] = g_base64Chars.find(charArray4[j]) & 0xff;
        }

        charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
        charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) {
            result += charArray3[j];
        }
    }

    return result;
}

string ToHexString(const string &str)
{
    constexpr const size_t bytePrintWidth = 2;
    stringstream output;
    output << hex;
    for (size_t i = 0, size = str.size(); i < size; ++i) {
        output << setfill('0') << setw(bytePrintWidth) << right << static_cast<unsigned int>(str.at(i));
    }
    return output.str();
}

uint64_t GetCurrentTimestamp()
{
    using namespace chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint64_t GenerateRandomLong()
{
    using Limits = numeric_limits<uint64_t>;
    random_device seed;
    mt19937_64 gen(seed());
    uniform_int_distribution<uint64_t> dis(Limits::min(), Limits::max());
    return dis(gen);
}

vector<uint8_t> StringBytes(const string &str)
{
    vector<uint8_t> bytes;
    for (auto &c : str) {
        bytes.push_back(static_cast<uint8_t>(c));
    }
    return bytes;
}

optional<string> GenerateMD5(const vector<uint8_t> &bytes)
{
    int errCode = ERR_OK;
    HcfMd *md = nullptr;
    uint8_t *buf = nullptr;
    auto size = bytes.size();
    HcfBlob inputBlob = { .data = nullptr, .len = 0 };
    HcfBlob outputBlob = { .data = nullptr, .len = 0 };
    optional<string> result;
    uint32_t mdLen = 0;
    auto hcfRet = HcfMdCreate("MD5", &md);
    BOOL_CHECK(hcfRet == HCF_SUCCESS && md != nullptr, goto finally);
    buf = (uint8_t *)HcfMalloc(size, 0);
    FAIL_IF_NULL(buf, goto finally);
    errCode = memcpy_s(buf, size, bytes.data(), size);
    HANDLE_ERR("memcpy_s", errCode, goto finally);
    inputBlob.data = buf;
    inputBlob.len = size;
    hcfRet = md->update(md, &inputBlob);
    BOOL_CHECK(hcfRet == HCF_SUCCESS, goto finally);
    hcfRet = md->doFinal(md, &outputBlob);
    BOOL_CHECK(hcfRet == HCF_SUCCESS, goto finally);
    mdLen = md->getMdLength(md);
    result = string(outputBlob.data, outputBlob.data + mdLen);
finally:
    HcfBlobDataFree(&outputBlob);
    HcfFree(buf);
    if (md != nullptr) {
        md->base.destroy((HcfObjectBase *)(md));
    }
    return result;
}

void TimeLogger::TimeStart()
{
    startTime_ = GetCurrentTimestamp();
}

void TimeLogger::TimeEnd(const char *tag)
{
    auto current = GetCurrentTimestamp();
    NUMBER_IDENTITY_LOGI("%{public}s time cost: %{public}ld(ms)", tag, current - startTime_);
}

} // namespace Telephony
} // namespace OHOS