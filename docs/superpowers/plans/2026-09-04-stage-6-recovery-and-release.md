# DailyAccount Stage 6 Recovery, Security, Installers, Drills, and Release Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Close the first-version release loop defined by `docs/product-architecture.md` section 20 (Stage 6) and section 21: versioned JSON exchange and CSV export, consistent per-module backup sets, three rehearsed restore paths, four distinct data-lifecycle operations with an idempotent cloud-account deletion workflow, security/privacy/log/credential proof, Windows and Android installers with clean-device smoke tests, five fault drills, a full section 3.5/21 acceptance evidence package, and a recorded release decision.

**Architecture:** Enter only from accepted G5. Add export, import, backup, restore, recovery-profile, and lifecycle services as new application/platform layers that consume the existing schema-version-5 databases through the existing serial per-module executors, outbox, bootstrap, and reviewed-import machines; never copy live WAL/SHM files, never overwrite a live profile with an unvalidated database, never reuse stale cursors, never open a second online write path, and keep device-local rows (import raw text, balance snapshots, settings, tokens) out of every exchange, backup manifest payload, log, and cloud fixture.

**Tech Stack:** C++17, CMake 3.22.1+, Qt 6.9.3 Core/Network/SQL/Widgets/QML/Quick/Test, QSQLITE, SQLite Online Backup API or `VACUUM INTO`, Supabase CLI/PostgreSQL 15+/pgTAP, Kotlin 2.0.21 and Android API 28/35 per D-020/D-030, MinGW on Windows, GCC on Linux, Python 3, PowerShell, NSIS via CPack on the accepted Windows host, and the D-029/D-030 background-mode decisions.

**Source contracts:** `docs/product-architecture.md` sections 3.5, 6.3, 6.4, 15-17, 19.2-19.4, 19.6, 20 (Stage 6), 21.1-21.5, 23 items 11/13, and 24; `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md` (G6 gate, CP-07/CP-08, Acceptance Traceability, Completion Rule); `docs/superpowers/plans/2026-09-04-stage-0-baseline-and-prototypes.md` through `docs/superpowers/plans/2026-09-04-stage-5-accounting-workflows.md` (G5 handoff, schema version 5, D-020 through D-031, staging/bootstrap/outbox/conflict contracts, import-privacy boundary).

## Execution Rules

- Run tasks in order. Do not start Task 1 until the entry gate prints its exact PASS line, and do not start Task N+1 until Task N's green run and `git diff --check` pass.
- Accepted D-020 through D-031 outcomes override examples here. A contradiction stops execution for review rather than silently changing an accepted contract.
- Keep the Stage 1-5 public seams (`ISyncTransport`, `IAuthClient`, `ISecureStore`, `INotificationScheduler`, `IAccountingUnitOfWork`, repository contracts, codec command set, schema version 5 rows) unchanged. Stage 6 adds callers and one additive server lifecycle table plus lifecycle RPCs; it does not edit `001_initial.sql` through `005_import.sql` and does not bump the local schema version above 5.
- The ledger exchange format is its own versioned format (version 1) independent of the local schema version. Exchange and CSV export never include import raw text, full original import items, module settings, balance snapshots, sessions, tokens, device IDs, or cursor/epoch state.
- Every import path validates format version, owner target, stable IDs, referential closure, duplicates, currency, and untrusted size/type bounds, then writes only through the existing UoW transaction-plus-outbox path; nothing outside the reviewed import or exchange import may create business rows.
- Backup sets follow section 16.3: one manifest plus one consistent snapshot per module database produced by the serial executor (Online Backup API or `VACUUM INTO`), each file carrying schema version and SHA-256, with interrupted-set detection at startup. The platform never claims cross-file SQLite atomicity.
- Restore paths follow section 16.3 and CP-07: unbound staging validation with atomic activation; recovery profile opened isolated without binding or sync; synchronized-profile merge restore from a fresh server snapshot with reviewed mutations and never a stale cursor. Recovery must preserve D-031 occurrence/transaction aggregate invariants.
- Every destructive lifecycle operation (restore replacement, delete local copy, cloud-account deletion) requires a completed pre-change backup, an explicit atomic activation point, a tested failure path leaving the prior state usable, and a distinct, separately confirmed entry in every UI surface. No surface may offer profile- or ledger-wide clearing (spec sections 4.2 and 16.4, decision D-019); the legacy physical clear is removed and never emulated with repeated per-row deletion.
- Cloud-account deletion is the only path that removes cloud data, is admin-executed through a recorded runbook with stable request IDs, and is idempotent with persistent terminal state recorded outside the business schema. Sync/auth entry points reject writes while the account is `DELETING` or `DELETED`; offline devices are never claimed to be remotely erased.
- Logs and evidence may contain bounded codes, counts, timings, request IDs, region/plan labels, schema/format versions, and pass booleans; they may not contain credentials, tokens, emails, full transaction content, amounts, merchant/note text, raw import text, cursors, or full response bodies.
- Every behavior-changing task follows red-green-refactor: add the focused failing test, observe the stated failure, implement the smallest slice, then run focused and cumulative verification and `git diff --check`.
- Checkpoint commits are optional and may run only after explicit authorization in the implementation session. This planning change grants no commit authorization.
- Preserve unrelated worktree changes and generated-artifact exclusions. Never stash, reset, clean, or stage unrelated paths; never commit credentials, keystore material, private import samples, raw text, databases, WAL/SHM files, APKs, installers, build trees, device serials, or logs containing sensitive content.

## Stage 6 Entry Gate

- [ ] **Verify G5 evidence, the G5 checker, accepted decisions, and an absent stage-6 surface**
Run from the repository root:

```bash
test -f docs/validation/stage-5/g5-results.json
test -f docs/validation/stage-5/g5-evidence-index.md
test -f tests/cmake/check_g5.py
test ! -e docs/validation/stage-6
test ! -e tests/cmake/check_g6.py
for number in 020 021 022 023 024 025 026 027 028 029 030 031; do
  matches=(docs/decisions/D-${number}-*.md); test "${#matches[@]}" -eq 1; test -f "${matches[0]}"
done
```

Expected: every command exits `0`, each accepted ADR resolves to exactly one file, and neither `docs/validation/stage-6` nor `check_g6.py` exists yet.

- [ ] **Re-run the authoritative G5 checker without replacing accepted evidence**

```bash
python3 tests/cmake/check_g5.py --root . \
  --json /tmp/opencode/dailyaccount-stage6-g5-recheck.json
```

Expected stdout exactly: `G5 PASS: accounting workflows, recurring lifecycle, reminders, import privacy, and analytics`.

- [ ] **Compare accepted and fresh G5 records and confirm the inheritance surface**

```bash
python3 - <<'PY'
import json
from pathlib import Path
expected = {
    "gate": "G5", "result": "PASS", "localSchemaVersion": 5,
    "failureCount": 0,
}
for record in [
    json.loads(Path("docs/validation/stage-5/g5-results.json").read_text(encoding="utf-8")),
    json.loads(Path("/tmp/opencode/dailyaccount-stage6-g5-recheck.json").read_text(encoding="utf-8")),
]:
    for key, value in expected.items():
        assert record[key] == value, (key, record[key])
for path in (
    "src/modules/accounting/application/recurring_service.h",
    "src/modules/accounting/application/accounting_query_service.h",
    "src/modules/accounting/application/import_service.h",
    "src/modules/accounting/analytics/monthly_breakdown_query.h",
    "src/platform/sync/sync_coordinator.h",
    "src/platform/profile/profile_store.h",
    "src/platform/interfaces/secure_store.h",
):
    assert Path(path).exists(), path
print("Stage 6 entry gate: PASS (G5, D-020 through D-031)")
PY
```

