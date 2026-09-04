# DailyAccount Full Repair Implementation Plan

> **For agentic workers:** Execute each task in order, keep every slice buildable, and verify the focused tests before moving on.

**Goal:** Repair the identified data-integrity, UI-consistency, date/statistics, portability, packaging, and maintainability defects without losing compatibility with existing DAT files.

**Architecture:** Keep the application as a single-process Qt Widgets program with a Qt-free C++17 backend. Store exact monetary values as integer cents and commit records plus custom categories in one versioned `ledger.dat` snapshot through atomic replacement. Treat the old `records.dat` and `categories.dat` pair as a read-only migration source. Route all mutations through `Ledger`, then notify the Qt pages with typed signals.

**Tech Stack:** C++17, Qt 6 Widgets, qmake, MinGW on Windows, standard-library backend tests.

**Spec:** This document, section [Repair Contract](#repair-contract).

## Repair Contract

- Existing unversioned `records.dat` and `categories.dat` files remain readable and are never deleted by migration.
- New writes use one `#DAILYACCOUNT_V3` snapshot and safely encode separators and line breaks.
- Domain amounts use signed 64-bit integer cents; floating point is allowed only at Qt chart/input presentation boundaries.
- A failed persistence operation leaves the in-memory model unchanged and returns an actionable error to the UI.
- The application sets stable organization/application identities, stores data below `QStandardPaths::AppDataLocation`, and prevents concurrent writers with a process-lifetime `QLockFile`.
- Common legacy locations are copied only when the target is empty and exactly one source exists; ambiguous sources stop startup instead of being merged or overwritten.
- Every successful record/category/clear mutation refreshes all affected pages and the status bar.
- Dashboard monthly labels use current-month data, while “全部” statistics include every stored date.
- Clearing data removes records and custom categories in one user operation.
- No new third-party runtime dependency is introduced.
- No Git commit is created unless the user explicitly requests one.

---

### Task 1: Backend Regression Harness

**Files:**
- Create: `tests/backend_tests.cpp`
- Create: `tests/backend_tests.pro`

**Interfaces:**
- Tests consume the contracts listed below before their implementation exists.
- The test executable returns non-zero on any failed assertion and uses isolated temporary directories.

- [x] Write tests for exact-cent arithmetic, legacy loading, V3 round trips containing `|`, invalid input rejection, rollback after save failure, category mutations, and bulk clear.
- [x] Compile the tests against the current backend and confirm RED because the new money and transactional interfaces do not exist.

Run:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Ibackend tests/backend_tests.cpp backend/category.cpp backend/storage.cpp backend/ledger.cpp -o /tmp/opencode/dailyaccount_backend_tests
```

### Task 2: Exact Domain Model and Safe Storage

**Files:**
- Modify: `backend/record.h`
- Modify: `backend/category.h`
- Modify: `backend/category.cpp`
- Modify: `backend/storage.h`
- Modify: `backend/storage.cpp`

**Interfaces:**
- `using Money = std::int64_t`
- `bool moneyFromDouble(double, Money&)` and `double moneyToDouble(Money)` are presentation-boundary helpers.
- `Record` exposes `amountCents`, `category`, `subcategory`, and `displayCategory()`.
- `LedgerStorage::load(StoredData&)` and `save(const StoredData&)` define the persistence boundary and enable deterministic failure tests.
- `StorageManager` implements that boundary with `std::filesystem::path`, whole-snapshot validation, checked same-directory temporary writes, backup rotation, and atomic replacement.
- `CategoryManager::clearCustomCategories()` supports bulk reset.

- [x] Implement strict ISO-date, record-type, ID, amount, and category parsing.
- [x] Read legacy decimal six-field records and migrate known historical food/transport subcategories only when the literal full category is not defined.
- [x] Write one `#DAILYACCOUNT_V3` file containing next ID, custom categories, and records with separate subcategory fields and percent-encoded text.
- [x] Preserve the previous target as `.bak` and never truncate the live file before a complete temporary write.
- [x] Run the focused storage and money tests until GREEN.

### Task 3: Transactional Ledger Mutations

**Files:**
- Modify: `backend/ledger.h`
- Modify: `backend/ledger.cpp`
- Modify: `tests/backend_tests.cpp`

**Interfaces:**
- `bool Ledger::load()`
- `bool Ledger::addRecord(const Record&)`
- `bool Ledger::updateRecord(int, const Record&)`
- `bool Ledger::deleteRecord(int)`
- `bool Ledger::addCustomCategory(RecordType, const std::string&)`
- `bool Ledger::removeCustomCategory(RecordType, const std::string&)`
- `bool Ledger::clearAllData()`
- `const std::string& Ledger::lastError() const`

- [x] Validate all mutation inputs at the Ledger boundary.
- [x] Build a complete candidate state, persist it, then commit with swaps so persistence failure leaves live memory untouched.
- [x] Keep records, categories, totals, in-use sets, and next ID in the candidate; bulk clear is one snapshot replacement.
- [x] Aggregate summaries in exact cents and expose a date-range category summary.
- [x] Run the complete backend test executable until GREEN.

### Task 4: Stable Data Location and Legacy Discovery

**Files:**
- Modify: `gui/main_gui.cpp`

**Interfaces:**
- Set stable Qt organization/application names before resolving `AppDataLocation`.
- Acquire `QLockFile` before migration or reading and hold it until `app.exec()` returns.
- Discover only current-working-directory and executable-directory legacy `data` locations, deduplicate them, and copy a single unambiguous source without deleting it.

- [x] Create and validate the canonical application-data directory.
- [x] Stop startup with actionable paths if multiple legacy sources exist or migration copy fails.
- [x] Load both legacy files into temporary backend state and immediately create the V3 snapshot after successful validation.
- [x] Offer explicit `.bak` restoration when the canonical snapshot cannot be loaded.

### Task 5: Qt Mutation and Statistics Flow

**Files:**
- Modify: `gui/mainwindow.h`
- Modify: `gui/mainwindow.cpp`
- Modify: `gui/flowpage.h`
- Modify: `gui/flowpage.cpp`
- Modify: `gui/flowdialog.cpp`
- Modify: `gui/dashboardpage.cpp`
- Modify: `gui/statisticspage.h`
- Modify: `gui/statisticspage.cpp`
- Modify: `gui/categorypage.h`
- Modify: `gui/categorypage.cpp`
- Modify: `gui/otherpage.h`
- Modify: `gui/otherpage.cpp`

**Interfaces:**
- Mutating pages emit typed `dataChanged()` signals only after successful Ledger operations.
- `MainWindow::refreshAll()` is connected with compile-time checked signal/slot syntax.

- [x] Replace string-based `invokeMethod` calls and display Ledger errors on failed operations.
- [x] Refresh the dashboard during startup and update the status bar after all mutations.
- [x] Preserve category/subcategory as separate fields and keep parenthesized custom category names intact.
- [x] Compute dashboard cards and category breakdown from the current month.
- [x] Sort recent records by date then ID and make category-row cleanup ownership-safe.
- [x] Make “全部” consume all records and validate reversed flow date ranges.
- [x] Clear records and custom categories through one bulk Ledger call.
- [x] Group exports in one pass and format integer cents only at display boundaries.

### Task 6: Windows Distribution

**Files:**
- Modify: `build/build.bat`
- Modify: `.gitignore`
- Modify: `jizhang.pro`

- [x] Make `build.bat` independent of the caller's working directory and allow environment overrides.
- [x] Build tests separately, package to `build/dist`, and run `windeployqt` with compiler runtime deployment.
- [x] Add direct standard-library/Qt includes required by each translation unit.

### Task 7: Documentation and Final Verification

**Files:**
- Create: `README.md`
- Modify: stale comments in touched headers and sources.

- [x] Document architecture, data location, legacy-to-V3 migration, build, test, and packaging commands.
- [x] Remove claims about JSON, removed pagination, and obsolete seven-field records from touched documentation.
- [x] Run backend tests under AddressSanitizer and UndefinedBehaviorSanitizer.
- [x] Run strict syntax checks for all backend translation units.
- [x] Attempt qmake build when the tool is available; otherwise report the exact unavailable dependency.
- [x] Inspect `git diff --check`, `git diff`, and `git status --short` before reporting completion.

Verification commands:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Ibackend tests/backend_tests.cpp backend/category.cpp backend/storage.cpp backend/ledger.cpp -o /tmp/opencode/dailyaccount_backend_tests
/tmp/opencode/dailyaccount_backend_tests
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Ibackend -fsyntax-only backend/category.cpp backend/storage.cpp backend/ledger.cpp
git diff --check
```
