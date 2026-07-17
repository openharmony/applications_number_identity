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

import common from '@ohos.app.ability.common';
import dataShare from '@ohos.data.dataShare';
import dataSharePredicates from '@ohos.data.dataSharePredicates';
import zlib from '@ohos.zlib';
import fileIo from '@ohos.file.fs';
import type { ValuesBucket } from '@ohos.data.ValuesBucket';

import LogUtils from '../common/utils/LogUtils';
import {
  FIELD_CLASSIFY,
  FIELD_DATA,
  FIELD_DESCRIPTION,
  FIELD_ID,
  FIELD_IS_CLOUD,
  FIELD_IS_INTELLIGENT_DB,
  FIELD_MARKED_COUNT,
  FIELD_MARKER_MAPPING,
  FIELD_NAME,
  FIELD_NUMBER,
  FIELD_SAVE_TIMESTAMP,
  FIELD_SOURCE_MAPPING,
  FIELD_SUPPLIER,
  FIELD_SUPPLIER_ID,
  NUMBER_MARK_ABILITY_URI,
  NUMBER_MARK_DATA_FILE,
  NUMBER_MARK_DATA_URI,
  UPDATE_TIMESTAMP,
  UPDATE_TIMESTAMP_URI,
} from '../common/constants';
import { isNotNull, isObject, isRecord, isString } from '../common/utils/Misc';

interface NumberMarkRawData {
  [FIELD_DATA]: string[];
  [FIELD_SOURCE_MAPPING]: Record<string, string>;
  [FIELD_MARKER_MAPPING]: Record<string, string>;
}

interface NumberMarkDataRecord {
  md5: string;
  markerId: string;
  markerCnt: number;
  source: string;
  /**
   * Currently unused.
   */
  intercept: boolean;
}

interface NumberMarkEntity extends ValuesBucket {
  [FIELD_ID]: number | null;
  [FIELD_NUMBER]: string | null;
  [FIELD_NAME]: string | null;
  [FIELD_CLASSIFY]: string | null;
  [FIELD_MARKED_COUNT]: number | null;
  [FIELD_IS_CLOUD]: number | null;
  [FIELD_DESCRIPTION]: string | null;
  [FIELD_SAVE_TIMESTAMP]: string | null;
  [FIELD_SUPPLIER]: string | null;
  [FIELD_SUPPLIER_ID]: string | null;
  [FIELD_IS_INTELLIGENT_DB]: boolean;
}

export async function createNumberMarkDataShareHelper(
  context: common.ServiceExtensionContext
): Promise<dataShare.DataShareHelper | null> {
  try {
    return await dataShare.createDataShareHelper(context, NUMBER_MARK_ABILITY_URI);
  } catch (error) {
    LogUtils.e('CreateNumberMarkDataShareHelper', `datashare helper create failed: ${JSON.stringify(error)}`);
    return null;
  }
}

export class NumberMarkDataShareClient {
  private readonly TAG = 'NumberMarkDataShareClient';
  private readonly helper: dataShare.DataShareHelper;
  private readonly context: common.ServiceExtensionContext;
  constructor(helper: dataShare.DataShareHelper, context: common.ServiceExtensionContext) {
    this.helper = helper;
    this.context = context;
  }

  async importNumberMarkData(dataFile: string): Promise<boolean> {
    if (!(await this.unzipFile(dataFile, this.context.cacheDir))) {
      return false;
    }
    const data = await this.readFileAsText(`${this.context.cacheDir}/${NUMBER_MARK_DATA_FILE}`);
    if (data == null) {
      return false;
    }
    const rawData = this.parseNumberMarkRawData(data);
    if (rawData == null) {
      return false;
    }
    let logged = false;
    const records = rawData[FIELD_DATA].map((record, i) => {
      const parsed = this.parseNumberMarkRecord(record);
      if (!parsed && !logged) {
        LogUtils.w(this.TAG, `Got invalid record format at ${i}.`);
        logged = true;
      }
      return parsed;
    }).filter(isNotNull);
    if (!records.length) {
      LogUtils.w(this.TAG, `No record to import.`);
      return true;
    }
    const now = Date.now();
    const values = records.map((record) => this.toNumberMarkEntity(record, rawData, now));
    let ret = await this.helper.batchInsert(NUMBER_MARK_DATA_URI, values);
    LogUtils.i(this.TAG, `batchInsert ret = ${ret}`);
    ret = await this.helper.delete(
      NUMBER_MARK_DATA_URI,
      new dataSharePredicates.DataSharePredicates()
        .equalTo(FIELD_IS_INTELLIGENT_DB, true)
        .and()
        .lessThan(FIELD_SAVE_TIMESTAMP, `${now}`)
    );
    LogUtils.i(this.TAG, `delete ret = ${ret}`);
    ret = await this.helper.update(UPDATE_TIMESTAMP_URI, new dataSharePredicates.DataSharePredicates(), {
      [UPDATE_TIMESTAMP]: now,
    });
    LogUtils.i(this.TAG, `update ret = ${ret}`);
    return true;
  }

