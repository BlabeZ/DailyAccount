# DailyAccount V1 Target Architecture Master Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Evolve the current single-user Qt Widgets/DAT application into the local-first Windows and Android accounting application defined by `docs/product-architecture.md`, without losing existing data or weakening offline behavior.

**Architecture:** Keep the existing repaired DAT application as a reproducible migration baseline, then introduce CMake and stable module boundaries before changing persistence. Move the shared C++17 accounting domain to SQLite-backed repositories, add a separate Qt Quick Android application, and connect both clients through a provider-isolated authentication and atomic change-group synchronization protocol. Complete account, recurring-expense, import, analytics, recovery, and packaging workflows only after their lower-level contracts pass independent gates.

**Tech Stack:** C++17, CMake 3.22+, Qt 6.9.3 Core/Widgets/Quick/QML/SQL/Network, SQLite through QSQLITE, Kotlin only for narrow Android platform bridges, Android SDK/NDK/JDK versions frozen by Stage 0, managed PostgreSQL/Auth service selected by Stage 0, MinGW on Windows, GCC on Linux.

**Spec:** `docs/product-architecture.md`

## Plan Suite

Execute these plans in order. A later plan may start only when the preceding plan's exit gate is recorded as passed.

| Order | Plan | Exit gate |
| --- | --- | --- |
| 0 | `docs/superpowers/plans/2026-09-04-stage-0-baseline-and-prototypes.md` | G0: baseline, decisions, and blocking prototypes accepted |
| 1 | `docs/superpowers/plans/2026-09-04-stage-1-cmake-and-boundaries.md` | G1: CMake build and architectural boundaries pass |
| 2 | `docs/superpowers/plans/2026-09-04-stage-2-sqlite-and-migration.md` | G2: Windows runs from migrated SQLite data |
| 3 | `docs/superpowers/plans/2026-09-04-stage-3-android-offline.md` | G3: installable offline Android vertical slice passes |
| 4 | `docs/superpowers/plans/2026-09-04-stage-4-auth-and-sync.md` | G4: isolated accounts and two-device convergence pass |
| 5 | `docs/superpowers/plans/2026-09-04-stage-5-accounting-workflows.md` | G5: all first-version accounting workflows pass |
| 6 | `docs/superpowers/plans/2026-09-04-stage-6-recovery-and-release.md` | G6: recovery, security, installers, and release acceptance pass |

## Global Constraints

- Preserve all existing user changes in the worktree. Never reset, discard, or overwrite unrelated edits.
- Do not create Git commits unless the execution session has explicit user authorization. Each task lists an optional checkpoint command, but authorization is still required at execution time.
- Keep C++17 as the shared-language floor. Domain targets must not include Qt Widgets, Qt Quick, QML, SQL, Network, JNI, Android Context, or provider SDK types.
- Keep all monetary values as signed 64-bit integer minor units. Persist no financial amount as `double` or QML `int`.
- Use client-generated UUIDs for every synchronizable entity and command. Local integer IDs remain only inside the read-only DAT importer.
- Keep Windows and Android fully usable for local accounting while offline. UI success must follow the local SQLite transaction, never a network response.
- Keep each user's local files under a separate profile directory and enforce server ownership from the authenticated subject, never a client-supplied `userId`.
- Use bound SQL parameters, foreign keys, `CHECK`, `UNIQUE`, and supported `STRICT` tables. Never wait for network I/O inside a SQLite transaction.
- Keep each module's business rows, outbox, conflicts, and cursor in the same module database. Do not claim atomicity across separate SQLite files.
- Preserve `ledger.dat`, `records.dat`, and `categories.dat` as read-only migration sources. Never delete or rewrite source DAT files.
- Stop producing new DAT snapshots only after the SQLite migration gate passes. After the first SQLite write, never silently fall back to DAT.
- Use complete atomic commands for operations spanning multiple entities. A pull change group is applied completely or persisted completely as a conflict group.
- Treat Android reminders as a local native capability, separate from background synchronization. WorkManager failure may reduce background sync frequency but may not reduce the 90-day reminder contract.
- Treat imported files, notification text, cloud JSON, and provider responses as untrusted input with explicit size and type validation.
- Future payment signals may come only from user-selected bill files or explicitly authorized, per-App allowlisted Android notifications. They create candidates, never automatic `POSTED` transactions.
- Do not implement public registration, shared ledgers, end-to-end encryption, dynamic plugins, payment-provider credential collection, provider login/scraping, or payment execution in V1.
- Every behavior-changing task follows red-green-refactor: add a focused failing test, observe the expected failure, add the smallest implementation, then run focused and cumulative suites.
- Every migration or destructive lifecycle workflow requires a pre-change backup, an explicit atomic activation point, and a tested failure path that leaves the prior state usable.

