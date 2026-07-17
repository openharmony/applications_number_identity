/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import type workScheduler from '@ohos.resourceschedule.workScheduler';
import WorkSchedulerExtensionAbility from '@ohos.WorkSchedulerExtensionAbility';
import LogUtils from '../common/utils/LogUtils';
const TAG = 'DownloadFileWorkSheduler';
const URI = 'datashare:///com.ohos.downloadfileability';
import dataSharePredicates from '@ohos.data.dataSharePredicates';
import dataShare from '@ohos.data.dataShare';

export default class DownloadFileWorkSheduler extends WorkSchedulerExtensionAbility {
  async onWorkStart(workInfo: workScheduler.WorkInfo): Promise<void> {
    LogUtils.i(TAG, 'DownloadFileWorkSheduler begin');
    if (!workInfo) {
      LogUtils.e(TAG, 'workInfo is undefined');
      return;
    }
    let selectsEq = new dataSharePredicates.DataSharePredicates();
    let queryData = {};
    let workId = workInfo.workId;
    queryData = {
      'task_type':'work_sheduler',
      'work_id': `${workId}`
    };
    LogUtils.i(TAG, 'DownloadFileWorkSheduler workId:' + workId);
    try {
      let dataShareHelper = await dataShare.createDataShareHelper(this.context, URI);
      if (!dataShareHelper) {
        LogUtils.e(TAG, 'dataShareHelper is undefined');
        return;
      }
      dataShareHelper.update(URI, selectsEq, queryData, (err, data) => {
        if (err !== undefined) {
          LogUtils.e(TAG, `update error: code: ${err.code}, message: ${err.message} `);
          return;
        }
        LogUtils.i(TAG, 'update succeed, rowCount : ' + data);
      });
    } catch (err) {
      LogUtils.e(TAG, 'queryForConfig err:' + JSON.stringify(err));
    }
  }
}