  private async unzipFile(inFile: string, outFile: string): Promise<boolean> {
    try {
      await zlib.decompressFile(inFile, outFile, {
        level: zlib.CompressLevel.COMPRESS_LEVEL_DEFAULT_COMPRESSION,
        memLevel: zlib.MemLevel.MEM_LEVEL_DEFAULT,
        strategy: zlib.CompressStrategy.COMPRESS_STRATEGY_DEFAULT_STRATEGY,
      });
      return true;
    } catch (error) {
      LogUtils.e(this.TAG, `unzip failed: ${JSON.stringify(error)}`);
      return false;
    }
  }

  private async readFileAsText(path: string): Promise<string | null> {
    try {
      return await fileIo.readText(path, { encoding: 'utf-8' });
    } catch (error) {
      LogUtils.e(this.TAG, `read file failed: ${JSON.stringify(error)}`);
      return null;
    }
  }

  private parseNumberMarkRawData(data: string): NumberMarkRawData | null {
    let rawData: unknown = null;
    try {
      rawData = JSON.parse(data);
    } catch (error) {
      LogUtils.e(this.TAG, `Invalid JSON format: ${JSON.stringify(error)}`);
      return null;
    }
    if (!isObject(rawData)) {
      LogUtils.e(this.TAG, `Expect root to be object.`);
      return null;
    }
    if (!(FIELD_DATA in rawData && Array.isArray(rawData[FIELD_DATA]) && rawData[FIELD_DATA].every(isString))) {
      LogUtils.e(this.TAG, `Expect "data" to be string[].`);
      return null;
    }
    if (
      !(
        FIELD_SOURCE_MAPPING in rawData &&
        isObject(rawData[FIELD_SOURCE_MAPPING]) &&
        isRecord(rawData[FIELD_SOURCE_MAPPING], isString)
      )
    ) {
      LogUtils.e(this.TAG, `Expect "source_mapping_ds" to be Record<string, string>.`);
      return null;
    }
    if (
      !(
        FIELD_MARKER_MAPPING in rawData &&
        isObject(rawData[FIELD_MARKER_MAPPING]) &&
        isRecord(rawData[FIELD_MARKER_MAPPING], isString)
      )
    ) {
      LogUtils.e(this.TAG, `Expect "marker_mapping_ds" to be Record<string, string>.`);
      return null;
    }
    // Only returns the known fields, not the entire object.
    return {
      [FIELD_DATA]: rawData[FIELD_DATA],
      [FIELD_MARKER_MAPPING]: rawData[FIELD_MARKER_MAPPING],
      [FIELD_SOURCE_MAPPING]: rawData[FIELD_SOURCE_MAPPING],
    };
  }

  private parseNumberMarkRecord(record: string): NumberMarkDataRecord | null {
    const arr = record.split('|');
    let i = 0;
    const md5 = arr[i++];
    const markerId = arr[i++];
    const markerCnt = parseInt(arr[i++]);
    const source = arr[i++];
    const intercept = arr[i++] === '1';
    if (md5 && markerId && !isNaN(markerCnt) && source) {
      return {
        md5,
        markerId,
        markerCnt,
        source,
        intercept,
      };
    }
    return null;
  }

  private toNumberMarkEntity(record: NumberMarkDataRecord, rawData: NumberMarkRawData, now: number): NumberMarkEntity {
    return {
      [FIELD_ID]: null, // auto generated
      [FIELD_NUMBER]: record.md5,
      [FIELD_NAME]: rawData[FIELD_MARKER_MAPPING][record.markerId] || null,
      [FIELD_CLASSIFY]: this.getClassify(record.markerId),
      [FIELD_MARKED_COUNT]: record.markerCnt,
      [FIELD_IS_CLOUD]: 1,
      [FIELD_DESCRIPTION]: null,
      [FIELD_SAVE_TIMESTAMP]: `${now}`,
      [FIELD_SUPPLIER]: rawData[FIELD_SOURCE_MAPPING][record.source] || null,
      [FIELD_SUPPLIER_ID]: null,
      [FIELD_IS_INTELLIGENT_DB]: true,
    };
  }

  private getClassify(markerId: string): string {
    switch (markerId) {
      case '101':
        return 'express';
      case '102':
        return 'taxi';
      case '105':
        return 'insurance';
      case '107':
        return 'house_agent';
      case '108':
        return 'promote_sales';
      case '109':
        return 'crank';
      case '110':
        return 'fraud';
      case '103':
      case '104':
      case '106':
      default:
        return 'others';
    }
  }
}
