# DailyAccount Stage 5 First-Version Accounting Workflows Implementation Plan
> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
**Goal:** Close the first-version accounting loop by completing account mode with balance and reconcile workflows, atomic transfer and refund rules, category and tag workflows, the full recurring rule and occurrence lifecycle including the D-031 undo-confirmation decision, 90-day reminder planning on both clients, typed-text import with preview, matching and atomic privacy-respecting commit, monthly category-structure analytics with drill-down, the production workflow surfaces on Windows and Android, and negative payment-signal boundary proof.
**Architecture:** Enter only from accepted G4 and the accepted D-020 through D-030 outcomes. Extend the shared Stage 1-3 application services and query models behind existing controllers, facades, and view models; keep every outbox-bearing write inside one `accounting.sqlite` transaction; add cloud enforcement for transfer/refund, occurrence-lifecycle, and import-commit aggregates as reviewed command validations behind the existing three RPCs; keep import raw text local; keep mode switching presentation-only; and never implement an `IPaymentSignalSource` in V1.
**Tech Stack:** C++17, CMake 3.22.1+, Qt 6.9.3 Core/Widgets/QML/Quick/SQL/Network/Test, QSQLITE, Supabase CLI/PostgreSQL 15+/pgTAP, Kotlin 2.0.21 and Android API 28/35 per D-020/D-030, MinGW on Windows, GCC on Linux, Python 3, PowerShell, and the D-028/D-029 Android runtime and background decisions.
**Source contracts:** `docs/product-architecture.md` sections 9-13, 18, 20 (Stage 5), 21.1, 21.4, and 24; `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md` (G5 gate, Stable Contracts, Planned File Structure); `docs/superpowers/plans/2026-09-04-stage-0-baseline-and-prototypes.md` (D-024, D-025, D-027, D-030); `docs/superpowers/plans/2026-09-04-stage-2-sqlite-and-migration.md` (schema 2 repositories, commands, read models); `docs/superpowers/plans/2026-09-04-stage-3-android-offline.md` (reminder planner, native scheduling, recurring confirm slice); and `docs/superpowers/plans/2026-09-04-stage-4-auth-and-sync.md` (G4 boundary and handoff).
## Execution Rules
- Run tasks in order. Do not start Task 1 until the entry gate prints its exact PASS line, and do not start Task N+1 until Task N's green run and `git diff --check` pass.
- Accepted D-020 through D-030 and the D-031 produced by Task 4 override examples here. A contradiction stops execution for review rather than silently changing an accepted contract.
- Keep Stage 1 signatures, provider seams, immutable profile subject binding, schema/protocol/payload version independence, local business-plus-outbox atomicity, whole-group apply/quarantine/resolution, bootstraps, and per-module sync status unchanged from Stage 4.
- Do not edit `001_initial.sql`, `002_recurring.sql`, or `003_sync.sql`. Stage 5 adds forward migrations `004_account_workflows.sql` (Task 1) and `005_import.sql` (Task 6) with staging, rollback, migration-runner registry, and reader-version updates; module schema version is 4 after Task 1 and 5 after Task 6.
- Local commands still commit business state, unchanged confirmed `serverRevision`, local dirty state, and one stable outbox mutation in one SQLite transaction. Balance snapshots, module settings, and import batch/item raw text are device-local and never enter outbox payloads, sync, cloud rows, or JSON export.
- Transfer, refund, occurrence-lifecycle, and import-commit aggregate guarantees hold locally in one transaction and remotely in one reviewed command validation inside the existing `da_sync_push` RPC; no second online write path is added.
- Keep every financial amount as `MoneyMinor` in shared C++ and SQLite and as decimal text across QML; percentages and basis points are computed in C++ integer arithmetic per D-024, and charts receive non-monetary ratios only.
- The reminder contract keeps its D0-through-D0+90 inclusive natural-day window, stable `<rule UUID>:<periodKey>:<offsetDays>` keys, offsets `-2/-1/0`, and D-030 delivery. Android reminders never degrade to foreground compensation; Windows gains the same shared planner and reconciler but no V1 system-level notification claim.
- Text import obeys D-027 normalization version 1, byte/line/item/field limits, confidence rules, and fixture families. Complete raw text and full original item text stay local, are excluded from logs/sync/cloud/export, and are retained through confirmation unless the user selected post-commit clearing; payment signals in V1 are limited to typed text and TXT through `IImportParser`, and no `IPaymentSignalSource` implementation, bill importer, credential capture, provider login/scraping, automatic `POSTED`, or second online write path is created in this stage.
- Simple and account modes share one transaction model; switching modes copies nothing and migrates nothing, and the analysis口径 never depends on the UI mode.
- Every behavior-changing task follows red-green-refactor: add the focused failing test, observe the stated failure, implement the smallest slice, then run focused and cumulative verification and `git diff --check`.
- Checkpoint commits are optional and may run only after explicit authorization in the implementation session. This planning change grants no commit authorization.
- Preserve unrelated worktree changes and generated-artifact exclusions. Never stash, reset, clean, stage, or commit unrelated paths; never commit credentials, private import samples, raw text, databases, APKs, or build trees.
## Stage 5 Entry Gate
- [ ] **Verify G0-G4 evidence, the G4 checker, and accepted decisions exist**
Run from the repository root:

```bash
test -f docs/validation/stage-4/g4-results.json
test -f docs/validation/stage-4/g4-evidence-index.md
test -f tests/cmake/check_g4.py
test -f docs/validation/stage-0/import-fixtures.md
test -f docs/validation/stage-0/import-fixtures.sha256
test -f docs/validation/stage-3/g3-results.json
for number in 020 021 022 023 024 025 026 027 028 029 030; do
  matches=(docs/decisions/D-${number}-*.md); test "${#matches[@]}" -eq 1; test -f "${matches[0]}"
done
matches=(docs/decisions/D-031-*.md); test "${#matches[@]}" -eq 0
```
Expected: every command exits `0`, each accepted ADR resolves to exactly one file, and D-031 does not yet exist.
- [ ] **Re-run the authoritative G4 checker without replacing accepted evidence**

```bash
python3 tests/cmake/check_g4.py --root . \
  --json /tmp/opencode/dailyaccount-stage5-g4-recheck.json
```
Expected stdout exactly: `G4 PASS: Supabase auth isolation, atomic sync, bootstrap, and two-device convergence`.
- [ ] **Compare accepted and fresh G4 records and confirm the inheritance surface**

