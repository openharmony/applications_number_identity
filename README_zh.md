# Number Identity<a name="ZH-CN_TOPIC_0000001103421572"></a>

-   [简介](#section11660541593)
    -   [内容介绍](#section_intro_content_zh)
    -   [架构图](#section_arch_diagram_zh)
-   [目录](#section161941989596)
-   [相关仓](#section1371113476307)
-   [补充说明](#section1371113476308)

## 简介<a name="section11660541593"></a>

### 内容介绍<a name="section_intro_content_zh"></a>

Number Identity 是 OpenHarmony 电话子系统中的号码识别组件，为系统提供号码归属地查询、号码标记（来电识别）服务、来电信息查询扩展能力，以及相关数据存储与 DataShare 服务。

**号码标记**、**号码识别**等功能依赖设备端**本地数据库**对号码标记、号码黄页、号码归属地及来电关联信息进行持久化存储与查询；识别与展示结果均从该库读取，界面所展示信息与设备当前库内数据一致，**无在线识别功能**；号码归属地、黄页名称及其他展示信息**仅**依据设备上已有的本地数据解析。

**预置数据文件与系统路径：** 工程中号码归属地与号码黄页的预置数据文件分别为 **`etc/numberlocation.data`** 与 **`etc/yellowpage.data`**。随系统镜像部署到设备后，上述归属地与黄页相关数据在系统中的目录为 **`system/etc/telephony/`**（与电话子系统一并下发；从源码到运行时可按上述路径对应理解）。

### 核心功能：
   1. **骚扰标记**:支持对陌生号码进行用户标记（如骚扰、诈骗、广告推销等），帮助用户识别并规避潜在风险来电，增强通信安全防护能力。
   2. **陌生号码识别**:提供多维度的陌生号码信息展示，包括： 号码归属地：显示来电号码所属地区（省/市），便于用户快速判断来电来源。 号码黄页：识别并展示企业、机构等公开服务号码的名称及类型（如银行、快递、外卖等）。 号码标记：基于**本地数据库**中持久化的用户及社区标记数据，展示该号码的已有标记信息，辅助用户决策是否接听。

### 架构图<a name="section_arch_diagram_zh"></a>

![Number Identity 架构](./figures/numberidentity.png)

**entry（HAP）** 提供设置页、**DataShare** 与业务 **service**；**feature** 承载 **number_location / number_mark** 原生模块、**frameworks** 与 **interfaces** 对外扩展定义；**Framework** 为 NAPI、电话子系统、存储与 DataShare 等底座；**common** 归纳公共工具、**etc** 预置 **`numberlocation.data`** 与 **`yellowpage.data`**（随系统部署于设备 **`system/etc/telephony/`**）及 **innerkits** 类型声明等横切内容。

## 目录<a name="section161941989596"></a>

~~~
/NumberIdentity/
├── AppScope                               # 应用级 app.json5 与全局资源
├── entry                                  # 随系统编译的 HAP 入口（设置页等）
│   └── src
│       └── main
│           ├── ets                        
│           │   ├── Application            # Application 与进程级初始化
│           │   ├── DataShareExtAbility    # 对外暴露号码数据的 DataShare
│           │   ├── common                 # 公共工具与常量
│           │   ├── pages                  # 号码识别相关设置/展示页面
│           │   ├── service                # 归属地、标记、DataShare 服务实现
│           │   └── backup                 # 备份与恢复扩展
│           ├── resources                  # 模块内字符串与媒体资源
│           └── module.json5               # 模块能力与依赖声明
├── etc                                    # numberlocation.data / yellowpage.data；设备上对应 system/etc/telephony/
├── frameworks                             # 原生扩展与 JS 桥接
│   ├── extension                          # 来电信息查询 Extension 框架
│   │   ├── core                           # 扩展加载与生命周期核心
│   │   └── napi_helper                    # NAPI 注册与辅助封装
│   └── js                                 # 对外暴露的 NAPI 模块入口
├── interfaces                             # API 与类型定义（innerkits/kits）
│   ├── innerkits                          # 系统内部使用的类型与接口
│   └── kits                               # 对外 Kit 形式接口定义
│       ├── caller_info_query_extension_ability   # 来电信息查询 Extension 声明
│       └── caller_info_query_extension_context   # Extension 运行上下文类型
├── number_location                        # 号码归属地（C++ 实现）
│   ├── include                            # 对外头文件
│   └── src                                # 解析与查询实现源码
├── number_mark                            # 号码标记 / 黄页（C++ 实现）
│   ├── include                            # 对外头文件
│   └── src                                # 标记匹配与数据访问实现
├── BUILD.gn                               # GN 构建入口
├── bundle.json                            # 部件描述与子系统归属
├── signature                              # 预置签名与证书材料
└── LICENSE                                # 开源许可证
~~~

## 相关仓<a name="section1371113476307"></a>

[**number_identity**](https://gitcode.com/openharmony/applications_call.git)

## 补充说明<a name="section1371113476308"></a>

需要手动拷贝到 base/telephony 目录下随系统编译。
