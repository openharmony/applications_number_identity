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

export const noop = (): void => {};

export function withTimeout<T>(promise: Promise<T>, timeout: number): Promise<T> {
  let timer: ReturnType<typeof setTimeout> | null = null;
  const cleanup = (): void => {
    if (timer != null) {
      clearTimeout(timer);
      timer = null;
    }
  };
  return Promise.race([
    promise.finally(cleanup),
    new Promise<T>((_resolve, reject) => {
      timer = setTimeout(() => {
        cleanup();
        reject(`Timeout ${timeout} reached.`);
      }, timeout);
    }),
  ]);
}

export function isObject(value: unknown): value is object {
  return typeof value === 'object' && value != null;
}

export function isString(value: unknown): value is string {
  return typeof value === 'string';
}

export function isNotNull<T>(value: T | null): value is T {
  return value != null;
}

export function isRecord<T>(value: object, predicate: (value: unknown) => value is T): value is Record<string, T> {
  return Object.values(value).every(predicate);
}
