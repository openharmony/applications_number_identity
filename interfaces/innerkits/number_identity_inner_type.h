/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef NUMBER_LOCATION_INNER_TYPE_H
#define NUMBER_LOCATION_INNER_TYPE_H

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

namespace OHOS {
namespace Telephony {
/**
 * @brief Indicates Maximum length of a string.
 */
constexpr int16_t maxNumberLen = 100;

struct BlockMapStr {
    int32_t block_id = 0;
    int32_t block_size = 0;
    int32_t blockFilePos = 0;
};

enum class MMIAPI_DEV_GET_CTRL_ACTION {
    BLOCK_ID_PHONENUMLIST,
    BLOCK_ID_AUTODELFILEFLAG,
    BLOCK_ID_MAXCITYNUM,
    BLOCK_ID_MAXQUHAONUM,
    BLOCK_ID_MAXPHONENUMPREFIX,
    BLOCK_ID_PHONENUMPREFIXLIST,
    BLOCK_ID_CITYNAMEDATA,
    BLOCK_ID_QUHAODATA,
    BLOCK_ID_CITYNAMEDATA_GBK,
    BLOCK_ID_HAS_MOBILE_OP = 9,
    BLOCK_ID_MOBILE_OP_INDEX = 10,
    BLOCK_ID_MOBILE_OP_DATA = 11,
    BLOCK_ID_MOBILE_OP_NAMES = 12,
    BLOCK_ID_MAXNUM
};

enum class E_MOBILE_OPS {
    E_MOBILE_OP_NONE,
    E_MOBILE_OP_CMCC,
    E_MOBILE_OP_CUCC,
    E_MOBILE_OP_CTCC,
    E_MOBILE_OP_MAX = 10000
};

struct MET_SIXNUMINDEX {
    char phoneNum[maxNumberLen];
    short index;
};
} // namespace Telephony
} // namespace OHOS
#endif // NUMBER_LOCATION_INNER_TYPE_H
