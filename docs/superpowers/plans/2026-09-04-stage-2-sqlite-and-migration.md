# DailyAccount Stage 2 SQLite and DAT Migration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the Windows application's writable `Ledger`/DAT path with per-profile SQLite repositories, atomically migrate every supported DAT source without changing it, and leave a tested SQLite/recurring foundation for the Android offline slice.

**Architecture:** Begin only from accepted G1 artifacts and keep `dailyaccount_legacy_backend` as a test-only migration oracle. Standard-C++ domain/application services validate commands and create stable UUID mutations; one profile/module QSQLITE executor owns each connection, and a SQLite unit of work commits business rows, local dirty state, and outbox rows together. DAT input is parsed read-only into a same-directory staging database, validated against source summaries, and atomically activated before the existing Widgets UI is rebound through controllers and read-only view models.

**Tech Stack:** C++17, CMake 3.22.1+, CTest, Qt 6.9.3 Core/Concurrent/SQL/Widgets/Test, QSQLITE with the D-028 accepted backup mechanism, SQLite `STRICT` tables, MinGW-w64 13.1 on Windows, GCC 11.4+ for Qt-free tests on Linux, Python 3 standard library for gate validation, and PowerShell for Windows package and rollback drills.

**Spec:** `docs/product-architecture.md`; parent plan: `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md`; prerequisite plan: `docs/superpowers/plans/2026-09-04-stage-1-cmake-and-boundaries.md`; prerequisite evidence: `docs/validation/stage-1/g1-evidence-index.md`

## Global Constraints

- Start no implementation task until the G1 entry procedure below prints exactly `G1 PASS: Linux core, Windows desktop, 22 legacy regressions, CMake-only build` and both the accepted and fresh JSON records contain `gate=G1`, `result=PASS`, `legacyBackendTestCount=22`, and `failureCount=0`.
- Read D-020 through D-030 and both G0/G1 evidence indexes before implementation. The accepted Qt paths, profile state machine, CNY limit, recurrence grammar, QSQLITE connection/backup mechanism, and tool versions override any conflicting example in this plan; a conflict requires a reviewed plan/spec correction before code changes.
- Preserve unrelated worktree changes. Never stash, reset, clean, discard, overwrite, or stage work outside the current task.
- Do not create a Git commit unless the user explicitly authorizes commits in the execution session. Every checkpoint below is optional and remains authorization-gated.
- Treat accepted G1 public headers as immutable. Before Task 1, verify that G1 already includes the Task 3-required three-argument `IRepository::markDeleted(..., DeviceId)` and timestamped/expected `OutboxMutation`; the currently documented two-argument/four-field forms are insufficient to preserve delete metadata and build a `MutationEnvelope`. If accepted G1 still has those forms, stop and complete the reviewed contract correction required by the G1 handoff across the architecture, master plan, Stage 1 plan/evidence, and this plan, then rerun G1. Never hide that interface change inside a Stage 2 implementation commit.
- Keep C++17 as the shared-language floor, CMake 3.22.1 as the minimum, Qt exactly 6.9.3, and the G1 Windows compiler at MinGW-w64 13.1.
- Keep `dailyaccount_core_domain`, `dailyaccount_accounting_domain`, and `dailyaccount_accounting_application` free of Qt, SQL, Widgets, Network, JNI, Windows API, and provider SDK types. QSQLITE types stay under `src/platform/database/`, `src/platform/profile/`, `src/modules/accounting/data/sqlite/`, and desktop adapters.
- Keep all monetary values as signed 64-bit integer minor units. Persist no financial value as `REAL`; expose no application or view-model money field as `double`.
- Accept V1 writes only in `CNY`. Individual transaction/recurring amounts are `1..9,999,999,999` minor units; account opening balance magnitude is `0..9,999,999,999`; aggregate arithmetic fails on signed 64-bit overflow.
- Use client-created UUIDs for every synchronizable entity and mutation. A retry reuses the command's original entity and mutation IDs. Legacy integer IDs exist only in the read-only DAT importer and the local migration map.
- Use one serial QSQLITE executor per profile/module and a separate platform executor for `profiles.sqlite`. A connection is created, opened, used, closed, reset, and removed only on its worker thread; no `QSqlDatabase` or `QSqlQuery` crosses that thread boundary.
- Every connection enables `PRAGMA foreign_keys=ON`, verifies `journal_mode=WAL` and `synchronous=FULL`, and sets `QSQLITE_BUSY_TIMEOUT=5000`. No SQLite transaction waits for network I/O.
- Use bound SQL parameters for values. Schema identifiers come only from compiled migration resources, never user input.
- Every business command writes all affected entities, preserves their current `serverRevision`, marks them locally dirty, writes one stable outbox mutation with matching base-revision expectations, and commits once. Any failure rolls back all of those effects.
- Preserve `ledger.dat`, `ledger.dat.bak`, `records.dat`, and `categories.dat` as read-only migration sources. Never rename, delete, truncate, repair, rewrite, or place temporary files beside a source DAT file.
- A DAT migration targets an explicitly selected profile with no accounting entities. It never merges DAT into a non-empty profile and never binds unowned DAT to a cloud account.
- A migration or schema upgrade writes and validates a same-directory staging database before one atomic activation. A failure before activation leaves the active database and DAT source unchanged; after the first accepted SQLite business write, no code path may reopen DAT as writable storage.
- Preserve all 22 existing DAT regression cases. Relocation is allowed only after a coverage contract proves every named test moved with its assertions; the test-only legacy writer remains available as an oracle but is not linked into `DailyAccount.exe`.
- Keep current Windows income/expense create, edit, delete, filtering, category management, dashboard, statistics, TXT export, five-page navigation, window sizing, and error feedback behavior. Remove the legacy physical whole-ledger clear entry (old snapshot wipe plus next-ID reset) and do not emulate it with repeated deletes; V1 provides no ledger-wide clearing surface (spec sections 4.2 and 16.4, decision D-019).
- Stage 2 implements only the minimum recurring rule plus deterministic pending occurrence creation needed by Stage 3. Full confirm/defer/skip/cancel/undo UI, reminders, transfers, refunds, account reconciliation, text import, cloud sync, and conflict resolution remain owned by their later stages.
- Every behavior-changing task follows red-green-refactor: add one focused failing test, run it and verify the stated reason, implement the smallest behavior, run focused and cumulative suites, and finish with `git diff --check`. Because that command ignores untracked files, also run `git diff --no-index --check -- /dev/null <path>` for every untracked file created by the task before declaring its checkpoint green.

---

## G1 Entry Gate

No task below may begin until every checkbox in this section is checked. A missing artifact, hash mismatch, remaining G1 exception, or checker failure stops Stage 2 without changing production files.

- [ ] **Verify the required G1 artifacts and accepted decisions exist**

Run from the repository root:

```bash
test -f docs/validation/stage-1/g1-results.json
test -f docs/validation/stage-1/g1-evidence-index.md
test -f tests/cmake/check_g1.py
for number in 020 021 022 023 024 025 026 027 028 029 030; do
  test -f "docs/decisions/D-${number}-"*.md
done
```

Expected: every command exits `0`, and each ADR glob expands to exactly one accepted decision file.

- [ ] **Re-run the authoritative G1 checker without replacing accepted evidence**

```bash
python3 tests/cmake/check_g1.py \
  --root . \
  --json /tmp/opencode/dailyaccount-stage2-g1-recheck.json
```

Expected stdout:

```text
G1 PASS: Linux core, Windows desktop, 22 legacy regressions, CMake-only build
```

- [ ] **Compare accepted and fresh G1 results**

```bash
python3 - <<'PY'
import json
from pathlib import Path

accepted = json.loads(Path("docs/validation/stage-1/g1-results.json").read_text(encoding="utf-8"))
fresh = json.loads(Path("/tmp/opencode/dailyaccount-stage2-g1-recheck.json").read_text(encoding="utf-8"))
keys = ("gate", "result", "legacyBackendTestCount", "failureCount")
expected = ("G1", "PASS", 22, 0)
assert tuple(accepted[key] for key in keys) == expected
assert tuple(fresh[key] for key in keys) == expected
print("Stage 2 entry gate: PASS")
PY
```

Expected: `Stage 2 entry gate: PASS` and exit code `0`.

- [ ] **Read immutable inputs and record the starting worktree**

Read `docs/product-architecture.md`, the master plan, the Stage 1 plan, both evidence indexes, D-020 through D-030, every header under `src/core/`, `src/platform/`, and `src/modules/accounting/`, and the current `backend/`, `gui/`, and `tests/` sources. Then run:

```bash
git status --short
git diff --check
git ls-files '*.pro'
```

Expected: `git diff --check` is silent. Record pre-existing changed/untracked paths. Normally `git ls-files '*.pro'` is empty because G1 retired qmake; if an amended accepted G1 explicitly retained one of `jizhang.pro`, `tests/backend_tests.pro`, or `tests/registry_tests.pro`, record those exact paths for the conditional retirement in Task 15 and do not delete anything at entry.

- [ ] **Verify the accepted G1 write contracts can carry Stage 2 metadata**

Inspect `src/modules/accounting/application/accounting_repositories.h` and its accepted G1 contract evidence. Require these semantic fields/signatures, using the exact accepted names from the reviewed correction:

```cpp
virtual Result<void> markDeleted(
    const Id& id, UtcInstant deletedAt, DeviceId modifiedByDeviceId) = 0;

struct OutboxExpectation {
    std::string entityType;
    std::string entityId;
    std::uint64_t baseServerRevision;
};

struct OutboxMutation {
    MutationId mutationId;
    std::string commandType;
    std::uint32_t payloadVersion;
    std::string payloadJson;
    std::vector<OutboxExpectation> expectations;
    UtcInstant createdAt;
};
```

Expected: the accepted headers, architecture/master contracts, Stage 1 plan, G1 evidence hashes, and fresh G1 result all agree. If G1 still exposes `markDeleted(id, deletedAt)` or an outbox value without expectations/timestamp, stop Stage 2 and perform the reviewed G1 contract correction described above.

---

## Deliverables and File Map

Stage 2 owns this change surface. Existing Stage 1 files not named here remain unchanged, and generated build trees/databases/packages remain untracked.

```text
CMakeLists.txt
CMakePresets.json
README.md
build/build.bat
src/
  core/domain/
    date_time.h
    money.h
  platform/
    database/
      module_db_executor.h
      module_db_executor.cpp
      atomic_file_activation.h
      atomic_file_activation.cpp
    profile/
      local_profile.h
      profile_store.h
      profile_store.cpp
      profile_directory_locator.h
      profile_directory_locator.cpp
      migrations/
        001_profiles.sql
        002_legacy_migration_manifests.sql
  modules/accounting/
    accounting_module.cpp
    domain/
      accounting_rules.h
      accounting_rules.cpp
    application/
      accounting_commands.h
      accounting_repositories.h
      accounting_service.h
      accounting_service.cpp
      accounting_mutation_codec.h
      accounting_mutation_codec.cpp
      recurring_service.h
      recurring_service.cpp
      accounting_query_service.h
      accounting_view_models.h
    data/
      sqlite/
        accounting_database.h
        accounting_database.cpp
        accounting_migration_runner.h
        accounting_migration_runner.cpp
        accounting_unit_of_work.h
        accounting_unit_of_work.cpp
        sqlite_accounting_query_service.h
        sqlite_accounting_query_service.cpp
        migrations/
          001_initial.sql
          002_recurring.sql
      legacy/
        dat_importer.h
        dat_importer.cpp
        migration_manifest.h
        migration_manifest.cpp
        dat_migration_service.h
        dat_migration_service.cpp
  apps/desktop-widgets/
    accounting_view_models.h
    accounting_view_models.cpp
    desktop_controller.h
    desktop_controller.cpp
    desktop_composition.h
    desktop_composition.cpp
    profile_selection_dialog.h
    profile_selection_dialog.cpp
    register_modules.h
    register_modules.cpp
gui/
  main_gui.cpp
  mainwindow.h
  mainwindow.cpp
  dashboardpage.h
  dashboardpage.cpp
  flowpage.h
  flowpage.cpp
  flowdialog.h
  flowdialog.cpp
  categorypage.h
  categorypage.cpp
  statisticspage.h
  statisticspage.cpp
  otherpage.h
  otherpage.cpp
tests/
  legacy/dat_regression_tests.cpp
  unit/accounting_rules_tests.cpp
  unit/accounting_crud_tests.cpp
  unit/catalog_crud_tests.cpp
  unit/recurring_service_tests.cpp
  unit/dat_importer_tests.cpp
  integration/module_db_executor_tests.cpp
  integration/profile_store_tests.cpp
  integration/sqlite_schema_tests.cpp
  integration/sqlite_unit_of_work_tests.cpp
  integration/dat_migration_tests.cpp
  integration/accounting_query_tests.cpp
  widgets/desktop_controller_tests.cpp
  widgets/sqlite_flow_widgets_tests.cpp
  widgets/sqlite_dashboard_widgets_tests.cpp
  widgets/desktop_storage_switch_tests.cpp
  cmake/dat_regression_coverage_contract.cmake
  cmake/sqlite_desktop_boundary_contract.cmake
  cmake/check_g2.py
  cmake/test_check_g2.py
  windows/verify_migration_rollback.ps1
docs/validation/stage-2/
  linux-core.log
  windows-sqlite.log
  windows-ctest.log
  migration-fixtures-results.json
  migration-fault-matrix.json
  migration-rollback.log
  migration-rollback-results.json
  sqlite-contract-results.json
  windows-package-results.json
  source-tree.txt
  g2-evidence-index.md
  g2-results.json
```

Conditional qmake deletion surface, only when the G1 entry notes prove a path still exists under an accepted amended G1:

```text
jizhang.pro
tests/backend_tests.pro
tests/registry_tests.pro
```

### Target Dependency Graph

```text
dailyaccount_core_domain
  <- dailyaccount_accounting_domain
  <- dailyaccount_accounting_storage_contracts
  <- dailyaccount_accounting_application

Qt6::Core + Qt6::Concurrent + Qt6::Sql
  <- dailyaccount_platform_database
  <- dailyaccount_profile
  <- dailyaccount_accounting_sqlite

dailyaccount_legacy_backend (test only)
  <- dailyaccount_backend_tests

dailyaccount_dat_importer (standard C++, read only)
  <- dailyaccount_dat_migration

dailyaccount_accounting_application + dailyaccount_accounting_sqlite
  <- dailyaccount_desktop_controller
  <- dailyaccount_legacy_widgets
  <- dailyaccount_desktop
```

`dailyaccount_desktop` must have no transitive link to `dailyaccount_legacy_backend`. `dailyaccount_dat_importer` is reachable only from the explicit migration coordinator, never from normal transaction commands.

---

### Task 1: Relocate and Lock the 22-Test DAT Regression Oracle

**Files:**
- Create: `tests/legacy/dat_regression_tests.cpp`
- Create: `tests/cmake/dat_regression_coverage_contract.cmake`
- Modify: `CMakeLists.txt`
- Delete after green: `tests/backend_tests.cpp`
- Test: `tests/cmake/dat_regression_coverage_contract.cmake`
- Test: `tests/legacy/dat_regression_tests.cpp`

**Interfaces:**
- Consumes: G1 `tests/backend_tests.cpp`, `dailyaccount_legacy_backend`, shared test support, and the exact `22 test(s) passed` baseline.
- Produces: the unchanged executable/CTest name `dailyaccount_backend_tests`, now sourced from `tests/legacy/dat_regression_tests.cpp`; a textual contract that preserves all 22 named DAT behaviors and prevents the legacy target from entering a shipping link.

- [ ] **Step 1: Write the failing coverage contract**

Create `tests/cmake/dat_regression_coverage_contract.cmake` with this contract:

```cmake
if(NOT DEFINED DA_SOURCE_DIR)
    message(FATAL_ERROR "DA_SOURCE_DIR is required")
endif()

set(test_file "${DA_SOURCE_DIR}/tests/legacy/dat_regression_tests.cpp")
if(NOT EXISTS "${test_file}")
    message(FATAL_ERROR "missing migrated DAT regression file")
endif()
file(READ "${test_file}" source)
set(required_names
    "money helpers are exact and checked"
    "legacy files load as one validated state"
    "legacy long categories are preserved"
    "incomplete legacy pairs are rejected"
    "missing legacy catalog entries recover"
    "versioned round trip escapes text"
    "backup restores a complete snapshot"
    "failed mutations keep live state"
    "clear uses one snapshot and resets id"
    "failed reload keeps live state"
    "invalid V3 data reports its line"
    "V3 data requires an end marker"
    "V3 checksum rejects valid-looking corruption"
    "corrupt current preserves backup"
    "interrupted backup rotation recovers"
    "obsolete rotation artifacts do not block reads"
    "legacy state migrates in one snapshot"
    "record queries return snapshots"
    "impossible next ID is rejected"
    "preset catalog rows stay compatible"
    "invalid record types are rejected"
    "summaries use exact cents and ranges")
foreach(name IN LISTS required_names)
    string(FIND "${source}" "\"${name}\"" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "missing DAT regression: ${name}")
    endif()
endforeach()
list(LENGTH required_names count)
if(NOT count EQUAL 22)
    message(FATAL_ERROR "DAT regression contract count is not 22")
endif()
```

- [ ] **Step 2: Run it red before moving the suite**

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/dat_regression_coverage_contract.cmake
```

Expected: non-zero exit containing `missing migrated DAT regression file`.

- [ ] **Step 3: Move the complete suite without weakening assertions**

Move the full contents of `tests/backend_tests.cpp` to `tests/legacy/dat_regression_tests.cpp`. Retain every test function, fixture, failure injection, assertion, display name, and final count. Replace only Stage 1's old relative support includes if needed:

```cpp
#include "support/test_harness.h"
#include "support/temporary_directory.h"

#define CHECK(expression) DA_CHECK(expression)
using TempDirectory = dailyaccount::test::TemporaryDirectory;
using dailyaccount::test::readText;
using dailyaccount::test::writeText;
```

Keep the test-only `StorageManager::save`, backup rotation, interrupted-write recovery, and `Ledger::clearAllData` cases. They remain regression/migration oracles even though the shipping desktop stops linking their implementation.

- [ ] **Step 4: Point the existing target at the relocated source**

In `CMakeLists.txt`, change only the source path and register the contract:

```cmake
da_add_core_test(dailyaccount_backend_tests tests/legacy/dat_regression_tests.cpp)
target_link_libraries(dailyaccount_backend_tests PRIVATE dailyaccount_legacy_backend)

