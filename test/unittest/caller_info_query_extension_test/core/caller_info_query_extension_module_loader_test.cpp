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

#include "gtest/hwext/gtest-ext.h"
#include "gtest/hwext/gtest-tag.h"
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "caller_info_query_extension_callback_data.h"
#define private public
#include "caller_info_query_extension_module_loader.h"
#undef private
#include "caller_info_query_stub.h"
#include "caller_info_query_stub_impl.h"
#include "event_runner.h"
#include "ipc_object_stub.h"
#include "js_caller_info_query_extension.h"
#include "js_native_api_types.h"
#include "message_option.h"
#include "message_parcel.h"
#include "runtime.h"

using namespace std;
namespace OHOS {
namespace CallerInfoQuery {
using namespace testing::ext;
class CallerInfoQueryExtensionModuleLoaderTest : public testing::Test {
    public:
        void SetUp() override;
        void TearDown() override;
        static void SetUpTestCase();
        static void TearDownTestCase();
};

void CallerInfoQueryExtensionModuleLoaderTest::SetUp() {
    std::shared_ptr<JsCallerInfoQueryExtension> extension = nullptr;
}

void CallerInfoQueryExtensionModuleLoaderTest::TearDown() {
}

void CallerInfoQueryExtensionModuleLoaderTest::SetUpTestCase() {}

void CallerInfoQueryExtensionModuleLoaderTest::TearDownTestCase() {}

HWTEST_F(CallerInfoQueryExtensionModuleLoaderTest, CallerInfoQueryExtensionModuleLoaderTest_001, TestSize.Level0)
{
    std::unique_ptr<AbilityRuntime::Runtime> runtime = nullptr;
    CallerInfoQueryExtensionModuleLoader loader;
    auto ptr = CallerInfoQueryExtensionModuleLoader::GetInstance().Create(runtime);
    ASSERT_NE(ptr, nullptr);
    auto param = CallerInfoQueryExtensionModuleLoader::GetInstance().GetParams();
    auto iter = param.find("type");
    ASSERT_EQ(iter != param.end(), true);
    ASSERT_EQ(iter->second, "25");
};

} // namespace Telephony
} // namespace OHOS
