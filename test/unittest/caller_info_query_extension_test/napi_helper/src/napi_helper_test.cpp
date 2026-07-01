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
#include <sstream>
#include <vector>
#include "base/notification/common_event_service/services/include/ordered_event_handler.h"
#include "call_napi_async_data.h"
#include "caller_info_query_extension_callback_data.h"
#include "caller_info_query_stub.h"
#include "event_runner.h"
#include "js_caller_info_query_extension.h"
#include "js_native_api_types.h"

using namespace std;
namespace OHOS {
namespace CallerInfoQuery {
using namespace testing::ext;
class CallerInfoQueryExtensionCallbackDataTest : public testing::Test {
    public:
        void SetUp() override;
        void TearDown() override;
        static void SetUpTestCase();
        static void TearDownTestCase();
        static std::shared_ptr<EventFwk::EventHandler> eventHdr_;
        static std::shared_ptr<EventFwk::EventRunner> eventRunner_;
};

std::shared_ptr<EventFwk::EventHandler> CallerInfoQueryExtensionCallbackDataTest::eventHdr_ = nullptr;
std::shared_ptr<EventFwk::EventRunner> CallerInfoQueryExtensionCallbackDataTest::eventRunner_ = nullptr;

void CallerInfoQueryExtensionCallbackDataTest::SetUp() {
    eventRunner_ = EventFwk::EventRunner::Create("ExtHandler", AppExecFwk::ThreadMode::FFRT);
    eventHdr_ = make_shared<EventFwk::EventHandler>(eventRunner_);
}

void CallerInfoQueryExtensionCallbackDataTest::TearDown() {}

void CallerInfoQueryExtensionCallbackDataTest::SetUpTestCase() {}

void CallerInfoQueryExtensionCallbackDataTest::TearDownTestCase() {}

HWTEST_F(CallerInfoQueryExtensionCallbackDataTest, AsyncDataTst_001, TestSize.Level0)
{
    std::shared_ptr<CallerInfoAsyncData> data = std::make_shared<CallerInfoAsyncData>();
    ASSERT_NE(data, nullptr);
    auto result = data->GetQueryCallerInfoResult();
    ASSERT_EQ(result.code_, ERR_TIME_OUT);
    eventHdr_->PostTask([data](){
        napi_env env = nullptr;
        napi_value context = nullptr;
        data->HandlePromiseResolve(env, context);
    });
    bool ret = data->Await(200);
    ASSERT_EQ(ret, false);
};

HWTEST_F(CallerInfoQueryExtensionCallbackDataTest, AsyncDataTst_002, TestSize.Level0)
{
    std::shared_ptr<CallerInfoAsyncData> data = std::make_shared<CallerInfoAsyncData>();
    ASSERT_NE(data, nullptr);
    auto result = data->GetQueryCallerInfoResult();
    ASSERT_EQ(result.code_, ERR_TIME_OUT);
    eventHdr_->PostTask([data](){
        napi_env env = nullptr;
        napi_value context = nullptr;
        data->HandlePromiseReject(env, context);
    });
    bool ret = data->Await(200);
    ASSERT_EQ(ret, false);
}

HWTEST_F(CallerInfoQueryExtensionCallbackDataTest, AsyncDataTst_003, TestSize.Level0)
{
    std::shared_ptr<AsyncDataBase> data = std::make_shared<AsyncDataBase>();
    ASSERT_NE(data, nullptr);
    eventHdr_->PostTask([data](){
        napi_env env = nullptr;
        napi_value context = nullptr;
        data->HandlePromiseReject(env, context);
    });
    bool ret = data->Await(200);
    ASSERT_EQ(ret, false);
    eventHdr_->PostTask([data](){
        napi_env env = nullptr;
        napi_value context = nullptr;
        data->HandlePromiseResolve(env, context);
    });
    ret = data->Await(200);
    ASSERT_EQ(ret, true);
}

} // namespace Telephony
} // namespace OHOS