add_test(NAME dailyaccount_dat_regression_coverage_contract
    COMMAND "${CMAKE_COMMAND}"
        -DDA_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/dat_regression_coverage_contract.cmake)
```

Delete `tests/backend_tests.cpp` only after the relocated binary reports 22 passes.

- [ ] **Step 5: Run focused and cumulative checks**

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/dat_regression_coverage_contract.cmake
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: the contract exits `0`, the direct suite ends with exactly `22 test(s) passed`, and cumulative CTest has zero failures.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt tests/backend_tests.cpp \
  tests/legacy/dat_regression_tests.cpp \
  tests/cmake/dat_regression_coverage_contract.cmake
git commit -m "test: preserve dat regression oracle"
```

Without authorization, leave the verified move uncommitted.

---

### Task 2: Implement Target Entity Validation and Deterministic Recurring Identities

**Files:**
- Create: `src/modules/accounting/domain/accounting_rules.h`
- Create: `src/modules/accounting/domain/accounting_rules.cpp`
- Create: `tests/unit/accounting_rules_tests.cpp`
- Modify: `src/core/domain/money.h`
- Modify: `src/core/domain/date_time.h`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/accounting_rules_tests.cpp`
- Test: `tests/unit/date_money_tests.cpp`

**Interfaces:**
- Consumes: G1 `Transaction`, `Account`, `Category`, `Tag`, `RecurringRule`, `RecurringOccurrence`, typed UUIDv5, `Result<T>`, D-024 CNY bounds, and D-025 recurrence grammar.
- Produces: exact decimal parsing/formatting, date comparison/arithmetic, transaction/account/category/tag/recurring validators, canonical recurrence parsing, period-date calculation, and deterministic occurrence/pending-transaction IDs.

- [ ] **Step 1: Write focused validation and identity tests**

Create `tests/unit/accounting_rules_tests.cpp` with named cases covering all entities. Include these assertions:

```cpp
void deterministicRecurringIdsMatchTheFrozenVector()
{
    const auto ruleId = RecurringRuleId::parse(
        "11111111-1111-4111-8111-111111111111").value();
    DA_CHECK_EQ(occurrenceIdFor(ruleId, "2026-09").toString(),
                "2456c362-7021-512c-a5fc-4be1a167cd5c");
    DA_CHECK_EQ(pendingTransactionIdFor(
                    occurrenceIdFor(ruleId, "2026-09")).toString(),
                "faff3e00-97c1-5865-8c73-23ed45ca7f24");
}

void transferRequiresTwoDifferentCnyAccounts()
{
    Transaction transfer = validTransfer();
    transfer.destinationAccountId = transfer.accountId;
    const auto result = validateTransaction(transfer, validTransferContext());
    DA_CHECK(!result.hasValue());
    DA_CHECK_EQ(result.error().code, AccountingErrorCode::DomainConstraint);
}

void occurrenceRequiresItsDeterministicIdentityAndMatchingPendingTransaction()
{
    auto pair = validPendingOccurrencePair();
    pair.occurrence.id = RecurringOccurrenceId::random();
    const auto result = validateRecurringOccurrence(
        pair.occurrence, pair.transaction);
    DA_CHECK(!result.hasValue());
}
```

Also test expense/income account optionality, transfer/refund field matrices, pending restrictions, amount bounds, CNY-only writes, refund category/currency/limit checks, account opening-balance bounds, category parent/self/applicability checks, tag/category/account name bounds, all three D-025 frequency forms, invalid frequency text, monthly day 31 across leap/non-leap February, yearly February 29, interval key `D00000000`, lead days 1/2, `ClampToLastDay` only, and occurrence status/transaction linkage.

- [ ] **Step 2: Run the focused test red**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/unit/accounting_rules_tests.cpp src/core/domain/uuid.cpp \
  src/modules/accounting/domain/accounting_rules.cpp \
  -o /tmp/opencode/dailyaccount_accounting_rules_tests
```

Expected: compilation fails at `modules/accounting/domain/accounting_rules.h: No such file or directory`.

- [ ] **Step 3: Add exact money and date helpers**

Append these standard-C++ declarations and inline implementations to the existing G1 value headers:

```cpp
Result<MoneyMinor> parseCnyMinor(std::string_view decimalText);
std::string formatMoney(MoneyMinor amountMinor);

int compare(LocalDate left, LocalDate right) noexcept;
Result<LocalDate> addDays(LocalDate date, std::int32_t days);
std::string formatIsoDate(LocalDate date);
Result<LocalDate> parseIsoDate(std::string_view text);
```

`parseCnyMinor` accepts only ASCII `DIGITS` or `DIGITS.DIGIT{1,2}`, rejects signs, whitespace, separators, exponent notation, zero, excess precision, and values above `99999999.99`, and never converts through floating point. `formatMoney` handles `INT64_MIN` without signed overflow. Date helpers use Gregorian rules and the accepted year range `0100..9999`.

- [ ] **Step 4: Define the domain rule surface**

Use these exact declarations in `accounting_rules.h`:

```cpp
namespace dailyaccount {

struct TransactionValidationContext {
    std::optional<Account> account;
    std::optional<Account> destinationAccount;
    std::optional<Category> category;
    std::optional<Transaction> refundedExpense;
    MoneyMinor existingPostedRefundMinor = 0;
};

enum class RecurringFrequencyKind { Monthly, Yearly, IntervalDays };

struct RecurringFrequency {
    RecurringFrequencyKind kind;
    std::uint8_t month;
    std::uint8_t day;
    std::uint16_t intervalDays;
};

struct RecurringIdentity {
    RecurringOccurrenceId occurrenceId;
    TransactionId pendingTransactionId;
};

Result<void> validateTransaction(
    const Transaction& transaction,
    const TransactionValidationContext& context);
Result<void> validateAccount(const Account& account);
Result<void> validateCategory(
    const Category& category,
    const std::optional<Category>& parent);
Result<void> validateTag(const Tag& tag);
Result<RecurringFrequency> parseRecurringFrequency(std::string_view frequencySpec);
Result<void> validateRecurringRule(
    const RecurringRule& rule,
    bool timeZoneIsKnown);
Result<LocalDate> occurrenceDateFor(
    const RecurringRule& rule,
    std::string_view periodKey);
Result<void> validateRecurringOccurrence(
    const RecurringOccurrence& occurrence,
    const std::optional<Transaction>& linkedTransaction);
RecurringOccurrenceId occurrenceIdFor(
    const RecurringRuleId& ruleId,
    std::string_view periodKey);
TransactionId pendingTransactionIdFor(
    const RecurringOccurrenceId& occurrenceId);
RecurringIdentity recurringIdentityFor(
    const RecurringRuleId& ruleId,
    std::string_view periodKey);

}
```

- [ ] **Step 5: Implement every target invariant**

Implement the architecture's complete type matrix. The validators return `InvalidArgument` for malformed scalar input and `DomainConstraint` for invalid relationships. Use these exact non-migration write limits:

```cpp
inline constexpr std::size_t kAccountNameBytes = 128;
inline constexpr std::size_t kCategoryNameBytes = 256;
inline constexpr std::size_t kTagNameBytes = 128;
inline constexpr std::size_t kMerchantBytes = 256;
inline constexpr std::size_t kNoteBytes = 2048;
inline constexpr std::size_t kRecurringMarkerBytes = 128;
```

Enforce these relationship rules:

```text
EXPENSE: destination/refund are null; PENDING is allowed only for RECURRING.
INCOME: destination/refund are null and status is POSTED.
TRANSFER: source/destination exist, differ, are CNY, category/refund are null, status is POSTED.
REFUND: account and refunded POSTED EXPENSE exist, currencies match, category is inherited,
        cumulative live POSTED refunds including this value do not exceed the expense.
```

New account/category/tag names must contain a non-whitespace byte and fit their limits. A category cannot parent itself; a child has at most one top-level parent and its applicability must intersect the parent's. Existing overlength legacy names are import-only and cannot be created or enlarged through these validators. Recurring rules accept exactly D-025's three canonical strings, known non-empty IANA zone IDs, lead days 1 or 2, nonnegative tolerance at or below the V1 amount limit, `ClampToLastDay`, and a valid start/end range.

Use the exact identity formulas:

```cpp
return uuidV5<RecurringOccurrenceIdTag>(ruleId, periodKey);
return uuidV5<TransactionIdTag>(occurrenceId, "pending-transaction");
```

- [ ] **Step 6: Build the compiled domain target and run focused/cumulative tests**

Add `accounting_rules.cpp` to `dailyaccount_accounting_domain`, then run:

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_accounting_rules_tests
./build/cmake/linux-core/dailyaccount_date_money_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: all focused rule/value tests pass, the DAT oracle still reports `22 test(s) passed`, and cumulative CTest has zero failures.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/core/domain/money.h src/core/domain/date_time.h \
  src/modules/accounting/domain/accounting_rules.* \
  tests/unit/accounting_rules_tests.cpp
git commit -m "feat: validate target accounting entities"
```

Without authorization, do not commit.

---

### Task 3: Add Transaction CRUD Commands, Stable Outbox Expectations, and the Deletion Guard

**Files:**
- Create: `src/modules/accounting/application/accounting_commands.h`
- Create: `src/modules/accounting/application/accounting_mutation_codec.h`
- Create: `src/modules/accounting/application/accounting_mutation_codec.cpp`
- Create: `tests/unit/accounting_crud_tests.cpp`
- Modify: `src/modules/accounting/application/accounting_repositories.h`
- Modify: `src/modules/accounting/application/accounting_service.h`
- Modify: `src/modules/accounting/application/accounting_service.cpp`
- Modify: `tests/unit/accounting_application_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/accounting_crud_tests.cpp`

**Interfaces:**
- Consumes: Task 2 validators, G1 repositories/UoW, and G1 `OutboxMutation` fields.
- Produces: command-carried stable IDs; create/update/delete transaction use cases; base-revision expectations; canonical provider-neutral JSON; and an explicit `OccurrenceLinked` failure that writes neither tombstone nor outbox.

- [ ] **Step 1: Write a fake-UoW deletion-guard test**

Create fakes in `tests/unit/accounting_crud_tests.cpp`. The focused case is:

```cpp
void occurrenceLinkedDeleteWritesNothing()
{
    Fixture fixture;
    fixture.transactions.row = validPostedExpense();
    fixture.transactions.markDeletedResult = Result<void>::failure(
        {AccountingErrorCode::OccurrenceLinked,
         "transaction is linked to a live recurring occurrence"});

    DeleteTransactionCommand command{
        fixture.transactions.row->id,
        UtcInstant{1'788'480'000'000},
        DeviceId::random(),
        MutationId::random()};
    const auto result = fixture.service.deleteTransaction(command);

    DA_CHECK(!result.hasValue());
    DA_CHECK_EQ(result.error().code, AccountingErrorCode::OccurrenceLinked);
    DA_CHECK_EQ(fixture.transactions.markDeletedCalls, 1);
    DA_CHECK_EQ(fixture.transactions.lastDeleteDeviceId,
                command.modifiedByDeviceId);
    DA_CHECK_EQ(fixture.outbox.enqueueCalls, 0);
    DA_CHECK(!fixture.transactions.row->metadata.deletedAt.has_value());
}
```

Also prove create uses `serverRevision=0`, update preserves the stored revision, delete uses the stored revision in its expectation, the command's mutation ID and authoritative command timestamp reach outbox unchanged, validation happens before repository writes, not-found returns `NotFound`, and any repository/outbox error escapes unchanged while the fake UoW reports rollback.

- [ ] **Step 2: Run the focused test red**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/unit/accounting_crud_tests.cpp src/core/domain/uuid.cpp \
  src/modules/accounting/domain/accounting_rules.cpp \
  src/modules/accounting/application/accounting_service.cpp \
  src/modules/accounting/application/accounting_mutation_codec.cpp \
  -o /tmp/opencode/dailyaccount_accounting_crud_tests
```

Expected: compilation fails because `accounting_commands.h` is absent.

- [ ] **Step 3: Define exact commands and use the accepted G1 outbox expectations**

Use these declarations:

```cpp
namespace dailyaccount {

struct CreateTransactionCommand {
    Transaction transaction;
    MutationId mutationId;
};

struct UpdateTransactionCommand {
    Transaction transaction;
    MutationId mutationId;
};

struct DeleteTransactionCommand {
    TransactionId transactionId;
    UtcInstant deletedAt;
    DeviceId modifiedByDeviceId;
    MutationId mutationId;
};

}
```

Use the reviewed G1 shape verified at entry: `IRepository::markDeleted` takes `(const Id&, UtcInstant, DeviceId)`, and `OutboxMutation` preserves its original four fields then appends `std::vector<OutboxExpectation> expectations;` and `UtcInstant createdAt;`. Preserve `IOutboxRepository::enqueue(const OutboxMutation&)`. Document `ITransactionRepository::markDeleted` to return `OccurrenceLinked` when a live occurrence references the row. If the accepted G1 header does not already have this exact semantic capacity, stop rather than editing it here.

- [ ] **Step 4: Define the mutation codec and service signatures**

`accounting_mutation_codec.h` exposes only standard-C++ types:

```cpp
OutboxMutation transactionUpsertMutation(
    MutationId mutationId,
    const Transaction& transaction);
OutboxMutation transactionDeleteMutation(
    MutationId mutationId,
    const Transaction& transaction);
```

Extend `IAccountingService` and `AccountingService` without removing `findTransaction`:

```cpp
virtual Result<Transaction> createTransaction(
    const CreateTransactionCommand& command) = 0;
virtual Result<Transaction> updateTransaction(
    const UpdateTransactionCommand& command) = 0;
virtual Result<void> deleteTransaction(
    const DeleteTransactionCommand& command) = 0;
```

- [ ] **Step 5: Implement canonical payload and CRUD ordering**

The codec emits UTF-8 JSON with lexicographically fixed field order and proper escaping. An upsert has this exact outer shape:

```json
{"entityType":"transaction","entityId":"<uuid>","serverRevision":0,"entity":{"id":"<uuid>","type":"EXPENSE","status":"POSTED","amountMinor":12345,"currency":"CNY","occurredOn":"2026-09-04"}}
```

Include every nullable transaction field as either a JSON value or `null`, plus all metadata. A delete payload contains only `entityType`, `entityId`, and `deletedAt`; it does not repeat merchant/note/amount. Set `commandType` to `UPSERT_TRANSACTION` or `DELETE_TRANSACTION`, `payloadVersion=1`, one expectation `{transaction, id, stored serverRevision}`, and `createdAt` to the accepted entity `updatedAt` for create/update or `command.deletedAt` for delete.

Each service method executes exactly once through `IAccountingUnitOfWork`. Create requires no existing row, validates with repository-loaded references, inserts, and enqueues. Update requires a live existing row, replaces command metadata's `serverRevision` with the stored revision, validates, updates, and enqueues. Delete finds the live row, calls guarded `markDeleted(transactionId, deletedAt, modifiedByDeviceId)` first, and enqueues only after that succeeds. The UoW owns rollback; the service never retries with a new ID.

- [ ] **Step 6: Run focused and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_accounting_crud_tests
./build/cmake/linux-core/dailyaccount_accounting_application_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: transaction CRUD tests pass, the original application contract still passes, DAT reports 22 tests, and CTest has zero failures.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/modules/accounting/application \
  tests/unit/accounting_crud_tests.cpp \
  tests/unit/accounting_application_tests.cpp
git commit -m "feat: add guarded transaction commands"
```

Without authorization, do not commit.

---

### Task 4: Add Validated Account, Category, and Tag Application Writes

**Files:**
- Create: `tests/unit/catalog_crud_tests.cpp`
- Modify: `src/modules/accounting/application/accounting_commands.h`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.h`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.cpp`
- Modify: `src/modules/accounting/application/accounting_service.h`
- Modify: `src/modules/accounting/application/accounting_service.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/catalog_crud_tests.cpp`

**Interfaces:**
- Consumes: Task 2 account/category/tag validation and G1 row repositories.
- Produces: create/update/archive use cases for accounts, categories, and tags; stable outbox mutations; preset-category protection; and target archive semantics replacing physical category removal.

- [ ] **Step 1: Write focused catalog command tests**

Create fake repositories and include this target-behavior test:

```cpp
void archivingAnInUseCustomCategoryPreservesItsIdentity()
{
    Fixture fixture;
    Category category = validCategory();
    category.isPreset = false;
    fixture.categories.row = category;
    const auto mutationId = MutationId::random();

    const auto result = fixture.service.archiveCategory(
        ArchiveCategoryCommand{category.id, UtcInstant{1'788'480'000'000},
                               DeviceId::random(), mutationId});

    DA_CHECK(result.hasValue());
    DA_CHECK(fixture.categories.row->isArchived);
    DA_CHECK_EQ(fixture.categories.row->id, category.id);
    DA_CHECK_EQ(fixture.outbox.last.mutationId, mutationId);
    DA_CHECK_EQ(fixture.outbox.last.commandType, "UPSERT_CATEGORY");
}
```

Also test create/update account, category, and tag; archive account/tag; preset category archive rejection; missing parent rejection; unchanged server revision; duplicate ID rejection; invalid CNY/name failure before writes; and outbox failure rolling back each entity change.

- [ ] **Step 2: Run the new target red**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target dailyaccount_catalog_crud_tests --parallel 2
```

Expected: non-zero exit with an unknown target or compilation error for missing `ArchiveCategoryCommand`.

- [ ] **Step 3: Add exact command shapes**

Append these command templates and aliases:

```cpp
template <typename Entity>
struct CreateEntityCommand {
    Entity entity;
    MutationId mutationId;
};

template <typename Entity>
struct UpdateEntityCommand {
    Entity entity;
    MutationId mutationId;
};

template <typename Id>
struct ArchiveEntityCommand {
    Id id;
    UtcInstant archivedAt;
    DeviceId modifiedByDeviceId;
    MutationId mutationId;
};

using CreateAccountCommand = CreateEntityCommand<Account>;
using UpdateAccountCommand = UpdateEntityCommand<Account>;
using ArchiveAccountCommand = ArchiveEntityCommand<AccountId>;
using CreateCategoryCommand = CreateEntityCommand<Category>;
using UpdateCategoryCommand = UpdateEntityCommand<Category>;
using ArchiveCategoryCommand = ArchiveEntityCommand<CategoryId>;
using CreateTagCommand = CreateEntityCommand<Tag>;
using UpdateTagCommand = UpdateEntityCommand<Tag>;
using ArchiveTagCommand = ArchiveEntityCommand<TagId>;
```

Archiving sets `isArchived=true`, `updatedAt=archivedAt`, and `modifiedByDeviceId`; it does not set `deletedAt` and does not change identity.