## Verified Starting Point

Recorded on 2026-09-04 in the current working tree:

- Current application: one Qt Widgets executable built by `jizhang.pro`.
- Current domain/persistence: `backend/record.h`, `backend/category.*`, `backend/storage.*`, and `backend/ledger.*`.
- Current UI: `gui/main_gui.cpp`, `gui/mainwindow.*`, and five page implementations under `gui/`.
- Current storage: exact-cent V3 `ledger.dat`, legacy paired DAT import, checksummed snapshots, atomic replacement, backup, and recovery.
- Current tests: `tests/backend_tests.cpp`, custom Qt-free harness, 22 passing tests.
- Verified command:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Ibackend \
  tests/backend_tests.cpp backend/category.cpp backend/storage.cpp backend/ledger.cpp \
  -o /tmp/opencode/dailyaccount_backend_tests
/tmp/opencode/dailyaccount_backend_tests
```

- Verified result: `22 test(s) passed`.
- Available Linux tools: CMake 3.22.1 and GCC 11.4.0.
- Missing from the current Linux environment: `qmake` and `ninja`; Stage 0 must record Windows Qt build evidence instead of assuming it.
- No CMake, SQLite, QML, Kotlin, cloud migration, synchronization, or CI implementation currently exists.

## Execution Protocol

1. Read this master plan, `docs/product-architecture.md`, and the current stage plan before changing files.
2. Confirm the previous gate evidence exists under `docs/validation/` and contains no unresolved failure.
3. Run `git status --short` and preserve unrelated changes.
4. Mark only one task in progress in the execution task list.
5. Run the task's failing test before implementation and confirm the failure reason matches the missing behavior.
6. Implement only the task's declared outputs.
7. Run focused tests, the current stage cumulative suite, and `git diff --check`.
8. Request a fresh code review before declaring the task complete.
9. If commits are authorized, stage only the task's files and create the listed checkpoint commit. Otherwise leave the verified changes uncommitted.
10. Record gate evidence with command, environment, result, and artifact hashes before advancing stages.

## Planned File Structure

The paths below are authoritative for this plan suite. Existing `backend/` and `gui/` files remain in place until their migration tasks explicitly retire or adapt them.

```text
CMakeLists.txt
CMakePresets.json
cmake/
  DailyAccountOptions.cmake
  DailyAccountWarnings.cmake
build/
  build.bat
  build-android.bat
docs/
  decisions/
  validation/
  product-architecture.md