Expected: exactly `Stage 6 entry gate: PASS (G5, D-020 through D-031)`.

- [ ] **Confirm prerequisites and remaining open items**
Read the G5 handoff block of the Stage 5 plan, the accepted D-021/D-025/D-027/D-029/D-030/D-031 outcomes, and the Stage 4 bootstrap and whole-group machinery this plan consumes. `docs/product-architecture.md` section 23 item 13 is closed by D-031; item 11 (cloud backup retention after account deletion) must be recorded by Task 5 before any deletion runbook is exercised. Nothing in this plan guesses around a remaining open item; record the selected outcome of each consulted ADR in the execution notes.

## Inherited Public Boundary

Stage 6 consumes, without changing: schema version 5 databases per profile (`profiles.sqlite`, `platform.sqlite`, `accounting.sqlite` under `users/<profile-uuid>/`), the serial `module_db_executor` with drain-before-destroy semantics, outbox freeze/rebase and business-plus-outbox single-transaction writes, immutable profile subject binding and `remoteUserId`, server snapshot bootstrap with continuation tokens, commit-ordered opaque cursors and `minValidCursor`, whole change groups, the reviewed import batch machine, D-031 occurrence/transaction undo invariants, `IRemoteHealthCheck`, and the three RPCs `da_sync_push` / `da_sync_pull` / `da_sync_bootstrap`. Exchange format version, backup format, installer artifacts, and the new lifecycle table are new additive surfaces; they are not sync protocol or schema-version changes.

---

### Task 1: Add Versioned Ledger Exchange JSON Export and Reviewed Import

**Files:**
- Create: `src/modules/accounting/export/ledger_exchange_writer.h`
- Create: `src/modules/accounting/export/ledger_exchange_writer.cpp`
- Create: `src/modules/accounting/import/ledger_exchange_reader.h`
- Create: `src/modules/accounting/import/ledger_exchange_reader.cpp`
- Create: `src/modules/accounting/application/exchange_service.h`
- Create: `src/modules/accounting/application/exchange_service.cpp`
- Create: `tests/unit/ledger_exchange_tests.cpp`
- Create: `tests/integration/ledger_exchange_sqlite_tests.cpp`
- Create: `tests/fixtures/exchange/v1-valid.json`, `v1-cross-owner.json`, `v1-duplicate-id.json`, `v1-broken-ref.json`, `v1-future-version.json`, `v1-oversize.json`
- Modify: `src/apps/desktop-widgets/desktop_controller.h`
- Modify: `src/apps/desktop-widgets/desktop_controller.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**
- The exchange wrapper is versioned and self-describing; version 1 carries `format: "dailyaccount-ledger-exchange"`, `formatVersion: 1`, `exportedAtMs`, `currency: "CNY"`, `timeFormat: {"localDate": "YYYY-MM-DD", "utcInstantMs": "epoch"}` plus `localDate`/`UtcInstant` declarations, `ownerId` (the bound `remoteUserId`, or `null` for an unbound profile), `entitySets`, and entities with their stable IDs and full sync metadata. Writer output is canonical compact JSON with byte caps per entity and per set.
- The writer covers synchronizable accounting entities: transactions, accounts, categories, tags, recurring rules, recurring occurrences (including occurrence/transaction linkage), and minimal transaction provenance; it excludes import raw text, import batches/items, module settings, balance snapshots, sessions, tokens, cursors, and epochs. See section 16.3.
- The reader rejects: unsupported format/version, owner mismatch against the target profile binding, duplicate stable IDs inside the file, referential closure violations (missing categories/accounts/rules/original expenses), non-CNY rows, malformed or out-of-range types, oversize payloads, and rows whose content contradicts their entity kind. Every rejection is a structured error naming the offending entity; no partial write occurs.
- `ExchangeService::exportLedgerJson()` reads through the accounting query layer; `importLedgerJson(text, ImportMode)` writes via the normal UoW: direct import when the target profile is empty and unbound, reviewed-import mode (batch preview with per-item confirmation) when the profile already holds entities, per master contract 16.3/17.1. Outbox mutations are stable per imported entity and created in the same transaction as the rows.
- Desktop surfaces: export-to-file and import-from-file dialogs wired through the controller; no import touches a synced profile without the reviewed step.
- [ ] **Write the failing tests**

```cpp
void jsonRoundTripPreservesIdsTotalsAndRejectsCrossOwner()
{
    ExchangeFixture fixture; fixture.seedMonth(2026, 9, /*expenseMinor*/ 12850);
    const auto exported = fixture.exportLedgerJson().value();
    DA_CHECK(fixture.importIntoEmpty(exported).hasValue());
    DA_CHECK_EQ(fixture.importedCount(), fixture.seededCount());
    DA_CHECK_EQ(fixture.importedExpenseTotal(), MoneyMinor{-12850});
    DA_CHECK(!fixture.importCrossOwner(exported).hasValue());
}
```
Also test future-version, duplicate-ID, broken-ref, and oversize fixtures each fail with the named entity, reviewed import into a populated profile requires confirmation and then writes one outbox row per confirmed entity, direct import writes nothing when any validation fails, and exported text contains no raw import text or token.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_ledger_exchange_tests --parallel 2
```
Expected red: writer/reader/service targets and headers are absent.
- [ ] **Implement the minimum slice**
Implement writer, strict reader, and the two import modes on the existing UoW/review machinery with bounded JSON helpers from `src/platform/sync/bounded_json.h`, wire the desktop export/import commands, and register the new test targets in `CMakeLists.txt`.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_ledger_exchange_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "ledger_exchange|sqlite_unit_of_work|exchange" --output-on-failure
```
Expected green: round trip and every malicious fixture behave as specified on Linux and SQLite, reviewed import posts no entity before confirmation, raw text and tokens are absent from exported JSON, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/modules/accounting/export/ledger_exchange_writer.h \
  src/modules/accounting/export/ledger_exchange_writer.cpp \
  src/modules/accounting/import/ledger_exchange_reader.h \
  src/modules/accounting/import/ledger_exchange_reader.cpp \
  src/modules/accounting/application/exchange_service.h \
  src/modules/accounting/application/exchange_service.cpp \
  src/apps/desktop-widgets/desktop_controller.h \
  src/apps/desktop-widgets/desktop_controller.cpp \
  tests/unit/ledger_exchange_tests.cpp \
  tests/integration/ledger_exchange_sqlite_tests.cpp \
  tests/fixtures/exchange
git commit -m "feat: add versioned ledger exchange JSON export and import"
```

---

### Task 2: Add Human-Readable CSV Export with Injection- and Privacy-Safe Cells

