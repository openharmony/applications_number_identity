/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#include "numberlocation_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#define protected public
#include "number_location_ability.h"

using namespace OHOS::Telephony;
namespace OHOS {

void OnNumberLocationAbilityQuery(const uint8_t *data, size_t size)
{
    auto service = NumberLocationAbility::Create();
    if (service == nullptr) {
        return;
    }
    std::string uriStr(reinterpret_cast<const char *>(data), size);
    OHOS::Uri uri(uriStr);

    DataShare::DataSharePredicates predicates;
    std::vector<std::string> phoneNumbers;
    std::string number(reinterpret_cast<const char *>(data), size);
    phoneNumbers.push_back(number);
    predicates.SetWhereArgs(phoneNumbers);

    std::vector<std::string> columns;
    std::string isExactMatchStr(reinterpret_cast<const char *>(data), size);
    columns.push_back(isExactMatchStr);

    DataShare::DatashareBusinessError businessError;
    service->Query(uri, predicates, columns, businessError);
    return;
}

void DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    OnNumberLocationAbilityQuery(data, size);
    return;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