- [ ] **Step 4: Add exact service methods and mutation names**

Add matching pure virtual and concrete methods:

```cpp
Result<Account> createAccount(const CreateAccountCommand&);
Result<Account> updateAccount(const UpdateAccountCommand&);
Result<Account> archiveAccount(const ArchiveAccountCommand&);
Result<Category> createCategory(const CreateCategoryCommand&);
Result<Category> updateCategory(const UpdateCategoryCommand&);
Result<Category> archiveCategory(const ArchiveCategoryCommand&);
Result<Tag> createTag(const CreateTagCommand&);
Result<Tag> updateTag(const UpdateTagCommand&);
Result<Tag> archiveTag(const ArchiveTagCommand&);
```

The codec emits `UPSERT_ACCOUNT`, `UPSERT_CATEGORY`, and `UPSERT_TAG`, payload version 1, one base-revision expectation, full after-state payloads, and `createdAt` equal to the accepted entity `updatedAt`. Create expects revision 0; update/archive use the stored revision. Category create/update loads its optional parent inside the same UoW before validation. Reject preset archive with `DomainConstraint` and message `preset categories cannot be archived`.

- [ ] **Step 5: Run focused and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_catalog_crud_tests
./build/cmake/linux-core/dailyaccount_accounting_crud_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: all catalog/transaction/application tests pass, DAT remains at 22 passes, and CTest has zero failures.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/modules/accounting/application \
  tests/unit/catalog_crud_tests.cpp
git commit -m "feat: add accounting catalog commands"
```

Without authorization, do not commit.

---

### Task 5: Implement the Serial QSQLITE Executor and Connection Lifecycle

**Files:**
- Create: `src/platform/database/module_db_executor.h`
- Create: `src/platform/database/module_db_executor.cpp`
- Create: `tests/integration/module_db_executor_tests.cpp`
- Modify: `cmake/DailyAccountOptions.cmake`
- Modify: `CMakeLists.txt`
- Modify: `CMakePresets.json`
- Test: `tests/integration/module_db_executor_tests.cpp`

**Interfaces:**
- Consumes: D-028's accepted mechanism, Qt 6.9.3 Core/Concurrent/SQL, G1 `Result<void>`, and a profile/module/database identity.
- Produces: `ModuleDbExecutor`, one worker-owned QSQLITE connection named `da/<profile>/<module>/<sequence>`, asynchronous queueing, blocking execution for non-UI application workers, bounded busy behavior, consistent backup, and deterministic drain/close.

- [ ] **Step 1: Write the executor lifecycle test first**

The focused test records thread IDs and Qt SQL warnings:

```cpp
void queuedOperationsUseOneWorkerAndConnectionIsRemovedOnDrain()
{
    TemporaryDirectory directory("executor");
    ModuleDbExecutor executor({"profile-a", "accounting",
                               directory.path() / "accounting.sqlite", false});
    DA_CHECK(executor.open().hasValue());
    const QString connectionName = executor.connectionNameForTesting();
    std::set<Qt::HANDLE> threads;
    std::vector<QFuture<Result<void>>> jobs;
    for (int i = 0; i < 8; ++i) {
        jobs.push_back(executor.enqueue([&threads](QSqlDatabase& database) {
            threads.insert(QThread::currentThreadId());
            DA_CHECK(database.isOpen());
            return Result<void>::success();
        }));
    }
    for (auto& job : jobs) DA_CHECK(job.result().hasValue());
    DA_CHECK_EQ(threads.size(), std::size_t{1});
    DA_CHECK(executor.drainAndClose().hasValue());
    DA_CHECK(!QSqlDatabase::contains(connectionName));
}
```

Add cases for 8 producers/8,000 writes, `foreign_keys` rejecting an orphan, mandatory pragma read-back, a second writer timing out in 4,500-6,500 ms while the UI heartbeat fires, jobs rejected after drain starts, queued jobs committed before close, and no `QSqlDatabasePrivate::removeDatabase: connection ... is still in use` warning.

- [ ] **Step 2: Run the test red on the accepted Windows Qt environment**

```powershell
$env:QT_DIR = 'D:\tools\Qt\6.9.3\mingw_64'
$env:MINGW_DIR = 'D:\tools\mingw64\bin'
$env:Path = "$env:QT_DIR\bin;$env:MINGW_DIR;$env:Path"
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_module_db_executor_tests --parallel 2
```

Expected: non-zero exit because the target/header does not exist.

- [ ] **Step 3: Define the production executor API**

Use this exact header surface:

```cpp
namespace dailyaccount {

struct ModuleDbOptions {
    std::string profileIdentity;
    std::string moduleId;
    std::filesystem::path databasePath;
    bool readOnly;
};

using SqlOperation = std::function<Result<void>(QSqlDatabase&)>;

class ModuleDbExecutor final : public QObject {
    Q_OBJECT
public:
    explicit ModuleDbExecutor(ModuleDbOptions options, QObject* parent = nullptr);
    ~ModuleDbExecutor() override;

    Result<void> open();
    QFuture<Result<void>> enqueue(SqlOperation operation);
    Result<void> executeBlocking(SqlOperation operation);
    QFuture<Result<void>> backupTo(const std::filesystem::path& finalPath);
    Result<void> drainAndClose();
    bool isAcceptingJobs() const noexcept;
    QString connectionNameForTesting() const;

private:
    class Worker;
    std::unique_ptr<Worker> worker_;
    QThread workerThread_;
};

}
```

The public header may include Qt Core/SQL because it belongs to the outward database adapter. No caller may retain the `QSqlDatabase&` after its callback returns.

- [ ] **Step 4: Implement queue ownership and mandatory pragmas**

Create the worker connection only after the worker object is on `workerThread_`. Before `open`, set connect option `QSQLITE_BUSY_TIMEOUT=5000`; after `open`, execute and read back:

```sql
PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;
PRAGMA synchronous = FULL;
PRAGMA busy_timeout = 5000;
```

Reject open unless the driver is exactly `QSQLITE`, every mandatory value reads back, and the database path's parent already exists. Generate connection names with a process-local atomic sequence:

```cpp
"da/" + profileIdentity + "/" + moduleId + "/" + std::to_string(sequence)
```

`enqueue` rejects empty jobs and jobs submitted after drain begins. `executeBlocking` directly invokes the operation when already on the worker thread; otherwise it waits for `enqueue` and must never be called from the GUI thread. `drainAndClose` atomically rejects new work, executes already queued jobs, destroys callback/query locals, closes and resets the worker's database handle, calls `QSqlDatabase::removeDatabase(name)`, then quits and joins the thread.

- [ ] **Step 5: Implement the D-028-selected backup behind the queue**

The API and `.partial` protocol are identical for either accepted D-028 outcome:

```text
final path:       <requested path>
temporary path:   <requested path>.partial
validation:       read-only connection on a separate validation thread
acceptance:       integrity_check=ok, expected user_version, identity, row counts
activation:       same-directory atomic rename from .partial to final
```

If D-028 selected `VACUUM INTO`, bind the generated `.partial` path through the tested filename-expression helper and execute `VACUUM INTO` after all prior jobs. If D-028 selected the vendored Online Backup API, use the exact accepted SQLite source/version/hash from D-028 and bounded `sqlite3_backup_step` calls; do not recover a native handle from a connection on another thread. Any mismatch between D-028 and the implementation stops this task.

- [ ] **Step 6: Add the Qt SQL build option without affecting Linux core**

Add:

```cmake
option(DA_BUILD_SQLITE "Build the Qt SQL persistence adapters" OFF)
```

Set `DA_BUILD_SQLITE=ON` in `windows-desktop`; retain `OFF` in both Linux core presets. Under `if(DA_BUILD_SQLITE)`, require `find_package(Qt6 6.9.3 EXACT REQUIRED COMPONENTS Core Concurrent Sql Test)`, create `dailyaccount_platform_database`, and link Qt privately. Keep the Qt-free Linux graph unchanged.

- [ ] **Step 7: Run focused Windows and cumulative Linux checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_module_db_executor_tests.exe'
ctest --preset windows-desktop -R 'module_db_executor' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: all executor cases pass with measured busy duration in range and no connection warning; Linux still does not search for Qt and DAT reports 22 passes.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt CMakePresets.json cmake/DailyAccountOptions.cmake \
  src/platform/database/module_db_executor.* \
  tests/integration/module_db_executor_tests.cpp
git commit -m "feat: add serial qsqlite executor"
```

Without authorization, do not commit.

---

### Task 6: Add the Local Profile Store and Per-Profile Module Directories

**Files:**
- Create: `src/platform/profile/local_profile.h`
- Create: `src/platform/profile/profile_store.h`
- Create: `src/platform/profile/profile_store.cpp`
- Create: `src/platform/profile/profile_directory_locator.h`
- Create: `src/platform/profile/profile_directory_locator.cpp`
- Create: `src/platform/profile/migrations/001_profiles.sql`
- Create: `tests/integration/profile_store_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/profile_store_tests.cpp`

**Interfaces:**
- Consumes: D-023 profile states/bindings, G1 `IModuleDatabaseLocator`, typed IDs, and Task 5's separate platform QSQLITE executor.
- Produces: `profiles.sqlite`; immutable `ProfileId` and local ledger owner identity; local profile CRUD/state transitions; unique remote binding; and canonical `users/<profile>/platform.sqlite`, `accounting.sqlite`, and `backups/` locations.

- [ ] **Step 1: Write isolation and transition tests**

The first focused test asserts exact paths:

```cpp
void twoProfilesReceiveDifferentModuleDirectories()
{
    TemporaryDirectory root("profiles");
    ProfileDirectoryLocator locator(root.path());
    const auto first = ProfileId::parse(
        "aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa").value();
    const auto second = ProfileId::parse(
        "bbbbbbbb-bbbb-4bbb-8bbb-bbbbbbbbbbbb").value();

    DA_CHECK(locator.ensureProfileLayout(first).hasValue());
    DA_CHECK(locator.ensureProfileLayout(second).hasValue());
    DA_CHECK_EQ(locator.locate(first, "accounting").value().databasePath,
                root.path() / "users" / first.toString() / "accounting.sqlite");
    DA_CHECK_EQ(locator.locate(second, "platform").value().databasePath,
                root.path() / "users" / second.toString() / "platform.sqlite");
    DA_CHECK(locator.locate(first, "accounting").value().databasePath !=
             locator.locate(second, "accounting").value().databasePath);
}
```

Also test `profiles.sqlite` remains at the root, labels are non-empty and at most 128 UTF-8 bytes, state strings round-trip, invalid state transitions fail, duplicate `(providerId, remoteUserId)` fails, an existing profile cannot rebind, module IDs reject path separators/dots, and `locate` never creates a database file.

- [ ] **Step 2: Run the target red on Windows**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_profile_store_tests --parallel 2
```

Expected: unknown target or missing `platform/profile/local_profile.h`.

- [ ] **Step 3: Define profile values and exact store API**

Use:

```cpp
namespace dailyaccount {

enum class LocalProfileState {
    LocalUnbound,
    Initializing,
    Active,
    SignedOutRetained,
    RecoveryReadOnly,
    LocalDeletePending
};

struct LocalProfile {
    ProfileId profileId;
    UserId localLedgerOwnerId;
    std::string displayLabel;
    std::optional<std::string> maskedEmail;
    LocalProfileState state;
    std::optional<std::string> providerId;
    std::optional<UserId> remoteUserId;
    std::string timeZoneId;
    UtcInstant createdAt;
    UtcInstant updatedAt;
};

struct CreateLocalProfileRequest {
    ProfileId profileId;
    UserId localLedgerOwnerId;
    std::string displayLabel;
    std::string timeZoneId;
    UtcInstant now;
};

class ProfileStore final {
public:
    explicit ProfileStore(ModuleDbExecutor& executor);
    Result<void> migrate();
    Result<LocalProfile> create(const CreateLocalProfileRequest& request);
    Result<std::optional<LocalProfile>> find(const ProfileId& profileId);
    Result<std::vector<LocalProfile>> list();
    Result<void> transition(
        const ProfileId& profileId,
        LocalProfileState expected,
        LocalProfileState next,
        UtcInstant now);
    Result<void> bindRemote(
        const ProfileId& profileId,
        std::string providerId,
        UserId remoteUserId,
        std::string maskedEmail,
        UtcInstant now);
};

}
```

`localLedgerOwnerId` is a random per-profile local identity used to populate Stage 2 in-memory `EntityMetadata` in an unbound profile. It is not an authenticated server identity; Stage 4 derives remote ownership only from `AuthSession.userId` and never trusts this local value in a network payload.

- [ ] **Step 4: Implement the strict profile schema**

Embed and apply `001_profiles.sql` through a Qt resource. Its core DDL is:

```sql
-- DA_STATEMENT
CREATE TABLE profile_schema_migrations (
    version INTEGER PRIMARY KEY CHECK(version > 0),
    name TEXT NOT NULL UNIQUE,
    sha256 TEXT NOT NULL CHECK(length(sha256) = 64),
    applied_at_ms INTEGER NOT NULL
) STRICT;
-- DA_STATEMENT
CREATE TABLE local_profiles (
    profile_id TEXT PRIMARY KEY CHECK(length(profile_id) = 36),
    local_ledger_owner_id TEXT NOT NULL UNIQUE CHECK(length(local_ledger_owner_id) = 36),
    display_label TEXT NOT NULL CHECK(length(CAST(display_label AS BLOB)) BETWEEN 1 AND 128),
    masked_email TEXT,
    state TEXT NOT NULL CHECK(state IN (
        'LOCAL_UNBOUND','INITIALIZING','ACTIVE','SIGNED_OUT_RETAINED',
        'RECOVERY_READ_ONLY','LOCAL_DELETE_PENDING')),
    provider_id TEXT,
    remote_user_id TEXT CHECK(remote_user_id IS NULL OR length(remote_user_id) = 36),
    time_zone_id TEXT NOT NULL CHECK(length(CAST(time_zone_id AS BLOB)) BETWEEN 1 AND 255),
    created_at_ms INTEGER NOT NULL,
    updated_at_ms INTEGER NOT NULL,
    CHECK((provider_id IS NULL) = (remote_user_id IS NULL))
) STRICT;
-- DA_STATEMENT
CREATE UNIQUE INDEX local_profiles_remote_binding
ON local_profiles(provider_id, remote_user_id)
WHERE provider_id IS NOT NULL;
```

Allowed transitions in this stage are `LOCAL_UNBOUND -> INITIALIZING -> ACTIVE`, `ACTIVE -> SIGNED_OUT_RETAINED`, `SIGNED_OUT_RETAINED -> ACTIVE` only after Stage 4 authentication, any retained state to `LOCAL_DELETE_PENDING`, and explicit creation of `RECOVERY_READ_ONLY`. Reject every other edge atomically.

- [ ] **Step 5: Implement safe path derivation**

Expose:

```cpp
struct ProfileDirectoryLayout {
    std::filesystem::path root;
    std::filesystem::path userDirectory;
    std::filesystem::path platformDatabase;
    std::filesystem::path accountingDatabase;
    std::filesystem::path backupsDirectory;
};

class ProfileDirectoryLocator final : public IModuleDatabaseLocator {
public:
    explicit ProfileDirectoryLocator(std::filesystem::path applicationDataRoot);
    Result<ProfileDirectoryLayout> ensureProfileLayout(ProfileId profileId) const;
    Result<ModuleDatabaseLocation> locate(
        ProfileId ownerProfileId,
        std::string_view moduleId) const override;
    std::filesystem::path profilesDatabasePath() const;
};
```

Accept module IDs matching ASCII `[a-z][a-z0-9-]{0,63}`. `platform` maps to `platform.sqlite`; every other valid module maps to `<moduleId>.sqlite`. Create only `users/<uuid>/` and `backups/` in `ensureProfileLayout`; `locate` is side-effect free. Never place a user-supplied label/email in a path.

- [ ] **Step 6: Run focused and cumulative checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_profile_store_tests.exe'
ctest --preset windows-desktop -R 'profile_store|module_db_executor' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: profile isolation/state tests pass on Windows, Linux core remains Qt-free, and DAT remains at 22 passes.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/platform/profile \
  tests/integration/profile_store_tests.cpp
git commit -m "feat: add isolated local profiles"
```

Without authorization, do not commit.

---

### Task 7: Add Forward Accounting Schema Migrations with Database Constraints

**Files:**
- Create: `src/modules/accounting/data/sqlite/accounting_migration_runner.h`
- Create: `src/modules/accounting/data/sqlite/accounting_migration_runner.cpp`
- Create: `src/modules/accounting/data/sqlite/accounting_database.h`
- Create: `src/modules/accounting/data/sqlite/accounting_database.cpp`
- Create: `src/modules/accounting/data/sqlite/migrations/001_initial.sql`
- Create: `src/modules/accounting/data/sqlite/migrations/002_recurring.sql`
- Create: `tests/integration/sqlite_schema_tests.cpp`
- Modify: `src/modules/accounting/accounting_module.cpp`
- Modify: `tests/registry_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/sqlite_schema_tests.cpp`

**Interfaces:**
- Consumes: Task 5 executor, Task 6 profile identity/location, G1 module manifest, and SQLite `STRICT` support proven by D-028.
- Produces: accounting schema version 2, migration hash/history verification, owner checks before business queries, rejection of newer schemas, and database-level FK/CHECK/UNIQUE/occurrence-link protections.

- [ ] **Step 1: Write schema and rollback integration tests**

The focused test must execute raw bound statements against a migrated temporary database and assert exact SQLite rejection classes:

```cpp
void strictSchemaRejectsInvalidRowsAndPreservesVersionOnFailedMigration()
{
    auto fixture = AccountingDatabaseFixture::createAtVersion(0);
    DA_CHECK(fixture.migrateToLatest().hasValue());
    DA_CHECK_EQ(fixture.userVersion(), 2);
    DA_CHECK_EQ(fixture.scalar("PRAGMA integrity_check"), "ok");
    DA_CHECK(fixture.tableIsStrict("transactions"));
    DA_CHECK(fixture.insertOrphanTransaction().error().message.find(
                 "FOREIGN KEY constraint failed") != std::string::npos);
    DA_CHECK(fixture.insertZeroAmount().error().message.find(
                 "CHECK constraint failed") != std::string::npos);
    DA_CHECK(fixture.insertDuplicateMutationId().error().message.find(
                 "UNIQUE constraint failed") != std::string::npos);
}
```