**Files:**
- Create: `src/modules/accounting/export/csv_export_writer.h`
- Create: `src/modules/accounting/export/csv_export_writer.cpp`
- Create: `tests/unit/csv_export_writer_tests.cpp`
- Create: `tests/integration/csv_export_sqlite_tests.cpp`
- Modify: `src/modules/accounting/application/exchange_service.h`
- Modify: `src/modules/accounting/application/exchange_service.cpp`
- Modify: `src/apps/desktop-widgets/desktop_controller.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**
- One deterministic header row with ASCII names and UTF-8 values written with a BOM for Excel compatibility; money serializes as decimal text from `MoneyMinor` (CNY two decimals, e.g. `12850` as `128.50`), never as raw minor units in CSV; every financial amount stays an integer in shared C++ and is formatted only at the writer boundary. See section 16.3 and 9.1.
- Columns: `occurred_on,type,status,amount,currency,category,subcategory,tags,merchant,note,account,destination_account,refund_of`. Transfers appear with both account columns and never enter income/expense subtotals; pending recurring transactions carry `status=PENDING`.
- Cells beginning with `=`, `+`, `-`, or `@` are prefixed with a single quote (formula-injection neutralization) before RFC-4180 quoting; `\r\n` line endings; a trailing newline; bounded row count and byte limits; export covers all live (non-tombstoned) entities requested by the caller's month filter or full scope.
- Raw import text, occurrence rule snapshots with private notes, tokens, and device metadata never reach CSV columns; note/merchant are included as user-visible fields per export intent but the CSV exporter reuses the same privacy scan contract as the JSON writer.
- [ ] **Write the failing tests**

```cpp
void formulaCellsAndQuotesAreNeutralized()
{
    CsvFixture fixture; fixture.seedNote("=SUM(A1:A9)");
    fixture.seedMerchant("@cmd");
    const auto lines = fixture.exportLines();
    DA_CHECK(lines.at(1).find("'=SUM(A1:A9)") != std::string::npos);
    DA_CHECK(lines.at(1).find("'@cmd") != std::string::npos);
    DA_CHECK_EQ(lines.at(1).find("=SUM("), std::string::npos);
}
```
Also test cent formatting with negative-booked refunds, quoted commas/newlines, BOM presence, transfer/expense exclusion from a derived expense-total fixture, PENDING rows flagged, and absence of raw text in the produced CSV.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_csv_export_writer_tests --parallel 2
```
Expected red: `csv_export_writer` sources and the test target do not exist.
- [ ] **Implement the minimum slice**
Implement the writer, add `exportCsv(scope)` to the exchange service, wire the Windows save-as-CSV command through the controller, and register the targets.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_csv_export_writer_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "csv_export|ledger_exchange|sqlite_unit_of_work" --output-on-failure
```
Expected green: every CSV cell is injection-safe, decimal formatting is exact at cent boundaries, Windows CSV export opens in Excel without macro prompts on the accepted host, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/modules/accounting/export/csv_export_writer.h \
  src/modules/accounting/export/csv_export_writer.cpp \
  src/modules/accounting/application/exchange_service.h \
  src/modules/accounting/application/exchange_service.cpp \
  src/apps/desktop-widgets/desktop_controller.cpp \
  tests/unit/csv_export_writer_tests.cpp \
  tests/integration/csv_export_sqlite_tests.cpp
git commit -m "feat: add injection-safe CSV export"
```

---

### Task 3: Add Consistent Per-Module Backup Sets with Manifest and Hashes

**Files:**
- Create: `src/platform/backup/backup_manifest.h`
- Create: `src/platform/backup/backup_manifest.cpp`
- Create: `src/platform/backup/profile_backup_service.h`
- Create: `src/platform/backup/profile_backup_service.cpp`
- Create: `tests/unit/backup_manifest_tests.cpp`
- Create: `tests/integration/profile_backup_sqlite_tests.cpp`
- Modify: `src/apps/desktop-widgets/desktop_controller.h`
- Modify: `src/apps/desktop-widgets/desktop_controller.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**
- `ProfileBackupService::createBackupSet(ProfileId)` produces one backup set under `users/<profile-uuid>/backups/backup-<epoch-ms>/` containing `manifest.json` plus one consistent snapshot per module database of that profile (`platform.sqlite`, `accounting.sqlite`), per section 16.3.
- The manifest records `formatVersion: 1`, `profileId`, `createdAtMs`, `applicationVersion`, and per-file entries `{moduleId, fileName, schemaVersion, sha256}`; each snapshot is produced on that module's serial executor via Online Backup API or `VACUUM INTO` with the profile write queue drained first (per-file consistency, no cross-file atomicity claim). A set is valid only when every file matches its manifest hash and schema version.
- Snapshot generation is interrupted-safe: files are written to `.tmp` names and renamed only after hashing; at startup the backup service lists leftover `.tmp` or manifest-less directories, verifies or deletes them, and keeps the newest `backupRetention` (device-local setting, default 3) complete sets after each successful run.
- Live DB and WAL/SHM files are never copied; generation never blocks the accounting executor beyond one drained pass; a forced backup precedes any destructive operation later in this stage (restore replacement, delete local copy, cloud deletion request).
- Desktop settings expose backup now / backup location / retention with a visible last-success timestamp; Windows page wiring only, same rules as Task 1.
- [ ] **Write the failing tests**

```cpp
void interruptedSetIsCleanedAndValidSetVerifies()
{
    BackupFixture fixture; fixture.seedProfileWithTransactions();
    DA_CHECK(fixture.service().createBackupSet().hasValue());
    fixture.plantInterruptedTemp();            // manifest-less .tmp directory
    DA_CHECK(fixture.startupCleanup().hasValue());
    DA_CHECK_EQ(fixture.tempDirectories().size(), std::size_t{0});
    DA_CHECK(fixture.latestSet().manifest().entries.size() == 2);
    DA_CHECK(fixture.latestSet().verifyHashes().hasValue());
}
```
Also test manifest hash mismatch detection, schema-version recording per module, retention pruning keeps exactly the configured newest sets, backup with a mid-flight UI write still produces a consistent snapshot (write is queued behind the drain), and no backup ever contains WAL/SHM files or the live DB path.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_backup_manifest_tests --parallel 2
```
Expected red: backup targets and headers are absent.
- [ ] **Implement the minimum slice**
Implement manifest serialize/verify, the backup service on the module executors with drained pass and `.tmp` rename protocol, startup cleanup, retention, and the desktop backup command; register targets.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_backup_manifest_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "profile_backup|backup_manifest|module_db_executor" --output-on-failure
```
Expected green: backup sets verify, interrupted sets clean up, retention holds, snapshots open independently after the source profile is removed, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/platform/backup/backup_manifest.h src/platform/backup/backup_manifest.cpp \
  src/platform/backup/profile_backup_service.h \
  src/platform/backup/profile_backup_service.cpp \
  src/apps/desktop-widgets/desktop_controller.h \
  src/apps/desktop-widgets/desktop_controller.cpp \
  tests/unit/backup_manifest_tests.cpp \
  tests/integration/profile_backup_sqlite_tests.cpp
git commit -m "feat: add consistent per-module backup sets with manifest hashes"
```

---

### Task 4: Implement and Rehearse the Three Restore Paths

