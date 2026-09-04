# DailyAccount 产品与技术架构方案

> 状态：产品范围与核心技术约束已确认，详细设计草案
>
> 日期：2026-09-04
>
> 适用对象：项目维护者、后续开发者与自动化开发代理

## 1. 文档目的

本文档记录 DailyAccount 从当前单机 Qt Widgets 记账程序演进为本地优先、多端同步、可扩展综合工具平台的产品范围与技术方案。

本文档主要回答以下问题：

- 第一阶段必须交付哪些能力，哪些能力明确延后。
- Windows、Android、Linux 分别承担什么职责。
- 现有 C++、Qt Widgets 和 DAT 存储代码如何保留及迁移。
- 本地 SQLite、托管云端和多设备同步如何协作。
- 账号隔离、周期套餐、文本导入和分类分析采用什么数据模型。
- 后续工具模块如何接入平台，而不提前建设复杂插件系统。

本文档描述目标架构。当前程序的实际行为仍以代码和 `README.md` 为准，直到对应迁移阶段完成。同步协议、云端供应商和 Android 后台执行方式必须先通过本文列出的原型与测试门槛，才能转为实施基线。

## 2. 决策用语

本文使用以下约定区分决策成熟度：

- **已确认**：需求访谈中已经明确，不应在实现时自行改变。
- **目标设计**：为满足已确认需求而确定的技术设计，实施前可以通过验证结果微调细节。
- **候选方案**：尚需根据网络、成本或原型结果最终选择。
- **阻塞验证**：开始依赖该设计的大规模实现前必须完成的原型、ADR 或测试。
- **后续范围**：需要保留演进空间，但不进入首版交付。

## 3. 已确认的产品意图

### 3.1 目标

先交付一个稳定、低操作成本、可离线使用的个人记账工具。记账模块同时作为未来综合工具平台的第一个功能模块，但首版不为未来平台能力牺牲记账功能的可用性和交付速度。

### 3.2 用户

- 首版供本人及家人使用，共 2 至 3 个账号。
- 每个账号拥有完全独立的账本，通过应用和云端接口时不可查看或修改彼此数据。本地文件系统的首版威胁边界见 15.4 节。
- 底层数据库、API 和权限从第一天按多用户设计。
- 首版账号由管理员预先创建，不开放公众自助注册。
- 家庭共享账本、成员邀请和协作编辑不在首版范围内。

### 3.3 终端职责

| 终端 | 主要职责 | 首版地位 |
| --- | --- | --- |
| Android | 消费发生时随手记账、查看近期流水、接收套餐提醒、触发同步 | 主要日常入口 |
| Windows | 手工记账、批量整理、文本导入、分类维护、账户补漏与统计分析 | 完整管理端 |
| Linux | 开发、测试和必要的构建验证 | 非主要产品端 |

Linux 不要求与 Windows、Android 保持完整功能对等，但共享核心必须可以在 Linux 上构建和测试。

### 3.4 核心使用方式

- 记账以手工录入为主。
- 默认使用简洁收支模式，尽量减少必填项。
- 可在设置中切换到账户优先模式，用于余额核对、转账和查缺补漏。
- 两种模式只改变界面字段、默认值和信息层级，不创建两套账务模型。
- 每笔交易底层都使用同一结构。普通收入和支出允许账户暂时为空并在以后补充；转账和退款遵守各自的必填账户约束。

### 3.5 第一阶段关键结果

第一阶段完成后，用户应能够：

1. 在 Android 断网状态下快速记录一笔支出或收入。
2. 在 Windows 上查看、修改、分类和补全所有流水。
3. 联网后让同一账号的 Windows 与 Android 数据安全收敛。
4. 在月度分析中清楚看到钱主要花在哪些分类以及各自占比。
5. 为月卡、月付和其他周期套餐生成待确认支出并收到提醒。
6. 粘贴过去记事本中的机打账目，经过解析、预览和确认后批量入账。
7. 导出和恢复自己的数据，不因云端短时不可用而无法记账。

### 3.6 已确认的技术约束

- 保留共享 C++ 核心和现有 Windows Qt Widgets 客户端，Android 使用 Qt Quick/QML。
- 每个客户端使用本地 SQLite 保存完整的可同步记账实体，主要记账操作离线可用。
- 云端采用低维护托管服务和标准云安全模型，可以接受少量持续费用，不要求端到端加密。

## 4. 范围边界

### 4.1 首版必须具备

- Android 与 Windows 各自保存完整的可同步记账实体；原始导入文本和设备设置可以仅保留在本机。
- 断网新增、修改、删除和查询；所有 CRUD 均服从领域约束，不能绕过周期实例等聚合关系独立破坏关联实体。
- 收入、支出以及账户模式所需的转账、退款语义。
- 分类、子分类、标签、备注和可选账户。
- 本月收入、支出、结余及支出分类结构分析。
- 周期套餐、待确认支出、提醒与导入匹配。
- 原始文本粘贴或 TXT 导入、规则解析、人工确认和防重复。
- 2 至 3 个独立账号的认证和服务端数据隔离。
- 增量同步、失败重试、删除同步和冲突提示。
- CSV/JSON 导出、本地备份、旧 DAT 数据迁移。
- 编译期工具模块注册及平台共享服务。

### 4.2 明确不在首版范围内

- 家庭共享账本或多人共同编辑同一账本。
- 完整复式记账和通用会计科目体系。
- 公众注册、邮箱验证、找回密码、封禁、运营后台和风控体系。
- 端到端加密及跨设备密钥恢复。
- 保留当前 profile 或账号的一键整账本清空；首版只提供逐笔删除、删除本地副本和删除云端账号。
- 运行时插件、在线插件市场、插件独立下载或热更新。
- 银行或支付平台账号登录及自动抓取。
- PDF 账单识别、截图 OCR、照片 OCR 和手写识别。
- 预算控制、趋势预测、异常消费检测和订阅遗忘检测。
- 多人协作、审批、报销工作流和企业财务功能。

### 4.3 后续演进范围

以下能力应在数据模型和模块边界上留有扩展空间，但不得提前拖慢首版：

- 微信、支付宝和银行卡 CSV/XLSX 账单导入。
- PDF、截图和照片 OCR。
- Android 通知识别并生成待人工确认的导入候选。
- 预算、同比环比、异常检测和订阅优化建议。
- 公众账号注册、邮箱验证、找回密码、封禁和运营自动化。
- 私人账本与共享账本并存。
- 保留 profile 或账号但一键清空整个账本的受控操作。
- 新工具模块及必要时的运行时插件系统。
- 多币种和汇率换算。

## 5. 当前实现基线

### 5.1 已有技术栈

- C++17。
- Qt 6.9.3 Widgets。
- qmake 和 MinGW Windows 构建。
- Qt 无关的标准 C++ 后端。
- 自定义文本快照格式 `ledger.dat`，不是 SQLite。

当前 `jizhang.pro` 只链接 `core`、`gui` 和 `widgets`，没有链接 Qt SQL。`README.md` 记录的持久化文件为 `ledger.dat`、备份文件和运行锁。

### 5.2 已实现能力

- 流水新增、修改、删除和日期筛选。
- 收入、支出、分类、子分类和备注。
- 自定义分类及使用中分类保护。
- 日、月和分类汇总。
- Windows 仪表盘、流水、统计、分类和其他页面。
- UTF-8 文本导出。
- 精确到分的 64 位整数金额。
- DAT 校验和、临时文件提交、备份和损坏恢复。
- 旧 `records.dat`、`categories.dat` 到 V3 DAT 的迁移。
- 一组覆盖金额、日期、存储损坏、回滚和汇总逻辑的后端测试。

### 5.3 可直接保留的资产

- `backend/record.h` 中的整数金额原则和输入校验思路。
- `CategoryManager` 的分类规则和使用中保护思路。
- `Ledger` 中已经验证的收支汇总算法。
- Windows Qt Widgets 页面及其主要交互流程。
- DAT 解析、校验、备份恢复代码，作为只读旧数据迁移器。
- 现有后端测试场景，作为重构期间的回归基线。

### 5.4 必须改变的部分

- 本地递增 `int` ID 无法在多个离线设备上安全生成，必须换成客户端生成的 UUID。
- `LedgerStorage::load/save(StoredData)` 每次加载和保存完整账本，不适合行级查询、分页和增量同步。
- 删除当前为物理删除，远端设备无法得知删除事件。
- 记录没有用户归属、修订号、更新时间、设备来源或幂等标识。
- 当前程序只有一个数据目录和一个 `Ledger` 实例，无法表达账号隔离。
- `Ledger` 和各 Widgets 页面直接耦合，不适合后台同步和 QML 数据模型。
- qmake 不利于后续 Qt Android、多目标构建和模块化管理，应迁移到 CMake。

## 6. 总体技术决策

### 6.1 客户端路线

**已确认：**保留 C++ 核心和 Windows Qt Widgets，为 Android 单独开发 Qt Quick/QML 界面。

采用该路线的原因：

- 最大限度保护当前 Windows UI 和 C++ 后端投入。
- Android 可以使用面向触摸的 Qt Quick Controls，而不强行移植桌面 Widgets。
- 两端可以共享领域规则、SQLite 存储、导入、周期计算和同步实现。
- 不需要在 Kotlin 中重新实现整套财务规则。
- Android 原生能力仅通过窄接口桥接，控制长期维护成本。

不选择以下路线：

| 方案 | 不作为当前主路线的原因 |
| --- | --- |
| Windows 与 Android 全部迁移 QML | 需要重写现有桌面页面，延迟首版且桌面密集操作仍需单独设计 |
| Android 全部使用 Kotlin/Compose | Android 体验最好，但会形成两套业务实现或复杂 JNI 核心绑定 |
| 全部改用 Flutter | 现有 C++、Widgets 和测试大部分失去价值，重写成本不符合优先交付目标 |
| Widgets 直接运行 Android | 技术上可以尝试，但桌面式控件和交互不适合主要移动入口 |

### 6.2 构建系统

**目标设计：**迁移到 CMake，并在迁移验证完成后停止维护 qmake 配置。

CMake 至少应产生以下独立目标：

```text
dailyaccount_platform_interfaces
dailyaccount_accounting_domain
dailyaccount_accounting_application
dailyaccount_accounting_sqlite
dailyaccount_accounting_sync
dailyaccount_desktop
dailyaccount_android
dailyaccount_accounting_tests
```

桌面和 Android 使用不同 CMake toolchain 与构建目录，不要求一次 configure 同时生成两个平台产物。应用 target 显式调用已编译模块的平台无关及平台特定注册函数，具体合同见 8.2 节；不得依赖可能被链接器裁剪的全局静态自注册。Android QML 资源使用 `qt_add_qml_module` 注册。

共享库不得链接 Qt Widgets 或 Qt Quick。纯领域层尽可能保持标准 C++，Qt SQL、Qt Network 和 QObject/QML 适配代码放在外层目标。

### 6.3 本地数据库

**已确认：**每个客户端本地使用 SQLite，所有主要记账操作离线可用。

**目标设计：**通过 Qt SQL 的 QSQLITE 驱动访问 SQLite，并使用显式 repository 和 migration 层，不让 SQL 散落在 UI 中。

本地目录建议按用户和模块隔离：

```text
DailyAccount/
  profiles.sqlite
  users/
    <user-uuid>/
      platform.sqlite
      accounting.sqlite
      backups/
```

- `profiles.sqlite` 只保存本地账号索引和非敏感显示信息。
- `platform.sqlite` 保存该用户的平台设置和模块注册状态。
- `accounting.sqlite` 保存记账领域数据、迁移版本、同步 outbox 和同步游标。
- 每个账号独立数据库文件，降低账号切换时发生本地数据串读的风险。
- 每个工具模块默认拥有自己的数据库和迁移序列，避免未来模块相互污染表空间。
- outbox 必须和业务数据位于同一数据库，以便在一个 SQLite 事务中提交。

不得通过网盘、WebDAV 或云存储直接覆盖正在使用的 SQLite 文件。

每个用户、每个模块使用一个串行数据库执行器，`profiles.sqlite` 也有独立的平台级串行执行器。每个连接使用包含 profile/module/worker 身份的唯一名称，并且只在执行器所属线程创建、使用和移除。每个 `QSqlDatabase` 连接只能在其所属线程使用；UI、前台同步和 Android 后台任务不得共享跨线程连接。实现还必须：