```bash
python3 - <<'PY'
import json
from pathlib import Path
expected = {
    "gate": "G4", "result": "PASS", "provider": "Supabase",
    "localSchemaVersion": 3, "protocolVersion": 1, "payloadVersion": 1,
    "wholeGroupConflict": True, "twoDeviceConvergence": True,
    "offlineCrudPreserved": True, "failureCount": 0,
}
records = [
    json.loads(Path("docs/validation/stage-4/g4-results.json").read_text(encoding="utf-8")),
    json.loads(Path("/tmp/opencode/dailyaccount-stage5-g4-recheck.json").read_text(encoding="utf-8")),
]
for record in records:
    for key, value in expected.items():
        assert record[key] == value, (key, record[key])
for path in (
    "src/core/application/accounting_unit_of_work.h",
    "src/platform/interfaces/notification_scheduler.h",
    "src/modules/accounting/application/recurring_service.h",
    "src/modules/accounting/application/reminder_planner.h",
    "src/modules/accounting/application/accounting_query_service.h",
    "src/modules/accounting/sync/accounting_sync_codec.h",
):
    assert Path(path).exists(), path
for absent in (
    "src/modules/accounting/import/text_import_parser.h",
    "src/modules/accounting/application/import_service.h",
    "src/modules/accounting/analytics/monthly_breakdown_query.h",
):
    assert not Path(absent).exists(), absent
print("Stage 5 entry gate: PASS (G4, D-020 through D-030)")
PY
```
Expected: exactly `Stage 5 entry gate: PASS (G4, D-020 through D-030)`.
- [ ] **Confirm stage-5 prerequisites inside accepted decisions**
Read the accepted D-024 account formulas, D-025 recurrence and undo semantics, D-027 normalization and limits, D-030 alarm delivery, and the D-026 whole-group conflict and resolution rules. Record the single selected outcome of each ADR in the execution notes. If `docs/product-architecture.md` section 23 item 13 is still open, Task 4 must close it through D-031 before implementing undo; nothing else in this plan guesses around an open item.
## Inherited Public Boundary
`ISyncTransport` (push/pull/bootstrap), `IAuthClient`, and `INotificationScheduler::{replaceEvents,cancelPeriod,health}` remain unchanged. `IAccountingUnitOfWork::execute`, the repository/UoW row-level contract, the deterministic recurring identity formulas, the codec command set through `CONFIRM_RECURRING_OCCURRENCE`, schema version 3 rows, outbox freeze/rebase semantics, and the Stage 4 server retention/epoch/cursor behavior are the inputs Stage 5 workflows must use. New stage commands extend the codec and the reviewed server command validator; they never bypass outbox or `serverRevision`.
---
### Task 1: Add Account Mode Settings, Book Balances, Reconcile Snapshots, and Backfill Queries
**Files:**
- Create: `src/modules/accounting/data/sqlite/migrations/004_account_workflows.sql`
- Create: `src/modules/accounting/application/accounting_settings.h`
- Create: `src/modules/accounting/data/sqlite/sqlite_settings_store.h`
- Create: `src/modules/accounting/data/sqlite/sqlite_settings_store.cpp`
- Create: `src/modules/accounting/application/account_workflow_service.h`
- Create: `src/modules/accounting/application/account_workflow_service.cpp`
- Create: `tests/unit/account_workflow_tests.cpp`
- Create: `tests/integration/account_balance_sqlite_tests.cpp`
- Modify: `src/modules/accounting/application/accounting_query_service.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.cpp`
- Modify: `src/modules/accounting/data/sqlite/accounting_migration_runner.h`
- Modify: `src/modules/accounting/data/sqlite/accounting_migration_runner.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**

```cpp
enum class AccountingMode { Simple, Account };
class IAccountingSettingsStore {
public:
    virtual ~IAccountingSettingsStore() = default;
    virtual Result<AccountingMode> mode() = 0;
    virtual Result<void> setMode(AccountingMode) = 0;
    virtual Result<std::optional<AccountId>> defaultAccountId() = 0;
    virtual Result<void> setDefaultAccountId(const std::optional<AccountId>&) = 0;
    virtual Result<bool> clearRawTextAfterCommit() = 0;
    virtual Result<void> setClearRawTextAfterCommit(bool) = 0;
};
struct SaveBalanceSnapshotCommand {
    AccountId accountId; LocalDate snapshottedOn; MoneyMinor actualBalanceMinor;
    std::optional<std::string> note; DeviceId modifiedByDeviceId;
};
struct AccountBalanceRow {
    AccountId id; std::string name; AccountType type; bool isArchived;
    MoneyMinor openingBalanceMinor, netChangeMinor, bookBalanceMinor;
    std::optional<MoneyMinor> actualBalanceMinor, differenceMinor;
};
class AccountWorkflowService final {
public:
    Result<MoneyMinor> bookBalance(AccountId, LocalDate asOf);
    Result<void> saveBalanceSnapshot(const SaveBalanceSnapshotCommand&);
    Result<std::optional<MoneyMinor>> actualBalance(AccountId);
};
```
- `004_account_workflows.sql` adds device-local `module_settings(key TEXT PRIMARY KEY CHECK(key IN ('accounting_mode','default_account_id','clear_raw_text_after_commit')), value TEXT NOT NULL CHECK(length(CAST(value AS BLOB)) <= 512), updated_at_ms INTEGER NOT NULL)` and `account_balance_snapshots(account_id TEXT PRIMARY KEY REFERENCES accounts(id) ON DELETE RESTRICT, snapshotted_on TEXT NOT NULL CHECK(length(snapshotted_on) = 10), actual_balance_minor INTEGER NOT NULL, note TEXT, created_at_ms INTEGER NOT NULL, updated_at_ms INTEGER NOT NULL)` as `STRICT`, plus a delete guard trigger raising `DA_REFUND_LINKED` when a transaction with live refunds is tombstoned. Both tables carry no sync metadata and never enter the outbox.
- Register migration 004 in the runner so a fresh database reaches schema version 4 while 001-003 hashes stay unchanged; Task 6 raises the version to 5 with `005_import.sql`. `accounting_mode` stores canonical JSON `"Simple"` or `"Account"`; `default_account_id` is a canonical UUID string or `null` referencing a live non-archived account; the raw-text key stores `true` or `false`.
- Book balance per account is computed from live `POSTED` transactions at or before `asOf` with the D-024 formula applied verbatim: opening balance plus income and refund credits, minus expense and transfer-source debits, plus transfer-destination credits; `differenceMinor = actual - book`.
- Add to `IAccountingQueryService`: `accountBalances(LocalDate)`, `unassignedTransactions()`, and `listAccounts()`; every SQL statement is bound and filters `deleted_at_ms IS NULL`. Mode changes never copy, migrate, or re-validate transaction rows.
- [ ] **Write the failing tests**

```cpp
void savedSnapshotOverwritesPerAccountAndComputesDifference()
{
    AccountBalanceFixture fixture;
    fixture.seedExpense("2026-09-02", 2500, fixture.accountA());
    DA_CHECK(fixture.service().bookBalance(fixture.accountA(),
        LocalDate{2026, 9, 4}).value() == MoneyMinor{-2500});
    const SaveBalanceSnapshotCommand command{
        fixture.accountA(), LocalDate{2026, 9, 4}, MoneyMinor{-3000},
        std::nullopt, fixture.deviceId()};
    DA_CHECK(fixture.service().saveBalanceSnapshot(command).hasValue());
    const auto row = fixture.rows().value().at(0);
    DA_CHECK(row.actualBalanceMinor.value() == MoneyMinor{-3000});
    DA_CHECK(row.differenceMinor.value() == MoneyMinor{-500});
}
```
Also test mode round-trip with a `Simple`-created transaction remaining byte-identical after switching to `Account`, default-account set/clear validation, opening balance plus transfer/refund arithmetic on both legs, snapshot last-wins and rollback, and that no snapshot or settings write ever produces an outbox mutation.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_account_workflow_tests dailyaccount_account_balance_sqlite_tests --parallel 2
```
Expected red: unknown targets or missing `account_workflow_service.h`/migration 004.
- [ ] **Implement the minimum slice**
Implement the settings store and snapshot service on the module serial executor in plain transactions, the single-query balance aggregation, and the three query methods. Wire mode and default-account reads into desktop/mobile settings surfaces only at Task 8; Task 1 verifies the service contract.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_account_workflow_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "account_balance|accounting_query|migration" --output-on-failure
```
Expected green: mode/snapshot/balance arithmetic passes on Linux and SQLite, schema 2-3 databases migrate losslessly to 4, outbox count stays zero for snapshot/settings writes, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/modules/accounting/data/sqlite/migrations/004_account_workflows.sql \
  src/modules/accounting/application/accounting_settings.h \
  src/modules/accounting/application/account_workflow_service.h \
  src/modules/accounting/application/account_workflow_service.cpp \
  src/modules/accounting/data/sqlite/sqlite_settings_store.h \
  src/modules/accounting/data/sqlite/sqlite_settings_store.cpp \
  src/modules/accounting/application/accounting_query_service.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_query_service.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_query_service.cpp \
  src/modules/accounting/data/sqlite/accounting_migration_runner.h \
  src/modules/accounting/data/sqlite/accounting_migration_runner.cpp \
  tests/unit/account_workflow_tests.cpp tests/integration/account_balance_sqlite_tests.cpp
git commit -m "feat: add account mode balance and reconcile workflows"
```
---
### Task 2: Enforce Atomic Transfer and Refund Rules Locally and Remotely
**Files:**
- Create: `cloud/supabase/migrations/20260904050000_stage5_transfer_refund_guards.sql`
- Create: `cloud/supabase/tests/database/0005_stage5_transfer_refund.test.sql`
- Create: `tests/unit/transfer_refund_rules_tests.cpp`
- Create: `tests/integration/transfer_refund_sqlite_tests.cpp`
- Modify: `src/modules/accounting/application/accounting_service.h`
- Modify: `src/modules/accounting/application/accounting_service.cpp`
- Modify: `src/modules/accounting/application/accounting_repositories.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_unit_of_work.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_unit_of_work.cpp`
- Modify: `src/modules/accounting/sync/accounting_sync_codec.h`
- Modify: `src/modules/accounting/sync/accounting_sync_codec.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**

```cpp
// ITransactionRepository gains aggregate reads used inside UoW operations only.
virtual Result<std::vector<Transaction>> findLiveRefundsOf(
    const TransactionId& refundOfId) = 0;
virtual Result<std::vector<Transaction>> findLiveByAccount(
    const AccountId& accountId) = 0;
