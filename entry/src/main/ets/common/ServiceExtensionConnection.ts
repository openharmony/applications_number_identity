/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import type common from '@ohos.app.ability.common';
import type rpc from '@ohos.rpc';

import LogUtils from './utils/LogUtils';

export class ServiceExtensionConnection {
  private TAG: string;
  private context: common.ServiceExtensionContext;
  private connectionId: number | null;
  private remote: rpc.IRemoteObject | null;
  constructor(context: common.ServiceExtensionContext, remote: rpc.IRemoteObject, connectionId: number) {
    this.context = context;
    this.remote = remote;
    this.connectionId = connectionId;
    this.TAG = `IPCConnection(${connectionId})`;
  }
  async disconnect(): Promise<void> {
    if (this.connectionId != null) {
      LogUtils.i(this.TAG, `disconnecting`);
      try {
        await this.context.disconnectServiceExtensionAbility(this.connectionId);
      } catch (error) {
        LogUtils.e(this.TAG, `disconnect error: ${JSON.stringify(error)}`);
      }
    }
    this.connectionId = null;
    this.remote = null;
    LogUtils.i(this.TAG, `disconnected`);
  }
  getRemote(): rpc.IRemoteObject | null {
    if (!this.remote) {
      LogUtils.e(this.TAG, `remote is null`);
      return null;
    }
    if (this.remote.isObjectDead()) {
      LogUtils.e(this.TAG, `remote is dead`);
      return null;
    }
    return this.remote;
  }
}
