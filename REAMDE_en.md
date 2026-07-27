# Number Identity

## Introduction
**Number Identity** (bundle name: `com.ohos.numberidentity`) is a **number identity component** in the OpenHarmony telephony subsystem. It provides number location (attribution) query, number marking (caller identification), yellow-page recognition, caller-info query extension, and related local storage / DataShare services.

This is a preinstalled system component, deployed as a **system HAP plus native shared library**. Number marking and identification depend on the device's **local database / data files**. Results are read from those local sources, **with no online identification**. Prebuilt data files in the repository are **`etc/numberlocation.data`** and **`etc/yellowpage.data`**; on device they are installed under **`system/etc/telephony/`**.

### Core Capabilities

**Number Location Query**
- Parses local prebuilt/updated data files (`numberlocation.data` / `numberlocation.dat`) to return province/city/carrier information.
- `NumberLocationManager`, `MobilePhoneNumber`, and `FixPhoneNumber` implement mobile-segment matching, landline area-code matching, and number-format handling.
- Exposed via DataShare URI `com.ohos.numberlocationability` and NAPI `getNumberLocation`.

> Number Identity is positioned as a telephony data-service component. DataShare core logic is implemented in C++, not as a pure ArkTS app.

**Number Marking and Yellow Page**
- Supports user marking of unknown numbers (spam, fraud, advertising, and so on) into the `number_mark` table.
- Yellow-page data is parsed from `yellowpage.data` by `yellow_page` and imported into RDB, taking priority over community marks.
- The current production query order is local yellow pages → local user marks. `number_identity_switch` only gates the post-miss switch check. Intelligent-library import/storage and HSDR Helper code remain in the project, but are not connected to the main `QueryByPhoneNumber` path.

**DataShare Data Service**
- Unified `NumberIdentityDataShareStubImpl` routes three DataShare Extensions:
  - `com.ohos.numberlocationability` → location query
  - `com.ohos.numbermarkability` → number-mark query, update, delete, and batch import
  - `com.ohos.downloadfileability` → data download / update

**Caller Info Query Extension**
- `CallerInfoQueryExtensionAbility` supports third-party extensions for enterprise caller info.
- `interfaces/kits/` provides the `CallerInfoQueryExtensionAbility` JavaScript base class and the `CallerInfoQueryExtensionContext` wrapper module.

**Data Update and Lifecycle**
- `DownloadFileWorkSheduler` periodically downloads number-location / yellow-page data files according to the configured network policy. Downloaded data is stored locally; identification queries do not make online requests.
- `NumberIdentityServiceExtAbility` supports IPC import of number-mark packages.
- The project declares an empty `BackupExtension`. Its current `backup_config.json` lists call/contact data and does not include `number_identity.db`; Number Identity's own RDB schema upgrades are handled by database callbacks.

### Relationship with CallUI and Contacts

Number Identity, CallUI, and Contacts are telephony-ecosystem components. CallUI and Contacts **consume** data services provided by Number Identity.

**Events and call relationships**:
1. Number Identity runs as an independent process; the native `number_identity` shared library registers the DataShare creator on load.
2. CallUI and Contacts query number information cross-process via DataShare Helper or NAPI.
3. Number Identity does not interact with call UI directly; it provides indirect data services via DataShare / NAPI.

> Example of a typical incoming-call mark display flow:
> - CallUI receives an incoming call and obtains the caller number;
> - Queries `com.ohos.numbermarkability` via DataShare;
> - `NumberMarkAbility` queries yellow-page / mark data by priority;
> - Returns `NumberMarkInfo` for CallUI to display on the call screen.

## Architecture

Number Identity uses a layered, modular design and collaborates with consumers such as CallUI and Contacts.

### Position in the System

Number Identity sits in the application / data-service layer. It provides number identity capabilities to CallUI and Contacts through DataShare and NAPI, and depends on RDB plus prebuilt data for local parsing.

![Number Identity in OpenHarmony](./figures/numberidentity_in_os_en.png)

### Layered Design

Number Identity overall adopts a layered and modular architecture design, divided into three layers:

- Common capability layer (`shared/`, `etc/`, `utils/`, `frameworks/`, `interfaces/`): basic management framework and common utilities
- Feature layer (`number_location/`, `number_mark/`, `yellow_page/`): modular business features
- Product layer (`entry/`): product adaptation

**Basic principle**: Layers are divided by the reusability of module capabilities; each layer is divided by the business boundaries between modules.