```
- `createTransaction`/`updateTransaction` for `TRANSFER` and `REFUND` build the full D-024 `TransactionValidationContext` inside one UoW: live source/destination accounts, live posted original expense, and the cumulative live posted refund sum including the submitted value; validation failure returns `DomainConstraint` or `RefundLimitExceeded` and writes no row and no outbox.
- A `TRANSFER` never enters income or expense aggregates and requires two distinct live non-archived CNY accounts, a null category, and posted status. A `REFUND` inherits the original expense's category, adds only to its own account, is posted, and keeps the cumulative live refund sum at or below the original expense amount.
- Deleting or tombstoning the original expense while any live refund exists returns `DomainConstraint` with message `linked refunds must be removed first` and writes nothing; the migration-004 `DA_REFUND_LINKED` trigger is the storage-level backstop. A refund keeps `refundOfId` pointing at a live posted expense in the same currency.
- The cloud migration extends the `da_sync_push` validator for `UPSERT_TRANSACTION` and `DELETE_TRANSACTION`: under the same stream/entity lock it loads the refunded expense and sums live refunds before accepting a refund, rejects refund-original deletion while refunds live, and rejects transfer rows whose accounts are missing, archived, equal, or non-CNY. Rejection writes no entity, cursor, group, or idempotency row.
- [ ] **Write the failing tests**

```cpp
void refundOverOriginalLimitFailsAndWritesNothing()
{
    TransferRefundFixture fixture;
    fixture.seedPostedExpense(10000);              // 100.00
    DA_CHECK(fixture.postRefund(6000).hasValue());
    const auto second = fixture.postRefund(6000);  // 120.00 > 100.00
    DA_CHECK(!second.hasValue());
    DA_CHECK_EQ(second.error().code, AccountingErrorCode::RefundLimitExceeded);
    DA_CHECK_EQ(fixture.liveRefundCount(), 1);
}
```
Also test valid refund after edit, refund category inheritance, delete-original rejection with a live refund, transfer both-leg balance arithmetic, transfer edit/delete validation, legacy-name income never reinterpreted as transfer or refund, and account archive while transactions reference it.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_transfer_refund_rules_tests --parallel 2
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --target dailyaccount_transfer_refund_sqlite_tests --parallel 2
```
Expected red: repository aggregate reads and compiled service wiring do not exist.
- [ ] **Implement the minimum slice**
Populate the validation context in both command paths, add the repository aggregate reads and their SQLite implementations inside the existing UoW, and extend the codec so refund/transfer payloads validate aggregate references before canonical encoding. Add the Supabase guard migration and pgTAP file covering refund-over-limit, refund-original deletion, transfer shape, and cross-user isolation of every guard.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_transfer_refund_rules_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```bash
supabase start --workdir cloud && supabase db reset --workdir cloud && supabase test db --workdir cloud --file supabase/tests/database/0005_stage5_transfer_refund.test.sql && supabase test db --workdir cloud && supabase db lint --workdir cloud --level warning --fail-on error && supabase stop --workdir cloud
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "transfer_refund|sqlite_unit_of_work|codec" --output-on-failure
```
Expected green: local and remote refund-limit and transfer guards hold, atomic rollback leaves one live refund and one outbox row, whole-group pull and retention tests remain green, and DAT prints `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/modules/accounting/application/accounting_service.h \
  src/modules/accounting/application/accounting_service.cpp \
  src/modules/accounting/application/accounting_repositories.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_unit_of_work.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_unit_of_work.cpp \
  src/modules/accounting/sync/accounting_sync_codec.h \
  src/modules/accounting/sync/accounting_sync_codec.cpp \
  cloud/supabase/migrations/20260904050000_stage5_transfer_refund_guards.sql \
  cloud/supabase/tests/database/0005_stage5_transfer_refund.test.sql \
  tests/unit/transfer_refund_rules_tests.cpp \
  tests/integration/transfer_refund_sqlite_tests.cpp
git commit -m "feat: enforce atomic transfer and refund rules"
```
---
### Task 3: Complete Category and Tag Workflows for Transactions and Management
**Files:**
- Create: `tests/unit/category_tag_workflow_tests.cpp`
- Create: `tests/integration/category_tag_sqlite_tests.cpp`
- Modify: `src/modules/accounting/application/accounting_service.h`
- Modify: `src/modules/accounting/application/accounting_service.cpp`
- Modify: `src/modules/accounting/application/accounting_commands.h`
- Modify: `src/modules/accounting/application/accounting_query_service.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**

```cpp
struct AssignCategoryAndTagsCommand {
    TransactionId transactionId; std::optional<CategoryId> categoryId;
    std::vector<TagId> tagIds; DeviceId modifiedByDeviceId; MutationId mutationId;
};
struct CategoryUsageRow {
    CategoryId categoryId; std::string name; std::optional<CategoryId> parentId;
    std::uint64_t transactionCount;
};
struct TagUsageRow {
    TagId id; std::string name; bool isArchived; std::uint64_t transactionCount;
};
```
- `assignCategoryAndTags` loads the transaction, replaces its category and tag set in one UoW, and enforces: live transaction; category null or live, non-archived, with applicability covering the transaction type; all tags live and non-archived; and one `UPSERT_TRANSACTION` outbox row whose `tagIds` array equals the new set.
- Category archive keeps history and rejects only preset categories; used categories archive without rewriting transaction foreign keys. Children of an archived category stay visible in historical queries but never in assignment pickers. Tags follow archive-only semantics; pickers exclude archived rows.
- Add query methods `categoryUsage()` (per-category counts with descendants rolled into the parent row), `listTags()`, `transactionTags(TransactionId)`, and `uncategorizedTransactions()`; analytics and pickers read these rows so no second inventory exists.
- Parent-child enforcement keeps `validateCategory` behavior: a child's applicability must intersect the parent's and a category cannot parent itself; assignment never silently drops an invalid category to null.
- [ ] **Write the failing tests**

```cpp
void assignmentReplacesTagsAndRejectsArchivedCategory()
{
    CategoryTagFixture fixture;
    fixture.seedLiveCategories();
    fixture.archive(fixture.expenseCategory());
    DA_CHECK(!fixture.service().assignCategoryAndTags(
        AssignCategoryAndTagsCommand{fixture.expenseTransaction().id,
            fixture.expenseCategory().id, {fixture.tagA().id},
            fixture.deviceId(), MutationId::random()}).hasValue());
    fixture.restore(fixture.expenseCategory());
    DA_CHECK(fixture.service().assignCategoryAndTags(
        AssignCategoryAndTagsCommand{fixture.expenseTransaction().id,
            fixture.expenseCategory().id, {fixture.tagA().id},
            fixture.deviceId(), MutationId::random()}).hasValue());
    DA_CHECK_EQ(fixture.tagsOf(fixture.expenseTransaction().id).size(), std::size_t{1});
}
```
Also test income-category on an expense transaction rejection, child applicability intersection, rename preserving history, usage counts with descendants, tag replacement leaving no orphan rows, and archive filtering in picker queries.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_category_tag_workflow_tests --parallel 2
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --target dailyaccount_category_tag_sqlite_tests --parallel 2
```
Expected red: assignment command or query additions are absent.
- [ ] **Implement the minimum slice**
Add the command and service method, extend codec payload usage for tag replacement, and implement the four query methods in SQLite with bound parameters and tombstone filters. Preserve the existing `UPSERT_CATEGORY`/`UPSERT_TAG` behavior and preset protection from Stage 2.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_category_tag_workflow_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "category_tag|sqlite_unit_of_work|accounting_query" --output-on-failure
```
Expected green: assignment, archive filtering, and usage queries pass with no orphan tags or outbox duplication; DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/modules/accounting/application/accounting_commands.h \
  src/modules/accounting/application/accounting_service.h \
  src/modules/accounting/application/accounting_service.cpp \
  src/modules/accounting/application/accounting_query_service.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_query_service.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_query_service.cpp \
  tests/unit/category_tag_workflow_tests.cpp \
  tests/integration/category_tag_sqlite_tests.cpp
git commit -m "feat: complete category and tag workflows"
```
---
### Task 4: Add the Full Recurring Rule and Occurrence Lifecycle and Decide Undo Confirmation (D-031)
**Files:**
- Create: `docs/decisions/D-031-recurring-undo-confirm-interaction.md`
- Create: `src/modules/accounting/application/recurring_commands.h`
- Create: `tests/unit/recurring_lifecycle_tests.cpp`
- Create: `tests/integration/recurring_lifecycle_sqlite_tests.cpp`
- Create: `cloud/supabase/migrations/20260904060000_stage5_occurrence_lifecycle.sql`
- Create: `cloud/supabase/tests/database/0006_stage5_occurrence_lifecycle.test.sql`
- Modify: `docs/product-architecture.md`
- Modify: `src/modules/accounting/application/recurring_service.h`
- Modify: `src/modules/accounting/application/recurring_service.cpp`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.h`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.cpp`
- Modify: `src/modules/accounting/domain/accounting_rules.h`
- Modify: `src/modules/accounting/domain/accounting_rules.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**

```cpp
struct UpdateRecurringRuleCommand {
    RecurringRule rule; DeviceId modifiedByDeviceId; MutationId mutationId;
};
enum class ArchivePendingChoice { Keep, Skip, Cancel };
struct ArchiveRecurringRuleCommand {
    RecurringRuleId ruleId;
    std::map<RecurringOccurrenceId, ArchivePendingChoice> pendingChoices;
    DeviceId modifiedByDeviceId; MutationId mutationId;
};
struct DeferRecurringOccurrenceCommand {
    RecurringOccurrenceId occurrenceId; LocalDate deferredUntil;
    DeviceId modifiedByDeviceId; MutationId mutationId;
};
struct ResolveRecurringOccurrenceCommand {   // skip or cancel only
    RecurringOccurrenceId occurrenceId; RecurringStatus terminalStatus;
    DeviceId modifiedByDeviceId; MutationId mutationId;
};
struct UndoRecurringConfirmationCommand {
    RecurringOccurrenceId occurrenceId; bool detachImportMatch;
    DeviceId modifiedByDeviceId; MutationId mutationId;
};
class RecurringService final {
public:
    Result<RecurringRule> updateRule(const UpdateRecurringRuleCommand&);
    Result<void> archiveRule(const ArchiveRecurringRuleCommand&);
    Result<RecurringOccurrence> deferOccurrence(const DeferRecurringOccurrenceCommand&);
    Result<void> resolveOccurrence(const ResolveRecurringOccurrenceCommand&);
    Result<UndoResult> undoConfirmation(const UndoRecurringConfirmationCommand&);
};
```
- **Write D-031 before implementing undo.** D-031 has `Status: Accepted`, implements D-025 step 6 verbatim, and additionally freezes: undo is the only route from `POSTED` back to `PENDING`; with no linked import item it atomically restores the same transaction to `PENDING` preserving its ID and edited fields and restores the occurrence to `PENDING`; with a linked import item the UI requires the explicit `Detach import match and return to pending` choice, which clears `ImportItem.transactionId`, marks the matching provenance row with `detachedAt`, and preserves the import item and local raw text; posted occurrences cannot be skipped, cancelled, or generically deleted until a successful undo; undo is idempotent with a `changed` result; reminders replan after commit.
- `updateRule` affects only not-yet-generated periods; existing occurrences keep their snapshot JSON and edited values, and no past-period transaction, occurrence, or reminder event is silently changed. `archiveRule` stops generation and applies one of the three choices per pending occurrence atomically.
- `deferOccurrence` keeps `PENDING` and the same live transaction, sets `deferredUntil` only when later than the current effective date, and rejects otherwise. Skip and cancel tombstone the linked pending transaction, clear the occurrence `transactionId`, set the terminal status, and write one mutation over both entities.
- Reminder work (cancel old events, replace events per `deferredUntil` and rule lead, or drop the period) is composed after each committed lifecycle command; a scheduler failure never rolls back committed finance state.
- The cloud migration extends `da_sync_push` with the terminal, defer, undo, and archive aggregate shapes so two devices cannot produce a split pair, a duplicated terminal state, or a tombstone-less undo; pgTAP proves canonical race behavior.
- Update `docs/product-architecture.md`: replace section 23 item 13 with a link to D-031, append a D-031 row to section 24, and note in section 11.5 that undo confirmation follows D-031.
- [ ] **Write and validate D-031 before implementation**

```bash
cat > docs/decisions/D-031-recurring-undo-confirm-interaction.md <<'EOF'
# D-031 Recurring Confirmation Undo Interaction
Status: Accepted
Undo of a mistaken recurring confirmation follows D-025 step 6 exactly. Undo is the only route from POSTED back to PENDING. With no linked import item it restores the same transaction to PENDING preserving ID and edited fields, restores the occurrence to PENDING, and reschedules future events. With a linked import item the UI requires the explicit `Detach import match and return to pending` choice; that command clears ImportItem.transactionId, marks the provenance relation detachedAt, preserves the import item and local raw text, and restores the same occurrence/transaction IDs to pending. Posted occurrences cannot be skipped, cancelled, or generically deleted until this undo command succeeds. Undo is idempotent and returns changed; reminders replan after commit.
EOF
python3 -c "from pathlib import Path; p=Path('docs/decisions/D-031-recurring-undo-confirm-interaction.md').read_text(); required=['Status: Accepted','POSTED','PENDING','Detach import match and return to pending','detachedAt','ImportItem.transactionId','idempotent','changed']; assert all(x in p for x in required)"
```
Expected: exit code `0`, and the ADR is the only decision the undo implementation may follow.
- [ ] **Write the failing lifecycle tests**

```cpp
void undoAfterImportDetachRestoresPendingPairAndKeepsRawText()
{
    RecurringLifecycleFixture fixture;
    const auto pair = fixture.matchAndConfirm("2026-09");
    DA_CHECK(fixture.importItem().transactionId.has_value());
    DA_CHECK(fixture.service().undoConfirmation(
        UndoRecurringConfirmationCommand{pair.occurrence.id, true,
            fixture.deviceId(), MutationId::random()}).value().changed);
    DA_CHECK_EQ(fixture.occurrenceStatus(pair.occurrence.id), RecurringStatus::Pending);
    DA_CHECK_EQ(fixture.transactionStatus(pair.transaction.id), TransactionStatus::Pending);
    DA_CHECK(!fixture.importItem().transactionId.has_value());
    DA_CHECK(fixture.rawTextRetained());
}
```
Also test update-rule future-only semantics, archive with keep/skip/cancel choices, defer date validation, skip/cancel tombstoning the transaction and clearing the link, undo without import, duplicate undo retry, undo on a split pair writing nothing, and local SQLite atomicity on injected outbox failure.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_recurring_lifecycle_tests --parallel 2
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --target dailyaccount_recurring_lifecycle_sqlite_tests --parallel 2
```
Expected red: lifecycle commands and D-031 do not exist yet.
- [ ] **Implement the minimum slice**
Update the architecture file, then extend `accounting_rules` only for lifecycle transition checks and implement the five service commands in one-UoW aggregates with reviewed command types `DEFER_RECURRING_OCCURRENCE`, `SKIP_RECURRING_OCCURRENCE`, `CANCEL_RECURRING_OCCURRENCE`, `UNDO_RECURRING_CONFIRMATION`, and archive resolution folded into rule plus occurrence upserts. Apply the cloud lifecycle migration and pgTAP races.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_recurring_lifecycle_tests && ./build/cmake/linux-core/dailyaccount_recurring_confirmation_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```bash
supabase start --workdir cloud && supabase db reset --workdir cloud && supabase test db --workdir cloud --file supabase/tests/database/0006_stage5_occurrence_lifecycle.test.sql && supabase test db --workdir cloud && supabase stop --workdir cloud
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "recurring_lifecycle|recurring" --output-on-failure
```
Expected green: every D-025 transition vector passes, undo with and without import detach is atomic and idempotent, remote races return canonical pairs without orphans, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt docs/product-architecture.md \
  docs/decisions/D-031-recurring-undo-confirm-interaction.md \
  src/modules/accounting/application/recurring_commands.h \
  src/modules/accounting/application/recurring_service.h \
  src/modules/accounting/application/recurring_service.cpp \
  src/modules/accounting/application/accounting_mutation_codec.h \
  src/modules/accounting/application/accounting_mutation_codec.cpp \
  src/modules/accounting/domain/accounting_rules.h \
  src/modules/accounting/domain/accounting_rules.cpp \
  cloud/supabase/migrations/20260904060000_stage5_occurrence_lifecycle.sql \
  cloud/supabase/tests/database/0006_stage5_occurrence_lifecycle.test.sql \
  tests/unit/recurring_lifecycle_tests.cpp \
  tests/integration/recurring_lifecycle_sqlite_tests.cpp