**Files:**
- Create: `src/platform/profile/restore_service.h`
- Create: `src/platform/profile/restore_service.cpp`
- Create: `src/platform/profile/recovery_profile.h`
- Create: `src/platform/profile/recovery_profile.cpp`
- Create: `src/modules/accounting/application/merge_restore_service.h`
- Create: `src/modules/accounting/application/merge_restore_service.cpp`
- Create: `tests/unit/restore_validation_tests.cpp`
- Create: `tests/integration/restore_paths_sqlite_tests.cpp`
- Create: `tests/sync/merge_restore_sync_tests.cpp`
- Modify: `src/apps/desktop-widgets/desktop_controller.h`
- Modify: `src/apps/desktop-widgets/desktop_controller.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**
- Path 1 (unbound local restore, CP-07): pick a backup set, restore each snapshot into a fresh staging directory, verify manifest hashes, schema versions, `PRAGMA integrity_check`, owner profile metadata, and supported reader versions, then atomically activate by draining the profile executors, swapping directories under the platform's profile lock, and updating `profiles.sqlite`. Any failure keeps the current profile usable and deletes only staging.
- Path 2 (recovery profile): open a chosen backup set as an isolated recovery profile that is never bound to an account and never starts sync, can be browsed and exported with the existing query/export services, preserves D-031 occurrence/transaction invariants, cannot be activated while the same profile is open on the device, and shows a distinct "recovery" badge in Windows UI.
- Path 3 (synchronized-profile merge restore): for a bound profile, never replace the live DB with the old file. Fetch the current server snapshot and high-water cursor through the Stage 4 bootstrap machine, merge records missing from the backup as reviewed mutations through the Stage 5 reviewed-import machine, keep the fresh pull cursor, and leave conflicts in `sync_conflicts`. See sections 14.9 and 16.3.
- Restore commands surface only distinct actions with separate confirmations; each destructive activation runs a fresh pre-change backup first.
- [ ] **Write the failing tests**

```cpp
void syncedMergeRestoreNeverReusesTheOldCursor()
{
    MergeRestoreFixture fixture; fixture.seedBoundProfile(/*cursor*/ 40);
    fixture.deleteRemoteAfterCursor(40);           // server-only deletion
    const auto set = fixture.backupSet();          // taken earlier, cursor 12
    DA_CHECK(fixture.mergeRestore(set).hasValue());
    DA_CHECK_EQ(fixture.localCursor(), fixture.freshHighWaterCursor());
    DA_CHECK_EQ(fixture.reviewedMutationCount(), std::size_t{0});
    DA_CHECK_EQ(fixture.liveRowsReturnedByPull(), 0);  // no resurrected rows
}
```
Also test unbound staging failure on corrupted hash leaves the current profile byte-identical, recovery profile never creates outbox/cursor rows and refuses binding, D-031 pending pairs survive a recovery open, and a stale-cursor backup can never overwrite a synced profile.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_restore_validation_tests --parallel 2
```
Expected red: restore/recovery/merge headers and targets are absent.
- [ ] **Implement the minimum slice**
Implement staging validation and atomic activation, the isolated recovery profile loader, and the merge service over the Stage 4 bootstrap and Stage 5 reviewed-import machines; wire the three Windows restore commands with their confirmations and pre-change backups.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_restore_validation_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "restore_paths|merge_restore|profile_backup|sqlite_unit_of_work" --output-on-failure
```
Expected green: all three paths rehearse in integration/sync tests with no stale cursor, no resurrected tombstone, no partial activation, recovery profiles stay unbound, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/platform/profile/restore_service.h src/platform/profile/restore_service.cpp \
  src/platform/profile/recovery_profile.h src/platform/profile/recovery_profile.cpp \
  src/modules/accounting/application/merge_restore_service.h \
  src/modules/accounting/application/merge_restore_service.cpp \
  src/apps/desktop-widgets/desktop_controller.h \
  src/apps/desktop-widgets/desktop_controller.cpp \
  tests/unit/restore_validation_tests.cpp \
  tests/integration/restore_paths_sqlite_tests.cpp \
  tests/sync/merge_restore_sync_tests.cpp
git commit -m "feat: add three validated restore paths"
```

---

### Task 5: Keep Data-Lifecycle Operations Distinct and Add the Cloud-Account Deletion Workflow

**Files:**
- Create: `src/platform/profile/account_lifecycle_service.h`
- Create: `src/platform/profile/account_lifecycle_service.cpp`
- Create: `tests/unit/lifecycle_operations_tests.cpp`
- Create: `tests/integration/lifecycle_operations_sqlite_tests.cpp`
- Create: `cloud/supabase/migrations/20260904080000_stage6_account_lifecycle.sql`
- Create: `cloud/supabase/tests/database/0008_stage6_account_lifecycle.test.sql`
- Create: `cloud/supabase/admin/delete_account_runbook.md`
- Create: `docs/privacy/data-lifecycle-and-offline-limits.md`
- Modify: `src/apps/desktop-widgets/settings_page.h`
- Modify: `src/apps/desktop-widgets/settings_page.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**
- Local lifecycle operations stay distinct and independently confirmed: sign out with an explicit keep-local-copy or delete-local-copy choice (section 15.2), delete local copy (removes the selected profile directory after a fresh pre-change backup and signs this device out), delete selected transactions (existing tombstone path through sync, unchanged), and cloud-account deletion (runbook-led; never a client button that performs it). No UI, command, shortcut, or shipped string offers profile- or ledger-wide clearing (spec sections 4.2 and 16.4, decision D-019); a source scan in Task 6 asserts absence.
- `AccountLifecycleService` exposes `signOut(KeepLocalCopy|DeleteLocalCopy)`, `deleteLocalCopy(ProfileId)`, and `cloudDeleteGuidance(ProfileId)`; every destructive local action requires an explicit confirmation token and returns distinct structured results so callers cannot route them through one shared wipe path.
- Server workflow (CP-08, section 16.4): additive migration creates a lifecycle table outside the module business schema holding `request_id`, `user_id`, `state` (`ACTIVE`/`DELETING`/`DELETED`), step log, timestamps, and terminal reason; RPCs `da_lifecycle_start_delete(request_id)` (admin-only, idempotent by `request_id`), `da_lifecycle_status()` for the owner, and admin resume of recorded steps. Existing `da_sync_push/pull/bootstrap` and session refresh preconditions reject with structured `ACCOUNT_DELETING`/`ACCOUNT_DELETED` errors while not `ACTIVE`; the workflow drains in-flight module transactions, then deletes module business rows, idempotency records, change log, and snapshot tokens, and finally removes the auth identity. Terminal state survives outside the deleted business rows so interrupted deletion resumes and never leaves a half-deleted account.
- The runbook records operator identity, request ID generation, explicit confirmation tokens, step re-entry, and the Task-required retention decision closing architecture item 23.11 (provider point-in-time backup retention of 30 days after terminal `DELETED`, no self-service undo window in V1). `docs/privacy/data-lifecycle-and-offline-limits.md` documents the local OS trust boundary (section 15.4), the offline-device no-remote-erase limit (section 15.2), deletion semantics of each operation, and the cloud retention window.
- Client handling: a device receiving `ACCOUNT_DELETED` during sync marks the profile deleted, shows guidance to export first where possible, clears the session, and never re-attempts upload; offline devices are documented, not claimed erased.
- [ ] **Write the failing tests**

```cpp
void lifecycleOperationsRemainDistinctAndClearFree()
{
    LifecycleFixture fixture; fixture.seedSignedInProfile();
    DA_CHECK(fixture.service().signOut(KeepLocalCopy).hasValue());
    DA_CHECK(fixture.profileDirExists());
    DA_CHECK(fixture.service().signOut(DeleteLocalCopy).hasValue());
    DA_CHECK(!fixture.profileDirExists());
    DA_CHECK(!fixture.hasLedgerClearSurface());  // scans commands and strings
}
```
Also test pre-backup before delete-local-copy, delete-selected-transactions routes only tombstones, `cloudDeleteGuidance` never calls a server RPC itself, and a client payload carrying a wipe or clear-ledger intent is rejected by the codec.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_lifecycle_operations_tests --parallel 2
```
Expected red: lifecycle service headers and target are absent, and no cloud lifecycle migration exists.
- [ ] **Implement the minimum slice**
Implement the lifecycle service, settings-page entries with distinct confirmations, the additive Supabase migration and RPCs with `search_path` fixed and admin-only grants, pgTAP coverage, the runbook, and the privacy documentation.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_lifecycle_operations_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
```

```bash
supabase start --workdir cloud && supabase db reset --workdir cloud && supabase test db --workdir cloud --file supabase/tests/database/0008_stage6_account_lifecycle.test.sql && supabase db lint --workdir cloud --level warning --fail-on error && supabase stop --workdir cloud
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "lifecycle|restore_paths|codec" --output-on-failure
```
Expected green: pgTAP proves `DELETING` blocks new sync writes while draining, idempotent request IDs resume, terminal state persists after module-row deletion, cross-user status isolation holds, the wipe-absence scan is silent, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/platform/profile/account_lifecycle_service.h \
  src/platform/profile/account_lifecycle_service.cpp \
  tests/unit/lifecycle_operations_tests.cpp \
  tests/integration/lifecycle_operations_sqlite_tests.cpp \
  cloud/supabase/migrations/20260904080000_stage6_account_lifecycle.sql \
  cloud/supabase/tests/database/0008_stage6_account_lifecycle.test.sql \
  cloud/supabase/admin/delete_account_runbook.md \
  docs/privacy/data-lifecycle-and-offline-limits.md \
  src/apps/desktop-widgets/settings_page.h \
  src/apps/desktop-widgets/settings_page.cpp
git commit -m "feat: separate lifecycle operations and add cloud deletion workflow"
```