Add tests for fresh 0->1->2 migration, reopening version 2 without changes, a migration checksum mismatch, injected failure in migration 2 leaving version 1/data logically unchanged, `foreign_key_check`, future version 3 returning `UpgradeRequired`, owner-profile mismatch failing before a transaction query, every transaction type/status field matrix, duplicate category/tag names, duplicate `(rule_id, period_key)`, and recurring status/transaction-null constraints.

- [ ] **Step 2: Run the schema target red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_sqlite_schema_tests --parallel 2
```

Expected: unknown target or missing `accounting_migration_runner.h`.

- [ ] **Step 3: Define migration/database APIs**

Use:

```cpp
namespace dailyaccount {

struct AccountingDatabaseIdentity {
    ProfileId ownerProfileId;
    UserId localLedgerOwnerId;
};

class AccountingMigrationRunner final {
public:
    static constexpr std::uint32_t kLatestSchemaVersion = 2;
    Result<void> applyPending(
        QSqlDatabase& database,
        const AccountingDatabaseIdentity& identity,
        UtcInstant appliedAt);
};

class AccountingDatabase final {
public:
    static Result<void> createOrUpgradeStaging(
        const std::filesystem::path& stagingPath,
        const AccountingDatabaseIdentity& identity,
        UtcInstant now);
    static Result<void> validate(
        const std::filesystem::path& databasePath,
        const AccountingDatabaseIdentity& expectedIdentity,
        std::uint32_t expectedVersion);
};

}
```

Split scripts only at a line exactly equal to `-- DA_STATEMENT`; never split arbitrary semicolons. For each version, `BEGIN IMMEDIATE`, execute all statements, insert `(version,name,sha256,applied_at_ms)`, set `PRAGMA user_version=<version>`, and commit. Roll back on the first error. Compare every already-applied resource SHA-256 before running pending migrations.

- [ ] **Step 4: Implement migration 001 with strict base tables**

`001_initial.sql` creates `schema_migrations`, `database_identity`, `accounts`, `categories`, `tags`, `transactions`, `transaction_tags`, `outbox`, `legacy_import_markers`, and `legacy_id_map` as `STRICT`. Use these exact core constraints and columns:

```sql
-- DA_STATEMENT
CREATE TABLE database_identity (
    singleton INTEGER PRIMARY KEY CHECK(singleton = 1),
    owner_profile_id TEXT NOT NULL CHECK(length(owner_profile_id) = 36),
    local_ledger_owner_id TEXT NOT NULL CHECK(length(local_ledger_owner_id) = 36),
    module_id TEXT NOT NULL CHECK(module_id = 'accounting'),
    created_at_ms INTEGER NOT NULL,
    sqlite_writes_started_at_ms INTEGER
        CHECK(sqlite_writes_started_at_ms IS NULL OR sqlite_writes_started_at_ms >= 0)
) STRICT;
-- DA_STATEMENT
CREATE TABLE accounts (
    id TEXT PRIMARY KEY CHECK(length(id) = 36),
    name TEXT NOT NULL CHECK(length(CAST(name AS BLOB)) BETWEEN 1 AND 128),
    type TEXT NOT NULL CHECK(type IN ('CASH','BANK_CARD','ELECTRONIC_WALLET','CREDIT','OTHER')),
    currency TEXT NOT NULL CHECK(currency = 'CNY'),
    opening_balance_minor INTEGER NOT NULL
        CHECK(opening_balance_minor BETWEEN -9999999999 AND 9999999999),
    opening_balance_on TEXT NOT NULL CHECK(length(opening_balance_on) = 10),
    is_archived INTEGER NOT NULL CHECK(is_archived IN (0,1)),
    created_at_ms INTEGER NOT NULL,
    updated_at_ms INTEGER NOT NULL,
    server_revision INTEGER NOT NULL CHECK(server_revision >= 0),
    deleted_at_ms INTEGER,
    modified_by_device_id TEXT NOT NULL CHECK(length(modified_by_device_id) = 36),
    local_state TEXT NOT NULL CHECK(local_state IN ('CLEAN','DIRTY','IN_FLIGHT','ISOLATED'))
) STRICT;
-- DA_STATEMENT
CREATE TABLE categories (
    id TEXT PRIMARY KEY CHECK(length(id) = 36),
    name TEXT NOT NULL CHECK(length(CAST(name AS BLOB)) BETWEEN 1 AND 4096),
    parent_id TEXT REFERENCES categories(id) ON DELETE RESTRICT,
    applies_to TEXT NOT NULL CHECK(applies_to IN ('INCOME','EXPENSE','BOTH')),
    sort_order INTEGER NOT NULL,
    color TEXT NOT NULL,
    icon TEXT NOT NULL,
    is_preset INTEGER NOT NULL CHECK(is_preset IN (0,1)),
    is_archived INTEGER NOT NULL CHECK(is_archived IN (0,1)),
    created_at_ms INTEGER NOT NULL,
    updated_at_ms INTEGER NOT NULL,
    server_revision INTEGER NOT NULL CHECK(server_revision >= 0),
    deleted_at_ms INTEGER,
    modified_by_device_id TEXT NOT NULL CHECK(length(modified_by_device_id) = 36),
    local_state TEXT NOT NULL CHECK(local_state IN ('CLEAN','DIRTY','IN_FLIGHT','ISOLATED')),
    CHECK(parent_id IS NULL OR parent_id <> id)
) STRICT;
-- DA_STATEMENT
CREATE UNIQUE INDEX categories_live_unique_name
ON categories(COALESCE(parent_id, ''), name, applies_to)
WHERE deleted_at_ms IS NULL;
-- DA_STATEMENT
CREATE TABLE tags (
    id TEXT PRIMARY KEY CHECK(length(id) = 36),
    name TEXT NOT NULL CHECK(length(CAST(name AS BLOB)) BETWEEN 1 AND 128),
    is_archived INTEGER NOT NULL CHECK(is_archived IN (0,1)),
    created_at_ms INTEGER NOT NULL,
    updated_at_ms INTEGER NOT NULL,
    server_revision INTEGER NOT NULL CHECK(server_revision >= 0),
    deleted_at_ms INTEGER,
    modified_by_device_id TEXT NOT NULL CHECK(length(modified_by_device_id) = 36),
    local_state TEXT NOT NULL CHECK(local_state IN ('CLEAN','DIRTY','IN_FLIGHT','ISOLATED'))
) STRICT;
-- DA_STATEMENT
CREATE UNIQUE INDEX tags_live_unique_name
ON tags(name) WHERE deleted_at_ms IS NULL;
```

Use this transaction table contract:

```sql
-- DA_STATEMENT
CREATE TABLE transactions (
    id TEXT PRIMARY KEY CHECK(length(id) = 36),
    type TEXT NOT NULL CHECK(type IN ('EXPENSE','INCOME','TRANSFER','REFUND')),
    status TEXT NOT NULL CHECK(status IN ('PENDING','POSTED')),
    amount_minor INTEGER NOT NULL CHECK(amount_minor BETWEEN 1 AND 9999999999),
    currency TEXT NOT NULL CHECK(currency = 'CNY'),
    occurred_on TEXT NOT NULL CHECK(length(occurred_on) = 10),
    occurred_time TEXT,
    time_zone_id TEXT,
    category_id TEXT REFERENCES categories(id) ON DELETE RESTRICT,
    account_id TEXT REFERENCES accounts(id) ON DELETE RESTRICT,
    destination_account_id TEXT REFERENCES accounts(id) ON DELETE RESTRICT,
    merchant TEXT CHECK(merchant IS NULL OR length(CAST(merchant AS BLOB)) <= 256),
    note TEXT CHECK(note IS NULL OR length(CAST(note AS BLOB)) <= 2048),
    origin TEXT NOT NULL CHECK(origin IN ('MANUAL','TEXT_IMPORT','BILL_IMPORT','RECURRING')),
    origin_ref TEXT,
    refund_of_id TEXT REFERENCES transactions(id) ON DELETE RESTRICT,
    created_at_ms INTEGER NOT NULL,
    updated_at_ms INTEGER NOT NULL,
    server_revision INTEGER NOT NULL CHECK(server_revision >= 0),
    deleted_at_ms INTEGER,
    modified_by_device_id TEXT NOT NULL CHECK(length(modified_by_device_id) = 36),
    local_state TEXT NOT NULL CHECK(local_state IN ('CLEAN','DIRTY','IN_FLIGHT','ISOLATED')),
    CHECK((occurred_time IS NULL) = (time_zone_id IS NULL)),
    CHECK(type <> 'INCOME' OR status = 'POSTED'),
    CHECK(status <> 'PENDING' OR (type = 'EXPENSE' AND origin = 'RECURRING')),
    CHECK(
        (type IN ('EXPENSE','INCOME') AND destination_account_id IS NULL AND refund_of_id IS NULL)
        OR (type = 'TRANSFER' AND account_id IS NOT NULL AND destination_account_id IS NOT NULL
            AND account_id <> destination_account_id AND category_id IS NULL
            AND refund_of_id IS NULL AND status = 'POSTED')
        OR (type = 'REFUND' AND account_id IS NOT NULL AND destination_account_id IS NULL
            AND category_id IS NOT NULL AND refund_of_id IS NOT NULL AND status = 'POSTED')
    )
) STRICT;
-- DA_STATEMENT
CREATE INDEX transactions_occurred_on ON transactions(occurred_on, id);
-- DA_STATEMENT
CREATE INDEX transactions_category ON transactions(category_id, occurred_on);
-- DA_STATEMENT
CREATE TABLE transaction_tags (
    transaction_id TEXT NOT NULL REFERENCES transactions(id) ON DELETE CASCADE,
    tag_id TEXT NOT NULL REFERENCES tags(id) ON DELETE RESTRICT,
    PRIMARY KEY(transaction_id, tag_id)
) STRICT;
```

Use this local outbox contract:

```sql
-- DA_STATEMENT
CREATE TABLE outbox (
    local_sequence INTEGER PRIMARY KEY AUTOINCREMENT,
    mutation_id TEXT NOT NULL UNIQUE CHECK(length(mutation_id) = 36),
    command_type TEXT NOT NULL CHECK(length(command_type) BETWEEN 1 AND 64),
    payload_version INTEGER NOT NULL CHECK(payload_version > 0),
    expectations_json TEXT NOT NULL CHECK(length(CAST(expectations_json AS BLOB)) <= 262144),
    payload_json TEXT NOT NULL CHECK(length(CAST(payload_json AS BLOB)) <= 1048576),
    state TEXT NOT NULL CHECK(state IN ('UNSENT','IN_FLIGHT')),
    created_at_ms INTEGER NOT NULL,
    in_flight_at_ms INTEGER,
    attempt_count INTEGER NOT NULL DEFAULT 0 CHECK(attempt_count >= 0),
    CHECK((state = 'UNSENT' AND in_flight_at_ms IS NULL)
       OR (state = 'IN_FLIGHT' AND in_flight_at_ms IS NOT NULL))
) STRICT;
```

`legacy_import_markers` is keyed by `source_sha256`, stores target profile, ID namespace, mapping version 1, record/category counts, income/expense totals, min/max dates, and completion time. `legacy_id_map` has `source_sha256`, `legacy_kind`, `legacy_key`, `entity_type`, `entity_id`, with unique `(source_sha256,legacy_kind,legacy_key)` and unique `(entity_type,entity_id)` constraints.

- [ ] **Step 5: Implement migration 002 for recurring data and deletion protection**

Create `recurring_rules` and `recurring_occurrences` as `STRICT`, including all G1 fields/metadata. The mandatory constraints are:

```sql
-- DA_STATEMENT
CREATE TABLE recurring_rules (
    id TEXT PRIMARY KEY CHECK(length(id) = 36),
    name TEXT NOT NULL CHECK(length(CAST(name AS BLOB)) BETWEEN 1 AND 256),
    merchant TEXT CHECK(merchant IS NULL OR length(CAST(merchant AS BLOB)) <= 256),
    category_id TEXT REFERENCES categories(id) ON DELETE RESTRICT,
    account_id TEXT REFERENCES accounts(id) ON DELETE RESTRICT,
    note TEXT CHECK(note IS NULL OR length(CAST(note AS BLOB)) <= 2048),
    marker TEXT CHECK(marker IS NULL OR length(CAST(marker AS BLOB)) <= 128),
    expected_amount_minor INTEGER NOT NULL CHECK(expected_amount_minor BETWEEN 1 AND 9999999999),
    tolerance_minor INTEGER NOT NULL CHECK(tolerance_minor BETWEEN 0 AND 9999999999),
    currency TEXT NOT NULL CHECK(currency = 'CNY'),
    frequency_spec TEXT NOT NULL,
    next_due_on TEXT NOT NULL CHECK(length(next_due_on) = 10),
    time_zone_id TEXT NOT NULL CHECK(length(CAST(time_zone_id AS BLOB)) BETWEEN 1 AND 255),
    lead_days INTEGER NOT NULL CHECK(lead_days IN (1,2)),
    starts_on TEXT NOT NULL CHECK(length(starts_on) = 10),
    ends_on TEXT CHECK(ends_on IS NULL OR length(ends_on) = 10),
    short_month_policy TEXT NOT NULL CHECK(short_month_policy = 'CLAMP_TO_LAST_DAY'),
    enabled INTEGER NOT NULL CHECK(enabled IN (0,1)),
    created_at_ms INTEGER NOT NULL,
    updated_at_ms INTEGER NOT NULL,
    server_revision INTEGER NOT NULL CHECK(server_revision >= 0),
    deleted_at_ms INTEGER,
    modified_by_device_id TEXT NOT NULL CHECK(length(modified_by_device_id) = 36),
    local_state TEXT NOT NULL CHECK(local_state IN ('CLEAN','DIRTY','IN_FLIGHT','ISOLATED')),
    CHECK(ends_on IS NULL OR ends_on >= starts_on)
) STRICT;
-- DA_STATEMENT
CREATE TABLE recurring_occurrences (
    id TEXT PRIMARY KEY CHECK(length(id) = 36),
    rule_id TEXT NOT NULL REFERENCES recurring_rules(id) ON DELETE RESTRICT,
    period_key TEXT NOT NULL CHECK(length(CAST(period_key AS BLOB)) BETWEEN 4 AND 16),
    status TEXT NOT NULL CHECK(status IN ('PENDING','POSTED','SKIPPED','CANCELLED')),
    deferred_until TEXT CHECK(deferred_until IS NULL OR length(deferred_until) = 10),
    transaction_id TEXT UNIQUE REFERENCES transactions(id) ON DELETE RESTRICT,
    expected_on TEXT NOT NULL CHECK(length(expected_on) = 10),
    expected_amount_minor INTEGER NOT NULL CHECK(expected_amount_minor BETWEEN 1 AND 9999999999),
    rule_snapshot_json TEXT NOT NULL CHECK(length(CAST(rule_snapshot_json AS BLOB)) BETWEEN 2 AND 65536),
    created_at_ms INTEGER NOT NULL,
    updated_at_ms INTEGER NOT NULL,
    server_revision INTEGER NOT NULL CHECK(server_revision >= 0),
    deleted_at_ms INTEGER,
    modified_by_device_id TEXT NOT NULL CHECK(length(modified_by_device_id) = 36),
    local_state TEXT NOT NULL CHECK(local_state IN ('CLEAN','DIRTY','IN_FLIGHT','ISOLATED')),
    UNIQUE(rule_id, period_key),
    CHECK((status IN ('PENDING','POSTED') AND transaction_id IS NOT NULL)
       OR (status IN ('SKIPPED','CANCELLED') AND transaction_id IS NULL))
) STRICT;
-- DA_STATEMENT
CREATE TRIGGER transactions_prevent_linked_tombstone
BEFORE UPDATE OF deleted_at_ms ON transactions
FOR EACH ROW
WHEN OLD.deleted_at_ms IS NULL AND NEW.deleted_at_ms IS NOT NULL
 AND EXISTS (
     SELECT 1 FROM recurring_occurrences occurrence
     WHERE occurrence.transaction_id = OLD.id
       AND occurrence.deleted_at_ms IS NULL
       AND occurrence.status IN ('PENDING','POSTED'))
BEGIN
    SELECT RAISE(ABORT, 'DA_OCCURRENCE_LINKED');
END;
```

Update `accountingModuleDescriptor().databaseSchemaVersion` and its registry assertion from 1 to 2.

- [ ] **Step 6: Validate staging databases before activation**

`AccountingDatabase::validate` opens a unique read-only validation connection on its own thread and requires: exact owner/profile/module identity, exact `user_version=2`, applied migration names/hashes, `integrity_check=ok`, empty `foreign_key_check`, and no `-wal`/`-shm` handle left open after close. It returns `UpgradeRequired` for a newer schema and `StorageFailure` for every other mismatch.

- [ ] **Step 7: Run focused Windows and cumulative Linux checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_sqlite_schema_tests.exe'
ctest --preset windows-desktop -R 'sqlite_schema|registry|architecture' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: all schema/constraint/version tests pass, registry advertises schema 2, shared-boundary tests still pass, and DAT reports 22 passes.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/modules/accounting/accounting_module.cpp \
  src/modules/accounting/data/sqlite tests/integration/sqlite_schema_tests.cpp \
  tests/registry_tests.cpp
git commit -m "feat: add strict accounting schema migrations"
```

Without authorization, do not commit.

---

### Task 8: Implement SQLite Repositories and the Transactional Unit of Work

**Files:**
- Create: `src/modules/accounting/data/sqlite/accounting_unit_of_work.h`
- Create: `src/modules/accounting/data/sqlite/accounting_unit_of_work.cpp`
- Create: `tests/integration/sqlite_unit_of_work_tests.cpp`
- Modify: `src/modules/accounting/data/sqlite/accounting_database.h`
- Modify: `src/modules/accounting/data/sqlite/accounting_database.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/sqlite_unit_of_work_tests.cpp`

**Interfaces:**
- Consumes: G1 repository/UoW interfaces, Task 5 executor, Task 7 schema, and Tasks 3-4 services.
- Produces: bound row mappers/repositories scoped to one transaction; `SqliteAccountingUnitOfWork`; atomic business/outbox writes; local dirty state; tombstones; and SQL error mapping including `DA_OCCURRENCE_LINKED`.

- [ ] **Step 1: Write the atomic outbox test first**

Use a real temporary QSQLITE database and the real application service:

```cpp
void outboxFailureRollsBackTheBusinessInsert()
{
    Fixture fixture;
    const MutationId duplicate = MutationId::parse(
        "99999999-9999-4999-8999-999999999999").value();
    DA_CHECK(fixture.seedOutbox(duplicate).hasValue());
    auto command = validCreateTransactionCommand();
    command.mutationId = duplicate;

    const auto result = fixture.service.createTransaction(command);

    DA_CHECK(!result.hasValue());
    DA_CHECK_EQ(fixture.scalarInt("SELECT count(*) FROM transactions"), 0);
    DA_CHECK_EQ(fixture.scalarInt("SELECT count(*) FROM outbox"), 1);
}
```