src/
  core/
    domain/
      result.h
      uuid.h
      date_time.h
      money.h
    application/
      accounting_error.h
      accounting_unit_of_work.h
  platform/
    interfaces/
      auth_client.h
      sync_transport.h
      secure_store.h
      notification_scheduler.h
      module_database_locator.h
    modules/
      platform_registry.h
      platform_registry.cpp
    profile/
      local_profile.h
      profile_store.h
      profile_store.cpp
    database/
      module_db_executor.h
      module_db_executor.cpp
    sync/
      sync_contract.h
      sync_coordinator.h
      sync_coordinator.cpp
    notifications/
      reminder_event.h
  modules/
    accounting/
      domain/
        transaction.h
        account.h
        category.h
        recurring.h
        import.h
        accounting_rules.cpp
      application/
        accounting_repositories.h
        accounting_service.h
        accounting_service.cpp
        recurring_service.h
        recurring_service.cpp
        import_service.h
        import_service.cpp
        analytics_service.h
        analytics_service.cpp
      data/
        sqlite/
          accounting_database.h
          accounting_database.cpp
          accounting_unit_of_work.h
          accounting_unit_of_work.cpp
          migrations/
            001_initial.sql
            002_recurring.sql
            003_sync.sql
            004_account_workflows.sql
            005_import.sql
        legacy/
          dat_importer.h
          dat_importer.cpp
          migration_manifest.h
          migration_manifest.cpp
      sync/
        accounting_sync_codec.h
        accounting_sync_codec.cpp
      import/
        text_import_parser.h
        text_import_parser.cpp
      analytics/
        monthly_breakdown_query.h
        monthly_breakdown_query.cpp
  apps/
    android-qml/
      CMakeLists.txt
      main.cpp
      qml/
        Main.qml
        LoginPage.qml
        OverviewPage.qml
        QuickEntryPage.qml
        TransactionsPage.qml
        RecurringPage.qml
        AnalyticsPage.qml
        SettingsPage.qml
      bridge/
        android_notification_scheduler.h
        android_notification_scheduler.cpp
      android/src/main/java/local/dailyaccount/
        ReminderScheduler.kt
        ReminderReceiver.kt
        ReminderBootReceiver.kt
        SecureStoreBridge.kt
    desktop-widgets/
      desktop_controller.h
      desktop_controller.cpp
      recurring_page.h
      recurring_page.cpp
      import_page.h
      import_page.cpp
cloud/
  supabase/
    config.toml
    migrations/
    tests/
tests/
  support/
    test_harness.h
    temporary_directory.h
  unit/
  integration/
  sync/
  android/
  fixtures/
```

If Stage 0 rejects Supabase, do not create `cloud/supabase/`. Record the selected provider and create a replacement Stage 4 provider-adapter plan with concrete paths before continuing.

## Shared C++ Contract

All stage plans use the following names. Changing one requires updating this master plan and every unexecuted child plan in the same review.

### Primitive Types

Create these under `src/core/domain/` during Stage 1:

```cpp
namespace dailyaccount {

template <typename T>
class Result;

template <typename Tag>
class StrongUuid {
public:
    static Result<StrongUuid> parse(std::string_view text);
    static StrongUuid random();
    static StrongUuid v5(const StrongUuid& namespaceId, std::string_view name);
    std::string toString() const;
    friend bool operator==(const StrongUuid& left, const StrongUuid& right);
    friend bool operator!=(const StrongUuid& left, const StrongUuid& right);
};

using UserId = StrongUuid<struct UserIdTag>;
using ProfileId = StrongUuid<struct ProfileIdTag>;
using DeviceId = StrongUuid<struct DeviceIdTag>;
using TransactionId = StrongUuid<struct TransactionIdTag>;
using AccountId = StrongUuid<struct AccountIdTag>;
using CategoryId = StrongUuid<struct CategoryIdTag>;
using TagId = StrongUuid<struct TagIdTag>;
using RecurringRuleId = StrongUuid<struct RecurringRuleIdTag>;
using RecurringOccurrenceId = StrongUuid<struct RecurringOccurrenceIdTag>;
using ImportBatchId = StrongUuid<struct ImportBatchIdTag>;
using ImportItemId = StrongUuid<struct ImportItemIdTag>;
using MutationId = StrongUuid<struct MutationIdTag>;

using MoneyMinor = std::int64_t;

struct CurrencyCode {
    std::array<char, 3> value;
};

struct LocalDate {
    std::int32_t year;
    std::uint8_t month;
    std::uint8_t day;
};

struct LocalTime {
    std::uint8_t hour;
    std::uint8_t minute;
    std::uint8_t second;
};

struct UtcInstant {
    std::int64_t epochMilliseconds;
};

}  // namespace dailyaccount
```

### Result and Errors

```cpp
enum class AccountingErrorCode {
    InvalidArgument,
    NotFound,
    DomainConstraint,
    OccurrenceLinked,
    RefundLimitExceeded,
    DuplicateImport,
    StorageFailure,
    AuthenticationRequiredForSync,
    RevisionConflict,
    CursorExpired,
    StaleEpoch,
    UpgradeRequired
};

