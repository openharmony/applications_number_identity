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

import rpc from '@ohos.rpc';
import type common from '@ohos.app.ability.common';
import dataShare from '@ohos.data.dataShare';

import LogUtils from '../common/utils/LogUtils';
import {
  BUNDLE_NAME,
  ERR_FAIL,
  ERR_OK,
  FETCH_FILE_TIMEOUT,
  NUMBER_IDENTITY_SERVICE_DESCRIPTOR,
  NUMBER_MARK_DATA_ZIP_FILE,
  NUMBER_MARK_DATA_FILE_MODULE_NAME,
} from '../common/constants';
import { ServiceExtensionConnection } from '../common/ServiceExtensionConnection';
import { NumberMarkDataShareClient, createNumberMarkDataShareHelper } from './NumberMarkDataShareClient';

enum NumberIdentityServiceCommand {
  IMPORT_NUMBER_MARK_DATA_FILE = 0,
}

export class NumberIdentityService extends rpc.RemoteObject implements rpc.IRemoteBroker {
  private readonly TAG = 'NumberIdentityService';
  constructor(public readonly context: common.ServiceExtensionContext) {
    super(NUMBER_IDENTITY_SERVICE_DESCRIPTOR);
  }

  public asObject(): rpc.IRemoteObject {
    return this;
  }

  public override async onRemoteMessageRequest(
    code: number,
    data: rpc.MessageSequence,
    reply: rpc.MessageSequence,
    options: rpc.MessageOption
  ): Promise<boolean> {
    const descriptor = data.readInterfaceToken();
    if (descriptor !== this.getDescriptor()) {
      LogUtils.e(this.TAG, `Descriptor check failed, got ${descriptor}`);
      return false;
    }
    try {
      switch (code) {
        case NumberIdentityServiceCommand.IMPORT_NUMBER_MARK_DATA_FILE:
          return await this.importNumberMarkDataFile(reply);
        default:
          LogUtils.e(this.TAG, `Message code not supported: ${code}.`);
          return false;
      }
    } catch (error) {
      LogUtils.e(this.TAG, `onRemoteMessageRequest error: ${JSON.stringify(error)}`);
      return false;
    }
  }

  private async importNumberMarkDataFile(reply: rpc.MessageSequence): Promise<boolean> {
    LogUtils.i(this.TAG, 'Import number mark data file begin.');
    let ret = ERR_FAIL;
    let connection: ServiceExtensionConnection | null = null;
    let helper: dataShare.DataShareHelper | null = null;
    try {
      helper = await createNumberMarkDataShareHelper(this.context);
      if (helper == null) {
        return true;
      }
      const client = new NumberMarkDataShareClient(helper, this.context);
      if (!(await client.importNumberMarkData(""))) {
        return true;
      }
      ret = ERR_OK;
      return true;
    } catch (error) {
      LogUtils.e(this.TAG, `Import number mark data file failed: ${JSON.stringify(error)}`);
      return true;
    } finally {
      await helper?.close();
      await connection?.disconnect();
      reply.writeInt(ret);
    }
  }
}
