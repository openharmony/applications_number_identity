# Number Identity

## Introduction

**Number Identity** (bundle name: `com.ohos.numberidentity`) is a system component in the OpenHarmony telephony subsystem. It is deployed as a system HAP plus native shared library, providing **number location, number marking, and yellow pages** capabilities, and exposing DataShare and NAPI data services to call and contacts applications.

This is a pre-installed system component with no standalone end-user business UI (`entry` pages are placeholder skeletons). Yellow-page, location, and mark **business queries** always read from the **local database or data files** and **do not use real-time online query APIs**. The tree still contains `DownloadFile` / `DownloadFileWorkSheduler` and URI `datashare://com.ohos.downloadfileability`, plus related network permission declarations, but **online data update is not a documented core capability** (products may trim or disable it). Files under `etc/` such as `numberlocation.data` and `yellowpage.data` are **sample data only** (few prefixes and yellow-page entries); production data must be supplied by the product. System install path on device is `/system/etc/telephony/`. Number location also supports the binary library `numberlocation.dat` (different from the `.data` path; see below and the development section).

### Core Capabilities

**Number Location**
- Supports two local data sources. Queries try **`.data` first**, then fall back to **`.dat`** on miss (see `NumberLocationUtils`):
  - **`numberlocation.data` (text JSON lines)**: First line `{"version":"..."}`, then one segment record per line (`prefix` and `operator` required; `province` / `city` or `location` optional). Parsed by `NumberLocationParser`; matching prefers a 7-digit prefix, then a 3-digit prefix.
  - **`numberlocation.dat` (binary library)**: Block-map / city-name binary layout, parsed by `NumberLocationDbParse` APIs such as `QueryPhoneNumberLocation` and `QueryTelNumberLocation`; landline location mainly uses this path. Current `etc/BUILD.gn` **installs `.data` by default** (the `.dat` target is commented out), and the repository does not ship a full `.dat` sample.
- On-device paths always prefer the app sandbox update file, then the system preset:
  - `.data`: `/data/storage/el2/base/files/numberlocation.data` → `/system/etc/telephony/numberlocation.data`
  - `.dat`: `/data/storage/el2/base/files/numberlocation.dat` → `/system/etc/telephony/numberlocation.dat`
- Implements segment and landline area-code matching through `NumberLocationManager`, `MobilePhoneNumber`, `FixPhoneNumber`, and related classes.
- DataShare URI: `datashare://com.ohos.numberlocationability`; NAPI provides `getNumberLocation` and related APIs.

**Number Marking**
- Supports marking unknown numbers (spam, fraud, advertising, and so on), plus query, update, and delete.
- Data is written to `number_mark` related tables and served through `com.ohos.numbermarkability`.
- Mark query entries include C++ APIs such as `NumberMarkAbility::QueryByPhoneNumber`; the query order is local yellow pages → local user marks. When both miss, the strange-number identity switch in Settings is checked; **if the switch is off, an empty mark is returned** (with the switch on, the current main path also performs no extra query step).

**Yellow Pages**
- Parses `yellowpage.data` (text JSON lines), imports it into RDB, and prefers yellow-page matches during query.
- Parsing is handled by the `yellow_page/` module (for example `YellowPageParser`); import goes through `NumberIdentityDatabase::ImportYellowPageData`.

## Architecture

Number Identity uses a layered, modular design organized by product entry, feature capabilities, and common capabilities, as shown below:

![Architecture](./figures/numberidentity_en.png)

### Application Layer Design

The overall structure is divided into product layer, feature layer, and common layer:

| Layer | Main directories / components | Description |
| ----- | --------------------------- | ----------- |
| Product layer | `entry/` | Phone / tablet forms (phone, pad) |
| Feature layer | `number_location/`, `number_mark/`, `yellow_page/` | Number location, number marking, and yellow pages |
| Common layer | `shared/`, `etc/`, `frameworks/`, `utils/`, `interfaces/` | Shared RDB, preset data, NAPI, log tools, interface definitions, DFX, CallerInfoQuery |

**Feature layer module description**:

| Core capability | Modules | Description |
| --------------- | ------- | ----------- |
| Number location | `number_location/` (`NumberLocationManager`, `NumberLocationAbility`) | Location parsing and DataShare query |
| Number marking | `number_mark/` (`NumberMarkManager`, `NumberMarkAbility`) | User mark maintenance and query |
| Yellow pages | `yellow_page/` (`YellowPageParser`) | Yellow-page file parsing and RDB import |

### Relationship with Other Applications

Number Identity provides yellow-page, location, and mark number-data services to system apps such as **CallUI** and **Contacts**. It does not host call or contacts UI itself; core telephony capabilities are provided by the Telephony subsystem. It cooperates with **Settings** through system SettingsData: this component reads and writes setting keys, while Settings provides the switch UI and persistence.

**Invocation**:

- CallUI and Contacts access URIs such as `com.ohos.numberlocationability` and `com.ohos.numbermarkability` through DataShare Helper.
- Apps may also use NAPI modules `telephony.numberidentity` and `contact.numberlookup` (same implementation, different module names).
- Third-party apps can implement `CallerInfoQueryExtensionAbility` (base class under `interfaces/kits/`) to extend enterprise caller-info query.

**Relationship with Settings**:

- This component accesses SettingsData through `GetSettingsData` / `InsertSettingsData` / `UpdateSettingsData` in `shared` (URI: `datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA`).
- Key: `settings.telephony.number_identity_switch` (`"1"` means on). In mark query, after local yellow pages and local user marks both miss, the switch is read: if off, an empty mark is returned; if on, the current `QueryByPhoneNumber` main path also ends with no extra online route. **Location query does not depend on this switch**. Yellow-page and local user-mark hits are not blocked by the switch.
- The user-visible toggle usually lives in call / system Settings pages (not in this repository) and writes SettingsData; CallUI and other consumers may also read the same key for display policy. Read/write usually requires `ohos.permission.MANAGE_SETTINGS`.

**Invocation scenarios**:

Showing marks and location on the incoming-call UI, resolving numbers in contacts details, user marking of unknown numbers, toggling strange-number identity in Settings, and similar flows.

## Build

This project supports **standalone Hvigor HAP builds** and **OpenHarmony system GN integration** (HAP + native shared library + prebuilt data). The product bundle name is `com.ohos.numberidentity`.

### Environment Requirements
- OpenHarmony SDK (Hvigor project `compileSdkVersion` is 23; `compatibleSdkVersion` / `targetSdkVersion` are 20)
- OpenHarmony source tree (component path: `base/telephony/number_identity`) and GN toolchain (for system integration)
- DevEco Studio or command-line Hvigor
- System signing configuration (see `signature/`; certificates and profiles are provided by the product build environment)

### Build Commands

Run Hvigor in the project root (requires `hvigorw` on PATH, or use DevEco Studio):

```bash
hvigorw assembleHap
```

For system-tree integration, run from the OpenHarmony source root:

```bash
./build.sh --product-name {product_name} --build-target number_identity --ccache
```

> **Note**: For standalone development, place the project under `base/telephony/number_identity` and build with the system, or configure system signing and build with Hvigor.

### Build Artifacts (System GN)

| Type | Artifact / target | Description |
| ---- | ----------------- | ----------- |
| HAP | `NumberIdentity.hap` | entry module; install path follows product configuration (for example `/system/app/com.ohos.numberidentityability`) |
| Native shared library | `number_identity` | Core implementation for location, marks, yellow pages, and shared RDB |
| NAPI shared libraries | `numberlookup`, `numberidentity` | Loaded by Contacts and Telephony sides respectively |
| Prebuilt data | `numberlocation.data`, `yellowpage.data` (optional `numberlocation.dat`) | `.data` / yellow pages install under `/system/etc/telephony/` by default; enable `.dat` in `etc/BUILD.gn` and provide the file |

## Number Identity Development