struct AccountingError {
    AccountingErrorCode code;
    std::string message;
};

template <typename T>
class Result {
public:
    static Result success(T value);
    static Result failure(AccountingError error);
    bool hasValue() const;
    const T& value() const;
    const AccountingError& error() const;
};

template <>
class Result<void> {
public:
    static Result success();
    static Result failure(AccountingError error);
    bool hasValue() const;
    const AccountingError& error() const;
};
```

### Synchronizable Metadata

```cpp
struct EntityMetadata {
    UserId userId;
    UtcInstant createdAt;
    UtcInstant updatedAt;
    std::uint64_t serverRevision;  // 0 until the server accepts the entity.
    std::optional<UtcInstant> deletedAt;
    DeviceId modifiedByDeviceId;
};
```

Local dirty and in-flight state belongs to SQLite/outbox rows and is never serialized as `serverRevision`.

### Unit Of Work

```cpp
class ITransactionRepository;
class IAccountRepository;
class ICategoryRepository;
class ITagRepository;
class IRecurringRepository;
class IImportRepository;
class IOutboxRepository;

struct AccountingRepositories {
    ITransactionRepository& transactions;
    IAccountRepository& accounts;
    ICategoryRepository& categories;
    ITagRepository& tags;
    IRecurringRepository& recurring;
    IImportRepository& imports;
    IOutboxRepository& outbox;
};

class IAccountingUnitOfWork {
public:
    virtual ~IAccountingUnitOfWork() = default;
    virtual Result<void> execute(
        const std::function<Result<void>(AccountingRepositories&)>& operation) = 0;
};
```

Repositories expose row-level operations only inside `execute`. Application services own validation and aggregate transitions.

## Stable Domain Contracts

### Transaction

```cpp
enum class TransactionType { Expense, Income, Transfer, Refund };
enum class TransactionStatus { Pending, Posted };
enum class TransactionOrigin { Manual, TextImport, BillImport, Recurring };

struct Transaction {
    TransactionId id;
    EntityMetadata metadata;
    TransactionType type;
    TransactionStatus status;
    MoneyMinor amountMinor;
    CurrencyCode currency;
    LocalDate occurredOn;
    std::optional<LocalTime> occurredTime;
    std::optional<std::string> timeZoneId;
    std::optional<CategoryId> categoryId;
    std::optional<AccountId> accountId;
    std::optional<AccountId> destinationAccountId;
    std::optional<std::string> merchant;
    std::optional<std::string> note;
    TransactionOrigin origin;
    std::optional<std::string> originRef;
    std::optional<TransactionId> refundOfId;
};
```

### Account, Category, And Tag

```cpp
enum class AccountType { Cash, BankCard, ElectronicWallet, Credit, Other };
enum class CategoryApplicability { Income, Expense, Both };

struct Account {
    AccountId id;
    EntityMetadata metadata;
    std::string name;
    AccountType type;
    CurrencyCode currency;
    MoneyMinor openingBalanceMinor;
    LocalDate openingBalanceOn;
    bool isArchived;
};

struct Category {
    CategoryId id;
    EntityMetadata metadata;
    std::string name;
    std::optional<CategoryId> parentId;
    CategoryApplicability appliesTo;
    std::int32_t sortOrder;
    std::string color;
    std::string icon;
    bool isPreset;
    bool isArchived;
};

struct Tag {
    TagId id;
    EntityMetadata metadata;
    std::string name;
    bool isArchived;
};
```

### Recurring Expenses

```cpp
enum class RecurringStatus { Pending, Posted, Skipped, Cancelled };
enum class ShortMonthPolicy { ClampToLastDay, MoveToNextMonth };