git commit -m "feat: add recurring lifecycle and decide undo confirmation"
```
---
### Task 5: Drive the 90-Day Reminder Planner from Both Clients
**Files:**
- Create: `src/modules/accounting/application/reminder_reconciler.h`
- Create: `src/modules/accounting/application/reminder_reconciler.cpp`
- Create: `tests/unit/reminder_reconciler_tests.cpp`
- Create: `tests/integration/reminder_both_ends_tests.cpp`
- Modify: `src/apps/android-qml/mobile_recurring_facade.h`
- Modify: `src/apps/android-qml/mobile_recurring_facade.cpp`
- Modify: `src/apps/android-qml/mobile_composition.cpp`
- Modify: `src/apps/desktop-widgets/desktop_controller.h`
- Modify: `src/apps/desktop-widgets/desktop_controller.cpp`
- Modify: `src/apps/desktop-widgets/desktop_composition.cpp`
- Modify: `src/apps/desktop-widgets/accounting_view_models.h`
- Modify: `src/apps/desktop-widgets/accounting_view_models.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**

```cpp
struct ReminderReconcileContext {
    LocalDate d0; UtcInstant now; std::vector<RecurringRule> liveRules;
    std::vector<RecurringOccurrence> occurrences;
};
struct ReminderReconcileReport {
    std::size_t generatedOccurrences, plannedEvents, cancelledPeriods;
};
class ReminderReconciler final {
public:
    ReminderReconciler(
        IAccountingUnitOfWork& unitOfWork,
        IAccountingQueryService& queries,
        INotificationScheduler* scheduler);   // null on Windows in V1
    Result<ReminderReconcileReport> reconcile(const ReminderReconcileContext&);
    Result<ReminderReconcileReport> reconcileAfterSync(
        LocalDate d0, UtcInstant now, const std::vector<RecurringRuleId>& touchedRules);
};
```
- On every application launch, foreground resume, rule create/update/archive, lifecycle command, and successful sync pull, both clients run the shared reconciler: for each live enabled rule within start/end bounds, suppress periods whose occurrence is posted/skipped/cancelled, generate missing periods through the D-025 inclusive horizon with the 240-occurrence cap, derive the stable event set through `ReminderPlanner`, and call `replaceEvents` only after the SQLite commit. Android passes its native scheduler; Windows passes `nullptr` and only refreshes due-card projections, with no system-level notification claim.
- `reconcileAfterSync` replans only rules whose change groups touched occurrences or the rule, honoring D-025: another device's confirm/skip/cancel removes stale events and defer replaces them by the deferred date. A scheduler failure returns `REMINDER_RECONCILIATION_REQUIRED`, keeps the committed finance state, and lets the next launch/resume reconcile retry.
- Desktop view models expose `UpcomingReminderRow { ruleId, periodKey, title, amountText, targetLocalDate, offsetDays }` from the same planner output consumed by mobile, proving byte-identical golden event sets across ends.
- [ ] **Write the failing tests**

