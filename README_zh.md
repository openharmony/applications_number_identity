# 号码识别（Number Identity）

## 简介

**号码识别**（包名：`com.ohos.numberidentity`）是 OpenHarmony 电话子系统中的系统部件，以系统 HAP 与原生共享库形式部署，提供**号码归属地、号码标记与号码黄页**等能力，并向通话、联系人等应用提供 DataShare 与 NAPI 数据服务。

本部件为系统预置组件，无面向用户的独立业务界面（`entry` 中页面为占位骨架）。黄页、归属地、标记等**业务查询**均从**本地数据库或数据文件**读取，**不走实时在线查询接口**。工程内仍保留 `DownloadFile` / `DownloadFileWorkSheduler` 及 URI `datashare://com.ohos.downloadfileability` 等相关实现与网络权限声明，但**数据在线更新暂不作为对外核心能力**（产品侧可按需裁剪或禁用）。工程 `etc/` 中的 `numberlocation.data`、`yellowpage.data` 仅为**示例数据**（号段与黄页条目很少），产品正式数据需自行补充；设备侧系统预置路径为 `/system/etc/telephony/`。归属地另支持二进制库 `numberlocation.dat`（与 `.data` 查询逻辑不同，详见下文与开发章节）。

### 核心能力

**号码归属地**
- 支持两套本地数据源，查询时**优先 `.data`，未命中再回退 `.dat`**（见 `NumberLocationUtils`）：
  - **`numberlocation.data`（文本 JSON 行）**：首行 `{"version":"..."}`，后续每行一条号段记录（`prefix`、`operator` 必填；`province` / `city` 或 `location` 可选）。由 `NumberLocationParser` 解析，按 7 位号段优先、再 3 位前缀匹配。
  - **`numberlocation.dat`（二进制库）**：块映射 / 城市名等二进制结构，由 `NumberLocationDbParse` 的 `QueryPhoneNumberLocation`、`QueryTelNumberLocation` 等解析；固话归属地主要依赖此路径。当前 `etc/BUILD.gn` 默认**仅安装** `.data`（`.dat` 目标已注释），仓库内也未附带完整 `.dat` 示例文件。
- 设备路径均先读应用沙箱更新文件，再读系统预置：
  - `.data`：`/data/storage/el2/base/files/numberlocation.data` → `/system/etc/telephony/numberlocation.data`
  - `.dat`：`/data/storage/el2/base/files/numberlocation.dat` → `/system/etc/telephony/numberlocation.dat`
- 通过 `NumberLocationManager`、`MobilePhoneNumber`、`FixPhoneNumber` 等实现号段与固话区号匹配。
- DataShare URI：`datashare://com.ohos.numberlocationability`；NAPI 提供 `getNumberLocation` 等接口。

**号码标记**
- 支持对陌生号码打标（骚扰、诈骗、广告等）及查询、更新、删除。
- 数据写入 `number_mark` 相关表，经 `com.ohos.numbermarkability` 对外服务。
- 标记查询入口含 `NumberMarkAbility::QueryByPhoneNumber` 等 C++ 实现，查询顺序为本地黄页 → 本地用户标记；前两步未命中时检查 Settings 中陌生号码识别开关，**开关关闭则返回空标记**（当前该主路径在开关开启时亦无额外查询步骤）。

**号码黄页**
- 解析 `yellowpage.data`（文本 JSON 行）并导入 RDB，查询时优先匹配黄页记录。
- 由 `yellow_page/` 模块（如 `YellowPageParser`）完成解析，经 `NumberIdentityDatabase::ImportYellowPageData` 入库。

## 架构说明

Number Identity 采用分层与模块化设计，按产品入口、业务特性与公共能力组织代码，如图：

![架构说明](./figures/numberidentity.png)

### 应用层分层设计

整体可划分为产品层、特性层、公共层：