struct RecurringRule {
    RecurringRuleId id;
    EntityMetadata metadata;
    std::string name;
    std::optional<std::string> merchant;
    std::optional<CategoryId> categoryId;
    std::optional<AccountId> accountId;
    std::optional<std::string> note;
    std::optional<std::string> marker;
    MoneyMinor expectedAmountMinor;
    MoneyMinor toleranceMinor;
    CurrencyCode currency;
    std::string frequencySpec;
    LocalDate nextDueOn;
    std::string timeZoneId;
    std::uint8_t leadDays;  // V1 accepts only 1 or 2.
    LocalDate startsOn;
    std::optional<LocalDate> endsOn;
    ShortMonthPolicy shortMonthPolicy;
    bool enabled;
};

struct RecurringOccurrence {
    RecurringOccurrenceId id;
    EntityMetadata metadata;
    RecurringRuleId ruleId;
    std::string periodKey;
    RecurringStatus status;
    std::optional<LocalDate> deferredUntil;
    std::optional<TransactionId> transactionId;
    LocalDate expectedOn;
    MoneyMinor expectedAmountMinor;
    std::string ruleSnapshotJson;
};
```

`Pending` and `Posted` require a live `transactionId`. `Skipped` and `Cancelled` require no `transactionId`. Overdue and deferred are derived display states, not persisted enum values.

### Import And Provenance

```cpp
enum class ImportBatchStatus { Draft, Committed, Discarded };
enum class ImportConfidence { High, NeedsReview, Invalid };

struct ImportBatch;
struct ImportItemCandidate {
    std::optional<LocalDate> occurredOn;
    std::optional<MoneyMinor> amountMinor;
    std::optional<CurrencyCode> currency;
    std::optional<TransactionType> type;
    std::optional<CategoryId> categoryId;
    std::optional<AccountId> accountId;
    std::optional<std::string> merchant;
    std::optional<std::string> note;
};

struct ImportItem {
    ImportItemId id;
    ImportBatchId batchId;
    std::string rawText;
    std::string normalizedText;
    ImportItemCandidate candidate;
    ImportConfidence confidence;
    std::vector<AccountingError> errors;
    std::string fingerprint;
    std::optional<ImportItemCandidate> userCorrection;
    std::optional<TransactionId> transactionId;
    bool ignored;
    std::optional<std::string> overrideId;
};

struct ImportBatch {
    ImportBatchId id;
    std::uint32_t normalizationVersion;
    std::string batchHash;
    ImportBatchStatus status;
    UtcInstant createdAt;
};

struct RawPaymentSignal {
    std::string sourceType;
    std::string sourceEventKey;
    UtcInstant observedAt;
    std::string rawPayload;
    std::uint32_t sourceVersion;
};

class IImportParser {
public:
    virtual ~IImportParser() = default;
    virtual Result<std::vector<ImportItem>> parse(
        ImportBatchId batchId,
        std::string_view normalizedInput,
        std::uint32_t normalizationVersion) const = 0;
};

class IPaymentSignalSource {
public:
    virtual ~IPaymentSignalSource() = default;
    virtual Result<std::vector<RawPaymentSignal>> collect() = 0;
};
```

`IPaymentSignalSource` remains an unimplemented extension boundary in V1. V1 implements only typed text/TXT import through `IImportParser`.

## Stable Platform Contracts

```cpp
struct SignInRequest {
    std::string login;
    std::string password;
};

struct RefreshRequest {
    std::string refreshToken;
};

struct AuthSession {
    UserId userId;
    std::string accessToken;
    std::string refreshToken;
    UtcInstant expiresAt;
};

struct RevisionExpectation {
    std::string entityType;
    std::string entityId;
    std::uint64_t baseServerRevision;
};

struct MutationEnvelope {
    MutationId mutationId;
    std::string commandType;
    std::uint32_t payloadVersion;
    std::vector<RevisionExpectation> expectations;
    std::string payloadJson;
};

struct PushRequest {
    std::string moduleId;
    std::string streamEpoch;
    DeviceId deviceId;
    std::vector<MutationEnvelope> mutations;
};

enum class MutationResultStatus {
    Accepted,
    Conflict,
    Rejected,
    StaleEpoch,
    UpgradeRequired
};