```cpp
void deferReplacesEventsOnAndroidAndRefreshesRowsOnWindows()
{
    ReminderBothEndsFixture fixture;
    fixture.seedRuleWithTwoPendingPeriods();
    fixture.deferSecondPeriod("2026-12-05");
    const auto report = fixture.reconcile(fixture.context("2026-11-30"));
    DA_CHECK_EQ(report.cancelledPeriods, std::size_t{1});
    DA_CHECK_EQ(fixture.upcomingRows().size(), std::size_t{3});
}
```
Also test identical golden event sets on Linux and Windows offscreen fixtures for D0+89/D0+90 inclusion and D0+91 exclusion after defer, catch-up after long absence with the 240 cap, post-sync replanning only for touched rules, and facade reuse of the reconciler instead of duplicated period logic.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_reminder_reconciler_tests --parallel 2
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --target dailyaccount_reminder_both_ends_tests --parallel 2
```
Expected red: reconciler target absent or mobile/desktop compositions still own duplicated logic.
- [ ] **Implement the minimum slice**
Implement the reconciler over the existing `ReminderPlanner`, generation service, and query service; rewire `MobileRecurringFacade::reconcile` and its startup/resume/sync hooks to delegate here; add the desktop controller invokable and due-card read model. Keep the D-030 native path untouched and the receiver boundary scan green.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_reminder_reconciler_tests && ./build/cmake/linux-core/dailyaccount_reminder_planner_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && & 'build\cmake\windows-desktop\dailyaccount_reminder_both_ends_tests.exe' && ctest --preset windows-desktop -R "reminder|recurring" --output-on-failure
```

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/android_boundary_contract.cmake
```
Expected green: golden D0 boundaries match across ends, lifecycle and sync replanning pass, Android native scheduling remains independent, and DAT prints `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/modules/accounting/application/reminder_reconciler.h \
  src/modules/accounting/application/reminder_reconciler.cpp \
  src/apps/android-qml/mobile_recurring_facade.h \
  src/apps/android-qml/mobile_recurring_facade.cpp \
  src/apps/android-qml/mobile_composition.cpp \
  src/apps/desktop-widgets/desktop_controller.h \
  src/apps/desktop-widgets/desktop_controller.cpp \
  src/apps/desktop-widgets/desktop_composition.cpp \
  src/apps/desktop-widgets/accounting_view_models.h \
  src/apps/desktop-widgets/accounting_view_models.cpp \
  tests/unit/reminder_reconciler_tests.cpp \
  tests/integration/reminder_both_ends_tests.cpp
git commit -m "feat: reconcile reminders on both clients"
```
---
### Task 6: Add Text Import Parsing, Batch Lifecycle, Deduplication, Preview, Matching, Atomic Commit, and Privacy
**Files:**
- Create: `src/modules/accounting/data/sqlite/migrations/005_import.sql`
- Create: `src/modules/accounting/import/text_import_parser.h`
- Create: `src/modules/accounting/import/text_import_parser.cpp`
- Create: `src/modules/accounting/application/import_service.h`
- Create: `src/modules/accounting/application/import_service.cpp`
- Create: `src/modules/accounting/data/sqlite/sqlite_import_repository.h`
- Create: `src/modules/accounting/data/sqlite/sqlite_import_repository.cpp`
- Create: `src/modules/accounting/data/sqlite/sqlite_provenance_repository.h`
- Create: `src/modules/accounting/data/sqlite/sqlite_provenance_repository.cpp`
- Create: `cloud/supabase/migrations/20260904070000_stage5_import_commit.sql`
- Create: `cloud/supabase/tests/database/0007_stage5_import_commit.test.sql`
- Create: `tests/unit/text_import_parser_tests.cpp`
- Create: `tests/unit/import_commit_tests.cpp`
- Create: `tests/integration/import_batch_sqlite_tests.cpp`
- Create: `tests/fixtures/import/committed-batch-payload.json`
- Modify: `src/modules/accounting/data/sqlite/accounting_migration_runner.h`
- Modify: `src/modules/accounting/data/sqlite/accounting_migration_runner.cpp`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.h`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.cpp`
- Modify: `src/modules/accounting/sync/accounting_sync_codec.h`
- Modify: `src/modules/accounting/sync/accounting_sync_codec.cpp`
- Modify: `src/modules/accounting/data/sqlite/sqlite_settings_store.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_settings_store.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**

```cpp
class TextImportParser final : public IImportParser {
public:
    Result<std::vector<ImportItem>> parse(
        ImportBatchId batchId,
        std::string_view normalizedInput,
        std::uint32_t normalizationVersion) const override;
};
enum class ImportBatchStatus { Draft, Committed, Discarded };
struct RecurringMatchCandidate {
    RecurringOccurrenceId occurrenceId; std::string title;
    MoneyMinor expectedAmountMinor; LocalDate effectiveDueOn; RecurringStatus status;
};
struct ImportPreview {
    ImportBatch batch; std::vector<ImportItem> items;
    std::vector<AccountingError> batchErrors;
};
struct CommitImportBatchCommand {
    ImportBatchId batchId;
    std::map<ImportItemId, std::optional<RecurringOccurrenceId>> matches;
    bool forceDuplicates; DeviceId modifiedByDeviceId; MutationId mutationId;
    UtcInstant committedAt;
};
class IImportService {
public:
    virtual ~IImportService() = default;
    virtual Result<ImportBatch> createBatch(const CreateImportBatchCommand&) = 0;
    virtual Result<ImportPreview> preview(ImportBatchId) = 0;
    virtual Result<ImportItem> applyCorrection(const CorrectImportItemCommand&) = 0;
    virtual Result<void> discard(ImportBatchId, DeviceId) = 0;
    virtual Result<CommittedImportBatch> commitBatch(const CommitImportBatchCommand&) = 0;
};
```
- `005_import.sql` adds device-local `import_batches(id TEXT PRIMARY KEY, normalization_version INTEGER NOT NULL, batch_hash TEXT NOT NULL, status TEXT NOT NULL CHECK(status IN ('DRAFT','COMMITTED','DISCARDED')), raw_text TEXT NOT NULL, created_at_ms INTEGER NOT NULL, updated_at_ms INTEGER NOT NULL)` and `import_items(id TEXT PRIMARY KEY, batch_id TEXT NOT NULL REFERENCES import_batches(id) ON DELETE CASCADE, raw_text TEXT NOT NULL, normalized_text TEXT NOT NULL, confidence TEXT NOT NULL CHECK(confidence IN ('HIGH','NEEDS_REVIEW','INVALID')), candidate_json TEXT NOT NULL, errors_json TEXT NOT NULL, fingerprint TEXT NOT NULL, user_correction_json TEXT, transaction_id TEXT REFERENCES transactions(id), ignored INTEGER NOT NULL CHECK(ignored IN (0,1)), override_id TEXT, updated_at_ms INTEGER NOT NULL)`, plus synced `transaction_provenance(id TEXT PRIMARY KEY, transaction_id TEXT NOT NULL REFERENCES transactions(id) ON DELETE RESTRICT, kind TEXT NOT NULL CHECK(kind IN ('TEXT_IMPORT','BILL_IMPORT','RECURRING')), external_key TEXT NOT NULL, source_version INTEGER NOT NULL, detached_at_ms INTEGER, created_at_ms INTEGER NOT NULL, updated_at_ms INTEGER NOT NULL, server_revision INTEGER NOT NULL CHECK(server_revision >= 0), deleted_at_ms INTEGER, modified_by_device_id TEXT NOT NULL CHECK(length(modified_by_device_id) = 36), local_state TEXT NOT NULL CHECK(local_state IN ('CLEAN','DIRTY','IN_FLIGHT','ISOLATED')), UNIQUE(kind, external_key))` as `STRICT`; the runner reaches schema version 5.
- `createBatch` normalizes per D-027 (byte/line/item limits, NFC, one trailing LF), computes `batchHash` and item fingerprints, rejects over-limit input with `InvalidArgument`, and persists raw text locally with no outbox row. Reopening a `DRAFT` batch returns the same items; a second create with the same hash returns the existing `DRAFT` batch.
- The parser implements D-027 confidence rules over the five fixture families: explicit direction, date with the selected default year, and a valid amount may reach `HIGH` only together; missing/ambiguous/unknown fields stay `NEEDS_REVIEW`; contradictions, zero, overflow, and over-precision are `INVALID`. Unknown category words surface as mapping suggestions, never as auto-created categories. Deduplication is advisory only: same-batch fingerprint duplicates and same-hash batch repeats are flagged; approximate matches against live posted transactions (same normalized merchant, same amount, within a three-day window) are flagged with `DuplicateImport`; every duplicate still permits an explicit correction.
- Preview computes recurring-match candidates per architecture 11.4 over live `PENDING` occurrences: normalized merchant equality, amount within rule tolerance, effective due date within plus-or-minus three days, optional account equality, and no other committed item already matched. A unique high-confidence candidate is preselected; multiple or low-confidence candidates require explicit user selection; out-of-tolerance and already-matched occurrences never auto-select.
- `commitBatch` runs in one UoW transaction: every confirmed item creates or reuses its posted transaction with `origin=TEXT_IMPORT` or confirms the matched pending recurring pair, writes one minimal synced provenance row per committed item with `external_key` equal to the override ID when forcing duplicates and the item fingerprint otherwise, marks the batch `COMMITTED`, clears or preserves raw text per the `clear_raw_text_after_commit` setting, and enqueues exactly one `COMMIT_IMPORT_BATCH` outbox mutation carrying every after-state transaction, provenance row, and occurrence change. Any failure rolls back the whole batch and reports which item failed.
- An already committed fingerprint returns the existing canonical transaction and its provenance without writing duplicates unless `forceDuplicates` is set, in which case a new override ID is generated and retained with a hint to the original candidate. Ignored and invalid items are never committed.
- The cloud migration adds the `COMMIT_IMPORT_BATCH` validator: all entities of the group apply or none do, provenance keys are unique per kind within the user, matched occurrence transitions are legal, and accepted commands produce one unsplit change group.
- Privacy: raw batch/item text never appears in outbox payload JSON, cloud rows, sync fixtures, diagnostics, or logs; `committed-batch-payload.json` is the canonical `COMMIT_IMPORT_BATCH` payload fixture containing only after-state transactions, provenance rows, and occurrence changes with no `raw_text` member at any depth.
- [ ] **Write the failing parser and commit tests**