---

### Task 6: Prove Security, Privacy, Log Redaction, and Credential Protection

**Files:**
- Create: `src/platform/logging/redacting_log_sink.h`
- Create: `src/platform/logging/redacting_log_sink.cpp`
- Create: `tests/unit/log_redaction_tests.cpp`
- Create: `tests/integration/credential_protection_sqlite_tests.cpp`
- Create: `tests/cmake/stage6_boundary_contract.cmake`
- Modify: `src/platform/logging/log_router.cpp`
- Modify: `CMakeLists.txt`
**Interfaces and acceptance:**
- The default production log sink redacts known secret shapes (JWT payload sections, refresh tokens, passwords) and whole fields (merchant, note, amount, raw import text, cursor/epoch bodies) before write, and keeps bounded codes, counts, timings, request IDs, and pass booleans, per sections 6.5 and 16.2. Any unrecognized free-form caller text is dropped or quoted only at debug verbosity off by default.
- Runtime tests prove: a fake JWT and refresh token used in a sign-in never appear in the sink buffer; an auth failure log retains the bounded error code only; export/backup runs emit no transaction content; full raw import text placed into an import batch is absent from every log line produced during commit.
- Credential protection integration tests run against the real platform adapters on each accepted host: Android Keystore-wrapped token storage through `ISecureStore` and Windows Credential Manager on the accepted Windows host; negative cases assert plaintext fallback is impossible and stored blobs are not recoverable from the profile directory.
- `stage6_boundary_contract.cmake` scans the shipping source tree, cloud fixtures, and generated artifacts for: service-role keys, database passwords, Supabase private endpoints, TLS-ignore calls, plaintext credential stores, embedded keystore material, raw-text references in export/backup/log paths, and any ledger-clear wording; it must exit silently on a clean tree and name the offending file otherwise.
- [ ] **Write the failing tests**

```cpp
void tokensAndTransactionContentNeverReachTheLogSink()
{
    RedactingLogFixture fixture;
    const std::string token = fixture.fakeJwt();
    fixture.signInAndLog(token, "note=coffee 1234");
    DA_CHECK(fixture.buffer().find(token) == std::string::npos);
    DA_CHECK(fixture.buffer().find("coffee") == std::string::npos);
}
```
Also test structured rejection of a secret-shaped line in the cloud fixture corpus and the contract scan naming a planted `service_role` string.
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_log_redaction_tests --parallel 2
```
Expected red: redacting sink and its target do not exist.
- [ ] **Implement the minimum slice**
Implement the redacting sink and route production logging through it, add the credential-protection integration tests on both hosts, and implement the boundary contract scan over shipping source, fixtures, and artifacts.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ./build/cmake/linux-core/dailyaccount_log_redaction_tests && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/stage6_boundary_contract.cmake
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop -R "credential_protection|log_redaction|sync" --output-on-failure
```
Expected green: token/text redaction holds under the sink, Credential Manager and Keystore adapters pass positive and negative cases, the boundary scan is silent over the real tree, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  src/platform/logging/redacting_log_sink.h src/platform/logging/redacting_log_sink.cpp \
  src/platform/logging/log_router.cpp \
  tests/unit/log_redaction_tests.cpp \
  tests/integration/credential_protection_sqlite_tests.cpp \
  tests/cmake/stage6_boundary_contract.cmake
git commit -m "test: prove log redaction and credential protection"
```

---

### Task 7: Build Windows and Android Installers and Pass Clean-Device Smoke Tests

**Files:**
- Create: `packaging/windows/CMakeInstallRules.cmake` (referenced from `CMakeLists.txt`)
- Create: `build/package-windows.ps1`
- Create: `build/package-android.ps1`
- Create: `tests/cmake/installer_smoke_contract.cmake`
- Create: `tests/cmake/test_installer_smoke.py`
- Create: `tests/smoke/windows_clean_smoke.ps1`
- Create: `tests/smoke/android_clean_smoke.py`
- Create (generated, gate evidence): `docs/validation/stage-6/installer-windows-smoke.json`
- Create (generated, gate evidence): `docs/validation/stage-6/installer-android-smoke.json`
- Modify: `CMakeLists.txt` (install rules and CPack NSIS generator)
- Modify: `CMakePresets.json` (add `android-arm64-release`)
- Modify: `src/apps/desktop-widgets/main.cpp` (`--selftest` startup validation mode)
- Modify: `src/apps/android-qml/android/build.gradle` (release signing from environment only)
**Interfaces and acceptance:**
- Windows: `build/package-windows.ps1` configures and builds `windows-desktop`, runs `windeployqt`, stages the executable, Qt runtime DLLs, SQL drivers, and translations, and produces `artifacts/stage-6/DailyAccount-1.0.0-setup.exe` through CPack NSIS with per-user install, Start-menu entries, and uninstall support. `--selftest` opens the profile store offscreen, runs startup database validation, and exits `0` after printing exactly `DA_SELFTEST_OK`.
- Android: `build/package-android.ps1` builds preset `android-arm64-release` and signs with a release keystore whose path, alias, and passwords come only from environment variables (`DA_ANDROID_KEYSTORE`, `DA_ANDROID_KEYSTORE_PASS`, `DA_ANDROID_KEY_ALIAS`, `DA_ANDROID_KEY_PASS`); output `artifacts/stage-6/DailyAccount-1.0.0-arm64-v8a-release.apk` embeds no signing material and contains `libDailyAccount_arm64-v8a.so` plus Qt deployment files.
- Clean-device smoke (recorded JSON each): on a clean Windows VM snapshot, install silently, launch, create an offline profile, add one expense, relaunch, run `--selftest`, and uninstall; on wiped Android devices (API 28 and API 35), `adb uninstall` any prior install, `adb install` the release APK, cold-launch, create the same offline profile offline, add one expense, relaunch, and confirm a logcat marker `da-smoke-ok` with no crash. The smoke scripts write `installer-windows-smoke.json` and `installer-android-smoke.json`; `installer_smoke_contract.cmake` verifies both and writes the aggregate `docs/validation/stage-6/installer-smoke-results.json`. Smoke records OS/device alias, exact tool versions, APK/SHA-256, timings, and pass booleans only.
- Binary artifacts (installers, APKs) stay under `artifacts/stage-6/` and remain untracked unless repository policy separately changes; sanitized evidence JSONs and logs under `docs/validation/stage-6/` are tracked gate evidence and are listed in each task's Files and checkpoint sections.
- [ ] **Write the failing contract tests**

```python
def test_release_apk_has_no_signing_material_and_reports_sha256(self):
    record = self.read_json("docs/validation/stage-6/installer-android-smoke.json")
    assert record["result"] == "PASS"
    assert "keystorePass" not in json.dumps(record)
    assert re.fullmatch(r"[0-9a-f]{64}", record["apkSha256"])