| 层次   | 主要目录 / 组件 | 说明 |
|------| -------------- | ---- |
| 产品层 | `entry/` | 手机 / 平板形态（phone、pad） |
| 特性层 | `number_location/`、`number_mark/`、`yellow_page/` | 号码归属地、号码标记、号码黄页 |
| 公共层 | `shared/`、`etc/`、`frameworks/`、`utils/`、`interfaces/` | 共享 RDB、预置数据、NAPI、日志工具、接口定义、DFX、CallerInfoQuery |

**特性层模块说明**：

| 核心能力   | 模块 | 说明 |
|--------|----------------|------|
| 号码归属地   | `number_location/`（`NumberLocationManager`、`NumberLocationAbility`） | 归属地解析与 DataShare 查询 |
| 号码标记   | `number_mark/`（`NumberMarkManager`、`NumberMarkAbility`） | 用户标记维护与查询 |
| 号码黄页   | `yellow_page/`（`YellowPageParser`） | 黄页文件解析与 RDB 导入 |

### 与其它应用的关系

Number Identity 向 **CallUI**、**Contacts** 等系统应用提供黄页、归属地、标记等号码数据服务，不直接承载通话或联系人界面；电话核心能力由 Telephony 子系统提供。与 **Settings（设置 / 电话设置）** 通过系统 SettingsData 协同：本部件读写设置项，设置侧提供开关入口与持久化。

**调用方式**：

- CallUI、Contacts 通过 DataShare Helper 访问 `com.ohos.numberlocationability`、`com.ohos.numbermarkability` 等 URI。
- 应用侧亦可使用 NAPI 模块 `telephony.numberidentity`、`contact.numberlookup`（同一套实现，模块名不同）。
- 第三方应用可实现 `CallerInfoQueryExtensionAbility`（`interfaces/kits/` 基类）扩展企业来电信息查询。

**与 Settings 的关系**：

- 本部件通过 `shared` 中 `GetSettingsData` / `InsertSettingsData` / `UpdateSettingsData` 访问 SettingsData（URI：`datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA`）。
- 关键键：`settings.telephony.number_identity_switch`（值为 `"1"` 表示开启）。标记查询在本地黄页、本地用户标记均未命中时读取该开关：关闭则返回空标记；开启时当前 `QueryByPhoneNumber` 主路径亦直接结束（无额外在线路由）。**归属地查询不依赖该开关**。黄页与用户本地标记结果不受该开关拦截。
- 开关的用户可见入口一般在电话设置 / 设置相关页面（本仓无对应 UI），由设置侧写入 SettingsData；CallUI 等消费方亦可同步读取该键以控制展示策略。读写通常需 `ohos.permission.MANAGE_SETTINGS`。

**调用场景**：

来电界面展示标记与归属地、联系人详情号码解析、用户标记陌生号码、在设置中开关陌生号码识别等。

## 编译构建

本工程支持 **Hvigor 独立构建 HAP** 与 **OpenHarmony 系统 GN 合入**（HAP + 原生共享库 + 预置数据），产物包名 `com.ohos.numberidentity`。

### 环境要求
- OpenHarmony SDK（Hvigor 工程 `compileSdkVersion` 为 23，`compatibleSdkVersion` / `targetSdkVersion` 为 20）
- OpenHarmony 系统源码树（部件路径：`base/telephony/number_identity`）及 GN 工具链（系统合入时）
- DevEco Studio 或命令行 Hvigor
- 系统签名配置（见 `signature/`；证书与 profile 由产品构建环境提供）

### 编译命令

在工程根目录执行 Hvigor 构建（需本机已配置 `hvigorw`，或使用 DevEco Studio）：

```bash
hvigorw assembleHap
```

系统源码树中合入时，在源码根目录执行：

```bash
./build.sh --product-name {product_name} --build-target number_identity --ccache
```

> **说明**：独立开发可将工程置于 `base/telephony/number_identity` 后随系统编译，或配置系统签名后使用 Hvigor 构建。

### 构建产物（系统 GN）