Add success assertions that one transaction and one outbox row commit; the first successful application enqueue sets `database_identity.sqlite_writes_started_at_ms` to the mutation timestamp; an outbox failure leaves that marker null; update preserves `server_revision`; a failing callback rolls back; FK/CHECK errors map to `DomainConstraint`; a missing row maps to `NotFound`; duplicate mutation maps to `StorageFailure`; ordinary queries omit tombstones; and a live recurring link maps to `OccurrenceLinked` with no tombstone/outbox.

- [ ] **Step 2: Run the target red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_sqlite_unit_of_work_tests --parallel 2
```

Expected: unknown target or missing `accounting_unit_of_work.h`.

- [ ] **Step 3: Define the production UoW surface**

Use:

```cpp
namespace dailyaccount {

class SqliteAccountingUnitOfWork final : public IAccountingUnitOfWork {
public:
    SqliteAccountingUnitOfWork(
        ModuleDbExecutor& executor,
        ProfileId ownerProfileId,
        UserId localLedgerOwnerId);

    Result<void> execute(
        const std::function<Result<void>(AccountingRepositories&)>& operation) override;

private:
    ModuleDbExecutor& executor_;
    ProfileId ownerProfileId_;
    UserId localLedgerOwnerId_;
};

}
```

Concrete repository classes remain private to `accounting_unit_of_work.cpp`; no `QSqlDatabase` appears in an application header.

- [ ] **Step 4: Implement one transaction and bound row mapping**

`execute` calls `executor_.executeBlocking`, verifies `database_identity` before any business query, executes `BEGIN IMMEDIATE`, creates stack-scoped transaction/account/category/tag/recurring/import/outbox repositories sharing that connection, invokes the callback once, rolls back on failure/exception, and commits once on success. Convert exceptions to `StorageFailure` after rollback.

Every repository statement binds all values. The three-argument G1 tombstone contract supplies the device value used here:

```cpp
query.prepare("UPDATE transactions SET deleted_at_ms=:deleted_at, "
              "updated_at_ms=:updated_at, modified_by_device_id=:device, "
              "local_state='DIRTY' WHERE id=:id AND deleted_at_ms IS NULL");
query.bindValue(":deleted_at", deletedAt.epochMilliseconds);
query.bindValue(":updated_at", deletedAt.epochMilliseconds);
query.bindValue(":device", modifiedBy.toString().c_str());
query.bindValue(":id", id.toString().c_str());
```

Read operations add `deleted_at_ms IS NULL` unless an internal migration/diagnostic method explicitly requests tombstones. Row mappers parse every enum, UUID, date/time, currency, and optional field and fail closed on malformed stored data.

- [ ] **Step 5: Persist outbox expectations and map the recurring guard**

Serialize `expectations` as a canonical JSON array sorted by `(entityType,entityId)` and insert `state='UNSENT'`, `attempt_count=0`, and `created_at_ms=mutation.createdAt.epochMilliseconds`. In the same transaction, `SqliteOutboxRepository::enqueue` sets `database_identity.sqlite_writes_started_at_ms=COALESCE(sqlite_writes_started_at_ms, :created_at)`; DAT migration inserts outbox rows directly and therefore leaves this user-write marker null. Never increment business `server_revision` locally. Map SQLite extended constraint codes plus the trigger text:

```cpp
if (error.databaseText().contains("DA_OCCURRENCE_LINKED")) {
    return Result<void>::failure({
        AccountingErrorCode::OccurrenceLinked,
        "transaction is linked to a live recurring occurrence"});
}
```

The linked-deletion integration test seeds a rule, pending transaction, and occurrence, calls generic `deleteTransaction`, and asserts both rows plus outbox count are unchanged.

- [ ] **Step 6: Create and link the production adapter target**

Create `dailyaccount_accounting_sqlite` from the database/migration/UoW sources. Link it privately to Qt Core/SQL and publicly only to `dailyaccount_accounting_application`, `dailyaccount_accounting_storage_contracts`, and `dailyaccount_platform_database`. Add it to the architecture link-graph scanner as an allowed outward adapter, never as a dependency of a domain target.

- [ ] **Step 7: Run focused and cumulative checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_sqlite_unit_of_work_tests.exe'
ctest --preset windows-desktop -R 'sqlite_(schema|unit_of_work)|architecture' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: rollback/atomicity/guard tests pass; architecture boundaries pass; Linux core stays Qt-free; DAT remains at 22 passes.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/modules/accounting/data/sqlite \
  tests/integration/sqlite_unit_of_work_tests.cpp
git commit -m "feat: add sqlite repositories and unit of work"
```

Without authorization, do not commit.

---

### Task 9: Add Minimum Recurring Rule and Pending Occurrence Creation

**Files:**
- Create: `src/modules/accounting/application/recurring_service.h`
- Create: `src/modules/accounting/application/recurring_service.cpp`
- Create: `tests/unit/recurring_service_tests.cpp`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.h`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/recurring_service_tests.cpp`
- Test: `tests/integration/sqlite_unit_of_work_tests.cpp`

**Interfaces:**
- Consumes: Task 2 recurrence calculations/IDs, G1 recurring repository, and Task 8 UoW.
- Produces: validated recurring-rule upsert and one idempotent atomic command that creates a deterministic `PENDING` occurrence plus its deterministic `PENDING` transaction and one outbox mutation.

- [ ] **Step 1: Write the two-device identity/idempotency test**

```cpp
void theSameRuleAndPeriodCreateExactlyOnePair()
{
    Fixture fixture;
    const RecurringRule rule = validMonthlyRule();
    DA_CHECK(fixture.seedRule(rule).hasValue());
    const GenerateOccurrenceCommand first{
        rule.id, "2026-09", UtcInstant{1'788'480'000'000},
        DeviceId::random(), MutationId::random()};
    const GenerateOccurrenceCommand second{
        rule.id, "2026-09", UtcInstant{1'788'480'001'000},
        DeviceId::random(), MutationId::random()};

    const auto one = fixture.service.generateOccurrence(first);
    const auto two = fixture.service.generateOccurrence(second);

    DA_CHECK(one.hasValue());
    DA_CHECK(two.hasValue());
    DA_CHECK_EQ(one.value().occurrence.id, two.value().occurrence.id);
    DA_CHECK_EQ(one.value().transaction.id, two.value().transaction.id);
    DA_CHECK_EQ(fixture.occurrenceCount(), 1);
    DA_CHECK_EQ(fixture.transactionCount(), 1);
    DA_CHECK_EQ(fixture.outboxCount(), 1);
}
```

Also test missing/disabled rule, wrong period grammar, period before start/after end, monthly clamp, rule snapshot immutability, pending transaction field mapping, deterministic IDs matching Task 2's vector, all-or-nothing failure, and generic transaction deletion returning `OccurrenceLinked`.

- [ ] **Step 2: Run the recurring service target red**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target dailyaccount_recurring_service_tests --parallel 2
```

Expected: unknown target or missing `recurring_service.h`.

- [ ] **Step 3: Define exact recurring commands/results**

```cpp
namespace dailyaccount {

struct CreateRecurringRuleCommand {
    RecurringRule rule;
    MutationId mutationId;
};

struct GenerateOccurrenceCommand {
    RecurringRuleId ruleId;
    std::string periodKey;
    UtcInstant now;
    DeviceId modifiedByDeviceId;
    MutationId mutationId;
};

struct GeneratedOccurrence {
    RecurringOccurrence occurrence;
    Transaction transaction;
    bool created;
};

class RecurringService final {
public:
    RecurringService(IAccountingUnitOfWork& unitOfWork,
                     std::function<bool(std::string_view)> timeZoneIsKnown);
    Result<RecurringRule> createRule(const CreateRecurringRuleCommand& command);
    Result<GeneratedOccurrence> generateOccurrence(
        const GenerateOccurrenceCommand& command);
};

}
```

- [ ] **Step 4: Implement one atomic recurring change group**

`createRule` validates a known IANA zone, requires `serverRevision=0`, inserts the rule, and enqueues `UPSERT_RECURRING_RULE` version 1 with `createdAt=rule.metadata.updatedAt`. `generateOccurrence` loads the live enabled rule, validates `periodKey`, computes the date and deterministic IDs, and checks `findOccurrence(occurrenceId)`. If found with the same rule/key, return it with `created=false` and write nothing.

For a missing occurrence, build:

```cpp
Transaction pending{
    ids.pendingTransactionId,
    metadata,
    TransactionType::Expense,
    TransactionStatus::Pending,
    rule.expectedAmountMinor,
    rule.currency,
    expectedOn,
    std::nullopt,
    std::nullopt,
    rule.categoryId,
    rule.accountId,
    std::nullopt,
    rule.merchant,
    rule.note,
    TransactionOrigin::Recurring,
    rule.id.toString() + ":" + command.periodKey,
    std::nullopt};
```

Create the occurrence with a canonical, fully populated rule snapshot JSON. Insert transaction first, occurrence second, then one `GENERATE_RECURRING_OCCURRENCE` outbox row whose expectations contain both new IDs at base revision 0 and whose `createdAt` is `command.now`. A unique-key race re-reads and returns the canonical deterministic pair only when the whole UoW can safely treat it as the same rule/key; every other constraint error fails.

- [ ] **Step 5: Run focused and cumulative tests**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_recurring_service_tests
./build/cmake/linux-core/dailyaccount_accounting_rules_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop -R 'recurring|sqlite_unit_of_work' --output-on-failure
```

Expected: standard-C++ recurring tests and real SQLite aggregate/guard tests pass; DAT remains at 22 passes.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/modules/accounting/application/recurring_service.* \
  src/modules/accounting/application/accounting_mutation_codec.* \
  tests/unit/recurring_service_tests.cpp \
  tests/integration/sqlite_unit_of_work_tests.cpp
git commit -m "feat: create deterministic recurring occurrences"
```

Without authorization, do not commit.

---

### Task 10: Extract a Strict Read-Only DAT Importer and Adapt DAT Coverage

**Files:**
- Create: `src/modules/accounting/data/legacy/dat_importer.h`
- Create: `src/modules/accounting/data/legacy/dat_importer.cpp`
- Create: `tests/unit/dat_importer_tests.cpp`
- Modify: `backend/storage.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/dat_importer_tests.cpp`
- Test: `tests/legacy/dat_regression_tests.cpp`
- Fixture: `tests/fixtures/dat/v3-sanitized/ledger.dat`
- Fixture: `tests/fixtures/dat/legacy-sanitized/records.dat`
- Fixture: `tests/fixtures/dat/legacy-sanitized/categories.dat`
- Fixture: `tests/fixtures/dat/v3-corrupt-checksum/ledger.dat`

**Interfaces:**
- Consumes: the current strict V3/legacy parsers, all 22 DAT tests, and G0 sanitized fixtures.
- Produces: a side-effect-free parser for explicit V3 snapshot, V3 backup, or legacy pair sources; legacy integer records/categories; source summaries; and adapter reuse by the test-only `StorageManager::load`.

- [ ] **Step 1: Write a source-immutability importer test**

```cpp
void validV3ImportDoesNotChangeAnySourceEntry()
{
    TemporaryDirectory directory("dat-importer");
    copyFixtureTree("tests/fixtures/dat/v3-sanitized", directory.path());
    const auto before = snapshotTree(directory.path());

    DatImporter importer;
    const auto imported = importer.read(
        DatSource{DatSourceKind::V3Snapshot, directory.path()});

    DA_CHECK(imported.hasValue());
    DA_CHECK_EQ(snapshotTree(directory.path()), before);
    DA_CHECK_EQ(imported.value().summary.recordCount,
                expectedFixtureSummary("v3-sanitized").recordCount);
}
```

Add tests for exact V3 count/totals/dates/categories, exact legacy pair semantics, parentheses compatibility, delimiter-containing notes, overlength stored categories, missing catalog recovery, missing half-pair rejection, V3 checksum rejection, no fallback from corrupt primary to backup, explicit backup selection, duplicate/invalid IDs, and directory snapshots proving no creation/deletion/mtime/content change.

- [ ] **Step 2: Run the importer test red**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/unit/dat_importer_tests.cpp \
  src/modules/accounting/data/legacy/dat_importer.cpp \
  -o /tmp/opencode/dailyaccount_dat_importer_tests
```

Expected: compilation fails because `dat_importer.h` is absent.

- [ ] **Step 3: Define the read-only importer boundary**

```cpp
namespace dailyaccount {

enum class DatSourceKind { V3Snapshot, V3Backup, LegacyPair };
enum class LegacyRecordType { Income, Expense };

struct DatSource {
    DatSourceKind kind;
    std::filesystem::path directory;
};

struct LegacyCategoryRow {
    LegacyRecordType type;
    std::string name;
};

struct LegacyRecordRow {
    std::int32_t legacyId;
    std::string occurredOn;
    LegacyRecordType type;
    MoneyMinor amountMinor;
    std::string category;
    std::string subcategory;
    std::string note;
};

struct DatSummary {
    std::size_t recordCount;
    MoneyMinor incomeMinor;
    MoneyMinor expenseMinor;
    std::optional<std::string> minimumDate;
    std::optional<std::string> maximumDate;
    std::vector<std::pair<LegacyRecordType, std::string>> categories;
};

struct ImportedDat {
    DatSourceKind sourceKind;
    std::vector<LegacyCategoryRow> catalogRows;
    std::vector<LegacyRecordRow> records;
    DatSummary summary;
};

class DatImporter final {
public:
    Result<ImportedDat> read(const DatSource& source) const;
};

}
```

`V3Snapshot` reads exactly `ledger.dat`; `V3Backup` reads exactly `ledger.dat.bak`; `LegacyPair` requires both `records.dat` and `categories.dat`. The importer never probes alternatives after a selected source fails.

- [ ] **Step 4: Move parser logic, not writer/recovery logic**

Move V3 checksum/percent decoding, legacy decimal parsing, longest-known-category matching, record/catalog validation, and known `饮食(...)`/`交通(...)` compatibility splitting into `dat_importer.cpp`. Open sources only as `std::ifstream(path, std::ios::binary)`; do not call `create_directories`, `remove`, `rename`, `copy_file`, or any writable stream. Keep source path and line number in structured failure messages.

Adapt `StorageManager::load` to call the importer and convert its result to `StoredData`; retain its existing save/backup/recovery implementation only for `dailyaccount_legacy_backend` tests. This ensures every moved parser behavior remains exercised by the 22-case oracle rather than maintaining a second parser.

- [ ] **Step 5: Enforce target links**

Create standard-C++ target `dailyaccount_dat_importer`, link `dailyaccount_legacy_backend` to it, and link importer tests directly to it. Do not link it to Qt SQL or the application service. Keep all four G0 fixture paths unchanged.

- [ ] **Step 6: Run focused, fixture, and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_dat_importer_tests
./build/cmake/linux-core/dailyaccount_backend_tests
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/dat_regression_coverage_contract.cmake
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: importer fixtures pass, corrupt checksum fails with that reason, source snapshots are identical, DAT reports exactly 22 passes, and CTest has zero failures.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt backend/storage.cpp \
  src/modules/accounting/data/legacy/dat_importer.* \
  tests/unit/dat_importer_tests.cpp
