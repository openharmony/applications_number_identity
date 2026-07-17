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
#include "caller_info_query_stub.h"
#include "caller_info_query_stub_impl.h"
#include "event_runner.h"
#include "ipc_object_stub.h"
#include "js_caller_info_query_extension.h"
#include "js_native_api_types.h"
#include "message_option.h"
#include "message_parcel.h"

using namespace std;
namespace OHOS {
namespace CallerInfoQuery {
using namespace testing::ext;
class CallerInfoQueryStubTest : public testing::Test {
    public:
        void SetUp() override;
        void TearDown() override;
        static void SetUpTestCase();
        static void TearDownTestCase();
        static std::shared_ptr<CallerInfoQueryStub> stubPtr;
};

std::shared_ptr<CallerInfoQueryStub> CallerInfoQueryStubTest::stubPtr = nullptr;
void CallerInfoQueryStubTest::SetUp() {
    std::shared_ptr<JsCallerInfoQueryExtension> extension = nullptr;
    stubPtr = std::make_shared<CallerInfoQueryStubImpl>(extension);
}

void CallerInfoQueryStubTest::TearDown() {
    stubPtr = nullptr;
}

void CallerInfoQueryStubTest::SetUpTestCase() {}

void CallerInfoQueryStubTest::TearDownTestCase() {}

HWTEST_F(CallerInfoQueryStubTest, CallerInfoQueryStubTest_001, TestSize.Level0)
{
    std::string phoneNumber = "111111";
    QueryCallerInfoResult info;
    stubPtr->OnQueryCallerInfo(phoneNumber, info, 500);
    ASSERT_EQ(info.code_, ERR_TIME_OUT);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(stubPtr->GetDescriptor());
    data.WriteString("111111");
    data.WriteInt32(500);
    stubPtr->OnRemoteRequest( CallerInfoQueryStub::CallerInfoQueryCode::QUERY_CALLER_INFO, data, reply, option);
    auto ret = reply.ReadInt32();
    auto errMsg = reply.ReadString();
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(errMsg.empty(), true);
};

} // namespace Telephony
} // namespace OHOS