```
- [ ] **Run red**

```bash
python3 -m unittest tests/cmake/test_installer_smoke.py -v
```
Expected red: contract file and smoke records do not exist.
- [ ] **Implement packaging and smoke**
Add the CPack/install rules and release preset, `--selftest`, environment-only Android signing, packaging scripts, and the clean-device smoke scripts that write `installer-windows-smoke.json` and `installer-android-smoke.json`.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ctest --preset linux-core --output-on-failure
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
./build/package-windows.ps1
./tests/smoke/windows_clean_smoke.ps1
./build/package-android.ps1
python tests/smoke/android_clean_smoke.py
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/installer_smoke_contract.cmake
git diff --check
```
Expected green: installer smoke contracts print `PASS`, both clean-device runs report result `PASS`, `--selftest` prints `DA_SELFTEST_OK`, release signing used only environment secrets, and Linux CTest plus DAT (`22 test(s) passed`) remain green.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt CMakePresets.json \
  packaging/windows/CMakeInstallRules.cmake \
  build/package-windows.ps1 build/package-android.ps1 \
  tests/cmake/installer_smoke_contract.cmake \
  tests/cmake/test_installer_smoke.py \
  tests/smoke/windows_clean_smoke.ps1 tests/smoke/android_clean_smoke.py \
  src/apps/desktop-widgets/main.cpp \
  src/apps/android-qml/android/build.gradle \
  docs/validation/stage-6/installer-windows-smoke.json \
  docs/validation/stage-6/installer-android-smoke.json
git commit -m "build: add Windows and Android installers with clean-device smoke"
```

---

### Task 8: Execute Offline, Weak-Network, Token-Expiry, Corrupt-Database, and Cloud-Outage Drills

**Files:**
- Create: `tests/drills/run_drills.py`
- Create: `tests/drills/drill_tokens.h`
- Create: `tests/integration/corrupt_database_drill_tests.cpp`
- Create: `tests/sync/weak_network_drill_tests.cpp`
- Create: `tests/sync/token_expiry_drill_tests.cpp`
- Create (generated, gate evidence): `docs/validation/stage-6/drill-offline.json`
- Create (generated, gate evidence): `docs/validation/stage-6/drill-weak-network.json`
- Create (generated, gate evidence): `docs/validation/stage-6/drill-token-expiry.json`
- Create (generated, gate evidence): `docs/validation/stage-6/drill-corrupt-db.json`
- Create (generated, gate evidence): `docs/validation/stage-6/drill-cloud-outage.json`
- Create (generated, gate evidence): `docs/validation/stage-6/drill-results.json`
- Modify: `CMakeLists.txt`
**Drills and acceptance:**
- Offline drill: with network disabled (Windows host firewall rule and Android flight mode), both clients complete full local CRUD plus recurring confirm on seeded profiles, then re-enable network and converge with zero loss (reruns the G3/G4 device matrix subset with the stage-6 profile).
- Weak-network drill: at the `ISyncTransport` seam, throttled and randomly aborted requests with ACK loss force backoff/retry; the drill verifies exponential backoff with jitter, frozen-payload retries, idempotent dedupe, and final convergence in `weak_network_drill_tests.cpp`.
- Token-expiry drill: short-lived sessions expire mid-drill; offline CRUD and recurring posting continue, sync pauses with visible status, session refresh resumes the original outbox, and nothing rolls back (`token_expiry_drill_tests.cpp` per sections 15.2 and 21.2).
- Corrupt-database drill: the runner copies a seeded profile, flips bytes in `accounting.sqlite`, relaunches the application, observes integrity-check failure at open, confirms the app quarantines the corrupt file (never deletes it silently), offers and performs restore from the latest verified backup set from Task 3, and verifies post-restore totals (`corrupt_database_drill_tests.cpp` + host run).
- Cloud-outage drill: with `supabase stop`, the Windows and Android clients keep accounting, exporting, and backing up; sync status shows a retryable failure; after `supabase start` plus `db reset` to the seeded state, both clients converge with no duplicates; the runbook records that cloud failure never degrades local entry.
- `run_drills.py --drill <name>` executes each host drill, prints exactly `DRILL <name> PASS: <summary>`, and writes one sanitized JSON per drill plus the aggregate into `docs/validation/stage-6/` (`drill-offline.json`, `drill-weak-network.json`, `drill-token-expiry.json`, `drill-corrupt-db.json`, `drill-cloud-outage.json`, and `drill-results.json`).
- [ ] **Write the failing drill tests**

```cpp
void corruptDatabaseIsDetectedQuarantinedAndRestored()
{
    CorruptDrillFixture fixture; fixture.seedAndBackup();
    fixture.corruptBytes(/*page*/ 3);
    DA_CHECK(!fixture.openValidation().hasValue());
    DA_CHECK(fixture.quarantineKeptOriginal());
    DA_CHECK(fixture.restoreLatestBackup().hasValue());
    DA_CHECK_EQ(fixture.expenseTotal(), MoneyMinor{-12850});
}
```
- [ ] **Run red**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --target dailyaccount_corrupt_database_drill_tests --parallel 2
```
Expected red: drill targets and runner are absent.
- [ ] **Implement the drill harness**
Implement the four seeded scenarios plus the runner, and the cloud-outage orchestration documented in the runner's help.
- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core && cmake --build --preset linux-core --parallel 2 && ctest --preset linux-core --output-on-failure && ./build/cmake/linux-core/dailyaccount_backend_tests && git diff --check
python3 tests/drills/run_drills.py --all
```
Expected green on the accepted hosts and devices: each drill prints its exact PASS line, drill JSON records `result=PASS` with sanitized fields only, and DAT remains `22 test(s) passed`.
- [ ] **Conditional checkpoint**
Only after explicit authorization:

```bash
git add CMakeLists.txt \
  tests/drills/run_drills.py tests/drills/drill_tokens.h \
  tests/integration/corrupt_database_drill_tests.cpp \
  tests/sync/weak_network_drill_tests.cpp tests/sync/token_expiry_drill_tests.cpp \
  docs/validation/stage-6/drill-offline.json \
  docs/validation/stage-6/drill-weak-network.json \
  docs/validation/stage-6/drill-token-expiry.json \
  docs/validation/stage-6/drill-corrupt-db.json \
  docs/validation/stage-6/drill-cloud-outage.json \
  docs/validation/stage-6/drill-results.json