git commit -m "refactor: extract read-only dat importer"
```

Without authorization, do not commit.

---

### Task 11: Implement Crash-Safe DAT Manifest, Mapping, Staging, and Activation

**Files:**
- Create: `src/platform/profile/migrations/002_legacy_migration_manifests.sql`
- Create: `src/platform/database/atomic_file_activation.h`
- Create: `src/platform/database/atomic_file_activation.cpp`
- Create: `src/modules/accounting/data/legacy/migration_manifest.h`
- Create: `src/modules/accounting/data/legacy/migration_manifest.cpp`
- Create: `src/modules/accounting/data/legacy/dat_migration_service.h`
- Create: `src/modules/accounting/data/legacy/dat_migration_service.cpp`
- Create: `tests/integration/dat_migration_tests.cpp`
- Modify: `src/platform/profile/profile_store.h`
- Modify: `src/platform/profile/profile_store.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/dat_migration_tests.cpp`
- Fixture: all `tests/fixtures/dat/*` sources and expected JSON files

**Interfaces:**
- Consumes: Tasks 6-8 profile/schema/UoW, Task 10 importer, G0 expected fixture summaries, and CP-03 requirements.
- Produces: SHA-256 source identity, stable migration namespace/mapping, `PREPARED -> DATABASE_ACTIVATED -> COMPLETED` manifest state, deterministic category/transaction/outbox IDs, validated staging activation, and deterministic recovery after every crash point.

- [ ] **Step 1: Write the fault matrix before implementation**

Create one parameterized integration test over these exact injected points:

```cpp
enum class DatMigrationFaultPoint {
    None,
    AfterManifestPrepared,
    AfterStagingCommitted,
    BeforeDatabaseActivation,
    AfterDatabaseActivationBeforeManifestUpdate,
    AfterManifestActivated
};
```

For every non-`None` point, assert source tree hash/mtime is unchanged. Before activation, assert no active `accounting.sqlite` exists and rerun safely resumes/recreates staging. Immediately before activation, assert the staging connection is closed, `wal_checkpoint(TRUNCATE)` completed, and no staging `-wal`/`-shm` sidecar remains. After activation, assert recovery recognizes the in-database marker and completes the manifest without reimporting. Include:

```cpp
void crashAfterActivationIsReconciledFromTheDatabaseMarker()
{
    Fixture fixture(DatMigrationFaultPoint::AfterDatabaseActivationBeforeManifestUpdate);
    const auto first = fixture.migrateV3();
    DA_CHECK(!first.hasValue());
    DA_CHECK(std::filesystem::exists(fixture.accountingPath()));
    DA_CHECK_EQ(fixture.manifestState(), MigrationManifestState::Prepared);

    fixture.disableFault();
    const auto recovered = fixture.recover();
    DA_CHECK(recovered.hasValue());
    DA_CHECK_EQ(fixture.manifestState(), MigrationManifestState::Completed);
    DA_CHECK_EQ(fixture.transactionCount(), fixture.expected().recordCount);
}
```

Also test V3 and legacy parity; corrupt source; same hash/same profile retry; same hash/different profile allowed with a distinct namespace; non-empty target rejection; manifest-completed/database-missing stop; active hash mismatch stop; category/subcategory IDs; old `退款`/`个人转账` remaining `INCOME`; no fabricated time/timezone/account; and exact outbox count.

- [ ] **Step 2: Run the migration target red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_dat_migration_tests --parallel 2
```

Expected: unknown target or missing `dat_migration_service.h`.

- [ ] **Step 3: Add strict profile manifest storage**

`002_legacy_migration_manifests.sql` contains:

```sql
-- DA_STATEMENT
CREATE TABLE legacy_migration_manifests (
    target_profile_id TEXT NOT NULL REFERENCES local_profiles(profile_id) ON DELETE RESTRICT,
    source_sha256 TEXT NOT NULL CHECK(length(source_sha256) = 64),
    source_path TEXT NOT NULL,
    source_kind TEXT NOT NULL CHECK(source_kind IN ('V3_SNAPSHOT','V3_BACKUP','LEGACY_PAIR')),
    source_size_bytes INTEGER NOT NULL CHECK(source_size_bytes >= 0),
    id_namespace TEXT NOT NULL CHECK(length(id_namespace) = 36),
    mapping_version INTEGER NOT NULL CHECK(mapping_version = 1),
    state TEXT NOT NULL CHECK(state IN ('PREPARED','DATABASE_ACTIVATED','COMPLETED')),
    staging_path TEXT NOT NULL,
    active_database_sha256 TEXT,
    summary_json TEXT,
    prepared_at_ms INTEGER NOT NULL,
    activated_at_ms INTEGER,
    completed_at_ms INTEGER,
    PRIMARY KEY(target_profile_id, source_sha256),
    UNIQUE(id_namespace),
    CHECK(state = 'PREPARED' OR active_database_sha256 IS NOT NULL),
    CHECK(state <> 'COMPLETED' OR completed_at_ms IS NOT NULL)
) STRICT;
```

Apply profile migrations with the same hash/version rules as accounting migrations. Never put passwords, tokens, DAT content, or unmasked email in this table.

- [ ] **Step 4: Define manifest and migration APIs**

```cpp
namespace dailyaccount {

enum class MigrationManifestState { Prepared, DatabaseActivated, Completed };

struct MigrationManifest {
    ProfileId targetProfileId;
    std::string sourceSha256;
    DatSource source;
    std::uint64_t sourceSizeBytes;
    std::string idNamespace;
    std::uint32_t mappingVersion;
    MigrationManifestState state;
    std::filesystem::path stagingPath;
    std::optional<std::string> activeDatabaseSha256;
    std::optional<DatSummary> summary;
};

class MigrationManifestStore final {
public:
    explicit MigrationManifestStore(ModuleDbExecutor& profilesExecutor);
    Result<std::optional<MigrationManifest>> find(
        ProfileId targetProfileId, std::string_view sourceSha256);
    Result<MigrationManifest> prepare(
        ProfileId targetProfileId, const DatSource& source,
        std::string sourceSha256, std::uint64_t sourceSizeBytes,
        std::string idNamespace, std::filesystem::path stagingPath,
        UtcInstant now);
    Result<void> transition(
        ProfileId targetProfileId, std::string_view sourceSha256,
        MigrationManifestState expected, MigrationManifestState next,
        std::optional<std::string> activeDatabaseSha256,
        std::optional<DatSummary> summary, UtcInstant now);
};

struct DatMigrationRequest {
    DatSource source;
    LocalProfile targetProfile;
    std::filesystem::path accountingDatabasePath;
    UtcInstant now;
};

struct DatMigrationReport {
    std::string sourceSha256;
    std::size_t recordCount;
    std::size_t categoryCount;
    MoneyMinor incomeMinor;
    MoneyMinor expenseMinor;
    std::string minimumDate;
    std::string maximumDate;
};

class DatMigrationService final {
public:
    DatMigrationService(MigrationManifestStore&, ProfileStore&, DatImporter&,
                         DatMigrationFaultPoint = DatMigrationFaultPoint::None);
    Result<DatMigrationReport> migrate(const DatMigrationRequest& request);
    Result<DatMigrationReport> recover(const DatMigrationRequest& request);
};

Result<void> activateStagingFile(
    const std::filesystem::path& staging,
    const std::filesystem::path& active);

}
```

The fault enum is always compiled but only non-`None` construction is exposed to tests.

- [ ] **Step 5: Implement stable source hashes and mappings**

For V3 snapshot/backup, SHA-256 the exact selected file bytes. For a legacy pair, hash this unambiguous byte sequence:

```text
"records.dat\0" + uint64-big-endian(recordBytes.size) + recordBytes
+ "categories.dat\0" + uint64-big-endian(categoryBytes.size) + categoryBytes
```

Generate `idNamespace` once with UUIDv4 and persist it in `PREPARED`. On every retry derive:

```text
categoryId       = UUIDv5<CategoryId>(namespace, "category/<TYPE>/<parent>/<child-or-empty>")
transactionId    = UUIDv5<TransactionId>(namespace, "record/<legacy-int-id>")
deviceId         = UUIDv5<DeviceId>(namespace, "dat-migration-device")
categoryMutation = UUIDv5<MutationId>(categoryId, "dat-migration-upsert-v1")
recordMutation   = UUIDv5<MutationId>(transactionId, "dat-migration-upsert-v1")
```

Use the manifest's `prepared_at_ms` for all imported created/updated timestamps so retries are byte-stable. Exact current preset names set `isPreset=true`; a V3 `CATEGORY` row does not by itself imply custom status. Build real parent/child rows for non-empty subcategories. Map every record to `POSTED`, `MANUAL`, CNY, null account/time/timezone/merchant/refund; preserve integer minor units, ISO date, and note. Never infer transfer/refund from category text.

- [ ] **Step 6: Write and validate staging in one import transaction**

The staging path is the active path plus `.migrate-<first 16 source hash chars>.staging` in the same directory. Remove a stale staging file only after its embedded marker fails to match the current manifest. Create schema 2, then in one `BEGIN IMMEDIATE` transaction insert categories, transactions, deterministic outbox rows, every `legacy_id_map` row, and one `legacy_import_markers` row. Commit none on any mapping/constraint/outbox failure.

On the staging executor, run `PRAGMA wal_checkpoint(TRUNCATE)` after the import commit and require the returned busy count to be zero. Then drain and close every staging/validation handle, remove an empty closed `-wal`/`-shm` sidecar if SQLite left one, and require that neither sidecar exists before the single-file rename. Require exact source versus staging record count, income total, expense total, min/max date, and sorted category set; then require `integrity_check=ok`, empty `foreign_key_check`, schema 2, exact owner identity, and a null `sqlite_writes_started_at_ms`. A mismatch returns `StorageFailure` and leaves manifest `PREPARED`.

- [ ] **Step 7: Activate and reconcile crashes**

`activateStagingFile` rejects an existing active path, rejects any remaining staging WAL/SHM sidecar, flushes the closed staging file, uses `MoveFileExW(..., MOVEFILE_WRITE_THROUGH)` on Windows and same-filesystem `rename` plus parent-directory `fsync` on POSIX, and never performs copy-and-delete activation. After rename, hash/validate the active database, persist that immutable activation-image hash, transition manifest to `DATABASE_ACTIVATED`, then `COMPLETED`, and use the injected `ProfileStore` to transition the profile `INITIALIZING -> ACTIVE`.

The full-file SHA-256 is recovery evidence for the immutable activation image, not a perpetual hash of a writable database. Before the first business write, recovery requires the current file to match it. After `sqlite_writes_started_at_ms` is set, normal startup/recovery must not compare the mutable database file to the old activation hash; it instead requires the embedded source/profile/mapping marker, database owner/schema, `integrity_check`, and `foreign_key_check` to agree. If accepted architecture evidence interprets the activation hash as a perpetual full-file hash after legitimate writes, stop for a reviewed contract correction because that invariant cannot coexist with SQLite writes.

Recovery rules are exact:

```text
PREPARED + valid matching staging + no active DB: validate and activate.
PREPARED + matching active in-database marker and activation hash: validate, record DATABASE_ACTIVATED, complete.
DATABASE_ACTIVATED + matching active marker and activation hash: complete.
COMPLETED + no business-write marker + matching active marker/hash: return the stored report without writes.
COMPLETED + business-write marker + matching owner/source marker/schema/integrity: return the stored report without writes.
COMPLETED + missing active DB or mismatched immutable marker/owner: StorageFailure; no DAT fallback.
Any state + source hash changed: StorageFailure; preserve source/staging/active files.
```

- [ ] **Step 8: Run focused fixture/fault and cumulative checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_dat_migration_tests.exe' `
  --json docs\validation\stage-2\migration-fault-matrix.json
ctest --preset windows-desktop -R 'dat_migration|profile_store|sqlite_' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_dat_importer_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: every fault point recovers as specified; both valid fixtures match expected summaries; corrupt input activates nothing; DAT source snapshots remain identical; DAT oracle reports 22 passes.

- [ ] **Step 9: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/platform/profile src/platform/database \
  src/modules/accounting/data/legacy \
  tests/integration/dat_migration_tests.cpp
git commit -m "feat: migrate dat through crash-safe staging"
```

Without authorization, do not commit.

---

### Task 12: Add SQLite Read Models and an Asynchronous Desktop Controller

**Files:**
- Create: `src/modules/accounting/application/accounting_view_models.h`
- Create: `src/modules/accounting/application/accounting_query_service.h`
- Create: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.h`
- Create: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.cpp`
- Create: `src/apps/desktop-widgets/accounting_view_models.h`
- Create: `src/apps/desktop-widgets/accounting_view_models.cpp`
- Create: `src/apps/desktop-widgets/desktop_controller.h`
- Create: `src/apps/desktop-widgets/desktop_controller.cpp`
- Create: `tests/integration/accounting_query_tests.cpp`
- Create: `tests/widgets/desktop_controller_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/accounting_query_tests.cpp`
- Test: `tests/widgets/desktop_controller_tests.cpp`

**Interfaces:**
- Consumes: Tasks 3-4 application commands, Task 8 SQLite adapter, current Ledger query behavior, and Qt Concurrent for non-blocking Widgets calls.
- Produces: standard-C++ transaction/category/dashboard/statistics/export snapshots, SQL read queries that omit tombstones/pending formal totals, and a Qt controller that owns command/query scheduling without exposing repositories or SQL to pages.

- [ ] **Step 1: Write focused query and GUI-heartbeat tests**

Seed migrated posted income/expense rows and assert:

```cpp
void legacyParitySnapshotMatchesLedgerOrderingAndTotals()
{
    auto fixture = QueryFixture::fromSanitizedV3();
    const auto dashboard = fixture.queries.dashboard(LocalDate{2026, 9, 4});
    DA_CHECK(dashboard.hasValue());
    DA_CHECK_EQ(dashboard.value().totalIncomeMinor, fixture.expectedIncome());
    DA_CHECK_EQ(dashboard.value().totalExpenseMinor, fixture.expectedExpense());
    DA_CHECK(dashboard.value().recentTransactions.size() <= 10);
    DA_CHECK(std::is_sorted(
        dashboard.value().recentTransactions.begin(),
        dashboard.value().recentTransactions.end(), newestTransactionFirst));
}
```

The controller test holds a second write lock, submits a command, runs a 50 ms `QTimer`, and proves at least five heartbeat ticks occur before the bounded result signal. Also test one success/failure signal per request, no stale result after `detachModels`, exact UUID reuse from editor to service, and no money field represented as `double`.

- [ ] **Step 2: Run both targets red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target `
  dailyaccount_accounting_query_tests dailyaccount_desktop_controller_tests --parallel 2
```

Expected: unknown targets or missing `accounting_query_service.h`.

- [ ] **Step 3: Define standard-C++ read models**

```cpp
namespace dailyaccount {

struct TransactionFilter {
    std::optional<LocalDate> from;
    std::optional<LocalDate> through;
    std::optional<TransactionType> type;
};

struct TransactionListItem {
    TransactionId id;
    LocalDate occurredOn;
    TransactionType type;
    TransactionStatus status;
    MoneyMinor amountMinor;
    std::string categoryDisplayName;
    std::string note;
};

struct CategoryListItem {
    CategoryId id;
    std::string name;
    CategoryApplicability appliesTo;
    bool isPreset;
    bool isArchived;
    bool isInUse;
};

struct CategoryAmount {
    CategoryId categoryId;
    std::string name;
    MoneyMinor amountMinor;
    std::uint16_t basisPoints;
};

struct MonthlyAmount {
    std::string yearMonth;
    MoneyMinor incomeMinor;
    MoneyMinor expenseMinor;
};

struct DashboardSnapshot {
    MoneyMinor totalIncomeMinor;
    MoneyMinor totalExpenseMinor;
    MoneyMinor balanceMinor;
    std::vector<TransactionListItem> recentTransactions;
    std::vector<CategoryAmount> expenseCategories;
};

struct StatisticsSnapshot {
    MoneyMinor totalIncomeMinor;
    MoneyMinor totalExpenseMinor;
    MoneyMinor balanceMinor;
    std::size_t transactionCount;
    std::vector<CategoryAmount> expenseCategories;
    std::vector<CategoryAmount> incomeCategories;
    std::vector<MonthlyAmount> months;
};

class IAccountingQueryService {
public:
    virtual ~IAccountingQueryService() = default;
    virtual Result<std::vector<TransactionListItem>> listTransactions(
        const TransactionFilter& filter) = 0;
    virtual Result<std::vector<CategoryListItem>> listCategories() = 0;
    virtual Result<DashboardSnapshot> dashboard(LocalDate today) = 0;
    virtual Result<StatisticsSnapshot> statistics(
        const TransactionFilter& filter) = 0;
};

}
```

- [ ] **Step 4: Implement exact SQLite query semantics**

`SqliteAccountingQueryService` takes `ModuleDbExecutor&` plus expected owner identity. Every SQL statement uses bound dates/type and filters `deleted_at_ms IS NULL`. Transaction lists include both statuses; current Windows totals/charts include only `POSTED`. Transfers contribute zero; refunds follow D-024 even though current migrated data contains neither. Join child and parent category names to produce `parent(child)`. Sort transactions by `occurred_on DESC, id DESC`, categories by amount descending then stable ID, and months ascending with at most the last 12.

Compute category basis points in integer arithmetic by D-024 largest remainder, with stable category ID tie-breaks. Do not select financial values into `double`.

- [ ] **Step 5: Define the Qt controller boundary**

`DesktopController` exposes these invokables/slots and signals:

```cpp
class DesktopController final : public QObject {
    Q_OBJECT
public:
    DesktopController(IAccountingService& commands,
                      IAccountingQueryService& queries,
                      QObject* parent = nullptr);
    void requestTransactions(QDate from, QDate through, int typeFilter);
    void requestCategories();
    void requestDashboard(QDate today);
    void requestStatistics(QDate from, QDate through);
    void createTransaction(const DesktopTransactionEdit& edit);
    void updateTransaction(QString transactionId, const DesktopTransactionEdit& edit);
    void deleteTransaction(QString transactionId);
    void createCategory(QString name, bool income);
    void archiveCategory(QString categoryId);
    void detachModels();

signals:
    void transactionsReady(QVector<DesktopTransactionRow> rows);
    void categoriesReady(QVector<DesktopCategoryRow> rows);
    void dashboardReady(DesktopDashboardView view);
    void statisticsReady(DesktopStatisticsView view);
    void commandSucceeded();
    void commandFailed(QString code, QString message);
};
```

`DesktopTransactionEdit` carries date, enum type, decimal amount text, selected category UUID, subcategory UUID if present, and note. Qt view-model money fields use `qint64`; pie data uses integer basis points. Register metatypes explicitly. Each public call starts one `QtConcurrent::run` job, owns a `QFutureWatcher`, emits results back on the controller thread, and drops the result if the model generation changed after `detachModels`. Generate entity/mutation IDs once before launching the job.

- [ ] **Step 6: Run focused and cumulative checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_accounting_query_tests.exe'
$env:QT_QPA_PLATFORM = 'offscreen'
& 'build\cmake\windows-desktop\dailyaccount_desktop_controller_tests.exe'
ctest --preset windows-desktop -R 'accounting_query|desktop_controller|sqlite_' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: query parity and heartbeat/generation tests pass; Linux core and 22 DAT tests remain green.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/modules/accounting/application/accounting_query_service.h \
  src/modules/accounting/application/accounting_view_models.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_query_service.* \
  src/apps/desktop-widgets/accounting_view_models.* \
  src/apps/desktop-widgets/desktop_controller.* \
  tests/integration/accounting_query_tests.cpp \
  tests/widgets/desktop_controller_tests.cpp
git commit -m "feat: add sqlite desktop controller"
```

Without authorization, do not commit.

---

### Task 13: Switch Flow, Dialog, and Category Widgets from Ledger to the Controller

**Files:**
- Create: `tests/widgets/sqlite_flow_widgets_tests.cpp`
- Modify: `gui/flowpage.h`
- Modify: `gui/flowpage.cpp`
- Modify: `gui/flowdialog.h`
- Modify: `gui/flowdialog.cpp`
- Modify: `gui/categorypage.h`
- Modify: `gui/categorypage.cpp`
- Modify: `gui/flowpage.ui`
- Modify: `CMakeLists.txt`
- Test: `tests/widgets/sqlite_flow_widgets_tests.cpp`

**Interfaces:**
- Consumes: Task 12 `DesktopController` and Qt view models.
- Produces: controller-only transaction/category pages, exact decimal text input, UUID-backed rows/selections, asynchronous refresh/error behavior, and preserved add/edit/delete/filter/category workflows.

- [ ] **Step 1: Write an offscreen migrated-flow test**

Use a real temporary SQLite profile and object names. The focused flow is:

```cpp
void migratedFlowCanAddEditFilterAndDeleteWithoutDatWrites()
{
    WidgetsFixture fixture;
    const auto datBefore = fixture.sourceDatSnapshot();
    FlowPage page(fixture.controller);
    page.show();

    fixture.openAddDialog(page);
    fixture.enterDate("2026-09-04");
    fixture.enterAmount("123.45");
    fixture.chooseExpenseCategory("饮食");
    fixture.acceptEditor();
    DA_CHECK(fixture.waitForCommandSuccess());
    DA_CHECK_EQ(fixture.sqliteTransactionCount(), 1);
    DA_CHECK_EQ(fixture.sqliteOutboxCount(), 1);
    DA_CHECK_EQ(fixture.sourceDatSnapshot(), datBefore);
}
```

Complete the same test with edit, date/type filter, delete, and restart persistence. Add category create/archive tests, in-use custom category archive visibility, preset protection, invalid decimal/overflow errors, future-date confirmation, stable UUID in `Qt::UserRole`, linked-occurrence delete error text, one refresh per success, and no page member/constructor containing `Ledger` or `CategoryManager`.

- [ ] **Step 2: Run the Widgets target red**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_sqlite_flow_widgets_tests --parallel 2
```

Expected: unknown target or constructor mismatch because pages still require `Ledger`.

- [ ] **Step 3: Replace page dependencies and retain observable UI behavior**

Use exact constructors:

```cpp
explicit FlowPage(dailyaccount::DesktopController& controller,
                  QWidget* parent = nullptr);
explicit CategoryPage(dailyaccount::DesktopController& controller,
                      QWidget* parent = nullptr);
explicit FlowDialog(QVector<dailyaccount::DesktopCategoryRow> categories,
                    QWidget* parent = nullptr);
FlowDialog(QVector<dailyaccount::DesktopCategoryRow> categories,
           const dailyaccount::DesktopTransactionRow& existing,
           QWidget* parent = nullptr);
dailyaccount::DesktopTransactionEdit transactionEdit() const;
```

Remove all `Ledger&`, `CategoryManager&`, integer record IDs, `showLedgerError`, and direct backend includes from these files. Keep `dataChanged` as the page-level signal consumed by `MainWindow`, but emit it only after `commandSucceeded` and a refreshed model arrives.

- [ ] **Step 4: Replace floating money input with exact text**

Replace `QDoubleSpinBox` with `QLineEdit` object name `amountEdit` and a validator that accepts the lexical shape while `parseCnyMinor` remains authoritative on accept:

```cpp
auto* validator = new QRegularExpressionValidator(
    QRegularExpression(QStringLiteral("^(?:0|[1-9][0-9]{0,7})(?:\\.[0-9]{0,2})?$")),
    m_amountEdit);
m_amountEdit->setValidator(validator);
```

On accept, parse UTF-8 text with `parseCnyMinor`; show the existing `输入错误` warning on failure. Format an existing amount with `formatMoney`. Do not call `QDoubleSpinBox::value`, `moneyFromDouble`, or `moneyToDouble`.

- [ ] **Step 5: Bind UUID view models and asynchronous errors**

Store full transaction/category UUID strings in `Qt::UserRole`; display the full transaction UUID in the existing ID column. Preserve grouping by date descending and stable ID descending, six columns, date/type filters, double-click edit, delete confirmation, and filter expansion after create/edit. Disable the initiating button until a success/failure signal arrives.

Map `OccurrenceLinked` to:

```text
该记录由周期支出实例管理，不能在流水中单独删除。请到周期支出中处理。
```

Category lists preserve `[预设]`, `[自定义]`, and `(使用中)` labels. The existing delete button calls `archiveCategory`; archived categories disappear from new-entry choices and ordinary lists while historical transaction display keeps the name.

- [ ] **Step 6: Run focused and cumulative checks**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_sqlite_flow_widgets_tests.exe'
ctest --preset windows-desktop -R 'sqlite_flow_widgets|desktop_controller|sqlite_' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: all flow/category behavior passes against SQLite, no DAT source changes, controller heartbeat remains live, Linux and 22 DAT regressions pass.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt gui/flowpage.* gui/flowdialog.* gui/categorypage.* \
  tests/widgets/sqlite_flow_widgets_tests.cpp
git commit -m "refactor: bind flow widgets to sqlite controller"
```

Without authorization, do not commit.

---

### Task 14: Switch Dashboard, Statistics, Export, and Main Window View Models

**Files:**
- Create: `tests/widgets/sqlite_dashboard_widgets_tests.cpp`
- Modify: `gui/dashboardpage.h`
- Modify: `gui/dashboardpage.cpp`
- Modify: `gui/statisticspage.h`
- Modify: `gui/statisticspage.cpp`
- Modify: `gui/otherpage.h`
- Modify: `gui/otherpage.cpp`
- Modify: `gui/mainwindow.h`
- Modify: `gui/mainwindow.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/widgets/sqlite_dashboard_widgets_tests.cpp`

**Interfaces:**
- Consumes: Task 12 read models/controller and current Widgets presentation behavior.
- Produces: five-page controller composition; SQLite-backed dashboard/status/statistics/TXT export; integer-money chart boundaries; and removal of the legacy physical whole-ledger-clear UI with no V1 replacement.

- [ ] **Step 1: Write migrated-dashboard parity tests**

The fixture migrates the sanitized V3 source, opens a `MainWindow`, and asserts current observable behavior:

```cpp
void migratedDashboardAndStatisticsPreserveVisibleTotals()
{
    WidgetsFixture fixture;
    MainWindow window(fixture.controller);
    window.show();
    DA_CHECK(fixture.waitForInitialRefresh());
    DA_CHECK(window.findChild<QLabel*>("statusIncome")->text().contains(
        fixture.expectedIncomeText()));
    DA_CHECK(window.findChild<QLabel*>("statusExpense")->text().contains(
        fixture.expectedExpenseText()));
    DA_CHECK_EQ(window.findChild<QStackedWidget*>("mainStack")->count(), 5);
    DA_CHECK_EQ(fixture.recentTable()->rowCount(),
                std::min(10, fixture.expectedRecordCount()));
}
```

Also test current-month cards, all four statistics ranges, category ordering/percentages, latest 12 months, TXT export content/order/BOM, page navigation, minimum size 1100x700, status updates after a command, chart setters containing no `double` money, no clear-data button/slot/text, and no `Ledger`/`CategoryManager` member or include in any modified page.

- [ ] **Step 2: Run the target red**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_sqlite_dashboard_widgets_tests --parallel 2
```

Expected: constructor mismatch because `MainWindow` and pages still require `Ledger`.

- [ ] **Step 3: Replace all remaining constructors with the controller**

Use:

```cpp
explicit DashboardPage(dailyaccount::DesktopController&, QWidget* parent = nullptr);
explicit StatisticsPage(dailyaccount::DesktopController&, QWidget* parent = nullptr);
explicit OtherPage(dailyaccount::DesktopController&, QWidget* parent = nullptr);
explicit MainWindow(dailyaccount::DesktopController&, QWidget* parent = nullptr);
```

`MainWindow::refreshAll` requests dashboard, transactions, categories, and the active statistics range; pages render only signal-delivered immutable view models. Preserve the existing five navigation indexes/object names and `dataChanged` refresh wiring.

- [ ] **Step 4: Keep money integral through charts and status**

Change chart APIs to:

```cpp
struct PieSlice { QString name; quint16 basisPoints; };
struct BarPair { QString month; qint64 incomeMinor; qint64 expenseMinor; };
void PieChartWidget::setData(const QVector<PieSlice>& data,
                             const QStringList& colors);
void BarChartWidget::setData(const QVector<BarPair>& data,
                             const QString& positiveLabel,
                             const QString& negativeLabel);
```

Painter code converts only basis points or `amountMinor / maxAmountMinor` ratios to `double` for pixel geometry. Labels always use `formatMoney(qint64)`. Dashboard/status totals come directly from the same snapshot to avoid divergent sums.

- [ ] **Step 5: Preserve export and remove only the prohibited clear operation**

Keep the TXT export entry, desktop destination lookup, UTF-8 BOM, timestamped file name, totals, date grouping, daily subtotals, and transaction lines. Build content from a query snapshot, not repository/SQL access in `OtherPage`.

Remove `showClearDetail`, `createClearDetailPage`, `onClearData`, its card, its stacked page, all three confirmations, and every call/reference to `clearAllData`. Keep the top-level main stack at five pages; only `OtherPage`'s internal stack decreases from three pages to two (feature list and export detail). Do not add a replacement bulk-delete action.

- [ ] **Step 6: Run focused and cumulative checks**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_sqlite_dashboard_widgets_tests.exe'
ctest --preset windows-desktop -R 'sqlite_.*widgets|widgets_contract|desktop_controller' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: migrated UI parity/export tests pass, no clear operation is reachable, five-page desktop contract remains green, and DAT oracle reports 22 passes.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt gui/dashboardpage.* gui/statisticspage.* gui/otherpage.* \
  gui/mainwindow.* tests/widgets/sqlite_dashboard_widgets_tests.cpp
git commit -m "refactor: render widgets from sqlite view models"
```

Without authorization, do not commit.

---

### Task 15: Compose SQLite Startup, Stop DAT Writes, and Conditionally Retire Remaining qmake Files

**Files:**
- Create: `src/apps/desktop-widgets/desktop_composition.h`
- Create: `src/apps/desktop-widgets/desktop_composition.cpp`
- Create: `src/apps/desktop-widgets/profile_selection_dialog.h`
- Create: `src/apps/desktop-widgets/profile_selection_dialog.cpp`
- Create: `tests/widgets/desktop_storage_switch_tests.cpp`
- Create: `tests/cmake/sqlite_desktop_boundary_contract.cmake`
- Modify: `src/apps/desktop-widgets/register_modules.h`
- Modify: `src/apps/desktop-widgets/register_modules.cpp`
- Modify: `gui/main_gui.cpp`
- Modify: `CMakeLists.txt`
- Modify: `CMakePresets.json`
- Modify: `build/build.bat`
- Modify: `README.md`
- Delete only if still tracked after accepted G1: `jizhang.pro`
- Delete only if still tracked after accepted G1: `tests/backend_tests.pro`
- Delete only if still tracked after accepted G1: `tests/registry_tests.pro`
- Test: `tests/widgets/desktop_storage_switch_tests.cpp`
- Test: `tests/cmake/sqlite_desktop_boundary_contract.cmake`

**Interfaces:**
- Consumes: profile/migration/database/controller tasks, Stage 1 explicit module registration, current data discovery/locking behavior, and G1's CMake package path.
- Produces: a lifetime-safe SQLite desktop composition root; explicit profile/source confirmation; a one-way SQLite storage marker; no production DAT writer/link; CMake-only packaging; and exact shutdown draining.

- [ ] **Step 1: Write storage-switch and link-boundary tests**

The runtime test launches an isolated composition twice:

```cpp
void firstBusinessWriteTouchesOnlyAccountingSqlite()
{
    DesktopStorageFixture fixture;
    fixture.copyV3Source();
    DA_CHECK(fixture.confirmMigration().hasValue());
    const auto datBefore = fixture.datTreeSnapshot();
    const auto sqliteBefore = fixture.accountingHash();

    DA_CHECK(fixture.createExpense("2026-09-04", "12.34", "饮食").hasValue());

    DA_CHECK_EQ(fixture.datTreeSnapshot(), datBefore);
    DA_CHECK(fixture.accountingHash() != sqliteBefore);
    DA_CHECK_EQ(fixture.transactionCount(), fixture.expectedRecordCount() + 1);
    DA_CHECK_EQ(fixture.outboxCount(), fixture.expectedOutboxCount() + 1);
    DA_CHECK(fixture.reopenWithoutDatAccess().hasValue());
}
```

Add no-DAT blank-profile startup, explicit source/profile confirmation, multiple-source refusal until selection, migration failure with no active DB, owner mismatch, a null write marker immediately after migration, the first business command setting the marker to its outbox timestamp, active SQLite with corrupt/missing DAT still opening after its file hash legitimately changes, clean drain on close, and profile switch model detachment tests.

The CMake boundary test fails if `dailyaccount_desktop` transitively links `dailyaccount_legacy_backend`, if `gui/` includes `ledger.h`/`storage.h`/`record.h`/`category.h`, if `main_gui.cpp` constructs `Ledger`/`StorageManager`, or if a shipping source calls `saveTo`, `save(StoredData`, or `clearAllData`.

- [ ] **Step 2: Run runtime and boundary tests red**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_desktop_storage_switch_tests --parallel 2
```

```bash
cmake -DDA_SOURCE_DIR="$PWD" \
  -DDA_LINK_GRAPH=build/cmake/linux-core/dailyaccount-link-graph.txt \
  -P tests/cmake/sqlite_desktop_boundary_contract.cmake
```

Expected: Windows has an unknown target; the boundary script is absent or then fails on the current direct `Ledger`/DAT composition.

- [ ] **Step 3: Define the lifetime-safe composition root**

```cpp
namespace dailyaccount {

struct DesktopOpenRequest {
    std::filesystem::path applicationDataRoot;
    ProfileId profileId;
    std::optional<DatSource> confirmedDatSource;
};

class DesktopComposition final {
public:
    static Result<std::unique_ptr<DesktopComposition>> open(
        const DesktopOpenRequest& request);
    ~DesktopComposition();
    DesktopController& controller();
    Result<void> drainAndClose();

private:
    DesktopComposition() = default;
};

Result<void> registerDesktopModules(
    PlatformRegistry& platformRegistry,
    DesktopRegistry& desktopRegistry,
    DesktopController& controller);

}
```

Own objects in this destruction order: controller/watchers, query service, application/recurring services, UoW, accounting executor, profile store, profiles executor, registries. `drainAndClose` disables commands, calls `detachModels`, waits for controller jobs, drains accounting then profile executors, and is idempotent.

- [ ] **Step 4: Replace startup discovery with explicit profile/migration flow**

Keep `QStandardPaths::AppDataLocation` and one root `dailyaccount.lock`. Open `profiles.sqlite` on its platform executor. If no profile exists, `ProfileSelectionDialog` creates `本地账本` with random `ProfileId`/`localLedgerOwnerId`, system IANA time zone, and `LOCAL_UNBOUND`.

Discover but never mutate these exact source candidates: root `ledger.dat`, root `ledger.dat.bak`, `./data`, and `<applicationDir>/data`. Deduplicate physical directories. When any source exists, show its canonical path, kind, summary, and target profile label; migrate only after the user confirms. Multiple valid sources require explicit selection. A corrupt selected source shows the parser error and exits without trying another source.

For a profile with no DAT selection, create schema 2 through staging and activate an empty database. For an existing database, validate owner/schema before creating the controller. Once `accounting.sqlite` contains the matching identity, startup never examines DAT to decide normal storage and never falls back to it.

- [ ] **Step 5: Mark the SQLite one-way write point and remove production DAT dependencies**

On the first successful non-migration application command, Task 8's outbox repository sets `database_identity.sqlite_writes_started_at_ms` from `OutboxMutation.createdAt` in the same business transaction. On future opens, its presence makes DAT fallback an explicit `StorageFailure` even if someone restores an old executable configuration, and startup validates immutable database markers rather than comparing the now-mutable file with its activation-image hash. Migration rows themselves do not count as a user business write because Task 11 inserts their outbox rows directly, but an activated migrated database is still the sole storage source for this binary.

Link `dailyaccount_desktop` to desktop controller/composition, profile, SQLite, and read-only DAT migration targets; remove `dailyaccount_legacy_backend` and its include paths from every shipping target. The legacy backend remains linked only to `dailyaccount_backend_tests`.

- [ ] **Step 6: Handle qmake based on the recorded G1 state**

Run:

```bash
tracked_qmake="$(git ls-files '*.pro')"
if test -n "$tracked_qmake"; then
  printf '%s\n' "$tracked_qmake"
fi
```

If output is empty, do not create, delete, or edit any `.pro` file. If the G1 entry notes contain an accepted exception and output contains only the three declared paths, first run the current CMake Windows package/tests, then delete exactly those existing files and remove their references. Any other `.pro` path stops the task for review. `build/build.bat` remains CMake-only and adds no qmake/mingw32-make invocation.

- [ ] **Step 7: Update build/docs and run focused/cumulative checks**

Update README storage text to the exact `profiles.sqlite`/`users/<profile>/platform.sqlite`/`accounting.sqlite` layout, explicit DAT migration, source preservation, SQLite-only new writes, profile OS-trust boundary, and absence of a legacy physical whole-ledger clear with no V1 replacement (spec sections 4.2 and 16.4). Keep the existing TXT export documentation.

Run:

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_desktop_storage_switch_tests.exe'
ctest --preset windows-desktop --output-on-failure
cmd /d /c build\build.bat
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
cmake -DDA_SOURCE_DIR="$PWD" \
  -DDA_LINK_GRAPH=build/cmake/linux-core/dailyaccount-link-graph.txt \
  -P tests/cmake/sqlite_desktop_boundary_contract.cmake
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
test -z "$(git ls-files '*.pro')"
git diff --check
```

Expected: Windows package/tests pass; the first user write changes SQLite/outbox only; active SQLite opens independently of DAT condition; desktop link/source scanner passes; no tracked qmake file remains; Linux and 22 DAT regressions pass.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add -A -- CMakeLists.txt CMakePresets.json README.md build/build.bat \
  src/apps/desktop-widgets gui tests/widgets/desktop_storage_switch_tests.cpp \
  tests/cmake/sqlite_desktop_boundary_contract.cmake
for path in jizhang.pro tests/backend_tests.pro tests/registry_tests.pro; do
  if git ls-files --error-unmatch "$path" >/dev/null 2>&1 || test -e "$path"; then
    GIT_LITERAL_PATHSPECS=1 git add -A -- "$path"
  fi
done
git commit -m "feat: switch windows storage to sqlite"
```

The conditional loop records an accepted deletion without passing nonexistent qmake pathspecs to Git. Without explicit authorization, do not run any command in this checkpoint.

---

### Task 16: Rehearse Migration Rollback, Seal G2 Evidence, and Hand Off to Stage 3

**Files:**
- Create: `tests/windows/verify_migration_rollback.ps1`
- Create: `tests/cmake/check_g2.py`
- Create: `tests/cmake/test_check_g2.py`
- Create: `docs/validation/stage-2/linux-core.log`
- Create: `docs/validation/stage-2/windows-sqlite.log`
- Create: `docs/validation/stage-2/windows-ctest.log`
- Create: `docs/validation/stage-2/migration-fixtures-results.json`
- Create: `docs/validation/stage-2/migration-fault-matrix.json`
- Create: `docs/validation/stage-2/migration-rollback.log`
- Create: `docs/validation/stage-2/migration-rollback-results.json`
- Create: `docs/validation/stage-2/sqlite-contract-results.json`
- Create: `docs/validation/stage-2/windows-package-results.json`
- Create: `docs/validation/stage-2/source-tree.txt`
- Create: `docs/validation/stage-2/g2-evidence-index.md`
- Create: `docs/validation/stage-2/g2-results.json`
- Modify: `CMakeLists.txt`
- Test: `tests/windows/verify_migration_rollback.ps1`
- Test: `tests/cmake/test_check_g2.py`
- Test: `tests/cmake/check_g2.py`

**Interfaces:**
- Consumes: accepted G1 evidence/package hash, every Stage 2 test, G0 DAT expected summaries, CP-02/CP-03/CP-04 boundaries, and the final CMake package.
- Produces: machine-readable schema/outbox/profile/migration/Windows evidence, a pre-activation rollback drill, a post-activation no-DAT-fallback assertion, and exact `G2 PASS` output.

- [ ] **Step 1: Unit-test the G2 checker with synthetic repositories**

Use Python `unittest` and `tempfile.TemporaryDirectory`. Include these exact tests:

```python
def test_rejects_failed_g1(self):
    self.write_json("docs/validation/stage-1/g1-results.json", {"gate": "G1", "result": "FAIL"})
    self.assert_failure("G1 result is not PASS")

def test_rejects_missing_dat_regression(self):
    self.write_json("docs/validation/stage-2/sqlite-contract-results.json", {"legacyBackendTestCount": 21})
    self.assert_failure("legacy DAT regression count is not 22")

def test_rejects_non_atomic_outbox(self):
    self.write_json("docs/validation/stage-2/sqlite-contract-results.json", {"atomicOutbox": False})
    self.assert_failure("atomic outbox evidence is not PASS")

def test_rejects_dat_writer_in_desktop(self):
    self.write("build/cmake/windows-desktop/dailyaccount-link-graph.txt",
               "dailyaccount_desktop|dailyaccount_legacy_backend|\n")
    self.assert_failure("desktop still links the legacy DAT backend")

def test_rejects_incomplete_fault_matrix(self):
    self.write_json("docs/validation/stage-2/migration-fault-matrix.json", {"passed": 5, "total": 6})
    self.assert_failure("migration fault matrix is incomplete")

def test_accepts_complete_g2_fixture(self):
    self.assert_success(
        "G2 PASS: SQLite schema 2, atomic outbox, DAT migration parity, Windows SQLite-only")
```

- [ ] **Step 2: Run checker tests red**

```bash
python3 -m unittest tests/cmake/test_check_g2.py -v
```

Expected: import/file-not-found failure for `tests/cmake/check_g2.py`.

- [ ] **Step 3: Implement the authoritative G2 checker**

`check_g2.py --root DIR --json PATH` must:

- Require accepted G1 `PASS`, 22 legacy regressions, zero failures, and matching G1 artifact hashes.
- Require accounting schema version 2, exact migration names/hashes, `STRICT` table evidence, `foreign_keys=ON`, WAL/FULL/5000 settings, `integrity_check=ok`, empty `foreign_key_check`, future-reader rejection, and owner mismatch rejection.
- Require executor worker ownership, 8,000 serialized writes, bounded busy duration, drain-before-close, no connection warning, and the exact D-028 backup mechanism.
- Require two profile directories with no database/path overlap and unique remote-binding enforcement.
- Require successful business write evidence with one matching timestamped outbox mutation, stable mutation/entity UUIDs, preserved server revision, and the one-way SQLite write marker committed atomically; require injected outbox failure to roll back both the business row and marker.
- Require recurring deterministic vector IDs, unique rule/period enforcement, one pending transaction/occurrence pair, and `OccurrenceLinked` generic-delete rejection.
- Require V3 and legacy fixture record count, income, expense, dates, and category parity against their G0 `expected.json`; require corrupt checksum rejection and unchanged source hashes/mtimes.
- Require all six migration fault states and exact recovery outcomes, no partial activation, no WAL/SHM sidecar at activation, and a completed manifest/database marker/activation-hash match before user writes. After the write marker exists, require owner/source/schema/integrity validation and reject any checker that demands equality with the stale full-file activation hash.
- Require the pre-activation G1 rollback package to start against unchanged isolated DAT after injected G2 migration failure. Require post-activation G2 restart from SQLite with DAT unavailable and explicitly forbid using G1/DAT as a writable rollback after CP-04.
- Require Windows Widgets flow/dashboard/statistics/category/export/package tests, no clear-ledger UI, no GUI `Ledger`/`CategoryManager`, and no transitive desktop link to `dailyaccount_legacy_backend`.
- Reject tracked `.pro` files, qmake references in public build/docs, tracked databases/WAL/SHM files, deployed binaries/DLLs, private fixture inputs, tokens, and changes outside the declared Stage 2 surface except pre-recorded unrelated paths.
- Require Linux CTest `100% tests passed`, direct `22 test(s) passed`, Windows CTest zero failures, and one source tree ID shared by final Linux/Windows evidence.
- Write JSON only after all checks pass with `gate=G2`, `result=PASS`, `schemaVersion=2`, `legacyBackendTestCount=22`, `atomicOutbox=true`, `datMigrationParity=true`, `windowsStorage="SQLITE_ONLY"`, and `failureCount=0`.
- Print exactly `G2 PASS: SQLite schema 2, atomic outbox, DAT migration parity, Windows SQLite-only`.

- [ ] **Step 4: Implement the rollback verifier**

`verify_migration_rollback.ps1` accepts mandatory `-G1PackageRoot`, `-G2PackageRoot`, `-FixtureRoot`, `-ResultPath`, and `-WorkingRoot`. It copies only the sanitized fixture to isolated app-data, hashes every source file, launches G2 with test-only environment variable `DA_TEST_MIGRATION_FAULT=BEFORE_DATABASE_ACTIVATION`, requires a non-zero controlled migration-failure exit and no active `accounting.sqlite`, rehashes the source, then launches the immutable G1 package for eight seconds against that unchanged copy. It next performs a successful G2 migration in a fresh isolated root, creates one SQLite transaction, makes DAT unavailable, restarts G2, and verifies the new transaction remains visible.

Write JSON with:

```json
{
  "gate": "CP-03/CP-04",
  "result": "PASS",
  "preActivationSourceUnchanged": true,
  "preActivationG1PackageStarts": true,
  "postActivationG2RestartsWithoutDat": true,
  "postActivationDatFallbackAttempted": false
}
```

The environment fault hook is honored only by a test build (`DA_ENABLE_MIGRATION_FAULTS=ON`); release builds reject/ignore it and contain no command-line path that bypasses migration validation.

- [ ] **Step 5: Run the complete Linux and Windows evidence suites**

On Linux:

```bash
set -o pipefail
mkdir -p docs/validation/stage-2
cmake --preset linux-core 2>&1 | tee docs/validation/stage-2/linux-core.log
cmake --build --preset linux-core --parallel 2 2>&1 | tee -a docs/validation/stage-2/linux-core.log
ctest --preset linux-core --output-on-failure 2>&1 | tee -a docs/validation/stage-2/linux-core.log
./build/cmake/linux-core/dailyaccount_backend_tests 2>&1 | tee -a docs/validation/stage-2/linux-core.log
```

On the accepted Windows machine:

```powershell
cmake --preset windows-desktop 2>&1 |
  Tee-Object docs\validation\stage-2\windows-sqlite.log
cmake --build --preset windows-desktop --parallel 2 2>&1 |
  Tee-Object -Append docs\validation\stage-2\windows-sqlite.log
ctest --preset windows-desktop --output-on-failure 2>&1 |
  Tee-Object docs\validation\stage-2\windows-ctest.log
if ($LASTEXITCODE -ne 0) { throw 'Stage 2 Windows CTest failed' }
cmd /d /c build\build.bat
if ($LASTEXITCODE -ne 0) { throw 'Stage 2 package build failed' }
powershell -NoProfile -ExecutionPolicy Bypass -File tests\windows\verify_migration_rollback.ps1 `
  -G1PackageRoot artifacts\stage-1\windows `
  -G2PackageRoot build\dist `
  -FixtureRoot tests\fixtures\dat\v3-sanitized `
  -WorkingRoot $env:TEMP\dailyaccount-g2-rollback `
  -ResultPath docs\validation\stage-2\migration-rollback-results.json 2>&1 |
  Tee-Object docs\validation\stage-2\migration-rollback.log
```

Expected: all tests pass; DAT prints 22; migration fixture/fault executables write PASS JSON; the package starts; rollback JSON matches the exact object above.

- [ ] **Step 6: Record source tree and structured evidence**

Use a temporary index so evidence does not alter the real index:

```bash
STAGE2_INDEX=/tmp/opencode/dailyaccount-stage2-index
rm -f "$STAGE2_INDEX"
GIT_INDEX_FILE="$STAGE2_INDEX" git read-tree HEAD
GIT_INDEX_FILE="$STAGE2_INDEX" git add -A -- \
  CMakeLists.txt CMakePresets.json README.md build/build.bat src backend gui tests \
  docs/validation/stage-2
for path in jizhang.pro tests/backend_tests.pro tests/registry_tests.pro; do
  if GIT_INDEX_FILE="$STAGE2_INDEX" git ls-files --error-unmatch "$path" >/dev/null 2>&1 || test -e "$path"; then
    GIT_INDEX_FILE="$STAGE2_INDEX" GIT_LITERAL_PATHSPECS=1 git add -A -- "$path"
  fi
done
GIT_INDEX_FILE="$STAGE2_INDEX" git diff --cached --check
GIT_INDEX_FILE="$STAGE2_INDEX" git write-tree > docs/validation/stage-2/source-tree.txt
rm -f "$STAGE2_INDEX"
```

Create `sqlite-contract-results.json` from actual test output with schema/pragmas/constraints/executor/outbox/recurring/profile fields and `result=PASS`. Create `migration-fixtures-results.json` with both expected summaries, source before/after hashes, activated DB hashes, outbox counts, and `result=PASS`. Create `windows-package-results.json` with source tree, package hash, required Qt SQL driver file `sqldrivers/qsqlite.dll`, startup result, and `storage=SQLITE_ONLY`.

- [ ] **Step 7: Write the evidence index and run G2 green**

`g2-evidence-index.md` contains sections `Gate result`, `Base revision and source tree`, `G1 prerequisite`, `Linux core and DAT oracle`, `QSQLITE executor`, `Profile isolation`, `Schema and constraints`, `Repository and atomic outbox`, `Recurring minimum`, `V3 migration parity`, `Legacy-pair migration parity`, `Fault matrix`, `CP-02`, `CP-03`, `CP-04`, `Windows Widgets parity`, `Windows package`, `qmake status`, `Artifact hashes`, `Exceptions`, and `Stage 3 inputs`. Every section names command, UTC time, OS/tool version, exit code, raw log/result, and SHA-256. `Exceptions` is `None` unless an accepted G0/G1 decision explicitly applies.

Run:

```bash
python3 -m unittest tests/cmake/test_check_g2.py -v
python3 tests/cmake/check_g2.py \
  --root . \
  --json docs/validation/stage-2/g2-results.json
git diff --check
git status --short
```

Expected stdout:

```text
G2 PASS: SQLite schema 2, atomic outbox, DAT migration parity, Windows SQLite-only
```

Expected JSON: `gate=G2`, `result=PASS`, `schemaVersion=2`, `legacyBackendTestCount=22`, `atomicOutbox=true`, `datMigrationParity=true`, `windowsStorage=SQLITE_ONLY`, and `failureCount=0`. Status contains only declared Stage 2 changes plus recorded unrelated work; no database, WAL/SHM, package binary, private DAT, or secret is tracked.

- [ ] **Step 8: Request independent G2 review**

The reviewer reruns both platform suites, the 22-case DAT oracle, schema/fault/migration tests, desktop boundary scanner, checker unit tests, real G2 checker, Windows package smoke, and rollback script. The reviewer compares fixture expected JSON to SQLite queries, traces one successful and one failed command through entity/local-state/outbox transaction boundaries, verifies all query/database handles close before removal, and confirms post-CP-04 startup cannot select DAT.

- [ ] **Step 9: Use the optional final checkpoint only with explicit authorization**

After inspecting `git status`, `git diff`, and recent commits, and only with explicit authorization:

```bash
git add -A -- CMakeLists.txt CMakePresets.json README.md build/build.bat src backend gui tests \
  docs/validation/stage-2
for path in jizhang.pro tests/backend_tests.pro tests/registry_tests.pro; do
  if git ls-files --error-unmatch "$path" >/dev/null 2>&1 || test -e "$path"; then
    GIT_LITERAL_PATHSPECS=1 git add -A -- "$path"
  fi
done
git commit -m "feat: complete sqlite and dat migration"
```

Without authorization, leave all reviewed Stage 2 changes uncommitted.

---

## G2 Checklist

- [ ] The authoritative G1 checker still prints exactly `G1 PASS: Linux core, Windows desktop, 22 legacy regressions, CMake-only build`, and all D-020 through D-030 inputs remain accepted.
- [ ] All 22 DAT regression names/assertions remain in `tests/legacy/dat_regression_tests.cpp`; the direct target ends with exactly `22 test(s) passed`.
- [ ] Target Transaction, Account, Category, Tag, RecurringRule, and RecurringOccurrence validators enforce CNY/minor-unit/date/name/type/link constraints without Qt or SQL dependencies.
- [ ] UUIDv5 vector `2456c362-7021-512c-a5fc-4be1a167cd5c` and pending ID `faff3e00-97c1-5865-8c73-23ed45ca7f24` pass on Linux and Windows.
- [ ] Transaction create/update/delete and account/category/tag create/update/archive commands validate before writing, preserve server revisions, and reuse caller-provided stable IDs.
- [ ] Generic transaction deletion returns `OccurrenceLinked` and commits neither tombstone nor outbox when a live occurrence references the transaction.
- [ ] `profiles.sqlite` and every `users/<profile-id>/<module>.sqlite` use separate serial executors; two profiles have disjoint directories/databases and immutable owner checks.
- [ ] Every QSQLITE connection uses a unique `da/<profile>/<module>/<sequence>` name, exists only on its worker thread, applies foreign keys/WAL/FULL/5000 ms, drains queued work, and is removed without warnings.
- [ ] Accounting schema version 2 uses supported `STRICT` tables, bound values, FK/CHECK/UNIQUE constraints, migration hashes, owner identity, `integrity_check`, and `foreign_key_check`.
- [ ] A failed schema migration leaves the prior active database usable and unchanged; a newer schema is rejected with `UpgradeRequired` rather than downgraded.
- [ ] SQLite repositories implement every G1 repository/UoW contract without exposing Qt SQL outward; ordinary reads omit tombstones.
- [ ] Every successful business command commits business rows, local dirty state, base-revision expectations, and exactly one outbox mutation together; every injected failure rolls all of them back.
- [ ] Minimum recurring generation atomically creates one deterministic pending occurrence/transaction pair and one outbox change group; a repeated device/period attempt creates no duplicate.
- [ ] `DatImporter` opens only the explicitly selected source read-only, preserves every V3/legacy compatibility behavior, rejects corruption, and changes no source file/directory entry/mtime.
- [ ] V3 and legacy fixtures preserve exact record count, income, expense, dates, and category set; old refund/transfer-named incomes remain incomes and no time/account is invented.
- [ ] Migration IDs/outbox IDs are replay-stable from one persisted namespace, and the target profile must contain zero accounting entities.
- [ ] Manifest states are exactly `PREPARED`, `DATABASE_ACTIVATED`, and `COMPLETED`; all six fault points recover without partial activation or source mutation.
- [ ] Staging is same-directory, checkpointed, sidecar-free, fully closed, and validated before atomic activation; completed manifest, in-database marker, owner, source hash, and activation-image hash agree before user writes, while post-write startup validates immutable markers/integrity instead of a stale full-file hash.
- [ ] Flow/category/dashboard/statistics/export/status behavior runs through controller/read models against SQLite; money does not cross application/view-model boundaries as `double`.
- [ ] The legacy physical whole-ledger-clear UI and call path are absent; selected transaction deletion, local-profile deletion, sign-out, and cloud-account deletion remain distinct concepts.
- [ ] `dailyaccount_desktop` has no transitive `dailyaccount_legacy_backend` link; GUI/startup contains no `Ledger`, `CategoryManager`, `StorageManager`, or DAT save call.
- [ ] New Windows writes change only `accounting.sqlite` and outbox; no new DAT snapshot is created, and activated SQLite restarts when DAT is absent/corrupt.
- [ ] qmake files were left untouched when already retired; otherwise only the accepted remaining three `.pro` paths were removed after CMake parity. The final tracked tree contains no `.pro` file.
- [ ] Pre-activation failure evidence proves unchanged DAT still opens in the immutable G1 package. Post-CP-04 evidence explicitly forbids writable DAT rollback and proves G2 restarts from SQLite only.
- [ ] Windows package includes `sqldrivers/qsqlite.dll`, passes offscreen Widgets/runtime smoke, and shares the final source tree recorded by Linux evidence.
- [ ] `check_g2.py` prints exactly `G2 PASS: SQLite schema 2, atomic outbox, DAT migration parity, Windows SQLite-only`; `git diff --check` is silent; independent review accepts all evidence.
- [ ] No unrelated user change, generated database/WAL/SHM, build output, private DAT/text, credential, commit, or tag was modified without explicit authorization.

## Stage 3 Handoff

Stage 3 may begin only when every G2 checkbox is checked and `docs/validation/stage-2/g2-results.json` records `PASS`. Its executor must read the architecture, master plan, D-020 through D-030, G0/G1/G2 evidence indexes, and all public headers under `src/core/`, `src/platform/`, `src/modules/accounting/`, and `src/apps/desktop-widgets/` before adding Android code.

Stage 3 inherits these immutable inputs:

- `StrongUuid`, exact CNY/date helpers, entity validators, application commands, recurrence calculations, and deterministic occurrence/pending IDs are shared standard-C++ contracts; Android must call them rather than duplicate rules in QML/Kotlin.
- `ProfileDirectoryLocator`, `ModuleDbExecutor`, schema version 2, `SqliteAccountingUnitOfWork`, and `SqliteAccountingQueryService` are the local persistence path. Android creates its own profile/module executor and connection; it never shares the Windows connection model or opens a database on the QML thread.
- Every Android business success follows the same local entity plus outbox transaction. Stage 3 does not wait for authentication/network and does not change `serverRevision` locally.
- QML transaction amounts cross as decimal strings; C++ parses them to `MoneyMinor`. QML/JavaScript never carries or computes minor units in `int`/`Number`.
- The minimum recurring vertical slice may create the deterministic pending pair from this stage. Stage 3 adds list/home-card confirmation and the accepted native reminder bridge without weakening the `OccurrenceLinked` guard or aggregate atomicity.
- DAT migration remains Windows-only, explicit, and read-only. Android packages do not link `dailyaccount_legacy_backend` or expose DAT migration UI.
- Schema changes needed by Stage 3 must be a new forward migration with an incremented module schema version, staging/rollback tests, and updates to G2 handoff evidence; never edit an already-released migration resource in place.
- Cloud auth/sync transport, conflict tables/flows, complete recurring lifecycle, accounts/transfers/refunds, text import, analytics expansion, and recovery/export release hardening remain owned by Stages 4-6.
