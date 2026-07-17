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

#ifndef NUMBER_IDENTITY_UTILS_H
#define NUMBER_IDENTITY_UTILS_H

#include "number_identity_log_wrapper.h"

#include "errors.h"
#include "hisysevent.h"
#include "string_ex.h"
#include "uri.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <optional>
#include <ostream>
#include <regex>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#define HANDLE_ERR(msg, errCode, finally)                                                                              \
    do {                                                                                                               \
        NUMBER_IDENTITY_LOGD("%{public}s errCode = %{public}d", msg, errCode);                                         \
        if ((errCode) != OHOS::ERR_OK) {                                                                               \
            NUMBER_IDENTITY_LOGE("%{public}s failed with errCode: %{public}d", (msg), (errCode));                      \
            finally;                                                                                                   \
        }                                                                                                              \
    } while (0)

#define LOG_IF_ERR(errCode, msg, ...)                                                                                  \
    do {                                                                                                               \
        NUMBER_IDENTITY_LOGD("%{public}s errCode = %{public}d", msg, errCode);                                         \
        if ((errCode) != OHOS::ERR_OK) {                                                                               \
            NUMBER_IDENTITY_LOGE(msg " failed with errCode: %{public}d", ##__VA_ARGS__, (errCode));                    \
        }                                                                                                              \
    } while (0)

#define FAIL_IF_NULL(expr, fail)                                                                                       \
    do {                                                                                                               \
        NUMBER_IDENTITY_LOGD("checking %{public}s", #expr);                                                            \
        if ((expr) == nullptr) {                                                                                       \
            NUMBER_IDENTITY_LOGE("%{public}s is nullptr!", #expr);                                                     \
            fail;                                                                                                      \
        }                                                                                                              \
    } while (0)

#define FAIL_IF_NULLOPT(expr, fail)                                                                                    \
    do {                                                                                                               \
        NUMBER_IDENTITY_LOGD("checking %{public}s", #expr);                                                            \
        if ((expr) == std::nullopt) {                                                                                  \
            NUMBER_IDENTITY_LOGE("%{public}s is nullopt!", #expr);                                                     \
            fail;                                                                                                      \
        }                                                                                                              \
    } while (0)

#define BOOL_CHECK(call, finally)                                                                                      \
    do {                                                                                                               \
        NUMBER_IDENTITY_LOGD("checking %{public}s", #call);                                                            \
        if (!(call)) {                                                                                                 \
            NUMBER_IDENTITY_LOGE("%{public}s returns false!", #call);                                                  \
            finally;                                                                                                   \
        }                                                                                                              \
    } while (0)

#define TO_C_STR(expr) ToString(expr).c_str()

// Log expression value and return it, only for debug usage.
#ifndef NUMBER_IDENTITY_DEBUG
#define LOGD_EXPR(expr) Logged(LOG_DEBUG, expr, #expr)
#define LOGI_EXPR(expr) Logged(LOG_INFO, expr, #expr)
#else
#define LOGD_EXPR(expr) Logged(LOG_DEBUG, expr, #expr, GetRawFileName(__FILE__), __LINE__)
#define LOGI_EXPR(expr) Logged(LOG_INFO, expr, #expr, GetRawFileName(__FILE__), __LINE__)
#endif
namespace std {
// log expression with custom expression type log can be defined.
inline basic_ostream<char> &operator<<(basic_ostream<char> &out, const u16string &s)
{
    return out << OHOS::Str16ToStr8(s);
}
template <typename T> inline basic_ostream<char> &operator<<(basic_ostream<char> &buf, const vector<T> &v)
{
    return buf << ToString(v);
}

} // namespace std

namespace OHOS {
namespace Telephony {
using std::function;
using std::nullopt_t;
using std::optional;
using std::string;
using std::stringstream;
using std::u16string;
using std::vector;
using Byte = unsigned char;
using EventType = HiviewDFX::HiSysEvent::EventType;

constexpr const char DOMAIN_PHONE_UE[] = "PHONE_UE";


template <typename T, typename R>
inline void MapVector(const vector<T> &source, vector<R> &dist, function<R(const T &)> select)
{
    dist = vector<R>(source.size());
    std::transform(source.cbegin(), source.cend(), dist.begin(), select);
}

inline bool StartsWith(const string &str, const string &prefix)
{
    return str.find(prefix) == 0;
}

inline string ReplaceAll(const string &str, const string &from, const string &to)
{
    string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

inline bool RegexMatches(const string &str, const char *pattern)
{
    return std::regex_match(str, std::regex(pattern));
}

inline bool IsEmptyStr(const optional<string> &str)
{
    return !str.has_value() || str->empty();
}

inline string ToString(nullptr_t value)
{
    return "<null>";
}

inline string ToString(nullopt_t value)
{
    return "<nullopt>";
}

inline string ToString(const Byte value)
{
    using namespace std;
    constexpr const size_t bytePrintWidth = 2;
    stringstream hexbuf;
    hexbuf << hex << setfill('0') << setw(bytePrintWidth) << right << static_cast<unsigned int>(value);
    return hexbuf.str();
}

template <typename T> inline string ToString(const T &value)
{
    stringstream buf;
    buf << value;
    return buf.str();
}

template <typename T> inline string ToString(const optional<T> &opt)
{
    return opt.has_value() ? ToString(opt.value()) : "<null>";
}

template <typename T> inline string ToString(const vector<T> &value)
{
    stringstream buf;
    buf << "[";
    for (size_t i = 0, size = value.size(); i < size; ++i) {
        if (i > 0) {
            buf << ",";
        }
        buf << ToString(value[i]);
    }
    buf << "]";
    return buf.str();
}
#ifndef NUMBER_IDENTITY_DEBUG
template <typename T> const T &Logged(LogLevel lv, const T &value, const char *expr)
#else
template <typename T> const T &Logged(LogLevel lv, const T &value, const char *expr, const char *file, const int line)
#endif
{
    auto vstr = ToString(value);

#ifndef NUMBER_IDENTITY_DEBUG
    PRINT_NUMBER_IDENTITY_LOG(lv, "%{public}s = %{public}s", expr, vstr.c_str());
#else
    PRINT_NUMBER_IDENTITY_LOG(lv, "[%{public}s:%{public}d] %{public}s = %{public}s", file, line, expr, vstr.c_str());
#endif
    return value;
}

template <typename... Types> inline int ReportHiSysEvent(const string &eventName, EventType eventType, Types... args)
{
    NUMBER_IDENTITY_LOGI("Report HiSysEvent: %{public}s, type = %{public}d", eventName.c_str(), eventType);
    return HiSysEventWrite(DOMAIN_PHONE_UE, eventName, eventType, args...);
}

string Join(const vector<string> &strs, const string &delimiter = " ");

string GetUriPathName(const Uri &uri);

string EncodeBase64(const char *bytes, unsigned int size);

string DecodeBase64(const string &encoded);

string ToHexString(const string &str);

uint64_t GetCurrentTimestamp();

uint64_t GenerateRandomLong();

vector<uint8_t> StringBytes(const string &str);

optional<string> GenerateMD5(const vector<uint8_t> &bytes);

class TimeLogger {
  public:
    void TimeStart();
    void TimeEnd(const char *tag);

  private:
    uint64_t startTime_ = 0L;
};

} // namespace Telephony
} // namespace OHOS

#endif /* NUMBER_IDENTITY_UTILS_H */