```cpp
void fixtureOneLineParsesHighWithFullFields()
{
    TextImportFixture fixture;
    fixture.load("tests/fixtures/import/sample-01-one-line.txt");
    const auto batch = fixture.createBatch(/* defaultYear = */ 2026);
    const auto items = fixture.preview(batch.value().id).value().items;
    DA_CHECK_EQ(items.size(), std::size_t{1});
    DA_CHECK_EQ(items[0].confidence, ImportConfidence::High);
    DA_CHECK_EQ(items[0].candidate.amountMinor.value(), 6800);
    DA_CHECK_EQ(items[0].candidate.occurredOn.value(), (LocalDate{2026, 7, 18}));
}
void atomicCommitMatchesOneOccurrenceAndRollsBackOnFailure()
{
    ImportCommitFixture fixture;
    fixture.seedPendingRecurringPair("2026-09");
    const auto preview = fixture.preview(fixture.batchId()).value();
    DA_CHECK_EQ(preview.items[0].matchCandidate->occurrenceId,
        fixture.pendingOccurrenceId());
    fixture.injectItemFailure(1);
    DA_CHECK(!fixture.commit(CommitImportBatchCommand{
        fixture.batchId(), {{fixture.itemId(0), fixture.pendingOccurrenceId()}},
        false, fixture.deviceId(), MutationId::random(),
        UtcInstant{1'788'480'000'000}}).hasValue());
    DA_CHECK_EQ(fixture.postedTransactionCount(), 0);
    DA_CHECK_EQ(fixture.occurrenceStatus(fixture.pendingOccurrenceId()),
        RecurringStatus::Pending);
    DA_CHECK_EQ(fixture.batchStatus(), ImportBatchStatus::Draft);
}
```
Also cover date-heading grouping, missing-year with explicit default year, error and duplicate families, the oversized rejection at byte `1,048,577` (generated in memory per D-027), per-item correction persistence, batch reopen idempotency, unknown-category suggestions, high-confidence preselection, ambiguous and out-of-tolerance candidates, unmatched pending stays pending and later overdue, forced re-import override identity, raw-text retention and clearing, and privacy scans over codec output and sync tables.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_text_import_parser_tests dailyaccount_import_commit_tests --parallel 2
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --target dailyaccount_import_batch_sqlite_tests --parallel 2
```
Expected red: parser, import service, provenance repository, and migration 005 are absent.
- [ ] **Implement the minimum slice**
Implement D-027 normalization and the deterministic parser, the batch/item and provenance repositories over migration 005, the service methods with atomic plain-transaction persistence, recurring matching, and the single-transaction commit path with provenance writes and one outbox mutation; then add the cloud import-commit migration and pgTAP. Persist corrections so previews survive restart; keep every parse, match, and commit decision in C++.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_text_import_parser_tests && ./build/cmake/linux-core/dailyaccount_import_commit_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && python3 prototypes/stage0/import_fixtures/validate_text_fixtures.py --root tests/fixtures/import 2>/dev/null || true && git diff --check
```

```bash
supabase start --workdir cloud && supabase db reset --workdir cloud && supabase test db --workdir cloud --file supabase/tests/database/0007_stage5_import_commit.test.sql && supabase test db --workdir cloud && supabase stop --workdir cloud
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "import_batch|import_commit|sqlite_" --output-on-failure
```
Expected green: all five fixture families parse with D-027 confidence, committed batches post exactly once with no partial or split group locally or remotely, matching rules hold, raw text never appears in any outbox row or cloud row, migrations to schema 5 are lossless, and DAT prints `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/modules/accounting/data/sqlite/migrations/005_import.sql \
  src/modules/accounting/import/text_import_parser.h \
  src/modules/accounting/import/text_import_parser.cpp \
  src/modules/accounting/application/import_service.h \
  src/modules/accounting/application/import_service.cpp \
  src/modules/accounting/application/accounting_mutation_codec.h \
  src/modules/accounting/application/accounting_mutation_codec.cpp \
  src/modules/accounting/sync/accounting_sync_codec.h \
  src/modules/accounting/sync/accounting_sync_codec.cpp \
  src/modules/accounting/data/sqlite/sqlite_import_repository.h \
  src/modules/accounting/data/sqlite/sqlite_import_repository.cpp \
  src/modules/accounting/data/sqlite/sqlite_provenance_repository.h \
  src/modules/accounting/data/sqlite/sqlite_provenance_repository.cpp \
  src/modules/accounting/data/sqlite/sqlite_settings_store.h \
  src/modules/accounting/data/sqlite/sqlite_settings_store.cpp \
  src/modules/accounting/data/sqlite/accounting_migration_runner.h \
  src/modules/accounting/data/sqlite/accounting_migration_runner.cpp \
  cloud/supabase/migrations/20260904070000_stage5_import_commit.sql \
  cloud/supabase/tests/database/0007_stage5_import_commit.test.sql \
  tests/unit/text_import_parser_tests.cpp tests/unit/import_commit_tests.cpp \
  tests/integration/import_batch_sqlite_tests.cpp \
  tests/fixtures/import/committed-batch-payload.json
git commit -m "feat: add atomic privacy-safe text import"
```
---
### Task 7: Add Monthly Category-Structure Analytics with One Query and Drill-Down
**Files:**
- Create: `src/modules/accounting/analytics/monthly_breakdown_query.h`
- Create: `src/modules/accounting/analytics/monthly_breakdown_query.cpp`
- Create: `src/modules/accounting/application/analytics_service.h`
- Create: `src/modules/accounting/application/analytics_service.cpp`
- Create: `tests/unit/monthly_breakdown_tests.cpp`
- Create: `tests/integration/monthly_breakdown_sqlite_tests.cpp`
- Modify: `src/modules/accounting/application/accounting_query_service.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**

```cpp
struct MonthlySummary {
    MoneyMinor totalIncomeMinor, totalExpenseMinor, balanceMinor, pendingExpenseMinor;
    std::vector<CategoryAmount> expenseCategories, incomeCategories;
};
struct DrillDownQuery {
    LocalDate month; std::optional<CategoryId> categoryId; bool includeChildren;
};
class AnalyticsService final {
public:
    Result<MonthlySummary> monthlyBreakdown(LocalDate month);
    Result<std::vector<TransactionListItem>> drillTransactions(
        const DrillDownQuery& query);
};
```
- One SQL statement computes the whole monthly page: totals, per-top-level expense and income category amounts with integer basis points by D-024 largest remainder over the chart basis, the explicit uncategorized bucket, and the separately displayed pending total. Refunds reduce their inherited category's net spending, transfers contribute zero, and only live `POSTED` same-currency rows count.
- The chart and the list render from the same returned `MonthlySummary`; no second aggregation exists. Drill-down resolves a category to its children with amounts from the same month using identical aggregation semantics, then drills to individual live transactions for the selected node; basis points and totals stay identical at every level.
- Pending recurring occurrences appear only in `pendingExpenseMinor` and never feed totals, category rows, or drills.
- [ ] **Write the failing tests**

```cpp
void drillDownTotalsEqualParentRowTotals()
{
    AnalyticsFixture fixture;
    fixture.seedD024Vector();
    const auto summary = fixture.service().monthlyBreakdown(LocalDate{2026, 9}).value();
    DA_CHECK_EQ(summary.totalIncomeMinor, MoneyMinor{127000});
    DA_CHECK_EQ(summary.totalExpenseMinor, MoneyMinor{-57500});
    DA_CHECK_EQ(fixture.drillChildren("food").totalExpenseMinor,
        fixture.topRow("food").amountMinor);
}
```
Also test refund-reduced categories, uncategorized bucket presence, pending exclusion, transfer zero contribution, basis-point largest-remainder sums to 10000, month boundaries across leap years, and consistency between chart rows and drill results for every fixture row.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_monthly_breakdown_tests --parallel 2
```
Expected red: analytics query/service targets are absent.
- [ ] **Implement the minimum slice**
Implement the single-statement query in `monthly_breakdown_query.cpp` with bound month bounds and tombstone filters, the service wrapper, and the drill SQL reusing the same WHERE/aggregation fragments; expose both through `IAccountingQueryService` read models so desktop statistics and mobile analytics share identical numbers.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_monthly_breakdown_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```powershell
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "monthly_breakdown|accounting_query" --output-on-failure
```
Expected green: chart rows, lists, and drill rows are one consistent result, all D-024 vectors match, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/modules/accounting/analytics/monthly_breakdown_query.h \
  src/modules/accounting/analytics/monthly_breakdown_query.cpp \
  src/modules/accounting/application/analytics_service.h \
  src/modules/accounting/application/analytics_service.cpp \
  src/modules/accounting/application/accounting_query_service.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_query_service.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_query_service.cpp \
  tests/unit/monthly_breakdown_tests.cpp \
  tests/integration/monthly_breakdown_sqlite_tests.cpp