struct PushResponse {
    std::string responseJson;
};

struct PullRequest {
    std::string moduleId;
    std::string streamEpoch;
    std::string cursor;
    std::uint32_t limit;
};

struct PullResponse {
    std::string responseJson;
};

struct BootstrapRequest {
    std::string moduleId;
    std::optional<std::string> snapshotToken;
    std::optional<std::string> pageToken;
    std::uint32_t limit;
};

struct BootstrapPage {
    std::string responseJson;
};

class IAuthClient {
public:
    virtual ~IAuthClient() = default;
    virtual Result<AuthSession> signIn(const SignInRequest& request) = 0;
    virtual Result<AuthSession> refresh(const RefreshRequest& request) = 0;
    virtual Result<void> signOut() = 0;
};

class ISyncTransport {
public:
    virtual ~ISyncTransport() = default;
    virtual Result<PushResponse> push(const PushRequest& request) = 0;
    virtual Result<PullResponse> pull(const PullRequest& request) = 0;
    virtual Result<BootstrapPage> bootstrap(const BootstrapRequest& request) = 0;
};

class ISecureStore {
public:
    virtual ~ISecureStore() = default;
    virtual Result<void> put(
        std::string_view key,
        const std::vector<std::byte>& value) = 0;
    virtual Result<std::vector<std::byte>> get(std::string_view key) = 0;
    virtual Result<void> remove(std::string_view key) = 0;
};

struct ReminderEvent {
    std::string eventKey;
    RecurringRuleId ruleId;
    std::string periodKey;
    std::int8_t offsetDays;
    LocalDate targetLocalDate;
    std::string timeZoneId;
    std::string displaySnapshotJson;
};

enum class NotificationHealthCode {
    Ready,
    PermissionRequired,
    SchedulingRestricted,
    ForceStoppedUntilReopen
};

struct NotificationHealth {
    NotificationHealthCode code;
    std::string userMessage;
};

