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
#ifndef CALLER_INFO_QUERY_HILOG_H
#define CALLER_INFO_QUERY_HILOG_H

#include <string>
#include "hilog/log.h"

namespace OHOS {
namespace CallerInfoQuery {

// param of log interface.
enum CallerInfoQuerySubModule {
    CALLER_INFO_QUERY_MODULE_EXTENSION = 0,
    CALLER_INFO_QUERY_MODULE_BUTT
};

static constexpr unsigned int CALLER_INFO_QUERY_DOMAIN_ID = 0xD001F1A;

static constexpr const char* CALLER_INFO_QUERY_MODULE_LABEL[CALLER_INFO_QUERY_MODULE_BUTT] = {
    "CallerInfoQueryExtension"
};

#ifndef CALLER_INFO_QUERY_FUNC_FMT
#define CALLER_INFO_QUERY_FUNC_FMT "[%{public}s(%{public}s:%{public}d)]"
#endif

#ifndef CALLER_INFO_QUERY_FILE_NAME
#define CALLER_INFO_QUERY_FILE_NAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef CALLER_INFO_QUERY_FUNC_INFO
#define CALLER_INFO_QUERY_FUNC_INFO CALLER_INFO_QUERY_FILE_NAME, __FUNCTION__, __LINE__
#endif

#define CALLER_INFO_QUERY_PRINT_LOG(level, label, fmt, ...)                                                \
    ((void)HILOG_IMPL(LOG_CORE, level, CALLER_INFO_QUERY_DOMAIN_ID, CALLER_INFO_QUERY_MODULE_LABEL[label], \
        CALLER_INFO_QUERY_FUNC_FMT fmt, CALLER_INFO_QUERY_FUNC_INFO, ##__VA_ARGS__))

#define CALLER_INFO_QUERY_HILOGD(label, fmt, ...) CALLER_INFO_QUERY_PRINT_LOG(LOG_DEBUG, label, fmt, ##__VA_ARGS__)
#define CALLER_INFO_QUERY_HILOGI(label, fmt, ...) CALLER_INFO_QUERY_PRINT_LOG(LOG_INFO, label, fmt, ##__VA_ARGS__)
#define CALLER_INFO_QUERY_HILOGW(label, fmt, ...) CALLER_INFO_QUERY_PRINT_LOG(LOG_WARN, label, fmt, ##__VA_ARGS__)
#define CALLER_INFO_QUERY_HILOGE(label, fmt, ...) CALLER_INFO_QUERY_PRINT_LOG(LOG_ERROR, label, fmt, ##__VA_ARGS__)
#define CALLER_INFO_QUERY_HILOGF(label, fmt, ...) CALLER_INFO_QUERY_PRINT_LOG(LOG_FATAL, label, fmt, ##__VA_ARGS__)
}
}

#endif  // CALLER_INFO_QUERY_HILOG_H
