/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef NUMBER_LOCATION_GTEST
#define NUMBER_LOCATION_GTEST

#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <string_ex.h>
#include <thread>
#include <unordered_set>

namespace OHOS {
namespace Telephony {
constexpr int16_t SLEEP_THREE_SECONDS = 3;

class NumberLocationGtest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::string GeneratingNumber(int32_t number);

private:
    static constexpr int DECIMAL_NUMBER = 10;
    static constexpr const char *COLUMN_NUMBER_LOCATION = "number_location";
};
} // namespace Telephony
} // namespace OHOS

#endif