class INotificationScheduler {
public:
    virtual ~INotificationScheduler() = default;
    virtual Result<void> replaceEvents(
        RecurringRuleId ruleId,
        const std::vector<ReminderEvent>& events) = 0;
    virtual Result<void> cancelPeriod(
        RecurringRuleId ruleId,
        std::string_view periodKey) = 0;
    virtual NotificationHealth health() const = 0;
};
```

Stage 3 may use Qt containers only in the Android adapter layer. Shared contracts remain standard C++.

## Gate Definitions

### G0: Baseline And Prototype Gate

- Current backend tests pass with strict warnings and ASan/UBSan.
- A Windows Qt 6.9.3 package builds and starts from a frozen revision.
- All Stage 0 ADRs contain one selected outcome and measured evidence.
- Cloud, Android toolchain, QSQLITE threading/backup, and native reminder prototypes have explicit pass/fail records.
- A prototype failure either selects the documented fallback or stops the dependent plan. No later task may guess around it.

### G1: Build And Boundary Gate

- Linux core and Windows desktop configure, build, and test through CMake.
- Existing 22 backend regressions still pass.
- Domain targets have no Qt UI/SQL/Network dependencies.
- Registry tests reject duplicate IDs, missing dependencies, cycles, and platform UI leakage.
- `jizhang.pro` and `tests/backend_tests.pro` are retired only after equivalent CMake commands and packaging pass.

### G2: SQLite And Migration Gate

- SQLite schema, forward migrations, foreign keys, constraints, and per-profile thread ownership pass integration tests.
- Every business write commits its outbox mutation in the same transaction.
- DAT-to-SQLite fixtures preserve record count, total income, total expense, dates, and categories.
- Migration failure leaves source DAT and the last usable application untouched.
- Windows uses SQLite for all new writes and no longer creates DAT snapshots.

### G3: Android Offline Gate

- The APK installs and opens on every Stage 0 target device/API.
- Flight-mode transaction CRUD and restart persistence pass.
- QML sends money as decimal text and round-trips values beyond 32-bit integer range without loss.
- A recurring item appears in the list and home card, confirms locally, and posts exactly one transaction.
- A native reminder survives normal reboot and is delivered on the target local date.

### G4: Authentication And Sync Gate

- Two pre-created users cannot read or mutate each other's rows through any exposed API/RPC.
- Same-user Windows and Android converge after offline create/edit/delete.
- Duplicate requests, lost acknowledgements, concurrent edits, atomic change-group conflicts, cursor expiry, epoch changes, and bootstrap interruption lose no accepted data.
- Sync status is understandable and sync failure never blocks local accounting.

### G5: Accounting Workflow Gate

- Simple/account modes share one model and do not copy transactions.
- Accounts, transfers, refunds, category hierarchy, tags, recurring lifecycle, reminders, text import, and monthly analysis meet the detailed acceptance matrix in the Stage 5 plan.
- Recurring confirmation, cancellation, import matching, and refund limits remain atomic locally and remotely.
- Import raw text remains local and no candidate posts without explicit confirmation.

### G6: Release Gate

- Versioned JSON and CSV exports pass round trips and malicious-input tests.
- Unbound restore, recovery-profile viewing, and synchronized-profile merge restoration are rehearsed.
- Exit, local-copy deletion, and cloud-account deletion remain distinct operations with separate confirmations; the legacy physical whole-ledger clear entry is removed and V1 exposes no profile- or ledger-wide clearing surface (spec section 16.4 and decision D-019).
- Windows and Android installers pass clean-device smoke tests.
- Offline, weak-network, expired-token, corrupt-database, and cloud-outage drills pass.
- Security scans find no embedded service secret or sensitive transaction/import content in logs.
- Every `docs/product-architecture.md` section 3.5 and section 21 item has linked evidence.

## Migration And Rollback Checkpoints

| Checkpoint | Atomic activation | Required rollback behavior |
| --- | --- | --- |
| CP-00 baseline | Frozen revision and evidence manifest | Stop all migration work if the baseline cannot be reproduced |
| CP-01 qmake to CMake | CMake-built Windows package passes parity smoke tests | Return to the frozen qmake revision; do not maintain both indefinitely |
| CP-02 SQLite schema | One SQLite migration transaction updates schema and version | Failed migration leaves the prior database byte-for-byte usable |
| CP-03 DAT import | Validated staging SQLite database is atomically activated | Delete failed staging only; never modify source DAT |
| CP-04 Windows storage switch | First successful SQLite business write | Never silently reopen DAT after this point |
| CP-05 cloud schema | Expand-compatible server migration is deployed | Roll back API code while retaining additive schema until clients retire |
| CP-06 bootstrap | Fully validated staging database replaces live profile atomically | Resume or discard staging; never partially replace live data/outbox |
| CP-07 local restore | Validated unbound staging profile replaces target atomically | Keep current profile if validation or activation fails |
| CP-08 cloud deletion | Server state enters persistent `DELETING` workflow | Retry recorded steps; do not pretend an offline device was remotely erased |

## Acceptance Traceability

| Product result | Owning plan | Evidence gate |
| --- | --- | --- |
| Android offline quick entry | Stage 3 | G3 |
| Windows complete transaction management | Stages 2 and 5 | G2/G5 |
| Safe Windows/Android convergence | Stage 4 | G4 |
| Monthly category structure analysis | Stage 5 | G5 |
| Recurring list, reminders, overdue handling, one-click posting | Stages 3 and 5 | G3/G5 |
| Typed-text import with preview and deduplication | Stage 5 | G5 |
| Export, backup, and recovery during cloud outage | Stage 6 | G6 |
| Two to three isolated pre-created accounts | Stage 4 | G4 |
| Payment-signal extension boundary without credentials or automatic posting | Stages 1 and 5 | G1/G5 |

## Completion Rule

This master plan is complete only when all seven child plans exist, contain no placeholder work, pass a fresh-reader consistency review, and map every first-version requirement to a task and command. Product implementation is complete only when G0 through G6 evidence exists and the release checklist in the Stage 6 plan is fully checked.
