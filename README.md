# Number Identity<a name="EN-US_TOPIC_0000001103554544"></a>

-   [Introduction](#section11660541593)
    -   [Content](#section_intro_content_en)
    -   [Architecture diagram](#section_arch_diagram_en)
-   [File Tree](#section161941989596)
-   [Repositories Involved](#section1371113476307)
-   [Supplementary Note](#section1371113476308)

## Introduction<a name="section11660541593"></a>

### Content<a name="section_intro_content_en"></a>

Number Identity is a telephony component in OpenHarmony. It provides number location (number attribution) query, number mark (caller identification) service, caller info query extension ability, and related data storage and DataShare services for the system.

**Number marking**, **number identification**, and related capabilities depend on a **local database** on the device to persist and query number marks, yellow-page data, number location (attribution), and caller-related information; identification and display results are all read from this store, and the information shown in the UI is consistent with the data currently stored on the device, **with no online identification**; number attribution, yellow-page names, and other displayed information are resolved **only** from existing local data on the device.

**Prebuilt data files and on-device path:** In this repository, the prebuilt files for number location (attribution) and yellow page are **`etc/numberlocation.data`** and **`etc/yellowpage.data`**, respectively. After the system image is deployed to the device, the corresponding attribution and yellow-page data on the system is located under **`system/etc/telephony/`** (shipped together with the telephony subsystem; use the paths above to map from source to runtime layout).

### Core Functions:
1. **Harassment Marking**: Enables users to mark unknown numbers (such as spam, fraud, advertising sales, etc.) for identification, helping users identify and avoid potential risky calls, thereby enhancing communication security protection capabilities.
2. **Identification of Unknown Numbers**: Provides multi-dimensional display of unknown number information, including: Number Origin Location: Shows the region (province/city) where the incoming number is located, facilitating users to quickly determine the source of the call. Number Yellow Pages: Identifies and displays the names and types of public service numbers of enterprises, institutions, etc. (such as banks, courier services, food delivery, etc.). Number Marking: Based on user and community marking data stored in the **local database**, displays the existing marking information of the number, assisting users in making decisions on whether to answer.

### Architecture diagram<a name="section_arch_diagram_en"></a>

![Number Identity architecture](./figures/numberidentity_en.png)

**entry (HAP)** exposes settings UI, **DataShare**, and orchestrating **service** code; **feature** groups **number_location**, **number_mark**, **frameworks**, and **interfaces** for caller-info extensions; **Framework** lists NAPI, telephony services, storage, and DataShare stacks; **common** captures shared utilities, **etc** ships **`numberlocation.data`** and **`yellowpage.data`** (deployed under **`system/etc/telephony/`** on device), and **innerkits** type surfaces.

## File Tree<a name="section161941989596"></a>

~~~
/NumberIdentity/
├── AppScope
├── entry                 
│   └── src
│       └── main
│           ├── ets                        
│           │   ├── Application
│           │   ├── DataShareExtAbility
│           │   ├── common
│           │   ├── pages
│           │   ├── service
│           │   └── backup
│           ├── resources
│           └── module.json5
├── etc                                    # numberlocation.data, yellowpage.data (on device: system/etc/telephony/)
├── frameworks
│   ├── extension
│   │   ├── core
│   │   └── napi_helper
│   └── js
├── interfaces
│   ├── innerkits
│   └── kits
│       ├── caller_info_query_extension_ability
│       └── caller_info_query_extension_context
├── number_location
│   ├── include
│   └── src
├── number_mark
│   ├── include
│   └── src
├── BUILD.gn
├── bundle.json
├── signature
└── LICENSE
~~~

## Repositories Involved<a name="section1371113476307"></a>

[**number_identity**](https://gitcode.com/openharmony/applications_call.git)

## Supplementary Note<a name="section1371113476308"></a>

It needs to be manually copied to the base/telephony directory and compiled along with the system.