git commit -m "feat: add monthly analytics with drill-down"
```
---
### Task 8: Build the Windows and Android Workflow Surfaces End to End
**Files:**
- Create: `src/apps/desktop-widgets/accounts_page.h`
- Create: `src/apps/desktop-widgets/accounts_page.cpp`
- Create: `src/apps/desktop-widgets/recurring_page.h`
- Create: `src/apps/desktop-widgets/recurring_page.cpp`
- Create: `src/apps/desktop-widgets/import_page.h`
- Create: `src/apps/desktop-widgets/import_page.cpp`
- Create: `src/apps/android-qml/models/account_list_model.h`
- Create: `src/apps/android-qml/models/account_list_model.cpp`
- Create: `src/apps/android-qml/mobile_account_facade.h`
- Create: `src/apps/android-qml/mobile_account_facade.cpp`
- Create: `src/apps/android-qml/qml/AccountsPage.qml`
- Create: `tests/widgets/stage5_workflow_widgets_tests.cpp`
- Create: `tests/integration/stage5_mobile_workflow_tests.cpp`
- Create: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/WorkflowInstrumentationTest.kt`
- Create: `tests/android/run_stage5_matrix.sh`
- Modify: `src/apps/desktop-widgets/desktop_controller.h`
- Modify: `src/apps/desktop-widgets/desktop_controller.cpp`
- Modify: `src/apps/desktop-widgets/desktop_composition.cpp`
- Modify: `src/apps/desktop-widgets/register_modules.cpp`
- Modify: `src/apps/desktop-widgets/accounting_view_models.h`
- Modify: `src/apps/desktop-widgets/accounting_view_models.cpp`
- Modify: `gui/mainwindow.h`
- Modify: `gui/mainwindow.cpp`
- Modify: `gui/flowdialog.h`
- Modify: `gui/flowdialog.cpp`
- Modify: `gui/categorypage.h`
- Modify: `gui/categorypage.cpp`
- Modify: `gui/statisticspage.h`
- Modify: `gui/statisticspage.cpp`
- Modify: `gui/otherpage.h`
- Modify: `gui/otherpage.cpp`
- Modify: `src/apps/android-qml/qml/QuickEntryPage.qml`
- Modify: `src/apps/android-qml/qml/RecurringPage.qml`
- Modify: `src/apps/android-qml/qml/AnalyticsPage.qml`
- Modify: `src/apps/android-qml/qml/SettingsPage.qml`
- Modify: `src/apps/android-qml/qml/OverviewPage.qml`
- Modify: `src/apps/android-qml/qml/Main.qml`
- Modify: `src/apps/android-qml/mobile_composition.h`
- Modify: `src/apps/android-qml/mobile_composition.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**
- Windows pages: `accounts_page` shows balances, snapshots, and differences and links to the backfill list of unassigned transactions; `recurring_page` shows rule overview and per-period instances with per-period edit, confirm, defer, skip, cancel, and undo (with the D-031 detach choice); `import_page` shows batch creation, preview, per-item corrections, recurring matches, and atomic commit. The flow dialog supports transfer and refund entry and account fields; the category page manages hierarchy, tags, and archive; the statistics page drills from category to child to transactions; the settings page switches mode and default account and shows notification health text where applicable.
- Android QML: `AccountsPage.qml` (account mode only) lists balances and opens quick entry; `QuickEntryPage.qml` expands account/category/tag fields in account mode and supports transfer creation; `RecurringPage.qml` gains rule creation/editing, defer/skip/cancel/undo and per-period edits; `OverviewPage.qml` keeps the D-031 undo affordance on posted cards; `AnalyticsPage.qml` adds drill-down; `SettingsPage.qml` manages mode, default account, reminder health, and raw-text clearing. QML keeps money as decimal text and performs no monetary arithmetic.
- Mode switching on both ends changes only fields, defaults, and information hierarchy; it never copies or migrates transactions, and every analytics result is mode-independent.
- All Windows page actions run through `DesktopController` or desktop composition; all Android actions run through facades; no page constructs SQL, repositories, or providers.
- [ ] **Write the failing workflow tests**

```cpp
void windowsRecurringPageUndoKeepsOnePendingRow()
{
    WorkflowWidgetsFixture fixture;
    fixture.openRecurringPage();
    fixture.confirmCard("2026-09");
    DA_CHECK_EQ(fixture.pageModel()->postedCount(), 1);
    fixture.invokeUndo("2026-09", /* detach= */ false);
    DA_CHECK_EQ(fixture.pageModel()->postedCount(), 0);
    DA_CHECK_EQ(fixture.pageModel()->pendingRowCount("2026-09"), 1);
}
```
Also cover account-page snapshot save and difference display, transfer/refund dialog validation, category/tag picker filtering, statistics drill equality with the analytics service, import preview-to-commit round trip, mode-switch no-copy assertions, and QML object-name smoke checks.
- [ ] **Run red**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --target dailyaccount_stage5_workflow_widgets_tests dailyaccount_stage5_mobile_workflow_tests --parallel 2
```

```bash
cmake --preset android-x86_64-debug && cmake --build --preset android-x86_64-debug --target apk --parallel 2
```
Expected red: new pages, models, and facades are absent or compilation fails on their APIs.
- [ ] **Implement the minimum slice**
Add desktop pages and register them in `register_modules` and the main window navigation; extend the flow dialog and category/statistics/settings pages through the controller; add the mobile account model/facade and route `AccountsPage.qml`; extend the QML pages and mobile composition. Keep Widgets and QML pages read-only consumers of shared view models.
- [ ] **Run green and cumulative checks**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && & 'build\cmake\windows-desktop\dailyaccount_stage5_workflow_widgets_tests.exe' && & 'build\cmake\windows-desktop\dailyaccount_stage5_mobile_workflow_tests.exe' && ctest --preset windows-desktop --output-on-failure
```

```bash
cmake --preset android-x86_64-debug && cmake --build --preset android-x86_64-debug --target apk --parallel 2 && gradle -p build/cmake/android-x86_64-debug/android-build connectedDebugAndroidTest -Pandroid.testInstrumentationRunnerArguments.class=local.dailyaccount.WorkflowInstrumentationTest && bash tests/android/run_stage5_matrix.sh --api28-serial "$DA_ANDROID_API28_SERIAL" --api35-serial "$DA_ANDROID_API35_SERIAL" --x86-apk build/cmake/android-x86_64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk --result /tmp/opencode/dailyaccount-stage5-android-workflows.json && cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/android_boundary_contract.cmake && git diff --check
```

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests
```
Expected green: Windows and Android workflow surfaces pass with identical finance results, mode switches copy nothing, G3 reminder and G4 sync behavior remain green, and DAT prints `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt src/apps/desktop-widgets src/apps/android-qml \
  gui/mainwindow.h gui/mainwindow.cpp gui/flowdialog.h gui/flowdialog.cpp \
  gui/categorypage.h gui/categorypage.cpp gui/statisticspage.h \
  gui/statisticspage.cpp gui/otherpage.h gui/otherpage.cpp \
  tests/widgets/stage5_workflow_widgets_tests.cpp \
  tests/integration/stage5_mobile_workflow_tests.cpp \
  src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/WorkflowInstrumentationTest.kt \
  tests/android/run_stage5_matrix.sh
git commit -m "feat: add windows and android workflow surfaces"
```
---
### Task 9: Prove Negative Payment-Signal Boundaries and Seal G5 Evidence
**Files:**
- Create: `tests/cmake/payment_signal_boundary_contract.cmake`
- Create: `tests/unit/payment_signal_negative_tests.cpp`
- Create: `tests/cmake/check_g5.py`
- Create: `tests/cmake/test_check_g5.py`
- Create: `docs/validation/stage-5/linux-core.log`
- Create: `docs/validation/stage-5/windows-workflow.log`
- Create: `docs/validation/stage-5/cloud-local.log`
- Create: `docs/validation/stage-5/import-results.json`
- Create: `docs/validation/stage-5/analytics-results.json`
- Create: `docs/validation/stage-5/reminder-both-ends-results.json`
- Create: `docs/validation/stage-5/android-workflows-results.json`
- Create: `docs/validation/stage-5/source-tree.txt`
- Create: `docs/validation/stage-5/g5-evidence-index.md`
- Create: `docs/validation/stage-5/g5-results.json`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**
- Negative `IPaymentSignalSource` tests prove the V1 boundary by absence and by behavior: compile-time assertions that `IPaymentSignalSource` exposes only `collect()` returning `RawPaymentSignal` vectors, has no posting method, and no V1 type implements it; a source scan proving no class derives from `IPaymentSignalSource` and no shipped source accepts passwords, verification codes, cookies, tokens, or credentials; runtime assertions that a hypothetical bounded adapter output can only feed `IImportParser`, never a transaction write, and that raw payload text is absent from outbox JSON, cloud fixtures, diagnostics, and logs; and conformance that no import candidate becomes `POSTED` without explicit batch confirmation and that provider login, scraping, accessibility-service use, and payment execution appear nowhere in shipped code.
- `check_g5.py --root DIR --json PATH` verifies: accepted/fresh G4; single accepted D-020 through D-031 files with the D-031 tokens; schema version 5; transfer/refund, occurrence-lifecycle, and import-commit pgTAP; import fixture validation and privacy scans; analytics vector results; reminder both-ends results; Windows and Linux CTest; the Android workflow matrix; DAT 22; the stage-5 boundary scans; and a recorded source tree. It writes JSON only after all checks pass.
- [ ] **Write the failing checker and negative tests**

```python
def test_raw_text_is_absent_from_commit_payload(self):
    text = self.read("tests/fixtures/import/committed-batch-payload.json")
    assert "rawText" not in text and "raw_text" not in text
def test_payment_source_has_no_posting_surface(self):
    self.assert_failure("IPaymentSignalSource posts transactions")
```

```cpp
void noV1ImplementationAndNoPostingSurface()
{
    static_assert(std::is_abstract_v<IPaymentSignalSource>);
    DA_CHECK_EQ(paymentSignalSourceImplementations(), 0);
}
```
- [ ] **Run red**