git commit -m "test: add fault drills for offline, weak network, token, corruption, and outage"
```

---

### Task 9: Seal the Section 3.5/21 Evidence Package, Release Decision, and G6 Gate

**Files:**
- Create: `tests/cmake/check_g6.py`
- Create: `tests/cmake/test_check_g6.py`
- Create: `docs/validation/stage-6/acceptance-evidence-index.md`
- Create: `docs/validation/stage-6/export-backup-results.json`
- Create: `docs/validation/stage-6/restore-lifecycle-results.json`
- Create: `docs/validation/stage-6/security-results.json`
- Create: `docs/validation/stage-6/installer-smoke-results.json`
- Create: `docs/validation/stage-6/drill-results.json`
- Create: `docs/validation/stage-6/linux-core.log`
- Create: `docs/validation/stage-6/windows-stage6.log`
- Create: `docs/validation/stage-6/cloud-local.log`
- Create: `docs/validation/stage-6/source-tree.txt`
- Create: `docs/validation/stage-6/release-decision.json`
- Create: `docs/validation/stage-6/g6-evidence-index.md`
- Create: `docs/validation/stage-6/g6-results.json`
**Gate contract:**
- `check_g6.py --root DIR --json PATH` verifies: accepted/fresh G5; schema version 5; exchange format version 1 with round-trip and malicious fixtures; CSV injection fixture; backup manifest hashes and interrupted cleanup; all three restore paths; distinct lifecycle operations plus the pgTAP lifecycle suite and wipe-absence scan; security/log/credential results; both installer smoke records with artifact SHA-256 and no secret fields; all five drill JSONs; DAT 22; source scanners; the stage-6 source tree; and the acceptance index. It prints exactly:

```text
G6 PASS: export, backup, restore, lifecycle, installers, drills, security, and section 3.5/21 acceptance
```

- `acceptance-evidence-index.md` maps every one of the seven section 3.5 items and every bullet of sections 21.1-21.5 to its owning stage, evidence file, and command, and `release-decision.json` records `version: "1.0.0"`, `decision: "PASS"` or `"FAIL_STOP"` with reasons, artifact paths and SHA-256s, resolved open-item records (including the 23.11 retention record from Task 5), and an empty blocker list only at release time.
- [ ] **Write the failing checker tests**

```python
def test_acceptance_index_covers_all_section_35_items(self):
    index = self.read("docs/validation/stage-6/acceptance-evidence-index.md")
    for item in ("1.", "2.", "3.", "4.", "5.", "6.", "7."):
        assert f"3.5 item {item}" in index

def test_release_decision_has_no_blockers(self):
    decision = self.read_json("docs/validation/stage-6/release-decision.json")
    assert decision["blockers"] == []
```
- [ ] **Run red**

```bash
python3 -m unittest tests/cmake/test_check_g6.py -v
```
Expected red: import/file-not-found failure for `tests/cmake/check_g6.py`.
- [ ] **Implement the checker and acceptance index**
Aggregate every stage-6 artifact plus the inherited G2-G5 records into the successful JSON exactly:

```json
{"gate":"G6","result":"PASS","localSchemaVersion":5,"exchangeFormatVersion":1,"jsonRoundTrip":true,"csvInjectionSafe":true,"backupManifestHash":true,"restorePaths":3,"lifecycleOperationsDistinct":true,"cloudDeletionWorkflow":true,"installerWindows":true,"installerAndroid":true,"offlineDrill":true,"weakNetworkDrill":true,"tokenExpiryDrill":true,"corruptDbDrill":true,"cloudOutageDrill":true,"noSecretsEmbedded":true,"section35ItemCount":7,"releaseDecision":"PASS","failureCount":0}
```

- [ ] **Run final green verification**

```bash
set -o pipefail
mkdir -p docs/validation/stage-6
supabase start --workdir cloud && supabase db reset --workdir cloud && supabase db lint --workdir cloud --level warning --fail-on error && supabase test db --workdir cloud 2>&1 | tee docs/validation/stage-6/cloud-local.log && supabase stop --workdir cloud
cmake --preset linux-core 2>&1 | tee docs/validation/stage-6/linux-core.log
cmake --build --preset linux-core --parallel 2 2>&1 | tee -a docs/validation/stage-6/linux-core.log
ctest --preset linux-core --output-on-failure 2>&1 | tee -a docs/validation/stage-6/linux-core.log
./build/cmake/linux-core/dailyaccount_backend_tests 2>&1 | tee -a docs/validation/stage-6/linux-core.log
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/stage6_boundary_contract.cmake
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/installer_smoke_contract.cmake
python3 -m unittest tests/cmake/test_check_g6.py -v
python3 tests/drills/run_drills.py --all
STAGE6_INDEX=/tmp/opencode/dailyaccount-stage6-index; rm -f "$STAGE6_INDEX"
GIT_INDEX_FILE="$STAGE6_INDEX" git read-tree HEAD
GIT_INDEX_FILE="$STAGE6_INDEX" git add -A -- CMakeLists.txt CMakePresets.json cmake src tests docs cloud
GIT_INDEX_FILE="$STAGE6_INDEX" git rm --cached --ignore-unmatch docs/validation/stage-6/source-tree.txt
GIT_INDEX_FILE="$STAGE6_INDEX" git diff --cached --check
GIT_INDEX_FILE="$STAGE6_INDEX" git write-tree > docs/validation/stage-6/source-tree.txt
rm -f "$STAGE6_INDEX"
python3 tests/cmake/check_g6.py --root . --json docs/validation/stage-6/g6-results.json
git diff --check
git status --short
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop && cmake --build --preset windows-desktop --parallel 2 && ctest --preset windows-desktop --output-on-failure 2>&1 | Tee-Object docs\validation\stage-6\windows-stage6.log
if ($LASTEXITCODE -ne 0) { throw 'Stage 6 Windows CTest failed' }
```
Expected green: the checker prints exactly `G6 PASS: export, backup, restore, lifecycle, installers, drills, security, and section 3.5/21 acceptance`; `g6-results.json` records the exact JSON above; every drill, installer, and security record exists with sanitized content; and DAT prints `22 test(s) passed`.
- [ ] **Record evidence and obtain independent review**
`g6-evidence-index.md` records each command, UTC time, OS/device alias, exact tool versions, exit code, sanitized result path, source-tree identity, and SHA-256. The reviewer reruns G5/G6, pgTAP, exchange round-trip and malicious fixtures, backup verification and cleanup, all three restore paths, lifecycle pgTAP and wipe-absence scan, log redaction and credential cases, both installer smoke flows, and all five drills before the release decision is recorded.
- [ ] **Conditional final checkpoint**
After inspecting `git status --short`, `git diff`, and `git log --oneline -10`, and only after explicit authorization, commit the gate files separately from the preceding implementation checkpoints:

```bash
git add CMakeLists.txt tests/cmake/check_g6.py tests/cmake/test_check_g6.py \
  docs/validation/stage-6/acceptance-evidence-index.md \
  docs/validation/stage-6/export-backup-results.json \
  docs/validation/stage-6/restore-lifecycle-results.json \
  docs/validation/stage-6/security-results.json \
  docs/validation/stage-6/installer-smoke-results.json \
  docs/validation/stage-6/drill-results.json \
  docs/validation/stage-6/linux-core.log \
  docs/validation/stage-6/windows-stage6.log \
  docs/validation/stage-6/cloud-local.log \
  docs/validation/stage-6/source-tree.txt \
  docs/validation/stage-6/release-decision.json \
  docs/validation/stage-6/g6-evidence-index.md \
  docs/validation/stage-6/g6-results.json