Number Identity uses **ArkTS + C++** hybrid development: DataShare and yellow-page / location / mark core live in the C++ feature layer; ArkTS handles Extension declarations and service extensions. For ArkTS page extensions, see: [ArkUI Development Overview](https://gitcode.com/openharmony/docs/blob/master/zh-cn/application-dev/ui/arkts-ui-development-overview.md)

### Development Based on Existing Modules

Typical use cases: extend location / yellow-page / mark capabilities, adjust query priority, or add NAPI / CallerInfoQuery capabilities.

Locate the change by business boundary: `number_location/`, `number_mark/`, `yellow_page/` (feature layer) or `shared/`, `frameworks/` (common layer). Routing changes must stay in sync with `NumberIdentityDataShareStubImpl`.

Common modification scenarios:

**Scenario 1: Number location query path**

   - Orchestration: `number_location/src/number_location_utils.cpp` (`.data` first, then `.dat`)
   - `.data` load / prefix match: `number_location/src/number_location_db_parse.cpp` (`LoadNumberLocationData`, `QueryLocationFromDataFile`) and `number_location_parser.cpp`
   - `.dat` binary query: `QueryPhoneNumberLocation`, `QueryTelNumberLocation`, and related APIs in the same file
   - DataShare / NAPI entry: `number_location_ability.cpp`, `frameworks/js/src/napi_number_identity.cpp`

    Overall query flow (mobile location / carrier):

    ```cpp
    // number_location_utils.cpp — location: prefer .data, then fall back to .dat
    unicodeInformation = DelayedSingleton<NumberLocationDbParse>::GetInstance()
        ->QueryLocationFromDataFile(phoneNumber.c_str());
    if (!unicodeInformation.empty()) {
        return unicodeInformation;
    }
    // ... fall back to QueryPhoneNumberLocation (.dat) and trim UTF-8 text
    ```

**Scenario 2: Add / update `numberlocation.data`**

   - Source: `etc/numberlocation.data` (only a few sample prefixes in-repo; production segments must be supplied)
   - Install: `number_location_data_default` in `etc/BUILD.gn` → `/system/etc/telephony/numberlocation.data`
   - At runtime you may also place a full file under the sandbox (preferred over system): `/data/storage/el2/base/files/numberlocation.data`

    Format: version on the first line, then one JSON record per line. Example of adding a prefix:

    ```text
    {"version":"1"}
    {"prefix":"1810256","province":"广东","city":"广州","operator":"电信"}
    // ... existing sample records ...
    {"prefix":"1380013","province":"北京","city":"北京","operator":"移动"}
    ```

    Ways to supply data: (1) edit `etc/` and rebuild the system image; (2) push via `hdc` to the sandbox or (with remount) the system directory; (3) after hot-replacing `.data`, usually restart the related process because `LoadNumberLocationData` loads once per process.

**Scenario 3: Add / update `numberlocation.dat`**

   - Binary library used when `.data` misses or for landline / legacy parse paths; open order is sandbox → `/system/etc/telephony/numberlocation.dat`
   - In source: place `etc/numberlocation.dat` and uncomment `:number_location_default` in `group("number_location")` in `etc/BUILD.gn`
   - On device: push to `/data/storage/el2/base/files/numberlocation.dat` (preferred) or the system preset directory

    ```gn
    # etc/BUILD.gn — enable preset .dat (also provide etc/numberlocation.dat)
    group("number_location") {
      deps = [
        ":number_location_default",   # [Enable] install numberlocation.dat
        ":yellow_page_default",
        ":number_location_data_default",
      ]
    }
    ```

    ```cpp
    // number_location_db_parse.cpp — .dat open order
    FILE *fp = fopen(PATH_UPDATE, "rb");  // /data/storage/el2/base/files/numberlocation.dat
    if (fp == nullptr) {
        fp = fopen(PATH, "rb");        // /system/etc/telephony/numberlocation.dat
    }
    // ... block-map parse and QueryPhoneNumberLocation, etc.
    ```

**Scenario 4: Number marking**

   - Core: `number_mark/src/number_mark_ability.cpp`, `number_mark_manager.cpp`
   - CRUD is routed to `NumberMarkAbility` through `com.ohos.numbermarkability`

    For example, to add custom handling after a local yellow-page hit, extend `QueryByPhoneNumber`:

    ```cpp
    // number_mark_ability.cpp — QueryByPhoneNumber is one of the yellow-page / mark query entries
    int NumberMarkAbility::QueryByPhoneNumber(
        const string &phoneNumber, NumberMarkInfo &markInfo, DatashareBusinessError &businessError)
    {
        // Existing flow: local yellow pages → local user marks → number_identity_switch check
        NumberMarkQueryContext context;
        context.phoneNumber = phoneNumber;
        auto &yellowPages = context.dbYellowPages;
        auto &numberMarks = context.dbMarks;
        int errCode = this->QueryLocalYellowPage(phoneNumber, yellowPages, businessError);
        HANDLE_BUSINESS_ERROR("QueryYellowPage", errCode, businessError, return errCode);
        if (auto it = FindYellowPageBestMatch(phoneNumber, yellowPages); it.has_value()) {
            // [Add custom handling] extend here after a yellow-page hit
            // CustomProcessYellowPage(*it);
            markInfo.FromYellowPage(*it);
            return SetBusinessError(businessError, errCode);
        }
        // ... subsequent local user-mark query and switch check remain unchanged
        return errCode;
    }
    ```

**Scenario 5: Add / update yellow-page `yellowpage.data`**

   - Source: `etc/yellowpage.data` (carrier hotline samples in-repo); parsing: `yellow_page/src/yellow_page_parser.cpp`
   - Install: `yellow_page_default` in `etc/BUILD.gn` → `/system/etc/telephony/yellowpage.data`
   - Import: `NumberIdentityDatabase::ImportYellowPageData`; if the DB already stores a version (string) **≥** the file `version`, import is skipped; otherwise yellow-page tables are cleared and fully re-imported

    Format sketch (long fields such as `photo` omitted):

    ```text
    {"version":"123"}
    {"group":"电信","name":"中国移动","photo":"...","phone":[
      {"hot_points":0,"dial_map":"...","phone":"10086","name":"中国移动"}
    ]}
    // ... other yellow-page records ...
    ```

    For example, bump `version`, append a record, and rebuild the image; or place the new file under `/data/storage/el2/base/files/yellowpage.data` and call `ImportYellowPageData(path)`:

    ```cpp
    // shared — import yellow pages by path (file version must be higher than DB version)
    NumberIdentityDatabase::ImportYellowPageData(
        "/data/storage/el2/base/files/yellowpage.data");
    ```

**Scenario 6: DataShare / NAPI external interfaces**

   - DataShare routing: `GetOwner` in `number_location/src/number_identity_datashare_stub_impl.cpp`
   - Dual NAPI module registration:

    ```cpp
    // napi_number_identity.cpp — the same implementation is registered for Telephony and Contacts
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

Common modification entries:

| Target | Path |
| ------ | ---- |
| Process initialization | `entry/src/main/ets/Application/NumberLocationAbilityStage.ts` |
| DataShare routing | `number_location/src/number_identity_datashare_stub_impl.cpp` |
| Number location | `number_location/src/number_location_ability.cpp`, `number_location_manager.cpp`, `number_location_utils.cpp`, `number_location_db_parse.cpp` |
| Location / yellow-page preset data | `etc/numberlocation.data`, `etc/yellowpage.data` (optional `etc/numberlocation.dat`), `etc/BUILD.gn` |
| Number marking | `number_mark/src/number_mark_ability.cpp` |
| Yellow pages | `yellow_page/src/yellow_page_parser.cpp`, `ImportYellowPageData` in `shared` |
| Mark-data IPC service | `entry/src/main/ets/service/NumberIdentityServiceExtAbility.ts` |
| NAPI | `frameworks/js/src/napi_number_identity.cpp` |
| CallerInfoQuery extension base | `interfaces/kits/caller_info_query_extension_ability/` |
| Settings read/write | `shared/src/number_identity_settings.cpp` |
| RDB and models | `shared/` |
| Ability / DataShare declaration | `entry/src/main/module.json5` |
| System GN entry | `BUILD.gn`, `bundle.json` |

### Developing New Feature Capabilities

Typical use cases: extend location / yellow-page / mark dimensions, add DataShare URIs, extend NAPI, or support additional data-file formats.

> **Note**: Feature-layer code must be registered into the `number_identity` shared library in the root `BUILD.gn`. External URIs, permissions, and NAPI module names must stay consistent with consumers such as CallUI and Contacts.

**Scenario 1: Extend C++ feature modules**

1. Add implementations under `number_location/`, `number_mark/`, or `yellow_page/`.
2. Register sources in `ohos_shared_library("number_identity")` in `BUILD.gn`.
3. Add gtest cases under `test/unittest/` and register them in the corresponding `BUILD.gn`.

**Scenario 2: Declare DataShare and permissions**

For a new DataShare URI, update `NumberIdentityDataShareStubImpl::GetOwner` and `entry/src/main/module.json5` together. Example of the existing mark capability declaration:

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

**Scenario 3: Expose NAPI / Kit externally**

Register new APIs under `frameworks/js`, and update `interfaces/` innerkits / kits as needed. Register both Telephony and Contacts module names.

**Scenario 4: CallerInfoQuery extension**

Third parties implement `onQueryCallerInfo(number)` of `CallerInfoQueryExtensionAbility`, scheduled through IPC by the `frameworks/extension/` framework.

## Directory

```text
number_identity
├─AppScope                              # App-level configuration and multi-language resources
│  ├─app.json5                          # Hvigor bundleName, version, and so on
│  ├─app.json                           # App configuration used by system GN builds
│  └─resources/                         # Global string resources and so on
├─figures/                              # Architecture figure (numberidentity_en.png)
├─entry                                 # Product layer (HAP entry module numberidentity)
│  └─src/main/
│     ├─ets/
│     │  ├─Application/                 # NumberLocationAbilityStage process initialization
│     │  ├─DataShareExtAbility/         # DataShare placeholders (includes DownloadFileWorkSheduler)
│     │  ├─service/                     # NumberIdentityServiceExtAbility
│     │  ├─pages/                       # ArkTS placeholder page (Index.ets)
│     │  ├─common/                      # Product layer: constants, logging, connection utilities
│     │  └─backup/                      # Product layer: BackupExtension (scope from backup_config)
│     ├─resources/                      # Module resources, profile
│     ├─module.json5                    # Hvigor: Ability, DataShare, permissions
│     └─module.json                     # System GN module configuration
├─number_location/                      # Feature layer: number location, DataShare Stub
│  ├─include/
│  └─src/                               # Manager, Parser, Ability, and so on
├─number_mark/                          # Feature layer: number marking
│  ├─include/
│  └─src/                               # NumberMarkAbility, NumberMarkManager, and so on
├─yellow_page/                          # Feature layer: yellow-page parsing
│  ├─include/
│  └─src/                               # yellow_page_parser and so on
├─shared/                               # Common layer: shared RDB (DDL, Models, utilities)
├─frameworks/                           # Common layer: NAPI, CallerInfoQuery, DFX-related implementation
│  ├─js/                                # NAPI (telephony.numberidentity / contact.numberlookup)
│  └─extension/                         # CallerInfoQuery framework
├─interfaces/                           # Common layer: interface definitions (innerkits / kits)
├─etc/                                  # Common layer: preset data (numberlocation.data, yellowpage.data; optional numberlocation.dat)
├─utils/log/                            # Common layer: log tools (log macros and error codes)
├─tools/                                # Helper tools directory (may be empty)
├─test/                                 # gtest / fuzztest
├─signature/                            # System GN signing configuration
├─hvigor/                               # Hvigor configuration
├─BUILD.gn                              # System GN build entry
├─bundle.json                           # Component ownership telephony and build groups
├─build-profile.json5                   # Project-level SDK configuration
├─oh-package.json5
├─OAT.xml                               # Open-source compliance audit
├─LICENSE
├─README.md                             # English README
└─README_zh.md                          # Chinese README
```

## Constraints

- **Language**: ArkTS (entry orchestration) + C++ (yellow pages / location / marking and DataShare implementation)
- **Subsystem**: telephony (see `bundle.json`)
- **Deployment path**: OpenHarmony source tree `base/telephony/number_identity`; HAP install path follows product image configuration
- **Bundle name**: `com.ohos.numberidentity` (`AppScope/app.json5`)
- **Device types**: Hvigor (`entry/src/main/module.json5`) uses `default` and `tablet`; system GN (`entry/src/main/module.json`) also includes `2in1` and `wearable`
- **Runtime form**: Pre-installed system component (HAP + native so + prebuilt data), **no user-visible main UI**; `abilities` in `entry` is empty, and capabilities are provided by `extensionAbilities`
- **Query mode**: Yellow-page, location, and mark **business queries** read local RDB / data files only and **do not use real-time online query**; residual `DownloadFile` support in the tree is **not a documented core capability**
- **Prebuilt and on-device data paths**: Source `etc/` → device `/system/etc/telephony/` (default `numberlocation.data`, `yellowpage.data`; `numberlocation.dat` must be provided and the GN target enabled). Sandbox override path: same filenames under `/data/storage/el2/base/files/` (preferred over system)
- **Location data notes**: In-repo `.data` is sample prefixes only; production databases must be supplied. Mobile query order is **`.data` → `.dat`**; landline via `QueryUnicodeInformationByTelNum` / `QueryTelNumberLocation` **mainly uses `.dat`**. `.data` is cached once per process; hot updates usually require restarting the related process
- **Yellow-page data notes**: In-repo `yellowpage.data` is sample data; import compares `version` (string compare) and skips when the DB version is **≥** the file version; otherwise yellow-page tables are cleared and fully rewritten
- **Local database**: RDB file name is `number_identity.db` (see `NumberIdentityDatabase::dbFile` in `shared`), under the application sandbox database directory; schema migration is handled by database callbacks
- **DataShare URIs** (must stay consistent with consumers):
  - `datashare://com.ohos.numberlocationability` — location
  - `datashare://com.ohos.numbermarkability` — mark query and maintenance
  - `datashare://com.ohos.downloadfileability` — still declared in-tree (DownloadFile; **not a documented core capability**)
- **NAPI module names**: `telephony.numberidentity`, `contact.numberlookup` (same registration function)
- **Switches and settings**: Cooperates with Settings / SettingsData; key `settings.telephony.number_identity_switch` (`"1"` = on). When yellow pages and local user marks miss and the switch is off, an empty mark is returned (usually requires `MANAGE_SETTINGS` to read/write)
- **Backup**: `BackupExtension` is declared in the project; the current `backup_config.json` lists call / contact related data and **does not include** `number_identity.db`; upgrades mainly rely on RDB callbacks
- **Permissions**: Main permissions are as follows (`requestPermissions` in `entry/src/main/module.json5`; `SET_TELEPHONY_STATE` is the mark / DownloadFile DataShare `writePermission` and is **not** in `requestPermissions`)

  | Permission | Usage |
  | ---------- | ----- |
  | ohos.permission.GET_TELEPHONY_STATE | DataShare read and telephony-state related queries (also extension `readPermission`) |
  | ohos.permission.SET_TELEPHONY_STATE | Mark write (extension `writePermission`) |
  | ohos.permission.WRITE_CALL_LOG | Declared scene for the number-mark Ability |
  | ohos.permission.MANAGE_SETTINGS | Related system settings read/write |
  | ohos.permission.GET_BUNDLE_INFO / GET_BUNDLE_INFO_PRIVILEGED / GET_BUNDLE_RESOURCES | Bundle info and resource queries |
  | ohos.permission.hsdr.REQUEST_HSDR | Intelligent-library related capability (by product configuration) |
  | ohos.permission.GET_NETWORK_INFO / ohos.permission.INTERNET | Declared for DownloadFile and related network code (**not a documented core capability**) |

- **Build constraints**: Both Hvigor (`entry` HAP) and GN (`number_identity` so, prebuilt data, NAPI modules) must be maintained; **no standalone HAR artifact**; external capability is mainly so + NAPI + DataShare
- **Consumers**: CallUI, Contacts, Settings (switch), and similar apps cooperate via DataShare / NAPI / SettingsData; this component does not depend on specific UI project sources living in the same repository

## Contributing

Contributions of code and documentation are welcome. For the contribution process, see [Contributing](https://gitcode.com/openharmony/docs/blob/master/zh-cn/contribute/%E5%8F%82%E4%B8%8E%E8%B4%A1%E7%8C%AE.md).