```bash
python3 -m unittest tests/cmake/test_check_g5.py -v && cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_payment_signal_negative_tests --parallel 2
```
Expected red: `check_g5.py` import failure and the negative target being absent.
- [ ] **Implement the boundary contract and gate checker**
`payment_signal_boundary_contract.cmake` scans source trees for `IPaymentSignalSource` derivations, credential and login/scrape keywords, raw-text references in codec/cloud/export paths, and outbox payload fields; `check_g5.py` aggregates every stage-5 evidence artifact and prints exactly:

```text
G5 PASS: accounting workflows, recurring lifecycle, reminders, import privacy, and analytics
```
and rejects any missing artifact with a non-zero exit.
- [ ] **Run final green verification**

```bash
set -o pipefail
mkdir -p docs/validation/stage-5
supabase start --workdir cloud && supabase db reset --workdir cloud && supabase db lint --workdir cloud --level warning --fail-on error && supabase test db --workdir cloud 2>&1 | tee docs/validation/stage-5/cloud-local.log && supabase stop --workdir cloud
cmake --preset linux-core 2>&1 | tee docs/validation/stage-5/linux-core.log
cmake --build --preset linux-core --parallel 2 2>&1 | tee -a docs/validation/stage-5/linux-core.log
ctest --preset linux-core --output-on-failure 2>&1 | tee -a docs/validation/stage-5/linux-core.log
./build/cmake/linux-core/dailyaccount_backend_tests 2>&1 | tee -a docs/validation/stage-5/linux-core.log
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/payment_signal_boundary_contract.cmake
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/android_boundary_contract.cmake
python3 -m unittest tests/cmake/test_check_g5.py -v
STAGE5_INDEX=/tmp/opencode/dailyaccount-stage5-index; rm -f "$STAGE5_INDEX"
GIT_INDEX_FILE="$STAGE5_INDEX" git read-tree HEAD
GIT_INDEX_FILE="$STAGE5_INDEX" git add -A -- CMakeLists.txt src tests docs cloud docs/validation/stage-5
GIT_INDEX_FILE="$STAGE5_INDEX" git rm --cached --ignore-unmatch docs/validation/stage-5/source-tree.txt
GIT_INDEX_FILE="$STAGE5_INDEX" git diff --cached --check
GIT_INDEX_FILE="$STAGE5_INDEX" git write-tree > docs/validation/stage-5/source-tree.txt
rm -f "$STAGE5_INDEX"
python3 tests/cmake/check_g5.py --root . --json docs/validation/stage-5/g5-results.json
git diff --check
git status --short
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop --output-on-failure 2>&1 | Tee-Object docs\validation\stage-5\windows-workflow.log
if ($LASTEXITCODE -ne 0) { throw 'Stage 5 Windows CTest failed' }
```
Expected green: the checker prints exactly `G5 PASS: accounting workflows, recurring lifecycle, reminders, import privacy, and analytics`; `g5-results.json` records `gate=G5`, `result=PASS`, `localSchemaVersion=5`, `failureCount=0`, and the stage-5 boundary scans are silent.
- [ ] **Record evidence and obtain independent review**
`g5-evidence-index.md` records each command, UTC time, OS/device alias, exact tool versions, exit code, sanitized result path, source-tree identity, and SHA-256. The reviewer reruns G4/G5, pgTAP, transfer/refund guards, lifecycle and import races, import privacy scans, reminder D0 boundaries, analytics drill vectors, mode-switch no-copy checks, both workflow UIs, and the negative payment-signal tests.
- [ ] **Conditional final checkpoint**
After inspecting `git status --short`, `git diff`, and `git log --oneline -10`, and only after explicit authorization, commit the gate files separately from the preceding implementation checkpoints:

```bash
git add CMakeLists.txt tests/cmake/payment_signal_boundary_contract.cmake \
  tests/cmake/check_g5.py tests/cmake/test_check_g5.py \
  tests/unit/payment_signal_negative_tests.cpp \
  docs/validation/stage-5/linux-core.log \
  docs/validation/stage-5/windows-workflow.log \
  docs/validation/stage-5/cloud-local.log \
  docs/validation/stage-5/import-results.json \
  docs/validation/stage-5/analytics-results.json \
  docs/validation/stage-5/reminder-both-ends-results.json \
  docs/validation/stage-5/android-workflows-results.json \
  docs/validation/stage-5/source-tree.txt \
  docs/validation/stage-5/g5-evidence-index.md \
  docs/validation/stage-5/g5-results.json
git commit -m "test: seal G5 accounting workflow evidence"
```
Expected: only reviewed Stage 5 source, tests, and sanitized evidence are committed. Credentials, linked Supabase state, private import samples, raw text, APKs, databases, WAL/SHM files, build trees, device serials, and unrelated files remain untracked.
## G5 Checklist
Mapping of `docs/product-architecture.md` section 21.4 core-experience items to Stage 5 evidence:
| §21.4 item | Covered by | Evidence file |
| --- | --- | --- |
| Android quick entry needs no hidden account fields | Task 8 mobile quick entry in simple mode; stage-5 workflow matrix | `android-workflows-results.json` |
| Windows batch review of uncategorized/unassigned flows | Task 1 backfill queries; Task 3 uncategorized list; Task 8 accounts page | `linux-core.log`, `windows-workflow.log` |
| Text import writes no posted transaction before confirmation | Task 6 preview/correction and Task 6 commit command | `import-results.json` |
| Recurring page shows every unresolved period incl. overdue-plus-pending and archived leftovers | Task 4 lifecycle; Task 5 reconciler; Task 8 recurring pages both ends | `reminder-both-ends-results.json`, `windows-workflow.log` |
| Home card edits current-period amount/note and confirms once; duplicate taps and retries post once | Task 4 lifecycle; inherited G3 idempotent confirmation; Task 8 home cards | `android-workflows-results.json` |
| Unconfirmed due items never auto-post and stay overdue; prior overdue never blocks next period | Task 4 lifecycle; D-025 vectors | `linux-core.log`, pgTAP `0006` |
| Android 90-day natural-day reminder window with lead-day and due-day delivery | Task 5 reconciler both ends; D-030 native path; G3 matrix rerun | `reminder-both-ends-results.json`, `android-workflows-results.json` |
| Notification permission/scheduling restriction shows actionable status | Inherited G3 health surfaces; Task 8 settings surfaces | `android-workflows-results.json` |
| Force-stop limitation documented; reopen rebuilds reminders immediately | Inherited D-030; Task 5 reconcile-on-resume | `reminder-both-ends-results.json` |
| Monthly analysis drills from category share to children to transactions | Task 7 single-query analytics and drills | `analytics-results.json` |
| Mode switch never copies or migrates transactions; analysis is mode-independent | Task 1 mode service; Task 7 analytics; Task 8 mode UI | `linux-core.log`, `windows-workflow.log` |
| Simple/account models are one model | Task 1 and Task 8 no-copy assertions | `linux-core.log` |
- [ ] `check_g5.py` prints exactly `G5 PASS: accounting workflows, recurring lifecycle, reminders, import privacy, and analytics`; `g5-results.json` records `gate=G5`, `result=PASS`, `localSchemaVersion=5`, `failureCount=0`, and every evidence file listed above exists with sanitized content.
- [ ] The stage-5 boundary scans reject any `IPaymentSignalSource` implementation, posting surface, credential/login/scrape keyword, raw import text in sync/cloud/export/log paths, and second online write path.
- [ ] Transfer/refund, occurrence-lifecycle, and import-commit aggregates remain atomic locally and remotely with no split change group.
- [ ] Recurring confirmation, cancellation, import matching, and refund limits remain atomic locally and remotely.
- [ ] Mode switches and account-balance/backfill flows copy no transactions and leave analysis口径 unchanged.
- [ ] D-031 is accepted and recorded; architecture section 23 item 13 and section 24 now link the decision.
- [ ] Raw import text stays local and is excluded from logs, sync payloads, cloud rows, and the stage-5 committed payload fixture.
- [ ] `git diff --check` is silent and independent review accepts the evidence before the gate is recorded.
## Stage 6 Handoff
Stage 6 may begin only after every G5 checkbox is checked and `docs/validation/stage-5/g5-results.json` records `gate=G5`, `result=PASS`, `localSchemaVersion=5`, `failureCount=0`, and the D-031 decision is recorded. Its executor must read the architecture, master plan, D-020 through D-031, G0-G5 evidence indexes, and all public headers under `src/core/`, `src/platform/`, `src/modules/accounting/`, and `src/apps/` before starting recovery and release work.
Stage 6 inherits these immutable inputs:
- Schema version 5 with device-local import, snapshot, and settings tables, synced provenance, and the reviewed remote command validators for transfer/refund, occurrence lifecycle, and import commit.
- The G5 workflow surfaces on Windows and Android and their mode-switch, analytics, import, reminder, and recurring acceptance vectors; Stage 6 may not weaken the §21.4 behaviors or the D-030 reminder delivery path.
- Import raw text remains device-local and excluded from logs, sync, and ordinary export; Stage 6 JSON/CSV exporters and backup sets must keep the same boundary and must not claim any bill/payment-source channel exists.
- D-031 undo semantics are the reference for any later recovery workflows touching recurring pairs; recovery profiles must preserve occurrence/transaction aggregate invariants.
- Balance snapshots and module settings remain device-local; Stage 6 backup/restore must not treat them as synchronizable or expect them to converge across devices.
- Stage 6 owns versioned JSON/CSV export, automatic backups and restore drills, exit/local-copy/cloud-account deletion separation, installers, offline and fault drills, and release acceptance; these workflows build on the G5 accounting loop and must not open a second online write path.