**Responsibilities**
1. Common capability layer:
    - Hosts: the base capability set required for Number Identity to run.
    - Note: `shared/` is the base framework for RDB, data models, and common utilities, and is the required foundational module of Number Identity.
    - Common business contained in feature-layer modules can also sink into the common capability layer. For example, DFX utilities (logging, and so on), prebuilt data (`etc/`), and NAPI and CallerInfoQuery extensions (`frameworks/`, `interfaces/`) that need to be reused across multiple features are placed in the common capability layer;

2. Feature layer:
    - Hosts: a collection of abstract common features; each feature is highly cohesive and loosely coupled, and supports customization by the product layer.
    - Based on local number-identity businesses (location query, number mark, yellow-page recognition, and so on), capabilities are split into different modules by business boundary and placed in the feature layer;

3. Product layer:
    - Hosts: personalized businesses for the current device, and customization of the required feature layer and common capability layer.
    - Common capabilities and different features are integrated and customized by the product-layer main entry, and packaged into a directly deployable HAP package.

**Relationships**
1. Neither the common capability layer nor the feature layer can be deployed or run directly; they must be integrated by the product layer and built into a HAP package before they can run.
2. The product layer may depend downward on the feature layer and the common capability layer; the feature layer may depend downward on the common capability layer. The dependency direction is top-down; reverse dependencies are not allowed.

### Module Description

| Module | Path | Description |
| ---- | ---- | ---- |
| Application entry | entry/src/main/ets/Application/ | NumberLocationAbilityStage process init |
| ArkTS placeholder page | entry/src/main/ets/pages/ | Index.ets currently contains only an empty page skeleton |
| Domain service | entry/src/main/ets/service/ | NumberIdentityServiceExtAbility IPC service |
| DataShare declarations | entry/src/main/ets/DataShareExtAbility/ | TS placeholders; real logic in C++ |
| Data update scheduler | entry/src/main/ets/DataShareExtAbility/DownloadFileWorkSheduler.ts | Triggers local data-file downloads and updates according to the network policy |
| Backup extension | entry/src/main/ets/backup/ | Empty BackupExtension; scope is defined by backup_config.json |
| Number location | number_location/ | Manager, Parser, Ability, DownloadFile |
| Number mark | number_mark/ | NumberMarkAbility, CallerInfo, mark query and maintenance |
| Yellow page | yellow_page/ | YellowPageParser data import |
| Database and models | shared/ | RDB Helper, DDL, Models, JSON/Pinyin/HSDR utilities |
| Prebuilt data | etc/ | numberlocation.data, yellowpage.data |
| NAPI | frameworks/js/ | numberidentity / numberlookup |
| Extension framework | frameworks/extension/ | CallerInfoQueryExtension |
| Interfaces | interfaces/ | innerkits / kits |
| Logging | utils/log/ | Unified logging macros and error codes |

## Build

Number Identity supports dual build systems: standalone Hvigor HAP build, and system GN integration for HAP + shared library + prebuilt data.

The following diagram lists all source modules, HAP, native / NAPI shared libraries, extension loaders, and prebuilt data referenced by the `bundle.json` build groups. No standalone HAR is defined.

![Number Identity build and deploy](./figures/numberidentity_build_en.png)

### Environment Requirements
- OpenHarmony SDK (the standalone HAP project uses `compileSdkVersion` 23 and `compatibleSdkVersion` / `targetSdkVersion` 20)
- OpenHarmony source tree (component path: `base/telephony/number_identity`)
- DevEco Studio or Hvigor; system GN toolchain
- Signing configuration for the system-source build (see `signature/pm.gni`; certificates and profiles are supplied by the product build environment)

### Build Commands

1. **System GN build**

Run from the OpenHarmony source-tree root:

```bash
./build.sh --product-name {product_name} --build-target number_identity --ccache
```

2. **DevEco / Hvigor standalone build**

```bash
hvigorw assembleHap
```

> **Note**: For standalone development, copy the project to `base/telephony/number_identity` and build with the system, or configure system signing for Hvigor standalone build.

### Build Outputs

