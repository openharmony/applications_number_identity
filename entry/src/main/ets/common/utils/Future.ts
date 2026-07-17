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

import { noop } from './Misc';

export enum FutureStatus {
  Pending,
  Fullfilled,
  Rejected,
}

type PromiseInit<T> = ConstructorParameters<typeof Promise<T>>[0];

export class Future<T> {
  public resolve: Parameters<PromiseInit<T>>[0] = noop;
  public reject: Parameters<PromiseInit<T>>[1] = noop;
  private readonly promise: Promise<T>;
  private status: FutureStatus = FutureStatus.Pending;
  constructor() {
    this.promise = new Promise<T>((resolve, reject) => {
      this.resolve = (value): void => {
        if (this.status !== FutureStatus.Pending) {
          return;
        }
        resolve(value);
        this.status = FutureStatus.Fullfilled;
      };
      this.reject = (reason): void => {
        if (this.status !== FutureStatus.Pending) {
          return;
        }
        reject(reason);
        this.status = FutureStatus.Rejected;
      };
    });
  }

  public asPromise(): Promise<T> {
    return this.promise;
  }

  public getStatus(): FutureStatus {
    return this.status;
  }
}