git commit -m "test: seal G6 recovery and release evidence"
```
Expected: only reviewed Stage 6 source, tests, and sanitized evidence are committed. Credentials, keystore material, linked Supabase state, APKs, installers, databases, WAL/SHM files, build trees, device serials, raw text, and unrelated files remain untracked.

## G6 Checklist

Mapping of the master plan G6 gate and `docs/product-architecture.md` sections 3.5/21 to Stage 6 evidence:

| Requirement | Covered by | Evidence file |
| --- | --- | --- |
| §3.5 item 1 offline Android quick entry | Inherited G3/G5; Task 8 offline drill | `drill-offline.json` (via `drill-results.json`) |
| §3.5 item 2 Windows full transaction management | G2/G5; Task 1/2 export surfaces | `windows-stage6.log`, G5 index |
| §3.5 item 3 safe Windows/Android convergence | Inherited G4; Task 8 offline/weak-network drills | `drill-weak-network.json`, G4 index |
| §3.5 item 4 monthly category structure analysis | Inherited G5 analytics | `acceptance-evidence-index.md`, G5 index |
| §3.5 item 5 recurring pending items and reminders | Inherited G3/G5 | G3/G5 indexes via `acceptance-evidence-index.md` |
| §3.5 item 6 text import parse/preview/confirm | Inherited G5 | G5 index via `acceptance-evidence-index.md` |
| §3.5 item 7 export and restore during cloud outage | Tasks 1-4 and 8 | `export-backup-results.json`, `drill-cloud-outage.json` |
| §21.1 data correctness incl. money/occurrence uniqueness | G2/G5 plus Task 1 round trip | `export-backup-results.json`, G5 index |
| §21.2 offline CRUD, token expiry, retry, deletion, conflicts, cursor re-bootstrap, status | G4 plus Task 8 token/weak-network drills | `drill-token-expiry.json`, `drill-weak-network.json` |
| §21.3 account isolation and honest local boundary | G4 plus Task 6 boundary scan and Task 5 privacy doc | `security-results.json`, `docs/privacy/data-lifecycle-and-offline-limits.md` |
| §21.4 core experiences and reminder contract | G3/G5 plus D-030 records | `acceptance-evidence-index.md`, G5 index |
| §21.5 consistent backup and restore, no stale cursor | Tasks 3 and 4 | `restore-lifecycle-results.json`, `export-backup-results.json` |
| Versioned JSON/CSV round trips and malicious input | Tasks 1 and 2 | `export-backup-results.json` |
| Unbound, recovery-profile, and merged restores | Task 4 | `restore-lifecycle-results.json` |
| Exit, local-copy deletion, per-transaction deletion, cloud deletion distinct; no ledger-clear surface | Task 5 plus Task 6 scan | `restore-lifecycle-results.json`, `security-results.json` |
| Installers pass clean-device smoke | Task 7 | `installer-smoke-results.json` |
| Offline/weak-network/token/corrupt-DB/cloud-outage drills | Task 8 | `drill-results.json` |
| No embedded secrets; sensitive content absent from logs | Task 6 scans | `security-results.json` |
| Every §3.5 and §21 item has linked evidence | Task 9 index | `acceptance-evidence-index.md`, `g6-evidence-index.md` |

- [ ] `check_g6.py` prints exactly `G6 PASS: export, backup, restore, lifecycle, installers, drills, security, and section 3.5/21 acceptance`; `g6-results.json` records `gate=G6`, `result=PASS`, `localSchemaVersion=5`, `failureCount=0`, and every evidence file above exists with sanitized content.
- [ ] JSON exchange version 1 round-trips preserving IDs, totals, and occurrence/transaction linkage; every malicious fixture is rejected with a named entity and no partial write.
- [ ] CSV export is injection-safe, cent-exact, and free of raw import text and private device state.
- [ ] Backup sets verify by manifest hash and schema version; interrupted sets are cleaned at startup; live WAL/SHM files are never copied.
- [ ] Unbound restore, recovery-profile viewing, and synchronized-profile merge restoration are rehearsed; no restore carries a stale cursor or resurrects tombstones.
- [ ] Exit, local-copy deletion, per-transaction deletion, and cloud-account deletion are distinct operations with separate confirmations; no profile- or ledger-clearing surface exists anywhere (spec sections 4.2 and 16.4, decision D-019).
- [ ] The cloud deletion workflow is idempotent, blocks new sync writes while `DELETING`, drains then deletes module rows and change log, removes the auth identity last, records its terminal state outside business rows, and documents the 30-day cloud backup retention and offline no-remote-erase limits.
- [ ] Log redaction and credential-protection tests pass on both platforms; boundary scans find no service secret, TLS-ignore path, plaintext credential store, raw-text leak, or wipe wording.
- [ ] Windows and Android release artifacts build from clean checkouts, sign with environment-only secrets, and pass clean-device smoke with recorded SHA-256.
- [ ] All five drills print their exact PASS lines and record sanitized results.
- [ ] `release-decision.json` records `version=1.0.0`, `decision=PASS`, artifact SHA-256s, and an empty blocker list after independent review.
- [ ] `git diff --check` is silent and independent review accepts the evidence before the gate is recorded.

## Stage 7 Handoff

Stage 6 is the V1 release gate. A later stage may begin only after every G6 checkbox is checked, `docs/validation/stage-6/g6-results.json` records `gate=G6`, `result=PASS`, `localSchemaVersion=5`, and `failureCount=0`, and `release-decision.json` records `decision=PASS`.

Stage 6 leaves the product with these immutable boundaries: schema version 5 stays the shipped local schema; exchange format version 1 and the backup set format are the only interchange formats; the three restore paths and the lifecycle operations (sign out, delete local copy, cloud-account deletion) keep their distinct confirmations and backup-first rules, while per-transaction deletion remains separate CRUD and no profile- or ledger-wide clearing surface exists (spec sections 4.2 and 16.4, decision D-019); cloud-account deletion exists only behind the recorded admin runbook with its 30-day retention decision; raw import text and device-local rows stay out of exchange, backup payloads, logs, and cloud fixtures; and no second online write path exists. The release notes, in-app help text, and `docs/privacy/data-lifecycle-and-offline-limits.md` state the local OS trust boundary, the offline-device no-remote-erase limit, the deletion semantics of each operation, and the cloud retention window.

Future work belongs to Stage 7 and later evaluation: bill-file and payment-notification imports behind the D-018 boundary, budget and trend analytics, public registration, shared ledgers, and the one-click whole-ledger clear, which may return only through the separate protocol design described in `docs/product-architecture.md` section 16.4.