| 类型 | 产物 / 目标 | 说明 |
| ---- | -------------- | ---- |
| HAP | `NumberIdentity.hap` | entry 模块，安装路径以产品配置为准（如 `/system/app/com.ohos.numberidentityability`） |
| 原生共享库 | `number_identity` | 归属地、标记、黄页与 shared RDB 核心实现 |
| NAPI 共享库 | `numberlookup`、`numberidentity` | 分别供 Contacts、Telephony 侧加载 |
| 预置数据 | `numberlocation.data`、`yellowpage.data`（可选 `numberlocation.dat`） | 默认安装 `.data` / 黄页至 `/system/etc/telephony/`；`.dat` 需在 `etc/BUILD.gn` 中启用并提供文件 |

## Number Identity 开发

Number Identity 采用 **ArkTS + C++** 混合开发：DataShare 与黄页 / 归属地 / 标记核心在 C++ 特性层实现，ArkTS 负责 Extension 声明与服务扩展。扩展 ArkTS 页面可参考：[ArkUI 开发概述](https://gitcode.com/openharmony/docs/blob/master/zh-cn/application-dev/ui/arkts-ui-development-overview.md)

### 基于已有模块的开发

适用场景：扩展归属地 / 黄页 / 标记能力、调整查询优先级或补充 NAPI / CallerInfoQuery 能力。

明确改动点：按业务边界定位到 `number_location/`、`number_mark/`、`yellow_page/`（特性层）或 `shared/`、`frameworks/`（公共层），路由变更需同步 `NumberIdentityDataShareStubImpl`。

以下列举一些常见的修改场景：

**场景1：号码归属地查询链路**

   - 对外编排位于 `number_location/src/number_location_utils.cpp`（先 `.data` 后 `.dat`）
   - `.data` 加载 / 前缀匹配位于 `number_location/src/number_location_db_parse.cpp`（`LoadNumberLocationData`、`QueryLocationFromDataFile`）与 `number_location_parser.cpp`
   - `.dat` 二进制查询位于同文件的 `QueryPhoneNumberLocation`、`QueryTelNumberLocation` 等
   - DataShare / NAPI 入口：`number_location_ability.cpp`、`frameworks/js/src/napi_number_identity.cpp`

    总查询逻辑示意（移动号归属地 / 运营商）：

    ```cpp
    // number_location_utils.cpp — 归属地：优先 .data，未命中再走 .dat
    unicodeInformation = DelayedSingleton<NumberLocationDbParse>::GetInstance()
        ->QueryLocationFromDataFile(phoneNumber.c_str());
    if (!unicodeInformation.empty()) {
        return unicodeInformation;
    }
    // ... 回退 QueryPhoneNumberLocation（.dat）并截取 UTF-8 文本
    ```

**场景2：添加 / 修改归属地 `numberlocation.data`**

   - 源文件：`etc/numberlocation.data`（仓库内为少量示例，正式号段需自行补充）
   - 安装：`etc/BUILD.gn` 中 `number_location_data_default` → `/system/etc/telephony/numberlocation.data`
   - 运行时亦可将完整文件放到沙箱路径（优先于系统预置）：`/data/storage/el2/base/files/numberlocation.data`

    格式：首行版本，之后每行一条 JSON。例如新增号段：

    ```text
    {"version":"1"}
    {"prefix":"1810256","province":"广东","city":"广州","operator":"电信"}
    // ... 既有示例记录 ...
    {"prefix":"1380013","province":"北京","city":"北京","operator":"移动"}
    ```

    补充途径简述：① 改 `etc/` 后重编系统镜像；② `hdc` 推送到沙箱或（需 remount）系统目录；③ 热替换 `.data` 后通常需重启相关进程，因 `LoadNumberLocationData` 进程内只加载一次。

**场景3：添加 / 修改归属地 `numberlocation.dat`**

   - 二进制库，供 `.data` 未命中或固话等走原解析路径时使用；打开顺序为沙箱 → `/system/etc/telephony/numberlocation.dat`
   - 源码侧：将文件置于 `etc/numberlocation.dat`，在 `etc/BUILD.gn` 的 `group("number_location")` 中取消注释 `:number_location_default`
   - 设备侧：可推送到 `/data/storage/el2/base/files/numberlocation.dat`（优先）或系统预置目录

    ```gn
    # etc/BUILD.gn — 启用预置 .dat（需同时提供 etc/numberlocation.dat）
    group("number_location") {
      deps = [
        ":number_location_default",   # 【启用】安装 numberlocation.dat
        ":yellow_page_default",
        ":number_location_data_default",
      ]
    }
    ```

    ```cpp
    // number_location_db_parse.cpp — .dat 打开顺序
    FILE *fp = fopen(PATH_UPDATE, "rb");  // /data/storage/el2/base/files/numberlocation.dat
    if (fp == nullptr) {
        fp = fopen(PATH, "rb");        // /system/etc/telephony/numberlocation.dat
    }
    // ... 块映射解析与 QueryPhoneNumberLocation 等
    ```

**场景4：号码标记**

   - 核心位于 `number_mark/src/number_mark_ability.cpp`、`number_mark_manager.cpp`
   - 增删改查经 `com.ohos.numbermarkability` 路由至 `NumberMarkAbility`

    例如，需在本地黄页命中后增加自定义处理，可在 `QueryByPhoneNumber` 中扩展：

    ```cpp
    // number_mark_ability.cpp — QueryByPhoneNumber 为黄页 / 标记查询入口之一
    int NumberMarkAbility::QueryByPhoneNumber(
        const string &phoneNumber, NumberMarkInfo &markInfo, DatashareBusinessError &businessError)
    {
        // 原有流程：本地黄页 → 本地用户标记 → number_identity_switch 开关判断
        NumberMarkQueryContext context;
        context.phoneNumber = phoneNumber;
        auto &yellowPages = context.dbYellowPages;
        auto &numberMarks = context.dbMarks;
        int errCode = this->QueryLocalYellowPage(phoneNumber, yellowPages, businessError);
        HANDLE_BUSINESS_ERROR("QueryYellowPage", errCode, businessError, return errCode);
        if (auto it = FindYellowPageBestMatch(phoneNumber, yellowPages); it.has_value()) {
            // 【新增自定义处理】黄页命中后可在此扩展
            // CustomProcessYellowPage(*it);
            markInfo.FromYellowPage(*it);
            return SetBusinessError(businessError, errCode);
        }
        // ... 后续本地用户标记查询与开关判断保持不变
        return errCode;
    }
    ```

**场景5：添加 / 修改号码黄页 `yellowpage.data`**

   - 源文件：`etc/yellowpage.data`（仓库内为运营商客服等示例）；解析：`yellow_page/src/yellow_page_parser.cpp`
   - 安装：`etc/BUILD.gn` 中 `yellow_page_default` → `/system/etc/telephony/yellowpage.data`
   - 导入：`NumberIdentityDatabase::ImportYellowPageData`；若库中已存版本（字符串）**≥** 文件 `version` 则跳过；否则清空黄页相关表后全量导入

    格式示意（`photo` 等长字段可省略）：

    ```text
    {"version":"123"}
    {"group":"电信","name":"中国移动","photo":"...","phone":[
      {"hot_points":0,"dial_map":"...","phone":"10086","name":"中国移动"}
    ]}
    // ... 其它黄页记录 ...
    ```

    例如提高版本并追加一条记录后重编镜像，或将新文件放到沙箱 `/data/storage/el2/base/files/yellowpage.data` 后触发 `ImportYellowPageData(path)`：

    ```cpp
    // shared — 按路径导入黄页（文件 version 需高于库内版本才会生效）
    NumberIdentityDatabase::ImportYellowPageData(
        "/data/storage/el2/base/files/yellowpage.data");
    ```

**场景6：DataShare / NAPI 对外接口**

   - DataShare 路由位于 `number_location/src/number_identity_datashare_stub_impl.cpp` 的 `GetOwner`
   - NAPI 双模块注册：

    ```cpp
    // napi_number_identity.cpp — 同一套实现分别注册给 Telephony 与 Contacts
    static napi_module g_nativeNumberIdentityModule = {
        .nm_version = NATIVE_VERSION,
        .nm_flags = NATIVE_FLAGS,
        .nm_filename = nullptr,
        .nm_register_func = NapiNumberIdentity::RegisterNumberIdentityFunc,
        .nm_modname = "telephony.numberidentity",
        .nm_priv = nullptr,
        .reserved = { nullptr },
    };
    static napi_module g_nativeNumberLookupModule = {
        .nm_version = NATIVE_VERSION,
        .nm_flags = NATIVE_FLAGS,
        .nm_filename = nullptr,
        .nm_register_func = NapiNumberIdentity::RegisterNumberIdentityFunc,
        .nm_modname = "contact.numberlookup",
        .nm_priv = nullptr,
        .reserved = { nullptr },
    };
    ```

常用修改入口：

| 目标 | 路径 |
|------|------|
| 进程初始化 | `entry/src/main/ets/Application/NumberLocationAbilityStage.ts` |
| DataShare 路由 | `number_location/src/number_identity_datashare_stub_impl.cpp` |
| 号码归属地 | `number_location/src/number_location_ability.cpp`、`number_location_manager.cpp`、`number_location_utils.cpp`、`number_location_db_parse.cpp` |
| 归属地 / 黄页预置数据 | `etc/numberlocation.data`、`etc/yellowpage.data`（可选 `etc/numberlocation.dat`）、`etc/BUILD.gn` |
| 号码标记 | `number_mark/src/number_mark_ability.cpp` |
| 号码黄页 | `yellow_page/src/yellow_page_parser.cpp`、`shared` 中 `ImportYellowPageData` |
| 标记数据 IPC 服务 | `entry/src/main/ets/service/NumberIdentityServiceExtAbility.ts` |
| NAPI | `frameworks/js/src/napi_number_identity.cpp` |
| CallerInfoQuery 扩展基类 | `interfaces/kits/caller_info_query_extension_ability/` |
| Settings 读写 | `shared/src/number_identity_settings.cpp` |
| RDB 与模型 | `shared/` |
| Ability / DataShare 声明 | `entry/src/main/module.json5` |
| 系统 GN 入口 | `BUILD.gn`、`bundle.json` |

### 新特性能力的开发

适用场景：扩展归属地 / 黄页 / 标记维度、新增 DataShare URI、扩展 NAPI 或补充数据文件格式支持。

> **说明**：特性层代码需编入根目录 `BUILD.gn` 的 `number_identity` 共享库；对外 URI、权限与 NAPI 模块名需与 CallUI、Contacts 等消费方约定一致。

**场景1：扩展 C++ 特性模块**

1. 在 `number_location/`、`number_mark/` 或 `yellow_page/` 下新增实现。
2. 在 `BUILD.gn` 的 `ohos_shared_library("number_identity")` 中注册源文件。
3. 在 `test/unittest/` 中补充 gtest 并在对应 `BUILD.gn` 注册。

**场景2：声明 DataShare 与权限**

如需新 DataShare URI，同步更新 `NumberIdentityDataShareStubImpl::GetOwner` 与 `entry/src/main/module.json5`，例如现有标记能力声明：

```json
{
  "name": "NumberMarkAbility",
  "srcEntry": "./ets/DataShareExtAbility/DataShareExtAbility.ts",
  "readPermission": "ohos.permission.GET_TELEPHONY_STATE",
  "writePermission": "ohos.permission.SET_TELEPHONY_STATE",
  "type": "dataShare",
  "uri": "datashare://com.ohos.numbermarkability",
  "visible": true
}
```

**场景3：对外暴露 NAPI / Kit**

在 `frameworks/js` 注册新接口，并视需要更新 `interfaces/` 下 innerkits / kits；Telephony 与 Contacts 双模块名需同时注册。

**场景4：CallerInfoQuery 扩展**

第三方实现 `CallerInfoQueryExtensionAbility` 的 `onQueryCallerInfo(number)`，由 `frameworks/extension/` 框架 IPC 调度。

## 目录

```text
number_identity
├─AppScope                              # 应用级配置与多语言资源
│  ├─app.json5                          # Hvigor bundleName、版本号等
│  ├─app.json                           # 系统 GN 构建使用的应用配置
│  └─resources/                         # 全局字符串等资源
├─figures/                              # 架构说明图（numberidentity.png）
├─entry                                 # 产品层（HAP 入口模块 numberidentity）
│  └─src/main/
│     ├─ets/
│     │  ├─Application/                 # NumberLocationAbilityStage 进程初始化
│     │  ├─DataShareExtAbility/         # DataShare 占位声明（含 DownloadFileWorkSheduler）
│     │  ├─service/                     # NumberIdentityServiceExtAbility
│     │  ├─pages/                       # ArkTS 占位页（Index.ets）
│     │  ├─common/                      # 产品层：常量、日志、连接工具
│     │  └─backup/                      # 产品层：BackupExtension（backup_config 配置范围）
│     ├─resources/                      # 模块资源、profile
│     ├─module.json5                    # Hvigor：Ability、DataShare、权限
│     └─module.json                     # 系统 GN 模块配置
├─number_location/                      # 特性层：号码归属地、DataShare Stub
│  ├─include/
│  └─src/                               # Manager、Parser、Ability 等
├─number_mark/                          # 特性层：号码标记
│  ├─include/
│  └─src/                               # NumberMarkAbility、NumberMarkManager 等
├─yellow_page/                          # 特性层：号码黄页解析
│  ├─include/
│  └─src/                               # yellow_page_parser 等
├─shared/                               # 公共层：共享 RDB（DDL、Models、工具）
├─frameworks/                           # 公共层：NAPI、CallerInfoQuery、DFX 相关实现
│  ├─js/                                # NAPI（telephony.numberidentity / contact.numberlookup）
│  └─extension/                         # CallerInfoQuery 框架
├─interfaces/                           # 公共层：接口定义（innerkits / kits）
├─etc/                                  # 公共层：预置数据（numberlocation.data、yellowpage.data；可选 numberlocation.dat）
├─utils/log/                            # 公共层：日志工具（日志宏与错误码）
├─tools/                                # 辅助工具目录（可为空）
├─test/                                 # gtest / fuzztest
├─signature/                            # 系统 GN 签名配置
├─hvigor/                               # Hvigor 配置
├─BUILD.gn                              # 系统 GN 构建入口
├─bundle.json                           # 部件归属 telephony 与构建分组
├─build-profile.json5                   # 工程级 SDK 配置
├─oh-package.json5
├─OAT.xml                               # 开源合规审计
├─LICENSE
├─README.md                             # 英文说明文档
└─README_zh.md                          # 中文说明文档
```

## 约束

- **语言版本**：ArkTS（entry 编排层）+ C++（黄页 / 归属地 / 标记与 DataShare 实现）
- **子系统归属**：telephony（见 `bundle.json`）
- **部署路径**：系统源码树 `base/telephony/number_identity`；HAP 安装路径以产品镜像配置为准
- **包名**：`com.ohos.numberidentity`（`AppScope/app.json5`）
- **设备类型**：Hvigor（`entry/src/main/module.json5`）为 `default`、`tablet`；系统 GN（`entry/src/main/module.json`）另含 `2in1`、`wearable`
- **运行形态**：系统预置部件（HAP + 原生 so + 预置数据），**无用户可见主界面**；`entry` 中 `abilities` 为空，能力均由 `extensionAbilities` 提供
- **查询方式**：黄页、归属地、标记**业务查询**仅读本地 RDB / 数据文件，**不走实时在线查询**；工程内保留的 `DownloadFile` 相关能力**暂不作为对外核心能力**
- **预置与设备数据路径**：源码 `etc/` → 设备 `/system/etc/telephony/`（默认 `numberlocation.data`、`yellowpage.data`；`numberlocation.dat` 需自行提供并启用 GN 目标）。沙箱覆盖路径为 `/data/storage/el2/base/files/` 下同名文件（优先于系统预置）
- **归属地数据说明**：仓库内 `.data` 仅为示例号段；正式库需自行补充。移动号查询顺序为 **`.data` → `.dat`**；固话经 `QueryUnicodeInformationByTelNum` / `QueryTelNumberLocation` **主要走 `.dat`**。`.data` 进程内缓存加载一次，热更新后通常需重启相关进程
- **黄页数据说明**：仓库内 `yellowpage.data` 为示例；导入时比较 `version`（字符串比较），库内版本 **≥** 文件版本则跳过，否则清空黄页相关表后全量写入 RDB
- **本地数据库**：RDB 文件名为 `number_identity.db`（见 `shared` 中 `NumberIdentityDatabase::dbFile`），位于应用沙箱数据库目录；版本迁移由数据库回调完成
- **DataShare URI**（须与消费方保持一致）：
  - `datashare://com.ohos.numberlocationability` — 归属地
  - `datashare://com.ohos.numbermarkability` — 标记查询与维护
  - `datashare://com.ohos.downloadfileability` — 工程内仍声明（DownloadFile；**暂不作为对外核心能力**）
- **NAPI 模块名**：`telephony.numberidentity`、`contact.numberlookup`（同一注册函数）
- **开关与设置**：与 Settings / SettingsData 协同；关键键 `settings.telephony.number_identity_switch`（`"1"` 为开）。黄页与本地用户标记未命中且开关关闭时返回空标记（读写通常需 `MANAGE_SETTINGS`）
- **备份**：`BackupExtension` 为工程内声明；当前 `backup_config.json` 列出的是通话 / 联系人相关数据，**未包含** `number_identity.db`，升级迁移以 RDB 回调为主
- **权限**：主要权限如下（`requestPermissions` 见 `entry/src/main/module.json5`；`SET_TELEPHONY_STATE` 为标记 / DownloadFile DataShare 的 `writePermission`，**不在** `requestPermissions` 列表）

  | 权限 | 使用场景 |
  |------|----------|
  | ohos.permission.GET_TELEPHONY_STATE | DataShare 读、电话状态相关查询（亦为 extension `readPermission`） |
  | ohos.permission.SET_TELEPHONY_STATE | 标记写入（extension `writePermission`） |
  | ohos.permission.WRITE_CALL_LOG | 号码标记 Ability 声明场景 |
  | ohos.permission.MANAGE_SETTINGS | 相关系统设置项读写 |
  | ohos.permission.GET_BUNDLE_INFO / GET_BUNDLE_INFO_PRIVILEGED / GET_BUNDLE_RESOURCES | 包信息与资源查询 |
  | ohos.permission.hsdr.REQUEST_HSDR | 智能库相关能力（按产品配置） |
  | ohos.permission.GET_NETWORK_INFO / ohos.permission.INTERNET | 工程声明，供 DownloadFile 等网络相关实现使用（**暂非对外核心能力**） |

- **构建约束**：须同时维护 Hvigor（`entry` HAP）与 GN（`number_identity` so、预置数据、NAPI 模块）；**无独立 HAR 产物**，对外以 so + NAPI + DataShare 为主
- **消费方**：CallUI、Contacts、Settings（开关）等通过 DataShare / NAPI / SettingsData 协同；部件本身不依赖具体 UI 工程源码同仓

## 参与贡献

欢迎广大开发者贡献代码、文档等，具体的贡献流程和方式请参见[参与贡献](https://gitcode.com/openharmony/docs/blob/master/zh-cn/contribute/%E5%8F%82%E4%B8%8E%E8%B4%A1%E7%8C%AE.md)。