| Type | Output / GN target | Description |
| ---- | ------------------ | ----------- |
| HAP | `NumberIdentity.hap` (target `NumberIdentity`) | entry module installed to `/system/app/com.ohos.numberidentityability` |
| Native shared library | `number_identity` | Core number-location, number-mark, yellow-page, and shared RDB implementation |
| NAPI shared libraries | `numberlookup`, `numberidentity` | Installed to `module/contact` and `module/telephony`, respectively |
| CallerInfoQuery libraries | `caller_info_query_extension`, `caller_info_query_extension_module` | Extension framework and `extensionability/` loader |
| CallerInfoQuery NAPI | `callerinfoqueryextensionability_napi`, `callerinfoqueryextensioncontext_napi` | Ability and Context JS/NAPI modules |
| Prebuilt data | `numberlocation.data`, `yellowpage.data` | Installed to `/system/etc/telephony/` |
| HAR | None | No `ohos_har` or HAR module is defined; public capabilities are provided through native libraries and NAPI |

`bundle.json` declares ownership under the telephony subsystem. Build groups include `fwk_group` (NAPI / extension / etc) and `service_group` (HAP / so).

## Developing Number Identity

Number Identity uses **ArkTS + C++**. DataShare core logic is in C++; ArkTS handles Extension declarations and service orchestration. `pages/Index.ets` currently contains only an empty page skeleton. For future ArkTS page development, see [ArkUI Development Overview](https://gitcode.com/openharmony/docs/blob/master/en/application-dev/ui/arkts-ui-development-overview.md).

### Development Based on Existing Modules

Typical scenarios: add local query dimensions, extend CallerInfoQuery, or customize local data-file update strategies.

**Extending DataShare query**
1. Add query-path handling in the corresponding Ability (for example `NumberMarkAbility`).
2. Route via `NumberIdentityDataShareStubImpl` URI dispatch.
3. Wrap results with Bridge classes.

The existing Stub routes each URI to one of three local DataShare abilities:

```cpp
std::shared_ptr<DataShareExtAbility>
NumberIdentityDataShareStubImpl::GetOwner(const Uri &uri)
{
    OHOS::Uri uriTemp = uri;
    std::string path = uriTemp.GetPath();
    if (path.find("com.ohos.numberlocationability") != std::string::npos) {
        return GetNumberLocationAbility();
    }
    if (path.find("com.ohos.downloadfileability") != std::string::npos) {
        return GetDownloadFileAbility();
    }
    if (path.find("com.ohos.numbermarkability") != std::string::npos) {
        return GetNumberMarkAbility();
    }
    return nullptr;
}
```

**Adding NAPI APIs**
1. Register new APIs under `frameworks/js`.
2. Access the corresponding URI through `DataShareHelper`.
3. Update innerkits / Kit interface implementations under `interfaces/` as needed.

The same interfaces are registered under two module names for Telephony and Contacts:

```cpp
static napi_module g_nativeNumberIdentityModule = {
    .nm_register_func = NapiNumberIdentity::RegisterNumberIdentityFunc,
    .nm_modname = "telephony.numberidentity",
};

static napi_module g_nativeNumberLookupModule = {
    .nm_register_func = NapiNumberIdentity::RegisterNumberIdentityFunc,
    .nm_modname = "contact.numberlookup",
};
```

**Extending CallerInfoQuery**
1. Implement `CallerInfoQueryExtensionAbility` in a third-party app.
2. Return enterprise caller info in `onQueryCallerInfo(number)`.
3. The system invokes it through the `CallerInfoQueryExtension` framework over IPC.

The base class lives under `interfaces/kits/caller_info_query_extension_ability/`. Third-party apps can implement query logic based on this base class:

```javascript
class CallerInfoQueryExtensionAbility {
  async onQueryCallerInfo(number) {
    console.log('onQueryCallerInfo:' + number);
  }
}

export default CallerInfoQueryExtensionAbility;
```

### Developing New Features

Typical scenarios: add local identification dimensions, data-file update strategies, or external Kits.

**Step 1: Extend C++ feature modules**

Add or extend implementations in the feature layer, and register sources in the root `BUILD.gn` `number_identity` shared library:

```gn
ohos_shared_library("number_identity") {
  sources = [
    # ... existing sources ...
    "$NUMBER_IDENTITY_ROOT/number_location/src/number_location_parser.cpp",
    # "$NUMBER_IDENTITY_ROOT/<new_feature>/src/<new_module>.cpp",
  ]
}
```

**Step 2: Declare DataShare routing and permissions**

For new DataShare URIs, update `NumberIdentityDataShareStubImpl::GetOwner` routing and `module.json5`. For example, a new capability must declare its type, URI, and access permissions:

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

**Step 3: Expose external APIs**

For external exposure, update `interfaces/` and NAPI registration under `frameworks/js`. Register the same interfaces under both Telephony and Contacts module names:

```cpp
static napi_module g_nativeNumberIdentityModule = {
    .nm_register_func = NapiNumberIdentity::RegisterNumberIdentityFunc,
    .nm_modname = "telephony.numberidentity",
};

static napi_module g_nativeNumberLookupModule = {
    .nm_register_func = NapiNumberIdentity::RegisterNumberIdentityFunc,
    .nm_modname = "contact.numberlookup",
};
```

**Step 4: Add tests**

Add gtest cases under `test/unittest/` and register the sources in the corresponding `BUILD.gn`:

```gn
ohos_unittest("tel_number_location_gtest") {
  sources = [
    "src/number_location_gtest.cpp",
    "src/number_location_ability_gtest.cpp",
    # "src/<new_feature>_gtest.cpp",
  ]
}
```

## Directory
```text
number_identity
├─AppScope                              # App-level config and multi-language resources
│  ├─app.json                           # App profile used by the system GN build
│  ├─app.json5                          # bundleName and version used by Hvigor
│  └─resources/                         # Global string resources
├─entry                                 # HAP entry module
│  ├─src/main/                          # Main source directory
│  │  ├─ets/                            # ArkTS business source
│  │  │  ├─Application/                 # AbilityStage process init
│  │  │  ├─DataShareExtAbility/         # DataShare declarations and data-update WorkScheduler
│  │  │  ├─service/                     # NumberIdentityServiceExtAbility
│  │  │  ├─pages/                       # ArkTS placeholder (currently an empty page skeleton)
│  │  │  ├─common/                      # Constants, logging, connection helpers
│  │  │  └─backup/                      # Empty BackupExtension
│  │  ├─resources/                      # Module resources, multi-language, and so on
│  │  ├─module.json                     # Module profile used by the system GN build
│  │  └─module.json5                    # Ability, permission, and DataShare declarations used by Hvigor
│  ├─build-profile.json5                # Module-level build config
│  └─obfuscation-rules.txt              # Obfuscation rules
├─number_location/                      # Number location (C++)
├─number_mark/                          # Number mark (C++)
├─yellow_page/                          # Yellow-page parser (C++)
├─shared/                               # RDB / Models / shared utilities
├─frameworks/                           # NAPI and CallerInfoQuery extension
├─interfaces/                           # innerkits / kits public interfaces
├─etc/                                  # Preset data (on-device: system/etc/telephony/)
├─utils/log/                            # Shared log macros and error codes
├─figures/                              # Architecture diagrams
│  ├─numberidentity_in_os.png           # Position in the system (zh)
│  ├─numberidentity_architecture.png    # Layered architecture (zh)
│  ├─numberidentity_ipc.png             # Component and external dependencies (zh)
│  ├─numberidentity_build.png           # Build and deployment (zh)
│  ├─numberidentity_in_os_en.png        # Position in the system (en)
│  ├─numberidentity_architecture_en.png # Layered architecture (en)
│  ├─numberidentity_ipc_en.png          # Component and external dependencies (en)
│  └─numberidentity_build_en.png        # Build and deployment (en)
├─test/                                 # gtest / fuzztest
├─signature/                            # System GN signing config (pm.gni)
├─hvigor/                               # Build tool config
├─BUILD.gn                              # System GN build entry
├─bundle.json                           # Component ownership and build groups
├─build-profile.json5                   # Project-level SDK / signing / product config
├─oh-package.json5                      # Dependencies and package info
├─OAT.xml                               # Open-source compliance audit
├─LICENSE                               # Open-source license
├─README_zh.md                          # Chinese README
└─REAMDE_en.md                          # English README
```

## Constraints
- Languages: ArkTS + C++
- Subsystem: telephony
- Deploy path: `base/telephony/number_identity`
- Device types: `default`, `tablet` (see `module.json5`)
- Identification: local database / data files only, no online identification
- Preset data on device: `system/etc/telephony/`
- Local database: `/data/storage/el1/database/number_identity.db`
- Requires system permissions such as `MANAGE_SETTINGS` for telephony settings

## Contribution

Contributions of code, documentation, and more are welcome. For the contribution process, see [Contribute](https://gitcode.com/openharmony/docs/blob/master/en/contribute/contribution.md).

## Related Repositories
- [applications_call](https://gitcode.com/openharmony/applications_call) (consumer of number-identification data, including the call UI)
- [telephony_core_service](https://gitcode.com/openharmony/telephony_core_service) (telephony core services)
- [telephony_call_manager](https://gitcode.com/openharmony/telephony_call_manager) (call management service)