- 在每个连接上启用 `PRAGMA foreign_keys = ON`。
- 设置有上限的 `QSQLITE_BUSY_TIMEOUT`。
- 优先使用 `STRICT` 表、`CHECK`、`UNIQUE` 和外键约束弥补 SQLite 动态类型风险。
- 不在数据库事务中等待网络。
- 切换账号、恢复备份或关闭应用前排空执行队列，先销毁所有 query 和 database handle，再移除连接。
- 使用 SQLite Online Backup API 或 `VACUUM INTO` 生成一致备份，不复制正在写入的数据库和 WAL 文件；中断产生的临时备份必须在启动时识别、校验或清理。

### 6.4 云端服务

**已确认：**使用低维护托管服务，可以接受少量持续费用；采用标准云安全模型，不要求端到端加密。

云端供应商必须提供或允许实现以下能力：

- 稳定且不可变的内部账号 ID。
- 可验证的认证主体和会话刷新。
- 服务端强制的数据租户隔离。
- 事务性的幂等同步入口和提交有序的变更游标。
- 数据导出、账号删除、备份和迁移能力。
- 数据库/API schema 的版本化部署与测试。

**候选方案：**Supabase 托管 PostgreSQL、Auth、Row Level Security 和 REST/RPC API。Supabase 相关实现细节只在[供应商候选附录](#25-supabase-候选实现附录)中生效。

Supabase 符合当前约束的原因：

- 提供托管 PostgreSQL 和账号认证，减少自建服务运维。
- PostgreSQL RLS 可以在数据库层强制账号行级隔离。
- 自动 REST API 和 PostgreSQL 函数可以承载同步接口。
- 本地开发环境和数据库迁移可以纳入仓库。
- 首版 2 至 3 个账号负载很低，后续仍能扩展公开账号体系。

Supabase 尚不是不可替换的硬依赖。最终确定前必须验证：

- Windows 与 Android 目标网络环境中的可达性和延迟。
- 目标部署地域、费用和服务条款。
- 数据备份、导出和供应商迁移流程。
- Qt/C++ 直接调用 Auth 与 REST/RPC 接口的完整原型。

客户端必须通过以下边界隔离供应商：

```text
IAuthClient
ISyncTransport
IRemoteHealthCheck
```

业务层只理解账号身份、同步请求和同步结果，不理解任何供应商 URL、密钥或 SDK 类型。

### 6.5 网络层

使用 `QNetworkAccessManager` 发送 HTTPS JSON 请求。桌面和 Android 共享网络与序列化代码。

网络层必须：

- 设置有限连接和传输超时。
- 只接受有效 TLS 证书，不提供忽略证书错误的发布路径。
- 对响应 JSON 做结构和类型校验。
- 统一错误类型、重试分类和用户可读错误信息。
- 支持取消、退避重试和网络恢复后继续同步。
- 不在日志中记录访问令牌、完整备注、商户或金额。

### 6.6 Android 原生桥接

QML 和 C++ 负责主要 Android UI 与业务。仅在 Qt 跨平台 API 不能充分覆盖时增加窄 Kotlin 桥接：

- Android Keystore 密钥保护应用私有存储中的会话凭据。
- 系统通知权限和持久化本地提醒调度。
- WorkManager 等后台同步调度。
- 系统文件选择器、分享入口和应用生命周期事件。
- 后续 OCR 或经用户明确授权的支付通知读取能力。

桥接接口必须保持面向能力，例如 `ISecureStore`、`INotificationScheduler`，不得让领域层直接调用 JNI。

`INotificationScheduler` 的周期支出实现必须走不依赖网络、云端推送或 Qt 冷进程初始化的 Android 原生路径。规则或当期实例变化时把提醒计划提交给原生调度器；计划来源必须可持久恢复，并在设备重启、系统日期或时区变化以及应用升级后重建。交付目标是指定本地自然日内送达，不承诺精确到某一分钟。

Android Keystore 更适合保存加密密钥；体积较大的令牌应使用 Keystore 密钥加密后放入应用私有存储，而不是假定 Keystore 可以直接存放任意凭据。

QML 金额输入和金额 model role 必须使用十进制/格式化字符串，不得使用 32 位 QML `int` 或 JavaScript `Number` 承载、转换或计算分值。图表只接收 C++ 已计算的比例等非货币数值；所有金额解析、上限检查和运算继续在 C++ 完成。

WorkManager 能否在冷进程中可靠初始化 Qt/C++ 同步运行时属于阻塞验证。该限制只适用于后台同步和周期补算：原型完成前，两者可以采用尽力而为的后台触发，并在应用启动和恢复前台时补偿。提醒交付不得退化为只能在应用前台补偿，必须单独通过 Android 原生调度与重启恢复原型。

## 7. 分层与模块结构

### 7.1 目标目录结构

```text
src/
  core/
    domain/
    application/
  platform/
    auth/
    database/
    sync/
    notifications/
    settings/
    logging/
  modules/
    accounting/
      domain/
      application/
      data/
      import/
      analytics/
  apps/
    desktop-widgets/
    android-qml/
tests/
  unit/
  integration/
  sync/
  fixtures/
cloud/
  <selected-provider>/
    migrations/
    tests/
```

目录名称可在实施时贴合现有仓库逐步调整，但依赖方向必须保持：

```text
Desktop Widgets ─┐
                 ├─> Application Services ─> Domain
Android QML ─────┘             │
                               ├─> Repository interfaces
                               ├─> Sync interfaces
                               └─> Platform capability interfaces

SQLite / HTTP / managed-cloud / Android bridges implement outward interfaces.
```

### 7.2 领域层

领域层负责：

- 金额、币种、日期和稳定 ID。
- 交易、账户、分类、周期规则和导入候选的约束。
- 收支、退款、转账和余额计算。
- 周期实例生成与匹配规则。
- 分类结构分析。

领域层不得负责：

- QWidget、QML Item 或页面导航。
- SQL 字符串和数据库连接。
- HTTP、JWT、云服务 URL。
- Android Context、Activity、JNI 或 Windows API。

### 7.3 应用层

应用层以用例为边界，示例包括：

```text
CreateTransaction
EditTransaction
DeleteTransaction
ListTransactions
ReconcileAccount
GenerateRecurringOccurrences
ConfirmRecurringTransaction
ParseTextImport
CommitImportBatch
GetMonthlyCategoryBreakdown
SynchronizeAccount
```

每个用例在边界验证输入，在一个 repository transaction 中完成相关写入，并返回结构化成功或错误结果。

通用 `DeleteTransaction` 只适用于未关联其他聚合的交易。交易被周期 occurrence 引用时必须返回明确的领域约束错误，并引导调用周期实例的撤销确认、跳过或取消用例，禁止单独破坏关联。

### 7.4 UI 适配层

Windows 继续使用 Widgets 页面，但页面不再直接持有可变 `Ledger` 全局对象。页面通过 controller 或 application service 执行用例，通过只读 view model 展示结果。

Android 使用 QML 页面，通过 `QObject` service facade 和 `QAbstractListModel` 获取数据。C++ 不应从业务层主动查找或操纵 QML 对象。

桌面与 Android 共享：

- 字段语义和验证规则。
- 应用用例。
- repository、导入、周期和同步实现。
- 错误代码和同步状态。

桌面与 Android 不强求共享：

- 页面布局。
- 导航结构。
- 控件尺寸和交互手势。
- 桌面批量操作与移动端快速操作。

## 8. 编译期工具平台设计

### 8.1 平台化目标

首版平台化只解决模块隔离和共享基础服务，不解决第三方插件分发。

平台提供：

- 账号和会话。
- 用户目录及模块数据库定位。
- 同步调度。
- 安全凭据存储。
- 通知调度。
- 设置与主题。
- 日志和诊断。
- 导入、导出和备份能力。
- 桌面导航与移动端导航的模块注册点。

记账模块提供：

- 自己的领域实体、repository、迁移和同步集合。
- 平台无关的模块 manifest 和 service 注册。
- Windows 应用 target 中的页面注册。
- Android 应用 target 中的 QML route 注册。
- 模块设置项、导出器和后台任务。

### 8.2 模块注册原则

- 模块随应用编译和发布。
- 模块 ID 永久稳定，例如 `accounting`。
- 模块拥有独立数据库 schema 版本。
- 模块只通过平台能力接口访问认证、同步、通知和安全存储。
- 平台不得理解具体记账分类或交易规则。
- UI 注册分别位于桌面和 Android 应用层，不在共享领域接口中返回 `QWidget` 或 QML 对象。

首版使用三个显式入口，避免一个注册接口混入平台特定类型：

```text
registerAccountingCore(PlatformRegistry&)
registerAccountingDesktop(DesktopRegistry&)
registerAccountingMobile(MobileRegistry&)
```

`PlatformRegistry` 的 manifest 至少声明模块 ID、模块版本、依赖模块、所需平台能力、数据库定位、schema 版本、同步 stream ID、后台任务和导出器。`DesktopRegistry` 只接受 Widgets 页面工厂，`MobileRegistry` 只接受 QML module/route 描述。

注册规则：

- 重复模块 ID 或 stream ID 在启动时立即失败并给出诊断。
- 依赖按拓扑顺序初始化，缺失依赖或循环依赖阻止对应模块启动。
- registry 拥有 descriptor，模块 service 由应用容器管理到应用退出或账号切换。
- `platform.sqlite` 只保存用户可配置的模块启用状态和模块设置引用，不保存编译期注册事实。
- `accounting` 是首版必需模块，不能由普通设置禁用。
- 单元测试验证注册顺序、重复 ID、缺失依赖和桌面/移动入口完全分离。

### 8.3 不提前实现动态插件

动态插件会引入 ABI、签名、权限、版本兼容、数据库迁移和移动平台分发限制。当前没有真实第三方插件消费者，因此首版只保留清晰模块边界。只有出现独立发布或第三方扩展的具体需求后，才单独设计插件协议。

## 9. 记账领域模型

### 9.1 Money

- 金额继续使用有符号 64 位整数的最小货币单位保存，不使用浮点数持久化。
- 首版默认币种可设为 CNY，但交易和账户模型从开始保留 ISO 4217 币种字段。
- 首版不在不同币种之间自动汇总；多币种换算属于后续范围。
- UI 输入的小数必须在边界严格转换，非法精度和溢出必须拒绝。

### 9.2 Transaction

建议字段如下：

| 字段 | 类型/语义 | 说明 |
| --- | --- | --- |
| `id` | UUID | 客户端生成，全局稳定 |
| `userId` | UUID | 服务端租户归属，本地由当前 profile 隐式限定 |
| `type` | enum | `EXPENSE`、`INCOME`、`TRANSFER`、`REFUND` |
| `status` | enum | `PENDING`、`POSTED` |
| `amountMinor` | int64 | 正数金额，方向由类型决定 |
| `currency` | string | 首版默认 `CNY` |
| `occurredOn` | local date | 交易归属的本地自然日，也是历史 DAT 的无损日期字段 |
| `occurredTime` | local time/null | 用户提供时保存，否则为空 |
| `timeZoneId` | string/null | 存在明确时间时记录发生地时区 |
| `categoryId` | UUID/null | 转账可以为空 |
| `accountId` | UUID/null | 简洁模式允许未指定 |
| `destinationAccountId` | UUID/null | 转账目标账户 |
| `merchant` | string/null | 商户或交易对象 |
| `note` | string/null | 用户备注 |
| `originSource` | enum | 交易最初由 `MANUAL`、`TEXT_IMPORT`、`BILL_IMPORT` 或 `RECURRING` 创建；`BILL_IMPORT` 为后续预留 |
| `originRef` | string/null | 仅作为初始来源诊断信息，不承担唯一业务关联 |
| `refundOfId` | UUID/null | 退款关联原交易 |
| `createdAt` | UTC datetime | 创建时间 |
| `updatedAt` | UTC datetime | 最近修改时间，仅用于展示和诊断，不作为同步游标 |
| `serverRevision` | integer | 只由服务端分配；尚未同步的本地实体固定为 0 |
| `deletedAt` | UTC datetime/null | 同步删除标记 |

首版类型约束：

| 类型 | `accountId` | `destinationAccountId` | `categoryId` | 其他 |
| --- | --- | --- | --- | --- |
| `EXPENSE` | 可空 | 必须为空 | 可空，空值进入“未分类” | 手工为 `POSTED`，周期实例可为 `PENDING` |
| `INCOME` | 可空 | 必须为空 | 可空，空值进入“未分类” | 首版为 `POSTED` |
| `TRANSFER` | 必填 | 必填 | 必须为空 | 首版为 `POSTED` |
| `REFUND` | 必填 | 必须为空 | 从原支出继承 | `refundOfId` 必填，首版为 `POSTED` |

约束：

- `amountMinor` 必须大于 0 并在业务上限内。
- `EXPENSE` 在指定账户时扣减该账户，`INCOME` 在指定账户时增加该账户。
- `TRANSFER` 以一条记录原子扣减来源账户并增加目标账户；两个账户必须非空且不同，首版必须同币种，并且不计入收入或支出分析。转账手续费另记为支出。
- `REFUND` 的到账账户必须非空。它增加退款到账账户，必须关联同一用户、同一币种的 `POSTED` 支出，并继承原支出分类。所有未删除退款累计不得超过原支出金额。
- 首版退款计入退款实际发生月份，并抵减该月份对应分类支出，不回写原消费月份。
- 存在有效退款的原支出不能直接删除；必须先删除、改绑或处理关联退款。
- `PENDING` 周期交易默认不计入正式月度支出，但分析页可以单独显示待确认金额。
- 已删除记录不能在普通查询中出现，但 tombstone 在同步保留期内必须存在。
- 旧数据中名为“退款”或“个人转账”的普通收入记录迁移后仍保持 `INCOME`，不得根据分类名称推断为新退款或转账语义。

`serverRevision` 与本地编辑状态严格分离。本地是否存在未上传修改由 repository/outbox 维护，不允许客户端自行增加 `serverRevision`。

### 9.3 Account

账户模式是简洁模式的超集，不是另一套账本。

建议字段：

- `id`、`userId`、`name`。
- `type`：现金、银行卡、电子钱包、信用账户、其他。
- `currency`。
- `openingBalanceMinor` 和起始日期。
- `isArchived`。
- `createdAt`、`updatedAt`、`serverRevision`、`deletedAt`。

简洁模式下允许 `accountId = null`。账户模式提供：

- 默认账户。
- 未指定账户流水列表。
- 账户内收支和转账。
- 手工余额快照。
- 账面余额与实际余额差异提示。

首版不要求通过自动平账生成会计分录，也不实现完整资产负债表。

### 9.4 Category

分类必须使用稳定 ID，不能继续把名称作为身份。

建议字段：

- `id`、`userId`。
- `name`、`parentId`。
- `appliesTo`：收入、支出或两者。
- `sortOrder`、`color`、`icon`。
- `isPreset`、`isArchived`。
- 同步元数据。

规则：

- 重命名分类不改写历史交易外键。
- 已被使用的分类允许归档，不直接物理删除。
- 子分类通过 `parentId` 表达，不再依赖括号拼接。
- 分析默认按顶级分类汇总，并允许下钻到子分类和流水。

### 9.5 Tag

标签用于跨分类标记，例如旅行、家庭、报销或项目。标签与交易为多对多关系。标签不是首版快速记账的必填项。

### 9.6 RecurringRule

周期规则建议包含：

- 名称、预期商户、分类、账户以及可选备注/标记。
- 预期金额及允许匹配的金额容差。
- 月付、年付或自定义周期。
- 下次到期日、时区和提前提醒配置；首版允许提前 1 或 2 天提醒，并固定在到期日再次提醒。
- 是否启用、开始日期和可选结束日期。
- 月末日期处理策略。
- 同步元数据。

每个到期周期生成唯一 occurrence，其稳定键为：

```text
<recurring-rule-id>:<period-key>
```

`periodKey` 由规则时区和周期规范确定，例如月付使用 `YYYY-MM`、年付使用 `YYYY`，不能由设备当前时区自由格式化。该键必须在本地和服务端唯一，确保多个设备同时生成时不会产生重复待确认支出。

规则修改只影响尚未生成的未来周期。已经生成的 occurrence 保存规则快照，不随规则改名、改价或改日期而静默变化。归档规则保留历史 occurrence；对于当前 `PENDING` occurrence，由用户明确选择保留、跳过或取消。

### 9.7 RecurringOccurrence

周期实例独立于交易保存，作为本期套餐生命周期的权威记录。建议字段包括：

- `id`、`userId`、`ruleId`、`periodKey`。
- `status`：`PENDING`、`POSTED`、`SKIPPED`、`CANCELLED`。
- `deferredUntil`。
- 可选 `transactionId`；`PENDING` 和 `POSTED` 时必须指向有效的待确认或正式交易，`SKIPPED` 和 `CANCELLED` 时必须为空。
- 预计日期、预计金额及生成时采用的规则快照。
- 同步元数据。

`(userId, ruleId, periodKey)` 必须具有唯一约束。创建 `PENDING` occurrence 时必须同时创建一条待确认交易；确认扣款后 occurrence 与交易一起变为 `POSTED`。延期只更新 occurrence 并保留原交易，跳过或取消则原子撤销交易并清空关联。该聚合关系必须由显式 `transactionId` 和领域命令维护，不能只依赖交易的单一 `originRef`。

为保证不同设备离线生成相同身份，首版采用确定性 ID：

```text
occurrenceId = UUIDv5(ruleId, periodKey)
pendingTransactionId = UUIDv5(occurrenceId, "pending-transaction")
```

`SKIPPED` 表示本周期确认不会扣款，`CANCELLED` 表示该实例因规则取消或生成错误而作废，两者均为终态。延期时状态仍为 `PENDING`，只设置 `deferredUntil` 并保留可编辑的待确认交易。跳过或取消必须在同一原子命令中把待确认交易标记为 tombstone，并清空 occurrence 的 `transactionId`；同步和审计依赖确定性交易 ID、tombstone 保留期和 occurrence 终态，不能保留会在 tombstone 清理后失效的外键。

本期金额、实际日期和本期备注保存在关联交易中，可以在确认前调整且默认不回写周期规则。若用户要同时修改未来默认值，必须作为单独、明确的操作。`POSTED` 是“已支付”的持久状态；“已逾期”不另存为状态，而是根据 `PENDING` 且有效到期日早于当前本地日期派生，其中有效到期日优先使用 `deferredUntil`。

逾期实例绝不自动转为 `POSTED`，必须持续可见并等待用户选择已支付、延期、跳过或取消。每个 period 独立生成，上一期仍为待确认或逾期时不得阻止下一期生成。

### 9.8 ImportBatch 与 ImportItem

一次粘贴或文件导入形成一个 batch。每一条原始输入形成一个 item，并保存：

- 原始文本。
- 标准化文本。
- 解析出的候选字段。
- 解析置信状态和错误列表。
- 内容指纹。
- 用户修正结果。
- 最终交易 ID 或忽略状态。

`ImportItem.transactionId` 独立记录导入与交易的关联，因此一条由周期规则生成的交易仍可同时保留导入来源。

完整原始文本默认只保存在本地，不参与同步、云端备份或普通 JSON 导出。至少在导入确认完成前必须保留，以便用户核对；用户可以在设置中选择完成后立即清除或保留本地历史。云端默认只同步已确认交易和最小来源信息。

`batchId`、`itemId`、最终 `transactionId` 和提交 `mutationId` 必须在首次提交前生成并持久化，失败重试继续使用相同 ID。`CommitImportBatch` 在一个本地事务中写入全部被确认交易、最小 provenance 和 outbox；任一条失败时整批不入账。用户选择强制重复导入时生成新的 override ID，但仍保留与原候选的提示关系。

### 9.9 TransactionProvenance

结构化的最小来源关系与原始导入文本分开保存。一条交易可以同时关联周期 occurrence 和导入项。建议字段包括 `transactionId`、`kind`、`externalKey` 和必要的解析器/来源版本；不得包含整段原文。`(userId, kind, externalKey)` 在适用来源内建立唯一约束，用于服务端幂等和审计。

## 10. 简洁模式与账户模式

### 10.1 简洁模式

Android 默认快速录入建议只突出：

- 金额。
- 收入或支出。
- 分类。
- 发生时间，默认当前时间。

账户、商户、标签和备注可折叠。用户可配置默认账户，也可以保持未指定。

### 10.2 账户模式

账户模式增加：

- 来源账户和目标账户。
- 当前账面余额。
- 转账和退款入口。
- 未指定账户、余额差异和待补全项目。
- 导入项目的账户映射。

### 10.3 模式切换规则

- 设置保存默认模式，单次记账可以临时展开详细字段。
- 切换模式不得迁移或复制交易。
- 任何在简洁模式创建的交易都能在账户模式补全。
- 分析口径不因 UI 模式改变。

## 11. 周期支出、月付列表与提醒

已确认的产品需求是周期套餐生成待确认支出、到期提醒并在真实扣款导入时匹配。以下具体生命周期、提前 1 或 2 天的提醒策略和 Android 90 天滚动调度合同属于满足该需求的目标设计，必须通过阶段 0 原型后才能成为发布承诺。

### 11.1 生成流程

1. 到达生成窗口时计算本期 occurrence。
2. 使用规则 ID 与 period key 检查是否已存在。
3. 不存在时创建 `PENDING` occurrence 及其待确认交易。
4. 根据规则调度到期前 1 或 2 天以及到期日当天的本地通知。
5. 用户可以在确认前修正本期金额、实际日期和本期备注，而不改变未来周期默认值。
6. 用户确认实际扣款后，在一个原子命令中把 occurrence 和交易改为 `POSTED`，无需重新录入交易字段。
7. 用户可以延期 occurrence 并保留其待确认交易；跳过或取消时才在同一原子命令中撤销待确认交易，不能遗留普通查询可见的孤立记录。
8. 到达有效到期日后仍未确认的 occurrence 保持 `PENDING`，在界面派生为“已逾期”；它不会自动入账，也不会阻止下一周期生成。

周期补算必须在应用启动、恢复前台和同步完成后执行，即使 Android 后台任务被系统延迟，也不能永久漏掉 occurrence。

### 11.2 专用列表与首页确认

周期支出必须拥有独立列表，而不是只散落在流水或系统通知中。页面分为规则概览和周期实例两种视图：规则概览默认展示所有未归档项目、下一到期日和未解决实例数；实例视图每个 period 一行，因此同一规则可以同时显示上一期逾期实例和本期待确认实例。已归档规则不再生成新实例，但它已有的待确认或逾期实例必须继续显示到用户处理完毕。每条实例至少显示：

- 名称和可选备注/标记。
- 本期金额。
- 有效到期日。
- 到期前剩余天数、到期当天或逾期天数。
- 持久状态对应的待确认、已支付、已跳过或已取消，以及由 `deferredUntil`/有效到期日派生的已延期或已逾期显示状态。

首页优先展示所有已逾期实例，以及处于各自提前提醒窗口内和当天到期的实例，并直接提供“确认已支付”按钮。点击后不得要求用户再次手工输入已有字段；命令必须幂等，并在同一事务中完成 occurrence 确认、交易正式入账和 outbox 写入，重复点击或重试不能生成第二笔账目。

用户可以从列表或首页修改本期金额和本期备注。界面必须明确区分“只修改本期”和“同时修改以后各期”，避免一次涨价或优惠静默污染未来规则。

### 11.3 Android 提醒可靠性合同

- 提醒在本地调度，不依赖网络、云端推送或同步成功。
- 在应用最长连续 90 个自然日未打开、设备离线或正常重启后，已进入该窗口的提醒仍应在指定本地自然日内出现；只承诺按天送达，不承诺精确分钟。超过该窗口且期间从未重新打开应用不属于首版交付保证。
- 创建或修改周期规则以及应用每次启动时，C++ 周期计算器必须生成至少未来 90 个自然日的稳定 reminder events，并提交给原生调度器。事件键由规则、period 和提醒 offset 确定，重复提交必须幂等；这一滚动窗口保证无需在后台启动 Qt 也能覆盖“数周未打开”的承诺。
- 设最近一次成功生成计划的本地日期为 `D0`，首版保证所有目标日期不晚于 `D0 + 90 天`、且由本机已知启用规则生成的 reminder event。`D0 + 89 天` 和 `D0 + 90 天`必须送达，`D0 + 91 天`只有在窗口被重新填充后才纳入保证；其他设备新增但尚未同步到本机的规则不可能在离线时提醒。
- 原生侧持久保存 reminder events 及展示所需的最小快照，并负责在触发后标记投递；它不自行推导财务规则，也不创建 occurrence 或交易。用户从通知打开应用时，由正常周期补算生成或加载对应 occurrence。
- occurrence 被确认、跳过或取消时，必须取消该 period 尚未触发的 reminder events；延期时必须取消旧事件，并按 `deferredUntil` 和原提前提醒配置生成替代事件。同步拉取到这些生命周期变化时执行相同核对，避免其他设备留下过期提醒。
- 设备重启、系统日期或时区变化、应用升级以及周期规则变更后必须重建并核对提醒计划。
- 应用启动和恢复前台时必须补算遗漏 occurrence 并校验未来提醒，但该补偿不能作为正常提醒的唯一实现。
- 通知被展示或划掉不代表已支付；只有用户执行明确确认命令才可以正式入账。
- 通知权限未授予、被撤销或系统限制导致无法可靠调度时，首页和设置页必须显示可操作的异常状态，不得仍声称提醒正常。
- Android“强行停止应用”会由操作系统抑制后台行为，属于无法绕过的平台限制；应用重新打开后必须立即重建提醒，并在帮助信息中说明该例外。

Windows 是否提供系统级提醒仍是独立待确认项，不影响 Android 首版可靠性合同。

### 11.4 导入匹配

导入项目与待确认套餐的匹配因素包括：

- 当前用户。
- 标准化商户名称。
- 金额及容差。
- 到期日前后的日期窗口。
- 可选账户。
- 是否已经匹配其他导入项。

首版安全策略：

- 唯一高置信候选可以在预览中自动选中。
- 多个候选或低置信候选必须由用户选择。
- 导入确认前不静默合并或删除任何交易。
- 匹配成功后由 `ImportItem.transactionId` 关联已有待确认交易，再把交易和 occurrence 转为正式状态。
- 未匹配到真实扣款的待确认交易保持 `PENDING`；到期后按 9.7 节显示为逾期，直到用户明确处理。

### 11.5 原子性与并发

以下操作不是若干独立实体 PATCH，而是一个原子领域命令：

- 生成 occurrence 及待确认交易。
- 确认 occurrence 和交易。
- 跳过/取消 occurrence 并撤销待确认交易。
- 导入匹配、交易更新、occurrence 确认和最小 provenance 写入。

与 occurrence 关联的交易不得通过通用 `DeleteTransaction` 独立删除。用户需要纠正误确认时，必须通过周期实例用例在一个事务中同步恢复或终止 occurrence 及其交易，避免留下声称已支付但没有账目的实例；具体撤销确认交互在实现该用例前补充。

本地在一个 `accounting.sqlite` 事务中执行，服务端在一个租户事务中执行。服务端对 `(userId, ruleId, periodKey)` 争用进行唯一约束和必要锁定；若其他设备已先创建，返回现有 occurrence 与交易的 canonical IDs。客户端必须重绑本地引用并删除或 tombstone 未被服务端接受的孤立候选。

创建或修改退款时，服务端必须锁定原支出并在同一事务中校验所有有效退款累计金额。只依赖客户端校验不足以防止两台设备并发退款超过原支出。

## 12. 历史文本导入

### 12.1 首版输入

- 剪贴板粘贴的机打文字。
- UTF-8 TXT 文件。
- 一行一条或可由日期标题分组的多行文本。

首版不使用 OCR，因为原始文字可以直接取得。直接处理文本具有更高准确率、更低开发成本，也更方便用户修正。

### 12.2 处理流水线

```text
读取原文
  -> 编码与换行标准化
  -> 切分候选记录
  -> 识别日期、金额、收支词、分类和备注
  -> 领域校验
  -> 重复检测
  -> 周期套餐候选匹配
  -> 预览及人工修正
  -> 单事务批量入账和 outbox 写入
```

### 12.3 解析策略

- 使用可测试的确定性规则和正则表达式。
- 不依据金额正负之外的模糊推断直接改变收支方向。
- 缺少年份时只在用户明确选择默认年份后补全。
- 缺少日期、金额或收支方向的记录标记为需要确认。
- 未知分类保留原始词并建议映射，不自动创建大量错误分类。
- 解析器通过 `IImportParser` 扩展，后续账单、PDF 和 OCR 复用同一候选与确认流程。

### 12.4 防重复

防重复至少包含：

- 整批原文规范化后的 batch hash。
- 单条规范化文本的 item fingerprint。
- 与现有交易的日期、金额、类型和商户近似比较。
- 用户明确选择“仍然导入”的覆盖路径。

batch hash 和 item fingerprint 只是版本化的重复启发式信息，不能代替 `batchId`、`mutationId` 等幂等身份，也不能设置成阻止用户强制导入的全局唯一键。规范化算法改变时必须增加 `normalizationVersion`；用户选择“仍然导入”时生成新的 override ID。

防重复只用于提示和阻止意外重复，不得自动删除已有交易。

### 12.5 后续支付信号接入边界

后续电子支付检测复用现有导入候选与人工确认流程，不建立能够直接写正式交易的第二条通道：

```text
用户授权的来源
  -> IPaymentSignalSource
  -> RawPaymentSignal（仅本地）
  -> IImportParser
  -> ImportItem 候选
  -> 去重、周期匹配和预览
  -> 用户明确确认
  -> POSTED Transaction
```

`IPaymentSignalSource` 只负责从平台能力获取原始信号，不理解分类、周期匹配或账目写入。允许的实现仅包括用户主动选择的账单文件，以及用户在 Android 系统设置中明确授权后读取的通知；通知来源 App 必须由用户通过 allowlist 选择。这是后续版本仍须遵守的产品边界。任何来源适配器都不得收集支付平台密码、验证码、Cookie 或令牌，不得使用无障碍服务模拟登录、抓取数据或执行付款。

`RawPaymentSignal` 至少携带来源类型、来源事件键或确定性指纹、观察时间、受限长度的原始 payload 以及解析器版本。解析器输出金额、币种、商户、日期、账户提示、置信状态和警告等候选字段。外部文件和通知文本一律视为不可信输入，在边界执行大小、编码、字段类型和金额范围校验。

完整通知正文和原始文件内容默认只保存在当前设备，不同步到云端，也不进入普通日志。服务端只接收用户确认后的交易和最小 provenance。高置信候选可以在预览中预选或匹配已有周期实例，但不能绕过用户确认直接变为 `POSTED`。

## 13. 分类结构分析

### 13.1 首版核心问题

分析页优先回答：本月的钱主要花在了哪里，各类支出占比多少。

### 13.2 默认计算口径

- 使用当前用户本地时区确定自然月。
- 只统计 `POSTED` 支出。
- 转账不计入收入或支出。
- 与原交易关联的退款减少该分类净支出。
- 待确认套餐单独显示，不混入正式支出。
- 未分类交易进入明确的“未分类”桶。
- 首版只汇总同一币种，默认 CNY。

### 13.3 页面内容

- 本月总收入、总支出和结余。
- 按顶级分类排序的金额和占比。
- 分类图表与列表使用同一查询结果。
- 点击分类查看子分类，再下钻到具体流水。
- 显示未分类占比，推动用户修正数据质量。

同比环比、预算和异常检测后续增加，不能改变首版已定义的基础统计口径。

## 14. 本地优先同步设计

### 14.1 原则

- 本地 SQLite 是当前设备的即时读写来源。
- 云端是多设备交换和恢复来源，不是记账操作的前置条件。
- UI 不等待网络请求才完成本地记账。
- 不依赖客户端墙钟进行同步排序。
- 财务记录发生并发冲突时不得静默丢失修改。

### 14.2 同步元数据

每个可同步实体至少拥有：

- 稳定 UUID。
- 只由服务端分配的 `serverRevision`；新建且未同步的实体固定为 0。
- `createdAt`、`updatedAt`。
- `deletedAt` tombstone。
- 最近修改设备 ID。

本地额外保存：

- `outbox` 待上传变更。
- `sync_conflicts` 中互不覆盖的本地与远端冲突 payload；记录既能表示单实体冲突，也能保存一个完整原子 change group 及其 affected entities。
- 每个模块同步流的服务端 opaque cursor。
- 实体本地 dirty/in-flight 状态，该状态不上传为 `serverRevision`。
- 最近成功同步时间。
- 最近同步错误和重试时间。

每个模块数据库是独立同步流，拥有自己的 outbox、cursor 和服务端变更日志。平台同步调度器可以依次驱动多个模块，但不能宣称跨 SQLite 文件原子提交；未来跨模块操作必须使用可重试的幂等编排或 saga。

服务端必须返回 opaque cursor，其内部顺序必须按 `(user, module)` 的事务提交顺序确定。普通 PostgreSQL `SERIAL`、`BIGSERIAL` 或 `nextval()` 的分配顺序不等于提交顺序，不能单独作为安全 pull cursor。可行实现包括在同一事务中锁定并更新用户/模块计数器，同时写入业务实体和 change log。客户端不解析 cursor，也不以 `updatedAt` 作为游标。

### 14.3 本地事务规则

每次本地修改必须在同一个 SQLite 事务内：

1. 验证领域规则。
2. 写入或更新业务实体。
3. 保留实体最近一次已确认的 `serverRevision`，不得在客户端增加它。
4. 以该值记录 outbox mutation 的 `baseServerRevision`。
5. 标记实体存在本地未上传修改。
6. 写入唯一的 outbox mutation。
7. 提交事务。

任何一步失败都必须整体回滚。只有业务数据和 outbox 同时提交后，UI 才报告成功。

同一实体同一时间最多有一个 in-flight mutation。已经发出的 mutation payload 必须冻结；发送期间发生的新编辑形成后续 unsent mutation。只有尚未发送的连续编辑可以合并，前一个请求确认后再把后续 mutation 的 `baseServerRevision` 更新为新服务端版本。

应用崩溃或进程被杀后，无法确认结果的 in-flight mutation 在下次启动时恢复为 unsent，但保留原 `mutationId` 和冻结 payload。服务端幂等记录决定它是首次执行还是返回已有结果。

### 14.4 Mutation 包络

供应商无关的请求示例：

```json
{
  "moduleId": "accounting",
  "streamEpoch": "opaque-epoch",
  "deviceId": "uuid",
  "mutations": [
    {
      "mutationId": "uuid",
      "commandType": "UPSERT_TRANSACTION",
      "payloadVersion": 1,
      "expectations": [
        {
          "entityType": "transaction",
          "entityId": "uuid",
          "baseServerRevision": 3
        }
      ],
      "payload": {}
    }
  ]
}
```

要求：

- `mutationId` 全局唯一；首次处理时保存规范化请求摘要和完整结果。相同 ID、相同摘要的重试返回原结果，相同 ID 携带不同命令或 payload 时返回 `IDEMPOTENCY_KEY_REUSED`，不能误报成功。
- `moduleId`、`streamEpoch` 和 `payloadVersion` 必须参与路由与兼容检查。
- `userId` 以经过验证的认证主体为准，不能信任客户端 payload 自报的归属。
- 新实体的 `baseServerRevision` 固定为 0；已有实体使用最近确认的服务端值。
- `expectations` 列出命令涉及实体的 `baseServerRevision`，用于发现并发更新。
- 删除 mutation 写入 tombstone，不直接删除服务端行。
- 服务端在一个事务中完成幂等检查、epoch 和 `baseServerRevision` 检查、实体修改、幂等结果及变更日志写入。
- 服务端为接受的修改分配新 `serverRevision` 和提交有序 cursor。
- 一个 mutation 是一个原子领域命令。简单 CRUD 可以只影响一个实体；套餐生成、确认、导入匹配等命令可以影响多个实体，但必须整体成功或整体失败。
- 同一 push 批次中的不同 mutation 独立提交并分别返回结果，批次本身不承诺全有或全无。
- 首版 UPSERT 发送完整的命令后实体 payload，不发送依赖应用顺序的 JSON Patch。DELETE 发送实体身份、预期 revision 和必要删除原因，不重复整行敏感内容。
- `serverRevision` 作用域是单个实体：首次接受后为 1，此后每次服务端接受修改单调增加。它与模块级 commit cursor 是两套不同概念。

同一用户、同一模块的服务端事务必须共享提交排序机制，例如锁定唯一 stream-state 行后分配 commit cursor。处理顺序固定为：验证认证主体并只解析查找所需的最小包络；锁定 stream-state；按 `(userId, moduleId, mutationId)` 查询幂等记录并校验规范化请求摘要；命中时即使原 payload 版本已退役也返回原完整结果；仅在未命中时执行完整 payload、`streamEpoch` 和 revision expectations 校验。实体修改、commit cursor、change group 和幂等结果在同一事务中提交。

push 结果至少包含：

```json
{
  "streamEpoch": "opaque-current-epoch",
  "results": [
    {
      "mutationId": "uuid",
      "status": "ACCEPTED",
      "resultStreamEpoch": "opaque-result-epoch",
      "canonicalIds": {},
      "revisions": [
        {
          "entityType": "transaction",
          "entityId": "uuid",
          "serverRevision": 4
        }
      ]
    }
  ]
}
```

`status` 至少区分 `ACCEPTED`、`CONFLICT`、`REJECTED`、`STALE_EPOCH` 和 `UPGRADE_REQUIRED`。冲突结果携带当前服务端 payload/revision；周期唯一键争用结果携带 canonical IDs。`STALE_EPOCH` 必须返回当前 epoch、转换原因和安全 bootstrap 入口。幂等重试返回的 `resultStreamEpoch` 若不同于当前 `streamEpoch`，客户端只把它当作历史提交确认并删除对应 outbox，不得把旧结果重新应用为当前实体；随后进入安全 re-bootstrap。只有首次出现且版本不受支持的 payload 才返回 `UPGRADE_REQUIRED`，并保留客户端 outbox。

### 14.5 API 轮廓

第一版只需要少量稳定接口：

```text
POST /v1/modules/<moduleId>/sync/push
GET  /v1/modules/<moduleId>/sync/pull?cursor=<cursor>&limit=<limit>
GET  /v1/modules/<moduleId>/sync/bootstrap?snapshot=<token>&page=<token>&limit=<limit>
GET  /v1/health
```

实际路径由供应商适配器决定。客户端只依赖 `ISyncTransport` 返回的类型化合同。

客户端不得直接对服务端内部业务表和 change log 执行普通 INSERT、UPDATE 或 DELETE，否则可以绕过 `serverRevision`、幂等、tombstone 和变更日志合同。对外只能开放经过审查的同步函数或 API。租户隔离策略是纵深防御，不是同步协议的替代品。

pull 响应至少包含当前 stream epoch、`nextCursor`、`hasMore`、payload version 和按提交位置排列的 change groups。每个 group 携带稳定 `changeGroupId`、命令类型、完整 affected entities 列表以及各实体的结果 payload/tombstone 和 revision。输入 cursor 表示“已经完整应用或持久隔离到此位置”，下一页只返回严格位于其后的提交。一个原子命令产生的 change group 不得跨页拆分；`limit` 是目标上限，单个 group 较大时允许响应超过它。`nextCursor` 只能指向响应中最后一个完整 group，空页保持原 cursor。

统一错误结构：

```json
{
  "error": {
    "code": "REVISION_CONFLICT",
    "message": "The transaction changed on another device.",
    "details": {}
  }
}
```

### 14.6 同步循环

1. 针对一个模块同步流检测可刷新的会话和网络可用性。
2. 选择互不共享 affected entities 的 unsent mutation，冻结 payload 后标记为 in-flight。
3. 分批上传 outbox，默认建议每批不超过 100 条。
4. 对成功项记录 `serverRevision`；只删除已确认的具体 mutation，不得误删发送期间产生的后续编辑。结果属于旧 epoch 时只确认历史提交，不应用旧实体，并转入安全 re-bootstrap。
5. 对后续 unsent 编辑更新 `baseServerRevision`，并等待下一批发送。
6. 对冲突项把本地和服务端 payload 写入 `sync_conflicts`，保留本地修改。
7. 使用 opaque cursor 分页拉取该模块的服务端变更。
8. 按 change group 预检完整 affected entities；只有全部实体均可应用且应用后满足聚合约束时，才在一个 SQLite 事务中应用整个 group，禁止只应用其中一部分。
9. 如果 group 中任一实体命中本地 dirty 状态、已有未解决冲突或会导致聚合约束失效，则整个 group 均不应用。客户端把完整远端 group、本地相关实体快照和 outbox 引用原子写入 `sync_conflicts`，并把 affected entities 标记为隔离；后续触及这些实体的 group 按顺序一并隔离。
10. 一页内每个完整 group 都已经整体应用或完整持久隔离后，才在同一 SQLite 事务中推进 cursor。持久隔离允许其他无关实体继续同步，但不能让 cursor 前进后丢失冲突组内容。
11. 失败时指数退避并加入随机抖动，网络恢复或用户手动操作时允许立即重试。

### 14.7 冲突策略

| 情况 | 首版策略 |
| --- | --- |
| 不同实体分别修改 | 正常合并 |
| 同一实体只有一端修改 | 接受新 `serverRevision` |
| 同一实体两端并发修改 | 在 `sync_conflicts` 保留双方 payload 并要求选择 |
| 一端删除、另一端修改 | 作为显式冲突，不静默删除修改 |
| 原子远端 group 中任一实体与本地修改冲突 | 隔离整个 group 及后续相关 group，不产生部分应用的聚合状态 |
| 重复创建同一周期实例 | 依赖唯一 occurrence key 幂等合并 |
| 重复提交同一导入批次 | 依赖 batch ID 和 mutation ID 返回既有结果；内容 hash 只作提示 |

首版不需要实现字段级自动合并。冲突数量预计很少，优先保证可解释和不丢数据。用户解决单实体冲突后，以选定或人工合并的 payload 针对最新 `serverRevision` 创建新 mutation；不得直接改写冲突中的历史副本。原子 change group 冲突必须以完整聚合为单位解决，在同一命令中为全部 affected entities 产生一致结果，再按服务端顺序重放被隔离的后续相关 group。

解决 mutation 获得服务端 ACK 后，当前实体替换为服务端确认版本，本地 dirty 状态清除，对应 conflict 标记为 `RESOLVED` 并保留最小审计信息。解决 mutation 再次冲突时保留原冲突和新服务端版本，不能提前删除冲突记录。

### 14.8 同步触发

- 登录并完成本地数据库打开后。
- 应用启动和恢复前台时。
- 本地修改后延迟合并触发。
- 用户手动点击同步时。
- Android 系统允许的后台任务中。

后台任务不能作为数据完整性的唯一保障。应用每次恢复时必须执行补偿同步和周期补算。

### 14.9 首次同步与 bootstrap

首次同步必须区分两条路径：

**空设备：**

1. 登录后向服务端请求稳定 snapshot token 和对应 high-water cursor。
2. 分页下载该 snapshot，写入临时或 staging 数据库。
3. 每页可以重试，但不能混入 token 之后提交的新状态。
4. 初次 bootstrap 完成前该 profile 保持“正在初始化”且不接受业务写入；全部下载并校验后原子激活数据库。
5. 从 high-water cursor 开始普通 pull，补齐 snapshot 之后的变更。

**已有本地数据：**

1. 用户明确选择要绑定的云端账号。
2. 当前本地 profile 与该账号建立一次性绑定，暂停该模块同步，记录本地 mutation/outbox 高水位 `bootstrapStartSeq`，但继续把新编辑写入 outbox。
3. 请求稳定 snapshot token 和 high-water cursor，把服务端 snapshot 下载到 staging。
4. 使用一致性本地快照，把截至 `bootstrapStartSeq` 的状态逐实体合并到 staging：远端独有实体保留；本地独有实体和现有 outbox 保留为 dirty；内容相同的同 ID 实体合并；内容不同、删除/修改冲突或异常 UUID 碰撞写入 `sync_conflicts`，双方均不丢失。
5. 最终激活前取得该模块串行数据库执行器的短时写入门闩，把 `bootstrapStartSeq` 之后新增或变化的 dirty 实体、冲突和 outbox 按本地顺序重放到 staging，在同一受控窗口内写入 high-water cursor 并原子激活。门闩期间到达的 UI 写请求排队，激活后对新数据库执行；不得在复制增量与激活之间接受写入旧数据库。
6. 先按正常命令上传可提交的本地 outbox，再从 high-water cursor 普通 pull，取得 snapshot 之后发生的提交。
7. 冲突实体在用户解决前不上传覆盖远端。

中断的 bootstrap 必须能够使用 continuation token 继续或安全丢弃 staging 后重来。普通 cursor 不能冒充 snapshot token。snapshot/continuation token 必须在服务端签名或持久保存，并绑定经过认证的 `userId`、`moduleId`、stream epoch、high-water cursor 和有效期；每页请求重新校验当前认证主体。客户端 staging 同时绑定本地 `ownerProfileId` 和 `remoteUserId`，退出或切换账号时不得把 token 或 staging 复用于其他 profile。账号删除或使该 snapshot 失效的 epoch 变化必须让后续页面请求失败，不能继续返回旧账号数据。

### 14.10 Tombstone、离线期限与游标过期

服务端必须公布 `minValidCursor` 或等价的同步 epoch。客户端 cursor 早于该边界时，pull 返回 `CURSOR_EXPIRED`，客户端进入安全 re-bootstrap，而不是继续增量同步。

push 必须携带当前 stream epoch。服务端完成认证、取得同一用户/模块的 stream-state 锁并查询幂等结果后，才对首次出现的 mutation 校验 epoch；陈旧 epoch 返回 `STALE_EPOCH`，不能让旧 UPSERT 越过已清理的 tombstone。在受支持的幂等重试窗口内，已经提交的同 `mutationId` 重试必须返回原结果，不能因 epoch 已变化改报 `STALE_EPOCH`。

服务端清理 tombstone 前必须满足已定义的最大支持离线期限，并保留足以拒绝陈旧 UPSERT 的删除保护或 stream epoch。幂等记录至少覆盖最大离线/重试窗口以及仍受支持的客户端生命周期，不能早于同一 mutation 可能重放的时间清除。re-bootstrap 遇到本地 pending mutation 时必须先进入审查/重新提交流程，不能静默丢弃或让旧实体复活。具体保留天数在上线前根据成本和允许离线时长确定。

### 14.11 Schema 与协议兼容

每个模块定义：

- `schemaVersion`。
- `payloadVersion`。
- `minReaderVersion`。
- `minWriterVersion`。

本地 migration 只向前执行并使用事务；新版本数据库不能被旧客户端静默降级打开。应用升级时必须同时升级或保留旧 outbox payload 的解释能力。服务端采用 expand-migrate-contract 发布顺序，在所有受支持客户端停止发送旧 payload 前不得移除旧字段或语义。

`schemaVersion` 只描述本地数据库，`payloadVersion` 只描述网络实体/命令，二者不得假定同步递增。push、pull 和 bootstrap 中每个 payload 都携带自己的版本。客户端不能读取服务端版本时返回 `UPGRADE_REQUIRED` 且不推进 cursor；服务端不能接受客户端写版本时拒绝 mutation 但保留客户端 outbox。版本错误不得触发自动清库或自动 re-bootstrap。

### 14.12 多模块协调

- 每个模块独立 push、pull、cursor、outbox、change log 和 bootstrap。
- 平台调度器逐模块运行并汇总展示状态。
- 一个模块失败不能回滚另一个已经提交的模块。
- 跨模块功能必须使用稳定操作 ID 和可补偿步骤，不跨 SQLite 文件承诺 ACID。

## 15. 认证与账号隔离

### 15.1 首版认证

- 账号由管理员在托管认证服务中预先创建。
- 客户端提供登录、会话刷新和退出。
- 首版不在客户端嵌入服务端管理密钥。
- 认证方式可以采用托管服务支持的邮箱或用户名映射加密码，最终交互形式仍待确认。

首版“管理员”指部署维护者，不是客户端中的高权限家庭账号。账号创建和云端删除通过托管控制台或受控管理脚本执行，管理凭据不进入普通客户端；操作至少记录目标账号、操作者、时间和 request ID。每个用户可以在客户端导出自己的账本，但服务端全量删除由维护者在获得明确确认后执行。

### 15.2 离线会话语义

- 设备第一次登录必须联网，以验证身份并建立本地 profile 绑定。
- 第一次成功登录后，短期 access token 过期只暂停同步，不关闭本地数据库，也不阻止离线 CRUD。
- 网络不可用或临时刷新失败与服务端明确返回账号撤销是不同状态，客户端不得把前者当成退出。
- 恢复联网并刷新会话后继续同步原 outbox。
- 用户主动退出时删除安全存储中的会话、关闭数据库并清理内存；UI 必须让用户选择保留本地副本供下次重新登录，或同时删除本地副本。
- 主动退出后不得仅凭过期本地会话重新打开账本；重新登录需要联网。
- 服务端撤销账号无法擦除一台始终离线设备上已经存在的数据，这是本地优先架构的明确限制。

### 15.3 服务端隔离

每张包含用户数据的服务端表必须有不可变的内部 `user_id`。服务端同步入口必须从经过验证的认证主体得到该值，忽略或拒绝客户端 payload 自报的归属。

服务端必须：

- 默认拒绝匿名访问。
- 只开放同步 API 或受审查的数据库函数，不向客户端开放绕过同步合同的普通业务表 DML。
- 在同步事务中同时验证租户、`baseServerRevision`、幂等键和 payload 版本。
- 对 `user_id` 及游标查询条件建立索引。
- 使用数据库租户策略或等价机制作为纵深防御。
- 自动化证明用户 A 无法读取或修改用户 B 的任何数据。
- 把所有管理密钥和可绕过租户隔离的凭据限制在受控服务端环境。

### 15.4 本地隔离与威胁边界

- 每个用户使用独立目录和独立记账数据库。
- 每个数据库保存不可变 `ownerProfileId` 元数据，打开时必须和当前本地 profile 一致；云端绑定后另存不可变 `remoteUserId`。
- 退出账号后关闭数据库连接并清除内存中的敏感 view model。
- 切换账号前必须完成 UI model 解绑，不能只更换查询条件。
- 访问令牌和刷新令牌由系统安全设施保护，不以明文写入普通 SQLite 表或日志。

首版本地威胁边界是“信任已登录的操作系统用户”。独立目录和 owner 检查防止应用自身串账，但不能阻止同一个 Windows OS 账号下能够直接读取文件的恶意家庭成员。若以后要求同一 OS 账号内的强对抗隔离，必须明确引入 SQLCipher 或自定义加密 QSQLITE、独立本地解锁和密钥恢复设计，不能声称 stock QSQLITE 已提供该保护。

## 16. 安全、隐私与数据恢复

### 16.1 安全边界

**已确认：**云端可以保存服务端可解密的数据，不做端到端加密。

**目标设计：**网络使用 HTTPS；云端依赖认证、最小权限、租户隔离、托管磁盘加密和备份保护数据。

### 16.2 客户端要求

- 所有 SQL 使用绑定参数。
- 外部导入文本和云端响应均按不可信输入处理。
- 限制单条文本、导入文件和同步批次大小。
- Android 使用 Keystore 密钥加密应用私有存储中的令牌；Windows 使用 Credential Manager 或等价系统设施。
- 发布版本不得提供“一键忽略 TLS 错误”。
- 日志默认脱敏，不记录密码、JWT、完整交易内容和原始导入全文。
- 可以后续增加应用锁或生物识别，但它不是首版云端安全替代品。

### 16.3 备份与导出

- 保留自动本地备份，通过串行数据库执行器调用 SQLite Online Backup API 或 `VACUUM INTO` 生成一致副本。
- 提供版本化“账本交换 JSON”，覆盖可同步记账实体及其稳定 ID，用于跨版本迁移和受控重新导入；它不是包含令牌、设备设置和原始导入全文的完整 profile 镜像。
- 提供 CSV 导出用于人工查看和其他工具分析。
- JSON 导入器必须验证格式版本、owner 目标、稳定 ID、引用完整性和重复项，并通过正常事务/outbox 写入。
- 导出格式包含版本号、币种、时间格式和包含/排除实体说明。
- 云端必须支持管理员备份和用户数据导出。
- 尚未绑定云端的本地 profile 可以把备份恢复到 staging 数据库，校验后原子替换。
- 已同步 profile 不得用旧数据库替换当前文件后沿用旧 cursor。恢复工具应先取得当前服务端 snapshot，再把备份中缺失的有效记录作为待审查 mutation 导入。

云端不可用但必须紧急查看旧备份时，可以把备份打开为隔离的 recovery profile。该 profile 默认不绑定账号、不启动同步，可以本地查看和导出；云端恢复后，必须通过 14.9 节的 snapshot 合并和审查 mutation 流程才能重新加入原账号。

一个 profile 的备份集包含 manifest、各模块 SQLite 一致性快照、schema 版本和文件 hash。生成备份集时暂停该 profile 新写入并依次完成快照；平台不声称多个数据库文件具有单个 SQLite 事务的原子性。

### 16.4 账号与数据生命周期

以下操作必须分开命名和实现：

| 操作 | 本地数据 | 云端数据 | 会话 |
| --- | --- | --- | --- |
| 退出登录 | 用户选择保留或删除 | 不变 | 删除当前设备凭据 |
| 删除本地副本 | 删除选定 profile 目录 | 不变 | 当前设备退出 |
| 删除选定流水 | 写入本地 tombstone 并保留可同步删除 | 通过普通同步传播删除 | 保持登录 |
| 删除云端账号 | 发起设备可先导出，服务端确认后删除本地副本；其他在线设备标记账号已删除并提示清理，离线设备不保证远程擦除 | 按策略删除 | 撤销全部会话后删除认证身份 |

首版不提供“保留 profile 或账号但一键清空整个账本”。当前单机版本的“物理清空并把 next ID 归零”只属于现状基线；迁移到目标架构时必须移除该入口。用户仍可逐笔或分批选择流水删除、删除本地副本，或者在确实要移除全部云端数据时删除云端账号。

整账本清空若以后进入范围，必须先单独定义并验证服务端操作资源、永久或足够长期的操作幂等身份、普通写入串行化、ACK 丢失查询、快照令牌失效、变更日志与托管备份保留语义，以及其他离线设备的恢复流程；不能由客户端循环 DELETE 或直接沿用当前清空函数拼装实现。

云端账号删除是带稳定 request ID 和持久状态的幂等服务端工作流。认证和每个模块同步事务都必须在共享的账号生命周期锁内确认状态仍为 `ACTIVE`；删除工作流在同一边界先改为 `DELETING`，阻止新会话和新写入，等待或排空已经开始的模块事务，再删除各模块业务数据、幂等记录、change log 和未完成 snapshot token，最后删除认证身份。删除 request 的终态记录放在业务数据之外，以便步骤失败后继续，不能留下可登录但数据只删除一半的账号。用户确认后的逻辑删除是否允许短期撤销，以及托管备份保留期限，必须在上线前明确。始终离线的设备无法被远程擦除，必须在隐私说明中说明。

## 17. DAT 到 SQLite 迁移

### 17.1 迁移原则

- 现有 DAT 文件属于已经存在的持久化数据，必须提供兼容迁移。
- 原始 DAT、旧 `records.dat` 和 `categories.dat` 不得被删除或原地改写。
- DAT 解析器迁移后转为只读 importer，不再承担新数据保存。
- 迁移失败不得产生半完成 SQLite 或破坏当前可启动版本。
- 用户必须先选择或创建一个没有记账实体的目标本地 profile；非空 profile 使用账本交换 JSON 的审查导入流程，不执行 DAT 原地合并。
- 无归属 DAT 不得被后台自动绑定到任意云端账号。
- 每个来源通过迁移 manifest 保证重试安全和归属可审计。

### 17.2 迁移步骤

1. 用户明确选择目标本地 profile。
2. 对来源计算 SHA-256，并建立包含来源路径、格式、hash、目标 profile、ID namespace、映射版本和完成状态的 migration manifest。
3. 使用现有严格解析器读取并验证 DAT。
4. 在临时位置创建目标 SQLite 数据库并执行 schema migrations。
5. 为旧整数记录 ID 生成可重放的稳定 UUID；可以从 manifest namespace 确定性派生，或在激活前持久保存完整映射。
6. 将字符串分类和子分类映射为稳定分类 ID。
7. 把旧流水作为 `MANUAL`、`POSTED` 交易导入。
8. 在同一 SQLite 事务中写入所有迁移数据和本地完成标记。
9. 校验记录数、收入总额、支出总额、日期范围和分类集合。
10. 完成数据库完整性检查。
11. 原子提交目标数据库并更新 manifest 完成状态。
12. 保留原始 DAT 作为只读备份，并提示用户导出长期备份。

manifest 至少使用 `PREPARED`、`DATABASE_ACTIVATED`、`COMPLETED` 状态。目标数据库同时保存 source hash、目标 profile 和 migration version。应用崩溃后以数据库内标记和文件 hash 为准恢复 manifest：数据库已激活但 manifest 未完成时补记完成；manifest 声称完成但数据库缺失或 hash 不符时停止并要求人工恢复。相同 source hash 默认不得再次迁入同一 profile。

### 17.3 特殊兼容点

- V3 `CATEGORY` 数据可能同时包含被引用的预设分类和真正自定义分类，迁移时不能把所有行一概视为自定义分类。
- 旧分类中使用括号表达子分类的历史数据，应沿用现有兼容规则，不对同名合法分类做破坏性拆分。
- 金额继续按整数分迁移，禁止经过浮点转换。
- 旧 `YYYY-MM-DD` 原样迁移到 `occurredOn`，不得臆造发生时间或时区。
- 旧日期范围和当前 Qt 控件限制需要在迁移报告中明确记录。
- 旧“退款”“个人转账”等分类不触发新交易类型推断。
- 本地 profile 第一次绑定云端账号时必须再次显示来源和目标账号，绑定后只能通过显式迁移流程更换账号。

## 18. UI 设计职责

Android 与 Windows 必须遵守 11.2 节定义的共同周期支出交互语义：规则概览展示所有未归档项目，实例视图按 period 展示本期金额、倒计时/逾期天数、状态和备注/标记；同一规则存在多期未解决实例时必须分别显示，已归档规则遗留的未解决实例也不能隐藏。首页展示进入提前提醒窗口、当天到期和已逾期实例；确认已支付后直接形成正式账目。两端可以采用不同布局，但不得把通知展示误认为已经支付。

### 18.1 Android

首版优先页面：

- 登录。
- 今日/近期概览，包括周期支出首页确认卡片。
- 快速记一笔。
- 流水列表与详情编辑。
- 周期支出列表与待确认/逾期实例。
- 简化分类分析。
- 同步状态和设置。

Android 快速记账应尽可能在一个页面完成，金额输入默认聚焦，常用分类和最近选择优先展示。不得照搬桌面侧边栏和表格布局。

### 18.2 Windows

保留并逐步适配现有页面：

- 仪表盘，包括周期支出首页确认卡片。
- 流水列表、筛选和编辑。
- 分类分析。
- 分类管理。
- 导入预览和批量修正。
- 账户核对。
- 周期套餐管理。
- 导出、备份和同步诊断。

Windows 继续优化键盘、表格、批量操作和大屏信息密度，不追求与 Android 像素级一致。

### 18.3 同步状态

两端都必须提供可理解的同步状态：

- 已同步。
- 有本地待上传修改。
- 正在同步。
- 离线。
- 同步失败，可重试。
- 存在冲突，需要处理。

不得只显示旋转图标而不说明失败原因，也不得让同步失败阻止继续本地记账。

## 19. 测试策略

### 19.1 领域单元测试

- 金额边界、精度和溢出。
- 收入、支出、退款和转账计算。
- 分类归档及父子关系。
- 月度边界、时区和闰年。
- 周期规则、月末日期、提醒日期和 occurrence 幂等性。
- 本期金额/备注修改不污染未来规则，逾期不自动入账，未解决的上一期不阻止下一期生成。
- 延期保留同一条有效待确认交易；跳过/取消原子写入交易 tombstone 并清空 `transactionId`；通用交易删除拒绝破坏 occurrence 关联。
- 分类分析口径。

### 19.2 SQLite 集成测试

- 首次建库和逐版本 migration。
- CRUD、事务回滚、外键和唯一约束。
- 业务写入与 outbox 原子提交。
- tombstone 和清理策略。
- 多账号本地文件隔离。
- DAT 迁移前后记录数和汇总一致。
- 串行 executor、busy timeout、连接线程归属和账号切换排空。
- 使用 Online Backup API 或 `VACUUM INTO` 生成并恢复一致备份。
- 旧客户端拒绝打开超出其 `minReaderVersion` 的数据库。

### 19.3 同步测试

- 离线创建后恢复联网。
- 请求超时和重复提交。
- 上传成功但响应丢失后的幂等重试。
- 多页 pull 中途失败后的 cursor 一致性。
- 两个服务端事务以相反于 ID 分配顺序提交时仍不会漏拉。
- 同一实体发送期间再次编辑时，后续 mutation 不会被前一个 ACK 删除。
- 同一实体并发修改。
- 删除与修改冲突。
- 两台设备同时生成同一期套餐。
- 多设备争用周期唯一键时返回 canonical IDs，且不留下孤立待确认交易。
- 套餐确认、导入匹配和退款上限校验在服务端保持跨实体原子性。
- 本地待确认交易存在 dirty 修改时拉取远端确认/取消 change group，整组被隔离且 occurrence/交易不会进入拆分状态；解决后按序重放后续相关 group。
- 导入批次重复提交。
- 空设备的可恢复 snapshot bootstrap。
- 有本地迁移数据时 bootstrap 不覆盖本地行和 outbox；取得初始本地快照后发生的编辑会在写入门闩内重放到 staging，再原子激活。
- cursor 过期、tombstone 已清理和陈旧设备重新接入。
- 相同 `mutationId` 和相同请求摘要返回原结果；相同 ID 携带不同 payload 被明确拒绝。
- 已接受 mutation 的 ACK 丢失且随后 epoch 变化时，重试返回历史结果和当前 epoch；客户端确认 outbox 后进入 re-bootstrap，不把旧实体复活。
- 不同模块的独立 cursor、失败隔离和 payload 版本兼容。

### 19.4 权限测试

- 匿名用户无法访问业务表。
- 用户 A 无法读取、插入、更新或删除用户 B 的任何数据。
- 客户端伪造 `user_id` 被服务端拒绝。
- snapshot/continuation token 不能跨账号、模块、epoch 或有效期使用；退出、账号删除和失效 epoch 后不能继续下载旧 snapshot。
- 客户端无法通过普通表 DML 绕过同步函数的 `serverRevision`、幂等和 tombstone 逻辑。
- 账号进入 `DELETING` 后，新同步事务被拒绝，已经开始的事务在删除数据前排空，删除完成后不能再写入 change log 或创建 snapshot。
- 供应商的租户隔离机制对每个业务集合都有允许与拒绝用例。
- 服务端 secret key 不存在于客户端二进制、资源文件和仓库。

### 19.5 导入测试

- 使用真实脱敏记事本样本建立 fixture。
- 覆盖不同日期格式、空行、标题、负数、备注和格式错误。
- 解析错误保持原文并可人工修正。
- 重复 batch 和重复 item 得到稳定提示。
- 整批提交失败时没有部分入账。

### 19.6 UI 与构建验证

- Windows 关键 Widgets 流程冒烟测试。
- Android 登录、离线记账、恢复联网和周期支出确认流程测试。
- 周期支出实例视图同时显示同一规则的多期未解决实例，并持续显示已归档规则遗留的待确认/逾期实例。
- Android 在断网、最长连续 90 个自然日不启动应用和正常重启设备后，仍能在指定自然日展示窗口内已调度提醒；覆盖日期/时区变化后的重建。
- Android 覆盖应用升级后的提醒重建，以及被强行停止后重新打开应用时的立即重建。
- Android 以最近计划日期为 `D0` 验证滚动窗口：`D0 + 89 天`和 `D0 + 90 天`提醒必须送达，`D0 + 91 天`在未填充新窗口时不作保证。
- Android 验证本地及同步拉取的确认、跳过、取消会撤销旧提醒，延期会按新日期替换提醒。
- Android 通知权限拒绝或撤销时显示可操作的异常状态；通知展示或划掉不会改变账目状态。
- Android 金额输入和展示超过 32 位整数范围时仍通过字符串边界精确往返，QML 不进行货币运算。
- Android 冷启动后台同步原型证明 Qt 运行时可用；无法证明时只允许同步和周期补算回退为前台补偿，提醒仍须通过独立原生调度验证。
- Windows、Linux 核心和 Android 构建进入 CI。
- 每个迁移阶段保持当前后端回归测试可运行。

## 20. 实施路线

### 阶段 0：建立可回退基线

- 固化当前可用 Windows 版本和现有测试结果。
- 区分当前修复工作与新架构迁移，避免在同一变更中混合。
- 收集脱敏历史文本样本。
- 明确目标部署地域、预算上限、可接受延迟和测试账号，在实际 Windows/Android 网络中验证候选托管服务并完成供应商选型。
- 为 `serverRevision`、提交有序 cursor、bootstrap、tombstone/游标过期和协议版本分别形成可测试合同或 ADR。
- 明确 Android 最低/目标 API、ABI、设备矩阵及 SDK/NDK/JDK 版本，验证 Qt/C++ 认证与同步原型、Android 冷进程后台任务能否初始化所需 Qt 运行时，以及原生提醒在离线、重启和连续 90 个自然日未打开应用时的送达与重建路径。

完成标准：指定一个可追溯的代码 revision 及 Windows 构建环境作为基线；所有阻塞验证已有结论，托管供应商已经选择，失败项已有明确回退方案。

### 阶段 1：构建与模块边界

- 引入 CMake。
- 把平台接口、记账领域、存储、同步、桌面应用、Android 应用和测试拆成显式 target。
- 建立 application service 和 repository 接口。
- 建立应用显式调用的编译期模块注册表。
- 保持 Widgets 行为不变。

完成标准：Windows 应用和后端测试通过 CMake 构建；核心不依赖任何 UI 模块；repository/application 接口有契约测试；显式 registry 的顺序、重复 ID、依赖失败和平台 UI 分离测试通过。

### 阶段 2：SQLite 与稳定身份

- 定义首版 schema 和 migration 机制。
- 引入 UUID、账户、稳定分类 ID、交易状态、`serverRevision` 和本地 dirty/outbox 状态。
- 实现 SQLite repository。
- 建立目标本地 profile 和 migration manifest，实现并验证 DAT 只读迁移。
- Windows 切换到 SQLite，停止产生新 DAT。

完成标准：迁移前后记录数和金额汇总一致，失败可安全回退。

### 阶段 3：Android 垂直切片

- 建立 Qt Android CMake 构建。
- 添加 QML 登录占位、快速记账和流水列表。
- 添加最小周期支出垂直切片：专用列表、首页确认卡片和一条可跨正常重启恢复的本地提醒。
- 通过 QObject facade 和 list model 调用共享用例。
- 以十进制字符串输入金额并验证 64 位金额边界。
- 验证 Android SQLite、生命周期和发布打包。

完成标准：Android 在飞行模式下可以新增、查看和修改交易，重启后数据仍在；周期支出可以离线确认入账，已调度提醒在正常重启后仍能按自然日送达。

### 阶段 4：认证与同步

- 在阶段 0 已选托管服务上建立开发和测试环境。
- 建立服务端 schema、租户隔离、迁移和权限测试。
- 实现管理员预建账号及客户端登录。
- 实现每模块 outbox、`serverRevision`、push、commit-ordered cursor、pull、bootstrap、tombstone、cursor expiry 和冲突表。
- 禁止客户端绕过同步入口直接写业务表。
- 加入安全令牌存储和同步诊断。

完成标准：同一账号两台设备最终收敛，不同账号无法访问彼此数据，重复和乱序请求幂等，冲突不静默丢失。

### 阶段 5：首版记账能力闭环

- 完成简洁/账户模式。
- 完成账户补漏、转账和退款。
- 完成周期支出专用列表、本期调整、待确认/逾期生命周期、首页幂等确认和可靠提醒合同。
- 完成文本导入、预览、套餐匹配和防重复。
- 完成本月分类结构分析。

完成标准：Android/Windows 记账、同步、分类分析、周期套餐和文本导入形成端到端闭环；备份恢复与发布级故障演练在阶段 6 完成。

### 阶段 6：恢复与发布加固

- 完成 JSON/CSV 导出、自动备份和恢复演练。
- 完成 Windows 与 Android 安装包。
- 完成离线、弱网、令牌过期、数据库损坏和云端故障测试。
- 实现并演练退出、删除本地副本和删除云端账号三条独立流程；确认所有 profile 均不暴露整账本清空入口。
- 补充本地 OS 信任边界、远程无法擦除离线副本、数据删除和账号恢复说明。

完成标准：即使云端暂时不可用，用户仍能记账，并可把本地备份打开为隔离 recovery profile；恢复同步时经过安全合并；发布包不包含服务端秘密。第 3.5 节的七项第一阶段关键结果至此全部验收，阶段 6 是首版发布门槛。

### 阶段 7：后续能力

按真实使用反馈依次评估：

1. 微信、支付宝和银行卡 CSV/XLSX。
2. PDF 账单。
3. 截图或照片 OCR。
4. 经用户明确授权且按 App allowlist 限定的 Android 支付通知识别。
5. 预算、趋势和异常分析。
6. 公众注册、邮箱验证、找回密码和运营风控。
7. 共享账本。
8. 保留 profile 或账号的一键整账本清空及其多设备协议。
9. 其他工具模块及动态插件需求。

阶段 7 的账单文件和支付通知能力开始实施前，必须先补充来源 allowlist、权限撤销、输入上限、候选去重以及“不得自动 `POSTED`”的契约与安全测试。负向测试必须证明适配器不接受凭据、不代登录、不抓取或执行付款，并证明原始通知/文件内容不会进入同步 payload、云端导出或日志。任何新增来源若超出 4.3 和 12.5 节的边界，必须先形成新的明确产品决策，而不能只增加一个适配器。

## 21. 首版验收标准

### 21.1 数据正确性

- 金额全程使用整数最小货币单位。
- DAT 迁移前后记录数、总收入和总支出一致。
- 转账不进入收支统计，退款按既定口径抵减支出。
- 分类图表、列表和下钻使用同一统计结果。
- 同一期周期规则在任何设备数量下只产生一个 occurrence。

### 21.2 离线与同步

- 无网络时 Android 和 Windows 均可完成领域规则允许的完整 CRUD；关联周期实例的交易必须通过周期实例用例处理。
- 已在设备成功登录的 profile 即使 access token 过期也能继续执行领域规则允许的离线 CRUD；同步暂停直至会话刷新。
- 网络恢复后未确认同步前，本地数据不会丢失或回滚。
- 重复请求不会产生重复交易。
- 删除可以传播到其他设备。
- 并发冲突保留双方版本并要求用户处理。
- 周期实例等跨实体原子 change group 发生冲突时整体隔离，不出现 occurrence 与关联交易状态不一致的部分拉取结果。
- cursor 过期时进入安全 re-bootstrap，不遗漏删除也不静默丢弃本地 mutation。
- 用户可以看到当前同步状态和最后错误。

### 21.3 账号隔离

- 用户 A 在应用和服务端均无法访问用户 B 的数据。
- 切换账号不会短暂展示上一个账号缓存。
- 服务端根据已验证认证主体强制租户隔离，不依赖客户端传入过滤条件。
- 首版本地隔离明确以操作系统账号可信为前提，不虚假宣称能够抵御同一 OS 账号直接读取文件。

### 21.4 核心体验

- Android 快速记账不要求填写账户等隐藏字段。
- Windows 可以批量检查未分类和未指定账户流水。
- 文本导入在确认前不写入正式交易。
- 周期支出页面能看到全部未归档规则，并按 period 分别显示每个未解决实例的金额、倒计时或逾期天数、状态和备注/标记；上一期逾期与本期待确认可以同时出现，归档规则遗留实例持续可见。
- 首页能直接修改本期金额/备注并确认已支付；确认命令一次生成正式账目，重复点击或同步重试不产生重复交易。
- 未确认的到期项目不会自动入账，持续显示为逾期，直到用户确认、延期、跳过或取消；上一期逾期不阻止下一期生成。
- 若阶段 0 原型确认采用 11.3 节的建议合同，Android 在离线、正常重启和最长连续 90 个自然日未打开应用时，仍须在到期前 1 或 2 天以及到期日当天按自然日展示窗口内提醒；若原型不可行，必须先形成替代 SLA 决策并同步修订 11.3、19.6、21.4 和 D-017，不能静默降低发布标准。
- Android 无法获得通知权限或可靠调度受到系统限制时，首页和设置页明确显示异常及处理方式。
- Android 被用户强行停止期间不承诺后台提醒；重新打开后立即重建。超过阶段 0 最终确认的滚动计划窗口或规则尚未同步到本机的事件不在离线提醒保证内。
- 月度分析可以从分类占比下钻到具体流水。

### 21.5 可恢复性

- 本地数据库通过 SQLite 一致性备份机制生成并在临时位置验证后恢复。
- 已同步 profile 的旧备份以审查 mutation 方式恢复，不携带陈旧 cursor 直接覆盖当前数据库。
- 用户可以导出版本化 JSON 和可读 CSV。
- 云端故障不影响当前设备继续使用。
- 旧 DAT 在迁移后仍保留且不再被写入。

## 22. 主要风险与缓解措施

| 风险 | 影响 | 缓解措施 |
| --- | --- | --- |
| 把同步误当成简单 CRUD | 产生重复、覆盖或无法传播删除 | 先定义 mutation、`serverRevision`、cursor、tombstone 和冲突合同，再实现 UI |
| 使用非提交有序 sequence 作 cursor | 事务乱序提交时永久漏拉 | 使用 opaque、按用户/模块提交有序的 cursor 并测试乱序提交 |
| 客户端直接写云端业务表 | 绕过 `serverRevision`、幂等和 change log | 撤销普通 DML，只开放受审查同步入口 |
| 旧设备晚于 tombstone 清理后上线 | 删除丢失或旧实体复活 | 定义最大离线期、min valid cursor、stream epoch 和安全 re-bootstrap |
| SQLite 改造与 UI 重写同时进行 | 回归范围过大 | 先保持 Widgets 行为，分阶段切换 repository 和数据模型 |
| Android QML 与 C++ 边界混乱 | UI 与业务互相耦合，难以测试 | 使用 application service、QObject facade 和 list model，禁止领域层操纵 QML |
| Android 后台同步任务被系统延迟 | 云端数据收敛或周期补算不及时 | 启动/恢复前台时补偿，不把后台任务作为同步完整性的唯一保障 |
| Android 提醒因重启、权限或系统限制丢失 | 用户错过月付或租金日期，破坏核心价值 | 使用持久化原生调度和重启/日期变化重建；持续显示权限与调度健康状态，并测试目标设备矩阵 |
| 托管服务网络不可达或锁定 | 多端同步失效，迁移成本高 | 先做实际网络原型，通过接口隔离供应商并保留完整数据导出 |
| 平台化过度设计 | 首版长期无法交付 | 只做编译期模块、稳定 ID 和共享服务，不做动态插件 |
| 历史文本格式差异大 | 自动解析准确率低 | 使用真实脱敏 fixture、预览和人工确认，不静默入账 |
| 租户策略配置遗漏 | 账号间财务数据泄漏 | 默认拒绝、同步入口校验、数据库纵深防御和自动化 allow/deny 测试 |
| 并发冲突被静默覆盖 | 财务数据丢失 | 分离 `serverRevision` 与本地 dirty 状态，使用显式冲突表 |
| 用旧 SQLite 直接恢复已同步账号 | 已删除数据复活或 cursor 回退 | 先取得当前 snapshot，再把恢复内容作为审查 mutation 导入 |

## 23. 尚待确认的问题

以下问题不改变总体架构，但应在对应阶段开始前确定：

1. 托管服务部署地域以及 Supabase 在实际网络中的可达性。
2. 家庭账号采用邮箱、用户名映射还是其他登录标识。
3. 同一设备是否需要频繁切换多个账号。
4. 首版是否严格只允许 CNY，以及币种字段的展示方式。
5. 周期规则在每月 29、30、31 日遇到短月份时的处理策略。
6. Windows 是否需要系统级提醒，以及 Windows/Android 的默认提醒时段和静默时段。
7. tombstone 的默认保留期限和产品承诺的最大离线时长。
8. 同一交易发生冲突时的具体对比与选择界面。
9. 账户余额对信用账户、退款和期初余额的具体展示口径。
10. 首批真实历史文本的格式样本和解析规则优先级。
11. 云端账号删除后托管备份的保留期限。
12. Android WorkManager 冷进程同步原型的结果，以及同步失败时采用的前台补偿策略；该结果不得降低 11.3 节的本地提醒合同。
13. 误确认周期支出的撤销交互，以及恢复或终止 occurrence/交易时如何保留已有导入项和 provenance 关系；这是阶段 5 实现关联交易删除前的前置决策。
14. 提前 1 或 2 天的默认提醒策略及 90 天滚动送达窗口是否成为正式产品承诺；阶段 0 必须先用目标 Android API 和设备矩阵验证可行性。

## 24. 决策记录

| 编号 | 决策 | 状态 | 原因 |
| --- | --- | --- | --- |
| D-001 | 首版优先交付稳定记账，不先建设完整工具平台 | 已确认 | 用户选择优先获得可用记账工具 |
| D-002 | Android 为日常入口，Windows 为完整管理端，Linux 主要用于开发 | 已确认 | 三端使用职责不同 |
| D-003 | 每端保存完整可同步记账实体，离线可用，联网增量同步 | 已确认 | 记账不能依赖网络或云端可用性 |
| D-004 | 保留 Windows Widgets，Android 使用 QML，共享 C++ 核心 | 已确认 | 兼顾已有投入、移动体验和单人维护成本 |
| D-005 | 简洁模式和账户模式共用一套交易模型 | 已确认 | 避免未来迁移和两套统计口径 |
| D-006 | 周期套餐生成待确认支出，并在导入时自动匹配 | 已确认 | 避免扣款失败、涨价或提前续费造成错误流水 |
| D-007 | 首版导入原始机打文本，不先做 OCR | 已确认 | 原文可取得，直接解析更准确且成本更低 |
| D-008 | 使用低维护托管云端和标准云安全，不做端到端加密 | 已确认 | 优先降低长期运维和密钥管理复杂度 |
| D-009 | 底层支持多用户，首版只提供 2 至 3 个预建且互相隔离的账号 | 已确认 | 保留未来公开使用能力但避免首版账号系统膨胀 |
| D-010 | 首版平台采用编译期模块，不实现动态插件 | 已确认 | 满足扩展边界且控制首版复杂度 |
| D-011 | Supabase 作为首选托管候选 | 候选方案 | 仍需验证部署地域、网络、费用和 Qt REST 原型 |
| D-012 | `serverRevision` 只由服务端分配，本地编辑状态单独保存 | 目标设计 | 避免客户端与服务端双重版本权威 |
| D-013 | 每个工具模块使用独立同步流和 opaque commit-ordered cursor | 目标设计 | 避免跨 SQLite 文件伪原子性和 sequence 漏拉 |
| D-014 | 首版本地安全信任当前 OS 用户 | 目标设计 | 当前未确认引入本地数据库加密及密钥恢复成本 |
| D-015 | 手工记账是日常主入口，周期支出管理是并列的一级核心闭环 | 目标设计 | 已确认需求包含月卡、月付和租金日期管理；具体信息架构仍需界面验证 |
| D-016 | 周期实例到期后不自动入账，逾期持续可见且不阻止下一期生成 | 目标设计 | 防止制造假流水，同时保留漏付提醒和跨期追踪能力 |
| D-017 | Android 周期提醒离线、跨正常重启并在最近计划日期后 90 天的窗口内按自然日可靠送达 | 目标设计 | 这是可测试的建议可靠性合同，需通过阶段 0 目标设备原型后才能成为发布承诺 |
| D-018 | 后续支付检测严格遵守 12.5 节，只接入用户主动导入的账单文件和经授权、按 App allowlist 限定的 Android 通知 | 后续范围 | 所有信号只生成待人工确认候选，原文仅本地保存；禁止凭据收集、代登录、抓取和任何付款执行 |
| D-019 | 首版不提供保留 profile 或账号的一键整账本清空 | 已确认 | 避免在首版引入未经专项验证的破坏性分布式操作；仍保留选定流水删除、本地副本删除和云端账号删除 |

## 25. Supabase 候选实现附录

本节只在 Supabase 通过阶段 0 验证并被正式选中后适用，不属于供应商无关合同。

- 目录使用 `cloud/supabase/`，保存数据库 migration、函数和 pgTAP 权限测试。
- 客户端只持有 publishable key 和当前用户 JWT，不包含 secret/service role key。
- 对暴露 schema 中的所有内部业务表和 change log 启用 RLS，并撤销 `anon`、`authenticated` 的普通 DML。
- 只向 `authenticated` 授予经过审查的 sync RPC `EXECUTE` 权限；RPC 从 `auth.uid()` 获取用户，不接受调用者指定其他归属。
- RLS 继续限制 `auth.uid() = user_id`，作为同步函数校验之外的纵深防御。
- 每个 RPC 在一个 PostgreSQL 事务中执行租户校验、幂等检查、`baseServerRevision` 检查、实体写入和 commit-ordered change log 写入。
- 同一用户、同一模块的所有 mutation RPC 先锁定唯一 stream-state 行；锁内校验规范化请求摘要并优先返回已有 `mutationId` 结果，再对首次请求校验 epoch 和 revision，任何写入路径都不得绕过该排序点。
- 若 RPC 使用 `SECURITY DEFINER` 或由具备 `BYPASSRLS` 的角色拥有，必须固定 `search_path`、显式验证 `auth.uid()` 并按“RLS 不会保护该函数”进行测试。
- 如果 Edge Function 使用可绕过 RLS 的服务端 secret，必须先验证 JWT 并自行强制租户边界，不能依赖 RLS 自动保护。
- 每张表和每个 RPC 都必须包含用户本人允许、其他用户拒绝和匿名用户拒绝测试。
- Auth access token 到期只影响远端请求；整体会话和本地离线访问遵循 15.2 节。

Supabase 自动 REST 表接口不能直接替代本文同步合同。

## 26. 官方参考资料

- Qt for Android 6.9：<https://doc.qt.io/qt-6.9/android.html>
- Qt Quick Controls 6.9：<https://doc.qt.io/qt-6.9/qtquickcontrols-index.html>
- QML 与 C++ 集成：<https://doc.qt.io/qt-6.9/qtqml-cppintegration-overview.html>
- Qt SQL 与 QSQLITE：<https://doc.qt.io/qt-6.9/sql-driver.html#qsqlite-for-sqlite-version-3-and-above>
- Qt CMake 入门：<https://doc.qt.io/qt-6.9/cmake-get-started.html>
- QNetworkAccessManager：<https://doc.qt.io/qt-6.9/qnetworkaccessmanager.html>
- QSqlDatabase：<https://doc.qt.io/qt-6.9/qsqldatabase.html>
- Qt Android Services：<https://doc.qt.io/qt-6.9/android-services.html>
- SQLite Online Backup API：<https://sqlite.org/backup.html>
- PostgreSQL Sequence 注意事项：<https://www.postgresql.org/docs/current/functions-sequence.html>
- Supabase Auth：<https://supabase.com/docs/guides/auth>
- Supabase Session：<https://supabase.com/docs/guides/auth/sessions>
- Supabase REST API：<https://supabase.com/docs/guides/api>
- Supabase API 安全：<https://supabase.com/docs/guides/api/securing-your-api>
- Supabase Row Level Security：<https://supabase.com/docs/guides/database/postgres/row-level-security>
- Supabase 本地开发：<https://supabase.com/docs/guides/local-development/cli/getting-started>
