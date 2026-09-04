# DailyAccount Stage 3 Android Offline Vertical Slice Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Deliver an installable Qt Quick Android application that opens an isolated local profile, performs transaction CRUD entirely offline, preserves exact 64-bit money through a decimal-string QML boundary, confirms one recurring occurrence exactly once, and delegates durable day-level reminders to the accepted narrow Android-native bridge.

**Architecture:** Enter only from accepted G2 and Android ADR evidence. Compose the Stage 2 standard-C++ domain/application services, per-profile QSQLITE executor, and SQLite query adapter behind Android-only `QObject` facades and `QAbstractListModel` projections; QML owns presentation and navigation but never financial arithmetic. Kotlin is limited to Android Keystore encryption, Storage Access Framework/share intents, notification persistence and scheduling, broadcast receivers, and activity callbacks; authentication, transport, synchronization, and the complete recurring lifecycle remain outside this stage.

**Tech Stack:** C++17, CMake 3.22.1+, CTest, Qt 6.9.3 Core/Concurrent/Gui/QML/Quick/Quick Controls 2/SQL/Test, QSQLITE, Kotlin 2.0.21, Gradle 8.10, AGP 8.6.0, JDK 17, Android SDK 35, Build Tools 35.0.1, NDK 27.2.12479018, minimum API 28, target API 35, `arm64-v8a` release-device ABI, `x86_64` API 28/API 35 emulator ABI, Android Keystore AES/GCM, Android Storage Access Framework, and the alarm mechanism accepted by D-030.

**Spec:** `docs/product-architecture.md`; parent plan: `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md`; prerequisite plans: `docs/superpowers/plans/2026-09-04-stage-0-baseline-and-prototypes.md` and `docs/superpowers/plans/2026-09-04-stage-2-sqlite-and-migration.md`; prerequisite evidence: `docs/validation/stage-0/g0-evidence-index.md` and `docs/validation/stage-2/g2-evidence-index.md`

## Global Constraints

- Start no implementation task until the entry procedure below prints exactly `Stage 3 entry gate: PASS`, the accepted and fresh G2 JSON records both contain `gate=G2`, `result=PASS`, `schemaVersion=2`, `atomicOutbox=true`, `windowsStorage=SQLITE_ONLY`, and `failureCount=0`, and D-020, D-023, D-025, D-028, D-029, and D-030 are accepted.
- D-020 fixes all Android, Qt, compiler, SDK, NDK, JDK, Gradle, AGP, ABI, and device-matrix versions. D-028 fixes the QSQLITE lifecycle and backup mechanism. D-029 fixes either `BACKGROUND_QT_ENABLED` or `FOREGROUND_COMPENSATION`. D-030 fixes the alarm API and reminder SLA. Do not substitute a newer tool or a different mechanism inside Stage 3.
- Preserve unrelated worktree changes. Never stash, reset, clean, discard, overwrite, stage, or commit work outside the current task.
- Do not create a Git commit unless the user explicitly authorizes commits in the execution session. Every checkpoint below is optional and authorization-gated.
- Keep C++17 as the shared-language floor. `dailyaccount_core_domain`, `dailyaccount_accounting_domain`, and `dailyaccount_accounting_application` remain free of Qt, SQL, QML, Quick, JNI, Android framework, and provider SDK types.
- Keep accounting validation, UUID generation rules, recurrence/period calculations, occurrence lifecycle, reminder-event calculation, and outbox payload construction in shared C++. Kotlin must not create or mutate transactions, occurrences, rules, money, period keys, or outbox rows.
- Keep every financial amount as `MoneyMinor` in shared C++ and SQLite. Every QML-facing input, property, signal argument, and model role carrying money is a decimal `QString`; QML and JavaScript must not use `int`, `qint64`, `double`, `Number`, `parseFloat`, `toFixed`, arithmetic, comparison, or sorting on monetary values.
- Accept Stage 3 writes only in `CNY`; each transaction or recurring amount remains in `1..9,999,999,999` minor units. `parseCnyMinor` and `formatMoney` are the only mobile text/minor-unit conversion authority.
- Keep the Android application useful without network access. Stage 3 links no auth client, sync transport, provider adapter, or network login flow, and local success follows the SQLite business-row plus outbox transaction rather than a remote response.
- The Stage 3 `LoginPage.qml` is a local-profile shell only. It never collects a password, claims a cloud-authenticated session, binds a `remoteUserId`, reopens `SIGNED_OUT_RETAINED` without online authentication, or exposes public registration. Stage 4 owns real email/password sign-in and session refresh.
- Reuse `ProfileDirectoryLocator`, `ModuleDbExecutor`, schema version 2, `SqliteAccountingUnitOfWork`, and `SqliteAccountingQueryService`. Android opens a fresh connection on its own module worker and never opens SQLite on the QML thread or shares a desktop connection.
- Do not edit `001_initial.sql` or `002_recurring.sql`. This plan requires no Stage 3 accounting schema change. If implementation proves one unavoidable, stop and write a reviewed forward-migration amendment with schema version 3, rollback tests, registry-version updates, and revised G2 handoff evidence before changing SQL.
- Every successful transaction create, edit, delete, recurring generation, and recurring confirmation commits its entity changes, preserved server revisions, local dirty state, base-revision expectations, and exactly one stable outbox mutation in one `accounting.sqlite` transaction. No operation waits for Android or network work inside that transaction.
- Generic deletion of an occurrence-linked transaction continues to return `OccurrenceLinked`. Confirmation updates the existing deterministic pending transaction; it never inserts a replacement transaction.
- Native reminder persistence is separate from `accounting.sqlite`. It stores only the precomputed event identity, local target date/zone, delivery metadata, and minimal display snapshot supplied by C++; notification display or dismissal never marks a financial transaction paid.
- The inclusive rolling reminder horizon is `D0` through `D0 + 90` natural days in each rule's IANA zone. Stage 3 integration evidence must include `D0+89` and `D0+90` and exclude `D0+91` before replenishment. Full defer/skip/cancel/undo, rule-management UX, cross-device reminder reconciliation, and import matching remain Stage 5 scope.
- Keep the accepted normal-reboot/date/time-zone/package-replacement rebuild path independent of Qt cold-process startup, WorkManager, network, and cloud push. A user force-stop remains the documented platform exception; first reopen immediately reconciles native alarms.
- The Keystore bridge stores an AES/GCM key in Android Keystore and ciphertext in app-private preferences. Passwords are never stored. Stage 3 proves the capability with non-production probe bytes but does not persist an auth session.
- The file picker uses `ACTION_OPEN_DOCUMENT` for `text/plain`; the share bridge uses `ACTION_SEND` with `text/plain`. Request no broad storage permission, return raw bytes to C++, cap transport at `1,048,577` bytes so shared import validation can reject the D-027 over-limit case, and log no selected content.
- Every behavior-changing task follows red-green-refactor: write the focused test, observe the stated failure, implement the smallest behavior, run the focused test, run the cumulative Stage 3 and inherited suites, and run whitespace/source-boundary checks.
- Generated CMake trees, Gradle state, APKs, test databases, Keystore material, app-private files, device screenshots containing account content, and logcat captures containing free-form transaction text remain untracked. Validation JSON uses synthetic values only.

---

## G2 and Android Decision Entry Gate

No task below may begin until every checkbox in this section is checked. Missing evidence, an unaccepted Android decision, a toolchain mismatch, or a failed device prerequisite stops Stage 3 without changing production files.

- [ ] **Verify G0/G2 evidence, the G2 checker, and accepted Android decisions exist**

Run from the repository root:

```bash
test -f docs/validation/stage-0/g0-results.json
test -f docs/validation/stage-0/g0-evidence-index.md
test -f docs/validation/stage-2/g2-results.json
test -f docs/validation/stage-2/g2-evidence-index.md
test -f tests/cmake/check_g2.py
for number in 020 023 025 028 029 030; do
  matches=(docs/decisions/D-${number}-*.md)
  test "${#matches[@]}" -eq 1
  test -f "${matches[0]}"
done
```

Expected: every command exits `0`; each required ADR number resolves to exactly one file.

- [ ] **Verify the Android decisions are terminal and internally consistent**

Run:

```bash
python3 - <<'PY'
import glob
from pathlib import Path

requirements = {
    "020": ("Status: Accepted", "Qt 6.9.3", "API 28", "API 35", "NDK 27.2.12479018", "JDK 17"),
    "023": ("Status: Accepted", "LOCAL_UNBOUND", "ACTIVE", "SIGNED_OUT_RETAINED", "remoteUserId"),
    "025": ("Status: Accepted", "D0 + 90", "D0+89", "D0+90", "D0+91", "OccurrenceLinked"),
    "028": ("Status: Accepted", "QSQLITE_BUSY_TIMEOUT=5000", "drain", ".partial"),
    "029": ("Status: Accepted",),
    "030": ("Status: Accepted", "09:00", "BOOT_COMPLETED", "TIMEZONE_CHANGED", "MY_PACKAGE_REPLACED"),
}
for number, tokens in requirements.items():
    paths = glob.glob(f"docs/decisions/D-{number}-*.md")
    assert len(paths) == 1, (number, paths)
    text = Path(paths[0]).read_text(encoding="utf-8")
    assert all(token in text for token in tokens), (paths[0], tokens)
d029 = Path(glob.glob("docs/decisions/D-029-*.md")[0]).read_text(encoding="utf-8")
assert any(marker in d029 for marker in
           ("BACKGROUND_QT_ENABLED", "FOREGROUND_COMPENSATION"))
d030 = Path(glob.glob("docs/decisions/D-030-*.md")[0]).read_text(encoding="utf-8")
assert any(marker in d030 for marker in
           ("setAndAllowWhileIdle", "setExactAndAllowWhileIdle"))
print("Android decisions: PASS")
PY
```

Expected: `Android decisions: PASS`. Before Task 1, the executor records the single outcome explicitly selected by each accepted ADR in the execution notes; alternatives discussed but not selected in an ADR are not implementation branches.

- [ ] **Re-run the authoritative G2 checker without replacing accepted evidence**

```bash
python3 tests/cmake/check_g2.py \
  --root . \
  --json /tmp/opencode/dailyaccount-stage3-g2-recheck.json
```

Expected stdout:

```text
G2 PASS: SQLite schema 2, atomic outbox, DAT migration parity, Windows SQLite-only
```

- [ ] **Compare accepted and fresh G2 records**

```bash
python3 - <<'PY'
import json
from pathlib import Path

accepted = json.loads(Path("docs/validation/stage-2/g2-results.json").read_text(encoding="utf-8"))
fresh = json.loads(Path("/tmp/opencode/dailyaccount-stage3-g2-recheck.json").read_text(encoding="utf-8"))
expected = {
    "gate": "G2",
    "result": "PASS",
    "schemaVersion": 2,
    "legacyBackendTestCount": 22,
    "atomicOutbox": True,
    "datMigrationParity": True,
    "windowsStorage": "SQLITE_ONLY",
    "failureCount": 0,
}
for key, value in expected.items():
    assert accepted[key] == value, ("accepted", key, accepted[key])
    assert fresh[key] == value, ("fresh", key, fresh[key])
g0 = json.loads(Path("docs/validation/stage-0/g0-results.json").read_text(encoding="utf-8"))
assert g0["gate"] == "G0" and g0["result"] == "PASS" and g0["failureCount"] == 0
print("Stage 3 entry gate: PASS")
PY
```

Expected: exactly `Stage 3 entry gate: PASS` and exit code `0`.

- [ ] **Read immutable inputs and inspect the starting tree**

Read `docs/product-architecture.md`, the master plan, the Stage 0 and Stage 2 plans, D-020 through D-030, all G0/G1/G2 evidence indexes, and every public header under `src/core/`, `src/platform/`, `src/modules/accounting/`, and `src/apps/desktop-widgets/`. Then run:

```bash
git status --short
git diff --check
git ls-files '*.pro' '*.apk' '*.aab' '*.sqlite' '*-wal' '*-shm'
```

Expected: `git diff --check` is silent; no tracked qmake, APK/AAB, database, WAL, or SHM file is reported. Record all pre-existing changed or untracked paths and leave them untouched.

- [ ] **Verify exact tools and attach the accepted target matrix**

Run:

```bash
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
export ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/27.2.12479018"
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
export QT_HOST_PATH="$HOME/Qt/6.9.3/gcc_64"
export QT_ANDROID_ARM64="$HOME/Qt/6.9.3/android_arm64_v8a"
export QT_ANDROID_X86_64="$HOME/Qt/6.9.3/android_x86_64"
test -x "$JAVA_HOME/bin/java"
test -x "$ANDROID_SDK_ROOT/platform-tools/adb"
test -f "$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake"
test -f "$QT_ANDROID_ARM64/lib/cmake/Qt6/qt.toolchain.cmake"
test -f "$QT_ANDROID_X86_64/lib/cmake/Qt6/qt.toolchain.cmake"
"$JAVA_HOME/bin/java" -version
"$ANDROID_SDK_ROOT/platform-tools/adb" version
cmake --version
gradle --version
"$ANDROID_SDK_ROOT/platform-tools/adb" devices -l
```

Expected: JDK `17`, CMake `3.22.1` or the D-020 exact accepted patch, Gradle `8.10`, both Qt `6.9.3` kits, NDK `27.2.12479018`, both clean x86_64 boundary AVDs, and one accepted physical ARM64 target are available. A missing mandatory target is a stop condition, not grounds to shrink G3.

---

## Deliverables and File Map

Stage 3 owns the following production, test, build, and evidence surface. Existing Stage 2 files not listed here remain unchanged. The APK paths under `artifacts/stage-3/` are evidence artifacts and remain untracked unless repository policy is separately changed.

```text
CMakeLists.txt
CMakePresets.json
cmake/
  DailyAccountOptions.cmake
src/
  modules/accounting/
    accounting_module.h
    accounting_mobile_module.cpp
    application/
      accounting_query_service.h
      accounting_view_models.h
      accounting_mutation_codec.h
      accounting_mutation_codec.cpp
      recurring_service.h
      recurring_service.cpp
      reminder_planner.h
      reminder_planner.cpp
    data/sqlite/
      sqlite_accounting_query_service.h
      sqlite_accounting_query_service.cpp
      accounting_unit_of_work.cpp
  platform/modules/
    mobile_registry.h
    mobile_registry.cpp
  apps/android-qml/
    CMakeLists.txt
    main.cpp
    mobile_composition.h
    mobile_composition.cpp
    mobile_router.h
    mobile_router.cpp
    mobile_profile_facade.h
    mobile_profile_facade.cpp
    mobile_accounting_facade.h
    mobile_accounting_facade.cpp
    mobile_recurring_facade.h
    mobile_recurring_facade.cpp
    models/
      profile_list_model.h
      profile_list_model.cpp
      transaction_list_model.h
      transaction_list_model.cpp
      category_list_model.h
      category_list_model.cpp
      recurring_list_model.h
      recurring_list_model.cpp
      transaction_editor.h
      transaction_editor.cpp
    bridge/
      android_secure_store.h
      android_secure_store.cpp
      android_file_share_bridge.h
      android_file_share_bridge.cpp
      android_notification_scheduler.h
      android_notification_scheduler.cpp
    qml/
      Main.qml
      LoginPage.qml
      OverviewPage.qml
      QuickEntryPage.qml
      TransactionsPage.qml
      TransactionDetailPage.qml
      RecurringPage.qml
      SettingsPage.qml
      components/MoneyField.qml
      components/TransactionRow.qml
      components/RecurringCard.qml
    android/
      AndroidManifest.xml
      build.gradle
      res/drawable/ic_stat_dailyaccount.xml
      res/values/strings.xml
      src/main/java/local/dailyaccount/
        DailyAccountActivity.kt
        RuntimeDiagnostics.kt
        SecureStoreBridge.kt
        FileShareBridge.kt
        ReminderContract.kt
        ReminderStore.kt
        ReminderScheduler.kt
        ReminderReceiver.kt
        ReminderBootReceiver.kt
      src/androidTest/java/local/dailyaccount/
        SecureStoreInstrumentationTest.kt
        FileShareInstrumentationTest.kt
        OfflineCrudInstrumentationTest.kt
        ProcessDeathInstrumentationTest.kt
        ReminderInstrumentationTest.kt
tests/
  unit/
    mobile_registry_routes_tests.cpp
    mobile_models_tests.cpp
    recurring_confirmation_tests.cpp
    reminder_planner_tests.cpp
  integration/
    mobile_accounting_query_tests.cpp
    mobile_facade_tests.cpp
    mobile_lifecycle_tests.cpp
    recurring_confirmation_sqlite_tests.cpp
  cmake/
    android_build_contract.cmake
    android_boundary_contract.cmake
    qml_money_boundary_contract.cmake
    check_g3.py
    test_check_g3.py
  android/
    run_stage3_matrix.sh
    run_process_death_smoke.sh
    run_reminder_smoke.sh
    verify_apk.py
docs/validation/stage-3/
  linux-core.log
  windows-mobile-tests.log
  android-build.log
  android-runtime-results.json
  android-offline-results.json
  android-process-death-results.json
  android-reminder-results.json
  android-apk-results.json
  source-tree.txt
  g3-evidence-index.md
  g3-results.json
artifacts/stage-3/
  DailyAccount-stage3-arm64-v8a-debug.apk
  DailyAccount-stage3-x86_64-debug.apk
```

### Target Dependency Graph

```text
dailyaccount_accounting_domain
  <- dailyaccount_accounting_application
  <- dailyaccount_accounting_sqlite

dailyaccount_platform_interfaces + dailyaccount_mobile_registry
  <- dailyaccount_accounting_mobile_registration

Qt6::Core + Qt6::Concurrent
  <- dailyaccount_mobile_models
  <- dailyaccount_mobile_facades

Qt6::Core + Android JNI/framework capability calls
  <- dailyaccount_android_bridges

dailyaccount_profile + dailyaccount_accounting_sqlite
  + dailyaccount_mobile_facades + dailyaccount_android_bridges
  + dailyaccount_accounting_mobile_registration
  + Qt6::Gui + Qt6::Qml + Qt6::Quick + Qt6::QuickControls2 + Qt6::Sql
  <- dailyaccount_android
```

`dailyaccount_android` must not link `dailyaccount_legacy_backend`, `dailyaccount_dat_importer`, `dailyaccount_dat_migration`, `dailyaccount_desktop_registry`, `dailyaccount_accounting_desktop_registration`, `dailyaccount_legacy_widgets`, or `dailyaccount_desktop`. Kotlin sources are package inputs only and are not linked into shared C++ targets.

---

### Task 1: Add the Qt Android CMake Target, QML Module, and Minimal APK

**Files:**
- Modify: `cmake/DailyAccountOptions.cmake`
- Modify: `CMakeLists.txt`
- Modify: `CMakePresets.json`
- Create: `src/apps/android-qml/CMakeLists.txt`
- Create: `src/apps/android-qml/main.cpp`
- Create: `src/apps/android-qml/qml/Main.qml`
- Create: `src/apps/android-qml/android/AndroidManifest.xml`
- Create: `src/apps/android-qml/android/build.gradle`
- Create: `src/apps/android-qml/android/res/values/strings.xml`
- Create: `src/apps/android-qml/android/res/drawable/ic_stat_dailyaccount.xml`
- Create: `tests/cmake/android_build_contract.cmake`
- Test: `tests/cmake/android_build_contract.cmake`

**Interfaces:**
- Consumes: the accepted D-020 toolchain/ABI matrix, Stage 1 `dailyaccount_accounting_mobile_registration`, Stage 2 SQLite/profile targets, and Qt's Android deployment tool.
- Produces: option `DA_BUILD_ANDROID`; presets `android-arm64-debug` and `android-x86_64-debug`; executable target `dailyaccount_android`; QML URI `DailyAccount.Accounting` version `1.0`; package ID `local.dailyaccount`; minimum API 28 and target API 35 APKs.

- [ ] **Step 1: Write the failing Android build-graph contract**

Create `tests/cmake/android_build_contract.cmake`:

```cmake
if(NOT DEFINED DA_SOURCE_DIR)
    message(FATAL_ERROR "DA_SOURCE_DIR is required")
endif()
file(READ "${DA_SOURCE_DIR}/cmake/DailyAccountOptions.cmake" options)
set(app "")
set(manifest "")
if(EXISTS "${DA_SOURCE_DIR}/src/apps/android-qml/CMakeLists.txt")
    file(READ "${DA_SOURCE_DIR}/src/apps/android-qml/CMakeLists.txt" app)
endif()
if(EXISTS "${DA_SOURCE_DIR}/src/apps/android-qml/android/AndroidManifest.xml")
    file(READ "${DA_SOURCE_DIR}/src/apps/android-qml/android/AndroidManifest.xml" manifest)
endif()
foreach(required
        "option(DA_BUILD_ANDROID"
        "qt_add_executable(dailyaccount_android"
        "qt_add_qml_module(dailyaccount_android"
        "URI DailyAccount.Accounting"
        "QT_ANDROID_MIN_SDK_VERSION 28"
        "QT_ANDROID_TARGET_SDK_VERSION 35")
    string(FIND "${options}\n${app}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "missing Android build contract: ${required}")
    endif()
endforeach()
foreach(required "package=\"local.dailyaccount\"" "android:usesCleartextTraffic=\"false\"")
    string(FIND "${manifest}" "${required}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "missing manifest contract: ${required}")
    endif()
endforeach()
```

- [ ] **Step 2: Run the contract red**

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/android_build_contract.cmake
```

Expected: non-zero exit containing `missing Android build contract: option(DA_BUILD_ANDROID` or the first absent app-target token.

- [ ] **Step 3: Add Android and host-mobile-test build switches and presets**

Append to `DailyAccountOptions.cmake`:

```cmake
option(DA_BUILD_ANDROID "Build the Qt Quick Android application" OFF)
option(DA_BUILD_MOBILE_TESTS "Build host-runnable Qt mobile adapter tests" OFF)
if(DA_BUILD_ANDROID AND NOT ANDROID)
    message(FATAL_ERROR "DA_BUILD_ANDROID requires an Android toolchain")
endif()
```

Add Android configure/build presets with these exact cache values; `android-x86_64-debug` differs only in kit, ABI, and binary directory:

```json
{
  "name": "android-arm64-debug",
  "generator": "Ninja",
  "binaryDir": "${sourceDir}/build/cmake/android-arm64-debug",
  "cacheVariables": {
    "CMAKE_BUILD_TYPE": "Debug",
    "CMAKE_TOOLCHAIN_FILE": "$env{QT_ANDROID_ARM64}/lib/cmake/Qt6/qt.toolchain.cmake",
    "QT_HOST_PATH": "$env{QT_HOST_PATH}",
    "ANDROID_SDK_ROOT": "$env{ANDROID_SDK_ROOT}",
    "CMAKE_ANDROID_NDK": "$env{ANDROID_NDK_ROOT}",
    "ANDROID_PLATFORM": "android-28",
    "ANDROID_ABI": "arm64-v8a",
    "DA_BUILD_ANDROID": "ON",
    "DA_BUILD_DESKTOP": "OFF",
    "DA_BUILD_SQLITE": "ON",
    "DA_BUILD_MOBILE_TESTS": "OFF",
    "DA_ENABLE_SANITIZERS": "OFF"
  }
}
```

Set `DA_BUILD_MOBILE_TESTS=ON` in `windows-desktop` so later QObject/model tests run against the accepted desktop Qt kit without pretending to test Android framework code.

- [ ] **Step 4: Create the Android executable and QML module**

Conditionally call `add_subdirectory(src/apps/android-qml)` from the root. Use this target foundation:

```cmake
find_package(Qt6 6.9.3 EXACT REQUIRED COMPONENTS
    Core Concurrent Gui Qml Quick QuickControls2 Sql)

qt_add_executable(dailyaccount_android MANUAL_FINALIZATION main.cpp)
qt_add_qml_module(dailyaccount_android
    URI DailyAccount.Accounting
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES qml/Main.qml)
target_link_libraries(dailyaccount_android PRIVATE
    Qt6::Core Qt6::Concurrent Qt6::Gui Qt6::Qml Qt6::Quick
    Qt6::QuickControls2 Qt6::Sql
    dailyaccount_profile
    dailyaccount_accounting_sqlite
    dailyaccount_accounting_mobile_registration
    dailyaccount_mobile_registry)
set_target_properties(dailyaccount_android PROPERTIES
    OUTPUT_NAME DailyAccount
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    QT_ANDROID_MIN_SDK_VERSION 28
    QT_ANDROID_TARGET_SDK_VERSION 35)
da_enable_strict_warnings(dailyaccount_android)
qt_finalize_executable(dailyaccount_android)
```

`main.cpp` loads only the registered module and returns failure when it has no root object:

```cpp
QGuiApplication app(argc, argv);
QQmlApplicationEngine engine;
engine.loadFromModule("DailyAccount.Accounting", "Main");
if (engine.rootObjects().isEmpty()) {
    return EXIT_FAILURE;
}
return app.exec();
```

`Main.qml` is a real touch-safe startup surface with `ApplicationWindow`, minimum logical size `360x640`, `visible: true`, title `DailyAccount`, and a centered `Label` with object name `startupStatus` and text `本地账本准备中`.

- [ ] **Step 5: Add the package baseline without broad permissions**

Use package `local.dailyaccount`, app label `DailyAccount`, `android:minSdkVersion="28"`, `android:targetSdkVersion="35"`, `android:allowBackup="false"`, `android:fullBackupContent="false"`, and `android:usesCleartextTraffic="false"`. The initial manifest declares only the Qt activity; notification and boot permissions/receivers arrive with their implementation task. It must not request `INTERNET`, `READ_EXTERNAL_STORAGE`, `WRITE_EXTERNAL_STORAGE`, `MANAGE_EXTERNAL_STORAGE`, contacts, accessibility, SMS, or account permissions.

Configure Gradle with `compileSdk 35`, `minSdk 28`, `targetSdk 35`, Kotlin JVM target 17, AGP 8.6.0, Kotlin 2.0.21, and instrumentation runner `androidx.test.runner.AndroidJUnitRunner`. Use the vector drawable as the eventual notification small icon and do not embed a raster asset copied from another application.

- [ ] **Step 6: Run focused and cumulative verification**

```bash
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
export ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/27.2.12479018"
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
export QT_HOST_PATH="$HOME/Qt/6.9.3/gcc_64"
export QT_ANDROID_ARM64="$HOME/Qt/6.9.3/android_arm64_v8a"
export QT_ANDROID_X86_64="$HOME/Qt/6.9.3/android_x86_64"
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/android_build_contract.cmake
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
cmake --preset android-arm64-debug
cmake --build --preset android-arm64-debug --target apk --parallel 2
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: the contract passes; both APK targets build; each generated APK contains `libDailyAccount_arm64-v8a.so` or `libDailyAccount_x86_64.so` plus Qt Quick/QML/SQL deployment files; Linux CTest has zero failures and the DAT oracle still prints exactly `22 test(s) passed`.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

If and only if the user authorizes a commit in the execution session:

```bash
git add CMakeLists.txt CMakePresets.json cmake/DailyAccountOptions.cmake \
  src/apps/android-qml tests/cmake/android_build_contract.cmake
git commit -m "build: add Qt Android application target"
```

Without authorization, leave the verified files uncommitted and do not stage generated APKs or build trees.

---

### Task 2: Register All Accounting Mobile Routes and Add a Registry-Backed Router

**Files:**
- Modify: `src/platform/modules/mobile_registry.h`
- Modify: `src/platform/modules/mobile_registry.cpp`
- Modify: `src/modules/accounting/accounting_module.h`
- Modify: `src/modules/accounting/accounting_mobile_module.cpp`
- Create: `src/apps/android-qml/mobile_router.h`
- Create: `src/apps/android-qml/mobile_router.cpp`
- Create: `tests/unit/mobile_registry_routes_tests.cpp`
- Modify: `tests/platform_ui_registry_tests.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/mobile_registry_routes_tests.cpp`
- Test: `tests/platform_ui_registry_tests.cpp`

**Interfaces:**
- Consumes: Stage 1 `MobileRouteDescriptor`, `MobileRegistry`, `registerAccountingMobile`, and stable module ID `accounting`.
- Produces: atomic `MobileRegistry::registerRoutes`; five stable accounting route IDs; `MobileRouter` properties `currentRouteId`, `currentQmlType`, and `canGoBack`; invokables `navigate` and `back`.

- [ ] **Step 1: Write the failing route-set test**

Create the focused assertion:

```cpp
void accountingMobileRegistrationPublishesTheCompleteStage3RouteSet()
{
    MobileRegistry registry;
    DA_CHECK(registerAccountingMobile(registry).hasValue());
    DA_CHECK_EQ(registry.size(), std::size_t{5});
    DA_CHECK_EQ(registry.find("accounting.quick-entry")->qmlType, "QuickEntryPage");
    DA_CHECK_EQ(registry.find("accounting.transactions")->qmlType, "TransactionsPage");
    DA_CHECK_EQ(registry.find("accounting.transaction-detail")->qmlType,
                "TransactionDetailPage");
    DA_CHECK_EQ(registry.find("accounting.recurring")->qmlType, "RecurringPage");
}
```

Also test that a batch containing one duplicate leaves registry size and contents unchanged, navigation rejects an unknown route, and `back()` restores the previous descriptor rather than accepting an arbitrary QML URL.

- [ ] **Step 2: Run the route test red**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target dailyaccount_mobile_registry_routes_tests --parallel 2
```

Expected: unknown target before registration or, after adding only the test target, failure because current registration has size `1` and no `registerRoutes` method.

- [ ] **Step 3: Add atomic route registration and exact stable constants**

Extend the registry without changing the descriptor's standard-C++ fields:

```cpp
class MobileRegistry {
public:
    Result<void> registerRoute(MobileRouteDescriptor descriptor);
    Result<void> registerRoutes(std::vector<MobileRouteDescriptor> descriptors);
    const MobileRouteDescriptor* find(std::string_view routeId) const noexcept;
    std::size_t size() const noexcept;
};
```

`registerRoutes` validates all non-empty fields, duplicates inside the incoming vector, and collisions with existing routes before moving any descriptor into storage. Keep the existing compatibility constant `kAccountingMobileRouteId` equal to overview and add:

```cpp
inline constexpr std::string_view kAccountingMobileRouteId = "accounting.overview";
inline constexpr std::string_view kAccountingQuickEntryRouteId = "accounting.quick-entry";
inline constexpr std::string_view kAccountingTransactionsRouteId = "accounting.transactions";
inline constexpr std::string_view kAccountingTransactionDetailRouteId =
    "accounting.transaction-detail";
inline constexpr std::string_view kAccountingRecurringRouteId = "accounting.recurring";
```

Register this exact ordered batch, all under QML URI `DailyAccount.Accounting`: `OverviewPage`, `QuickEntryPage`, `TransactionsPage`, `TransactionDetailPage`, and `RecurringPage`.

- [ ] **Step 4: Implement the QObject router as an Android-app adapter**

Use this complete public surface:

```cpp
class MobileRouter final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentRouteId READ currentRouteId NOTIFY routeChanged)
    Q_PROPERTY(QString currentQmlType READ currentQmlType NOTIFY routeChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY routeChanged)
public:
    explicit MobileRouter(const MobileRegistry& registry, QObject* parent = nullptr);
    QString currentRouteId() const;
    QString currentQmlType() const;
    bool canGoBack() const noexcept;
    Q_INVOKABLE bool navigate(const QString& routeId);
    Q_INVOKABLE bool back();
    void resetToOverview();
signals:
    void routeChanged();
    void navigationRejected(QString routeId, QString reason);
};
```

Initialize the stack to `accounting.overview`. `navigate` converts UTF-8, looks up the descriptor, rejects unknown routes without changing the stack, suppresses a duplicate top entry, and appends only a registered route. It exposes a registered QML type name, never a caller-supplied URL. `back` is false at overview and otherwise pops exactly one route.

- [ ] **Step 5: Run focused and cumulative verification**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_mobile_registry_routes_tests
./build/cmake/linux-core/dailyaccount_platform_ui_registry_tests
ctest --preset linux-core -R 'mobile_registry|platform_ui_registry|architecture' --output-on-failure
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
git diff --check
```

Expected: five routes appear in the stated order, failed batch registration is atomic, router tests pass, inherited registry/boundary tests pass, Android APK builds, CTest has zero failures, and DAT remains at 22 passes.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/platform/modules/mobile_registry.* \
  src/modules/accounting/accounting_module.h \
  src/modules/accounting/accounting_mobile_module.cpp \
  src/apps/android-qml/CMakeLists.txt src/apps/android-qml/mobile_router.* \
  tests/unit/mobile_registry_routes_tests.cpp tests/platform_ui_registry_tests.cpp
git commit -m "feat: register accounting mobile routes"
```

Run those commands only after explicit authorization; otherwise leave the files uncommitted.

---

### Task 3: Compose the Local Profile Shell Without Network Authentication

**Files:**
- Create: `src/apps/android-qml/models/profile_list_model.h`
- Create: `src/apps/android-qml/models/profile_list_model.cpp`
- Create: `src/apps/android-qml/mobile_profile_facade.h`
- Create: `src/apps/android-qml/mobile_profile_facade.cpp`
- Create: `src/apps/android-qml/mobile_composition.h`
- Create: `src/apps/android-qml/mobile_composition.cpp`
- Create: `src/apps/android-qml/qml/LoginPage.qml`
- Modify: `src/apps/android-qml/qml/Main.qml`
- Modify: `src/apps/android-qml/main.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Create: `tests/integration/mobile_facade_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/mobile_facade_tests.cpp`

**Interfaces:**
- Consumes: D-023 states, `ProfileStore`, `ProfileDirectoryLocator`, `AccountingDatabase`, `AtomicFileActivation`, `ModuleDbExecutor`, `PlatformRegistry`, `MobileRegistry`, and `registerAccountingCore/registerAccountingMobile`.
- Produces: `ProfileListModel`; `MobileProfileFacade::initialize/createLocalProfile/openLocalProfile`; `MobileComposition::create/activateProfile/drainAndClose`; a local-only profile chooser with no auth/network dependency.

- [ ] **Step 1: Write the failing local-profile shell test**

Use a real temporary profile root:

```cpp
void firstRunCreatesAndOpensAnUnboundLocalProfileWithoutAuth()
{
    TemporaryDirectory root("mobile-profile");
    auto composition = MobileComposition::create({root.path(), "Asia/Shanghai"});
    DA_CHECK(composition.hasValue());
    QSignalSpy active(&composition.value()->profileFacade(),
                      &MobileProfileFacade::profileActivated);
    composition.value()->profileFacade().createLocalProfile("本地账本");
    DA_CHECK(active.wait(5000));
    DA_CHECK_EQ(composition.value()->profileFacade().activeProfileLabel(), "本地账本");
    DA_CHECK(!composition.value()->profileFacade().cloudAuthenticated());
    DA_CHECK(std::filesystem::exists(
        root.path() / "users" / active.at(0).at(0).toString().toStdString() /
        "accounting.sqlite"));
}
```

Also test two profiles have disjoint databases, `SIGNED_OUT_RETAINED` cannot open through this shell, owner mismatch clears no model and opens no business query, initialization failure stays retryable in `INITIALIZING`, and no password or `IAuthClient` is referenced by the facade or QML.

- [ ] **Step 2: Run the host-mobile target red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_mobile_facade_tests --parallel 2
```

Expected: unknown target or compilation failure for missing `mobile_composition.h`.

- [ ] **Step 3: Define profile model roles and facade API**

Use exact roles `profileId`, `displayLabel`, `maskedEmail`, `state`, and `canOpenOffline`. `canOpenOffline` is true only for `LOCAL_UNBOUND`, `INITIALIZING`, or `ACTIVE` profiles with no remote binding in Stage 3. Expose:

```cpp
class MobileProfileFacade final : public QObject {
    Q_OBJECT
    Q_PROPERTY(ProfileListModel* profiles READ profiles CONSTANT)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool cloudAuthenticated READ cloudAuthenticated CONSTANT)
    Q_PROPERTY(QString activeProfileId READ activeProfileId NOTIFY profileActivated)
    Q_PROPERTY(QString activeProfileLabel READ activeProfileLabel NOTIFY profileActivated)
    Q_PROPERTY(QString errorCode READ errorCode NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
public:
    Q_INVOKABLE void initialize();
    Q_INVOKABLE void createLocalProfile(const QString& displayLabel);
    Q_INVOKABLE void openLocalProfile(const QString& profileId);
signals:
    void profilesChanged();
    void busyChanged();
    void profileActivated(QString profileId);
    void errorChanged();
};
```

All three invokables dispatch blocking profile/SQLite work away from the GUI thread and deliver one terminal signal on the facade thread.

- [ ] **Step 4: Define a lifetime-owning composition root**

Use:

```cpp
struct MobileCompositionOptions {
    std::filesystem::path applicationDataRoot;
    std::string defaultTimeZoneId;
};

class MobileComposition final {
public:
    static Result<std::unique_ptr<MobileComposition>> create(
        MobileCompositionOptions options);
    ~MobileComposition();
    MobileProfileFacade& profileFacade();
    MobileRouter& router();
    Result<void> activateProfile(ProfileId profileId);
    Result<void> closeActiveProfile();
    Result<void> drainAndClose();
};
```

Own registries, the profiles executor/store, locator, active accounting executor, UoW, services, facades, and models in reverse-safe destruction order. `activateProfile` applies this exact sequence: reject commands; clear/detach old models; close the previous accounting executor; load and validate the selected local profile; reject `SIGNED_OUT_RETAINED`, `RECOVERY_READ_ONLY`, and `LOCAL_DELETE_PENDING`; ensure its UUID-only directory; create schema 2 through same-directory staging when absent; validate immutable owner and schema; open one `da/<profile>/accounting/<sequence>` executor; construct fresh services/models; transition `LOCAL_UNBOUND -> INITIALIZING -> ACTIVE` or resume `INITIALIZING -> ACTIVE`; then emit activation. It never writes `providerId`, `remoteUserId`, email, token, or password.

- [ ] **Step 5: Build the local-profile QML shell**

`LoginPage.qml` displays `本地账本` as its heading, an explicit `当前阶段仅打开本机账本，不进行网络登录` explanation, the profile model, a 128-byte-bounded display-label field, `新建本地账本`, and `打开` actions. Assign object names `profileList`, `profileLabelField`, `createLocalProfileButton`, and `openLocalProfileButton`. There is no password field, registration link, forgotten-password link, or fake signed-in status.

`Main.qml` shows `LoginPage` until `profileActivated`, then shows a basic application shell headed by the selected profile label. `main.cpp` derives the root from `QStandardPaths::AppDataLocation`, rejects an empty path, creates the composition, exposes `profileFacade` and `mobileRouter` as context properties, and calls `profileFacade.initialize()` after the QML root is ready.

- [ ] **Step 6: Run focused and cumulative verification**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_mobile_facade_tests.exe'
ctest --preset windows-desktop -R 'mobile_facade|profile_store|sqlite_schema|module_db_executor' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
git diff --check
```

Expected: the local profile activates without network/auth, profile isolation and owner rejection pass, the GUI event loop remains responsive, Android packages the shell, inherited suites pass, and DAT remains at 22 passes.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/apps/android-qml/CMakeLists.txt \
  src/apps/android-qml/main.cpp src/apps/android-qml/mobile_composition.* \
  src/apps/android-qml/mobile_profile_facade.* src/apps/android-qml/models/profile_list_model.* \
  src/apps/android-qml/qml/Main.qml src/apps/android-qml/qml/LoginPage.qml \
  tests/integration/mobile_facade_tests.cpp
git commit -m "feat: add Android local profile shell"
```

Use the checkpoint only with explicit authorization; otherwise do not commit.

---

### Task 4: Add QAbstractListModel DTOs and Enforce the Decimal-String Money Boundary

**Files:**
- Create: `src/apps/android-qml/models/transaction_list_model.h`
- Create: `src/apps/android-qml/models/transaction_list_model.cpp`
- Create: `src/apps/android-qml/models/category_list_model.h`
- Create: `src/apps/android-qml/models/category_list_model.cpp`
- Create: `src/apps/android-qml/models/transaction_editor.h`
- Create: `src/apps/android-qml/models/transaction_editor.cpp`
- Create: `src/apps/android-qml/qml/components/MoneyField.qml`
- Create: `tests/unit/mobile_models_tests.cpp`
- Create: `tests/cmake/qml_money_boundary_contract.cmake`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/mobile_models_tests.cpp`
- Test: `tests/cmake/qml_money_boundary_contract.cmake`

**Interfaces:**
- Consumes: Stage 2 `TransactionListItem`, `CategoryListItem`, `MoneyMinor`, `parseCnyMinor`, and `formatMoney`.
- Produces: immutable `MobileTransactionRow`/`MobileCategoryRow` DTOs, list-model roles usable from QML, string-only `TransactionEditor`, and a lexical `MoneyField` whose authoritative acceptance remains C++.

- [ ] **Step 1: Write the failing 64-bit model-role test and QML scanner**

The focused model case is:

```cpp
void maximumCnyAmountCrossesEveryQmlRoleAsQString()
{
    TransactionListModel model;
    model.replace({MobileTransactionRow{
        "aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa", "2026-09-04",
        "EXPENSE", "支出", "POSTED", "99999999.99", "饮食", "边界样例"}});
    const QModelIndex index = model.index(0, 0);
    const QVariant value = model.data(index, TransactionListModel::AmountTextRole);
    DA_CHECK_EQ(value.metaType().id(), QMetaType::QString);
    DA_CHECK_EQ(value.toString(), QStringLiteral("99999999.99"));
}
```

Create `qml_money_boundary_contract.cmake` to recursively read `src/apps/android-qml/qml/*.qml`, reject `amountMinor`, `parseFloat`, `parseInt`, `Number(`, `.toFixed(`, unary conversion of `amountText`, and declarations matching `property (int|real|double) .*amount`, and require `MoneyField` to expose `property alias amountText: input.text`.

- [ ] **Step 2: Run both checks red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_mobile_models_tests --parallel 2
```

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/qml_money_boundary_contract.cmake
```

Expected: the Windows target is unknown or missing the model header; the CMake script is absent or reports the missing `MoneyField` contract.

- [ ] **Step 3: Define exact DTOs and model roles**

Use only display-safe Qt values:

```cpp
struct MobileTransactionRow {
    QString id;
    QString occurredOn;
    QString typeCode;
    QString typeLabel;
    QString statusCode;
    QString amountText;
    QString categoryName;
    QString note;
};

class TransactionListModel final : public QAbstractListModel {
    Q_OBJECT
public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        OccurredOnRole,
        TypeCodeRole,
        TypeLabelRole,
        StatusCodeRole,
        AmountTextRole,
        CategoryNameRole,
        NoteRole
    };
    Q_ENUM(Role)
    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void replace(QVector<MobileTransactionRow> rows);
    void clear();
};
```

`CategoryListModel` roles are `categoryId`, `name`, `appliesTo`, `isPreset`, and `isArchived`. It contains no amount. `replace` uses `beginResetModel/endResetModel`, validates the GUI thread, and owns copied DTOs.

- [ ] **Step 4: Define string-only editor state and MoneyField**

`TransactionEditor` exposes `QString transactionId`, `amountText`, `typeCode`, `occurredOn`, `categoryId`, `note`, and `bool existing`; it has no numeric amount property. `MoneyField.qml` uses a text input with `Qt.ImhFormattedNumbersOnly`, maximum length 11, automatic focus on quick entry, and this lexical validator:

```qml
RegularExpressionValidator {
    regularExpression: /^(?:0|[1-9][0-9]{0,7})(?:\.[0-9]{0,2})?$/
}
```

The validator improves typing only. Save always sends the unchanged string to C++, where `parseCnyMinor` rejects zero, incomplete text, overprecision, signs, separators, exponent notation, whitespace, and values above `99999999.99`.

- [ ] **Step 5: Run focused and cumulative verification**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_mobile_models_tests.exe'
ctest --preset windows-desktop -R 'mobile_models|mobile_facade' --output-on-failure
```

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/qml_money_boundary_contract.cmake
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_date_money_tests
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
git diff --check
```

Expected: every money role/editor property is `QString`, `99999999.99` stays exact, invalid lexical/domain cases fail, the QML scanner is silent, both platform builds pass, and inherited tests remain green.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/apps/android-qml/CMakeLists.txt \
  src/apps/android-qml/models/transaction_list_model.* \
  src/apps/android-qml/models/category_list_model.* \
  src/apps/android-qml/models/transaction_editor.* \
  src/apps/android-qml/qml/components/MoneyField.qml \
  tests/unit/mobile_models_tests.cpp tests/cmake/qml_money_boundary_contract.cmake
git commit -m "feat: add string-safe mobile models"
```

Without authorization, leave the verified files uncommitted.

---

### Task 5: Extend Shared SQLite Reads for Mobile Detail and Recurring Projections

**Files:**
- Modify: `src/modules/accounting/application/accounting_view_models.h`
- Modify: `src/modules/accounting/application/accounting_query_service.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_accounting_query_service.cpp`
- Create: `tests/integration/mobile_accounting_query_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/mobile_accounting_query_tests.cpp`

**Interfaces:**
- Consumes: schema 2 transaction/rule/occurrence tables, D-025 display semantics, and Stage 2 query ownership/filtering rules.
- Produces: `TransactionDetailItem`, `RecurringDisplayState`, `RecurringOccurrenceListItem`, `transactionDetail`, `listRecurringOccurrences`, and `listEnabledRecurringRules`, all standard C++ and all executed on the serial database worker.

- [ ] **Step 1: Write failing projection tests against a real schema-2 database**

Include this exact multi-period case:

```cpp
void unresolvedRecurringProjectionKeepsPriorOverdueAndCurrentPendingRows()
{
    QueryFixture fixture;
    fixture.seedMonthlyRule(false);
    fixture.seedPendingOccurrence("2026-08", LocalDate{2026, 8, 31}, 8800);
    fixture.seedPendingOccurrence("2026-09", LocalDate{2026, 9, 30}, 9900);
    const auto rows = fixture.queries.listRecurringOccurrences(
        LocalDate{2026, 9, 4}, RecurringOccurrenceScope::Unresolved);
    DA_CHECK(rows.hasValue());
    DA_CHECK_EQ(rows.value().size(), std::size_t{2});
    DA_CHECK_EQ(rows.value()[0].periodKey, "2026-08");
    DA_CHECK_EQ(rows.value()[0].displayState, RecurringDisplayState::Overdue);
    DA_CHECK_EQ(rows.value()[1].amountMinor, MoneyMinor{9900});
}
```

Also test detail returns all editable transaction fields and category display, tombstones are absent, pending transactions never enter formal dashboard totals, deferred future dates display `Deferred`, a deferred past date displays `Overdue`, posted/skipped/cancelled states remain distinguishable in `All`, archived/disabled-rule unresolved occurrences remain visible, and enabled-rule enumeration excludes deleted/disabled rules only from future schedule generation.

- [ ] **Step 2: Run the query target red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_mobile_accounting_query_tests --parallel 2
```

Expected: compilation fails because `RecurringOccurrenceScope`, `TransactionDetailItem`, and the new virtual methods do not exist.

- [ ] **Step 3: Define exact standard-C++ read contracts**

Append:

```cpp
struct TransactionDetailItem {
    Transaction transaction;
    std::string categoryDisplayName;
};

enum class RecurringOccurrenceScope { Unresolved, All };
enum class RecurringDisplayState {
    Pending,
    DueToday,
    Overdue,
    Deferred,
    Posted,
    Skipped,
    Cancelled
};

struct RecurringOccurrenceListItem {
    RecurringOccurrenceId occurrenceId;
    RecurringRuleId ruleId;
    std::string periodKey;
    std::string ruleName;
    std::optional<std::string> note;
    std::optional<std::string> marker;
    MoneyMinor amountMinor;
    CurrencyCode currency;
    RecurringStatus persistentStatus;
    RecurringDisplayState displayState;
    LocalDate effectiveDueOn;
    std::int32_t daysUntilDue;
    std::uint8_t leadDays;
    bool canConfirm;
};
```

Extend `IAccountingQueryService`:

```cpp
virtual Result<std::optional<TransactionDetailItem>> transactionDetail(
    const TransactionId& id) = 0;
virtual Result<std::vector<RecurringOccurrenceListItem>> listRecurringOccurrences(
    LocalDate today, RecurringOccurrenceScope scope) = 0;
virtual Result<std::vector<RecurringRule>> listEnabledRecurringRules() = 0;
```

- [ ] **Step 4: Implement bound SQL and exact display derivation**

`transactionDetail` left-joins category parent/child names and filters transaction/category tombstones appropriately. `listRecurringOccurrences` joins occurrences to rules and the linked transaction, uses the linked transaction amount/note as current-period truth, includes every unresolved occurrence even when the rule is disabled, sorts unresolved rows by effective due date then occurrence UUID, and filters `Unresolved` to persistent `PENDING` only.

Set `effectiveDueOn = deferredUntil.value_or(expectedOn)`. For `PENDING`: a date before `today` is `Overdue`; equal is `DueToday`; a future deferred date is `Deferred`; otherwise it is `Pending`. Set `daysUntilDue` with checked Gregorian date arithmetic, negative for overdue. Set `canConfirm=true` only for a live `PENDING` occurrence whose linked transaction is live and `PENDING`. Malformed rows fail closed with `StorageFailure`.

- [ ] **Step 5: Run focused and cumulative verification**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_mobile_accounting_query_tests.exe'
ctest --preset windows-desktop -R 'mobile_accounting_query|accounting_query|sqlite_' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: mobile/detail/recurring projections pass, existing dashboard/statistics parity remains unchanged, SQLite ownership tests pass, Linux has zero failures, and DAT remains at 22 passes.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/modules/accounting/application/accounting_view_models.h \
  src/modules/accounting/application/accounting_query_service.h \
  src/modules/accounting/data/sqlite/sqlite_accounting_query_service.* \
  tests/integration/mobile_accounting_query_tests.cpp
git commit -m "feat: add mobile accounting read projections"
```

Without explicit authorization, do not commit.

---

### Task 6: Implement the Offline Overview and Quick-Entry Facade and QML

**Files:**
- Create: `src/apps/android-qml/mobile_accounting_facade.h`
- Create: `src/apps/android-qml/mobile_accounting_facade.cpp`
- Create: `src/apps/android-qml/qml/OverviewPage.qml`
- Create: `src/apps/android-qml/qml/QuickEntryPage.qml`
- Create: `src/apps/android-qml/qml/components/TransactionRow.qml`
- Modify: `src/apps/android-qml/qml/Main.qml`
- Modify: `src/apps/android-qml/mobile_composition.h`
- Modify: `src/apps/android-qml/mobile_composition.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Modify: `tests/integration/mobile_facade_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/mobile_facade_tests.cpp`

**Interfaces:**
- Consumes: `IAccountingService`, `IAccountingQueryService`, Stage 2 create command, Task 4 models, active local owner/device identity, and shared money/date parsers.
- Produces: `MobileAccountingFacade::refreshOverview/loadCategories/createQuickTransaction`; overview text properties and recent/category models; one-page offline income/expense entry with optional category/note.

- [ ] **Step 1: Write the failing offline max-money quick-entry test**

```cpp
void quickEntryCommitsMaximumMoneyAndOutboxBeforeReportingSuccess()
{
    MobileFixture fixture;
    QSignalSpy success(&fixture.facade, &MobileAccountingFacade::commandSucceeded);
    fixture.facade.createQuickTransaction(
        "99999999.99", "EXPENSE", fixture.foodCategoryId(),
        "2026-09-04", "边界样例");
    DA_CHECK(success.wait(5000));
    DA_CHECK_EQ(fixture.scalarInt64("SELECT amount_minor FROM transactions"),
                std::int64_t{9'999'999'999});
    DA_CHECK_EQ(fixture.scalarInt("SELECT count(*) FROM outbox"), 1);
    DA_CHECK_EQ(fixture.facade.recentTransactions()->data(
                    fixture.facade.recentTransactions()->index(0, 0),
                    TransactionListModel::AmountTextRole).toString(),
                QStringLiteral("99999999.99"));
}
```

Also prove the UI heartbeat runs while a database lock delays completion, success is not emitted when outbox insertion fails, zero/over-limit/overprecision input writes nothing, default account remains null, income/expense map correctly, category is optional, and no auth/network mock is needed.

- [ ] **Step 2: Run the focused facade case red**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_mobile_facade_tests --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_mobile_facade_tests.exe'
```

Expected: compilation fails for missing `MobileAccountingFacade` methods or the focused case fails because no transaction is dispatched.

- [ ] **Step 3: Define the facade's exact QML surface**

```cpp
struct MobileCommandContext {
    UserId localLedgerOwnerId;
    DeviceId deviceId;
    std::function<UtcInstant()> now;
};

class MobileAccountingFacade final : public QObject {
    Q_OBJECT
    Q_PROPERTY(TransactionListModel* recentTransactions READ recentTransactions CONSTANT)
    Q_PROPERTY(CategoryListModel* categories READ categories CONSTANT)
    Q_PROPERTY(TransactionEditor* editor READ editor CONSTANT)
    Q_PROPERTY(QString totalIncomeText READ totalIncomeText NOTIFY overviewChanged)
    Q_PROPERTY(QString totalExpenseText READ totalExpenseText NOTIFY overviewChanged)
    Q_PROPERTY(QString balanceText READ balanceText NOTIFY overviewChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    Q_INVOKABLE void refreshOverview(const QString& todayIso);
    Q_INVOKABLE void loadCategories();
    Q_INVOKABLE void createQuickTransaction(
        const QString& amountText,
        const QString& typeCode,
        const QString& categoryId,
        const QString& occurredOn,
        const QString& note);
    void detachModels();
    void stopAcceptingCommands();
    void waitForJobs();
signals:
    void overviewChanged();
    void busyChanged();
    void commandSucceeded(QString command, QString entityId);
    void commandFailed(QString code, QString message);
};
```

Generate `TransactionId` and `MutationId` exactly once before dispatch. Build metadata from the active local owner, stable installation `DeviceId`, and injected clock; set `serverRevision=0`, `deletedAt=null`, `POSTED`, `MANUAL`, `CNY`, and null account/destination/refund. Parse strings before launching and let `AccountingService` perform authoritative aggregate validation inside the UoW. Results return through `QFutureWatcher` on the facade thread and are dropped after model-generation detachment.

- [ ] **Step 4: Compose a stable installation device identity**

Store one random `DeviceId` under QSettings organization/application `DailyAccount/DailyAccount`, key `installation/deviceId`. Parse and reuse it on restart; a malformed existing value returns `StorageFailure` and does not silently create a second identity. This non-secret setting is installation-wide, not profile-wide and not an authenticated user ID.

- [ ] **Step 5: Build touch-first overview and quick entry pages**

`OverviewPage.qml` displays three string totals, at most ten recent rows, an empty-state explanation, and primary actions with object names `quickEntryAction`, `transactionsAction`, and `recurringAction`. It navigates only via registered route IDs.

`QuickEntryPage.qml` keeps `MoneyField` focused, exposes a two-state expense/income selector, category chips/list, date defaulted by C++ as ISO text, and collapsed optional note. Object names are `quickAmountField`, `expenseTypeButton`, `incomeTypeButton`, `quickCategoryList`, `quickDateField`, `quickNoteField`, and `quickSaveButton`. Disable save while busy, preserve entered text on failure, clear only after `commandSucceeded`, and navigate to overview after the refreshed snapshot arrives.

- [ ] **Step 6: Run focused and cumulative verification**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_mobile_facade_tests.exe'
& 'build\cmake\windows-desktop\dailyaccount_mobile_models_tests.exe'
ctest --preset windows-desktop -R 'mobile_|accounting_crud|sqlite_unit_of_work' --output-on-failure
```

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/qml_money_boundary_contract.cmake
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
git diff --check
```

Expected: quick entry commits entity/outbox before success, max money round-trips exactly as text, invalid input remains local UI state with no row, the event-loop heartbeat passes, QML scanner is silent, APK builds, and inherited suites remain green. The minimum quick-entry page offers an optional category and deliberately leaves the optional account null.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/apps/android-qml/CMakeLists.txt \
  src/apps/android-qml/mobile_accounting_facade.* \
  src/apps/android-qml/mobile_composition.* src/apps/android-qml/qml \
  tests/integration/mobile_facade_tests.cpp
git commit -m "feat: add offline Android quick entry"
```

Without authorization, do not commit.

---

### Task 7: Add Offline Transaction List, Detail, Edit, and Delete

**Files:**
- Modify: `src/apps/android-qml/mobile_accounting_facade.h`
- Modify: `src/apps/android-qml/mobile_accounting_facade.cpp`
- Modify: `src/apps/android-qml/models/transaction_editor.h`
- Modify: `src/apps/android-qml/models/transaction_editor.cpp`
- Create: `src/apps/android-qml/qml/TransactionsPage.qml`
- Create: `src/apps/android-qml/qml/TransactionDetailPage.qml`
- Modify: `src/apps/android-qml/qml/Main.qml`
- Modify: `tests/integration/mobile_facade_tests.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Test: `tests/integration/mobile_facade_tests.cpp`

**Interfaces:**
- Consumes: Task 5 detail/list queries, Stage 2 update/delete commands, stable transaction IDs, `OccurrenceLinked`, and Task 4 editor/model string roles.
- Produces: `loadTransactions/openTransaction/saveTransaction/deleteTransaction`; editable detail state; tombstone-backed delete; list refresh after each local commit.

- [ ] **Step 1: Write the failing real-SQLite CRUD/restart test**

```cpp
void listDetailEditAndDeleteStayOfflineAndPreserveStableIdentity()
{
    MobileFixture fixture;
    const QString id = fixture.createExpense("12.34", "2026-09-04");
    fixture.facade.openTransaction(id);
    DA_CHECK(fixture.waitForEditor());
    fixture.facade.saveTransaction(id, "56.78", "EXPENSE",
                                   fixture.foodCategoryId(), "2026-09-05", "已修改");
    DA_CHECK(fixture.waitForCommand("UPDATE_TRANSACTION"));
    DA_CHECK_EQ(fixture.liveTransactionId(), id.toStdString());
    DA_CHECK_EQ(fixture.liveAmountMinor(), MoneyMinor{5678});
    fixture.reopen();
    DA_CHECK_EQ(fixture.visibleTransactionCount(), 1);
    fixture.facade.deleteTransaction(id);
    DA_CHECK(fixture.waitForCommand("DELETE_TRANSACTION"));
    DA_CHECK_EQ(fixture.visibleTransactionCount(), 0);
    DA_CHECK_EQ(fixture.outboxCount(), 3);
}
```

Also prove edit preserves origin/server revision/created time and nullable fields not exposed by the minimum editor, each operation uses a new stable mutation ID, delete writes a tombstone rather than removing the row, linked recurring delete reports `OccurrenceLinked`, double-tap while busy creates one command, and no QML amount conversion occurs.

- [ ] **Step 2: Run the focused test red**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_mobile_facade_tests --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_mobile_facade_tests.exe'
```

Expected: compilation fails for absent list/detail methods or the new case reports no editor/list result.

- [ ] **Step 3: Add exact facade operations**

```cpp
Q_INVOKABLE void loadTransactions(
    const QString& fromIso, const QString& throughIso, const QString& typeCode);
Q_INVOKABLE void openTransaction(const QString& transactionId);
Q_INVOKABLE void saveTransaction(
    const QString& transactionId,
    const QString& amountText,
    const QString& typeCode,
    const QString& categoryId,
    const QString& occurredOn,
    const QString& note);
Q_INVOKABLE void deleteTransaction(const QString& transactionId);
```

Empty filter dates mean no bound; non-empty values must parse exactly as ISO dates. `openTransaction` uses `transactionDetail`, maps `MoneyMinor` only through `formatMoney`, and populates `TransactionEditor`. `saveTransaction` reloads the current row on the worker, replaces only Stage 3 editable fields plus `updatedAt/modifiedByDeviceId`, preserves ID/owner/revision/createdAt/origin/account/refund/other fields, and submits `UpdateTransactionCommand`. `deleteTransaction` creates one `DeleteTransactionCommand`. After a commit, refresh list and overview; emit terminal success only once.

- [ ] **Step 4: Implement mobile list and detail interaction**

`TransactionsPage.qml` uses `ListView` rather than a desktop table, object name `transactionList`, stable UUID model keys, type/date filters, pull-to-refresh, and a delegate whose amount is rendered from `amountText`. Activating a row calls `openTransaction(id)` then navigates to `accounting.transaction-detail` only after editor load succeeds.

`TransactionDetailPage.qml` uses object names `detailAmountField`, `detailTypeSelector`, `detailCategoryList`, `detailDateField`, `detailNoteField`, `detailSaveButton`, and `detailDeleteButton`. Show a destructive confirmation dialog before delete. Map `OccurrenceLinked` to `该记录由周期支出实例管理，不能在流水中单独删除。请到周期支出中处理。` Preserve editor values on failure and pop only after refreshed models arrive.

- [ ] **Step 5: Run focused and cumulative verification**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_mobile_facade_tests.exe'
ctest --preset windows-desktop -R 'mobile_|accounting_crud|sqlite_unit_of_work|accounting_query' --output-on-failure
```

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/qml_money_boundary_contract.cmake
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
git diff --check
```

Expected: full local CRUD and reopen assertions pass, delete is a tombstone with an outbox row, linked deletion remains guarded, QML money scanning passes, APK builds, and all inherited tests remain green.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add src/apps/android-qml/mobile_accounting_facade.* \
  src/apps/android-qml/models/transaction_editor.* \
  src/apps/android-qml/qml/TransactionsPage.qml \
  src/apps/android-qml/qml/TransactionDetailPage.qml \
  src/apps/android-qml/qml/Main.qml src/apps/android-qml/CMakeLists.txt \
  tests/integration/mobile_facade_tests.cpp
git commit -m "feat: add offline Android transaction CRUD"
```

Do not run the checkpoint without explicit authorization.

---

### Task 8: Make Foreground, Background, Profile Close, and Application Shutdown Deterministic

**Files:**
- Modify: `src/apps/android-qml/mobile_accounting_facade.h`
- Modify: `src/apps/android-qml/mobile_accounting_facade.cpp`
- Modify: `src/apps/android-qml/mobile_profile_facade.h`
- Modify: `src/apps/android-qml/mobile_profile_facade.cpp`
- Modify: `src/apps/android-qml/mobile_composition.h`
- Modify: `src/apps/android-qml/mobile_composition.cpp`
- Modify: `src/apps/android-qml/main.cpp`
- Create: `tests/integration/mobile_lifecycle_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/mobile_lifecycle_tests.cpp`

**Interfaces:**
- Consumes: Stage 2 executor drain contract, D-023 switch barrier, Qt application-state signals, and facade job-generation cancellation.
- Produces: `MobileComposition::handleApplicationState`; idempotent `closeActiveProfile/drainAndClose`; ordered model detachment, watcher completion, executor closure, and foreground compensation callback.

- [ ] **Step 1: Write the failing drain-order and stale-result test**

```cpp
void shutdownRejectsNewCommandsDropsStaleModelsAndRemovesConnections()
{
    MobileFixture fixture;
    const QString connection = fixture.accountingConnectionName();
    fixture.holdSecondWriterLock();
    fixture.facade.createQuickTransaction("12.34", "EXPENSE", "", "2026-09-04", "");
    fixture.beginShutdown();
    fixture.releaseSecondWriterLock();
    DA_CHECK(fixture.finishShutdown().hasValue());
    DA_CHECK_EQ(fixture.facade.recentTransactions()->rowCount(), 0);
    DA_CHECK(!QSqlDatabase::contains(connection));
    fixture.facade.createQuickTransaction("1.00", "EXPENSE", "", "2026-09-04", "");
    DA_CHECK_EQ(fixture.businessRowCount(), fixture.outboxRowCount());
}
```

Capture Qt messages and assert no `QSqlDatabasePrivate::removeDatabase` warning. Also test repeated shutdown, background does not close a valid profile, foreground invokes one refresh/recurrence-compensation callback, profile close clears editor/category/transaction models before connection removal, and composition destruction performs the same drain if `aboutToQuit` was not observed.

- [ ] **Step 2: Run the lifecycle target red**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_mobile_lifecycle_tests --parallel 2
```

Expected: unknown target or missing `handleApplicationState`/ordered shutdown behavior.

- [ ] **Step 3: Implement the exact lifecycle state handling**

Add:

```cpp
enum class MobileRuntimeState { Starting, Foreground, Background, Closing, Closed };

Result<void> MobileComposition::handleApplicationState(Qt::ApplicationState state);
```

`ApplicationActive` changes `Starting/Background -> Foreground` and queues one compensation callback after the active profile is ready. `ApplicationInactive`, `ApplicationHidden`, and `ApplicationSuspended` change `Foreground -> Background` without closing the database; already-submitted business transactions finish on their executor. `aboutToQuit` changes to `Closing`, rejects new profile and accounting commands, increments every facade generation, clears editor/list/category projections, waits for all facade watchers, destroys services/query/UoW, drains and closes accounting executor, then drains profiles executor and sets `Closed`. Repeated calls return success without touching removed connections.

Connect `QGuiApplication::applicationStateChanged` and `aboutToQuit` in `main.cpp`; retain a destructor fallback. Do not call a blocking database close from Android `onPause`, and do not depend on `onDestroy` for process-death correctness.

- [ ] **Step 4: Run focused and cumulative verification**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_mobile_lifecycle_tests.exe'
ctest --preset windows-desktop -R 'mobile_|module_db_executor|profile_store' --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
git diff --check
```

Expected: all lifecycle/order/idempotency tests pass with no connection warning or stale model signal; Android and inherited targets build and test successfully.

- [ ] **Step 5: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/apps/android-qml/main.cpp \
  src/apps/android-qml/mobile_accounting_facade.* \
  src/apps/android-qml/mobile_profile_facade.* \
  src/apps/android-qml/mobile_composition.* \
  tests/integration/mobile_lifecycle_tests.cpp
git commit -m "fix: drain Android profile services on shutdown"
```

Without authorization, do not commit.

---

### Task 9: Add the Android Keystore-Backed ISecureStore Bridge Skeleton

**Files:**
- Create: `src/apps/android-qml/bridge/android_secure_store.h`
- Create: `src/apps/android-qml/bridge/android_secure_store.cpp`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/SecureStoreBridge.kt`
- Create: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/SecureStoreInstrumentationTest.kt`
- Modify: `src/apps/android-qml/mobile_composition.h`
- Modify: `src/apps/android-qml/mobile_composition.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Modify: `src/apps/android-qml/android/build.gradle`
- Create: `tests/cmake/android_boundary_contract.cmake`
- Modify: `CMakeLists.txt`
- Test: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/SecureStoreInstrumentationTest.kt`
- Test: `tests/cmake/android_boundary_contract.cmake`

**Interfaces:**
- Consumes: Stage 1 `ISecureStore`, Android Keystore, application context, and app-private SharedPreferences.
- Produces: `AndroidSecureStore::put/get/remove`; JNI methods that preserve arbitrary bytes including NUL; alias `dailyaccount.session.v1`; no token usage before Stage 4.

- [ ] **Step 1: Write the failing Keystore instrumentation test and boundary scan**

```kotlin
@Test
fun binaryValueRoundTripsEncryptedAndRemoveReturnsNotFound() {
    val context = InstrumentationRegistry.getInstrumentation().targetContext
    val key = "stage3-probe"
    val value = byteArrayOf(0, 1, 2, 0, 127, -1)
    assertEquals("", SecureStoreBridge.put(context, key, value))
    assertArrayEquals(byteArrayOf(0) + value, SecureStoreBridge.getEnvelope(context, key))
    val raw = context.getSharedPreferences("dailyaccount_secure_store", 0)
        .getString(SecureStoreBridge.storageKeyForTest(key), "")!!
    assertFalse(raw.contains(Base64.encodeToString(value, Base64.NO_WRAP)))
    assertEquals("", SecureStoreBridge.remove(context, key))
    assertArrayEquals(byteArrayOf(1), SecureStoreBridge.getEnvelope(context, key))
}
```

The CMake boundary scanner must fail if `QJniObject`, `QJniEnvironment`, `jni.h`, `android/`, `android.content`, or Kotlin package names appear under shared core/domain/application roots; allow them only under `src/apps/android-qml/bridge` and Android package sources.

- [ ] **Step 2: Run the focused checks red**

```bash
cmake -DDA_SOURCE_DIR="$PWD" \
  -DDA_LINK_GRAPH=build/cmake/linux-core/dailyaccount-link-graph.txt \
  -P tests/cmake/android_boundary_contract.cmake
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
gradle -p build/cmake/android-x86_64-debug/android-build \
  connectedDebugAndroidTest \
  -Pandroid.testInstrumentationRunnerArguments.class=local.dailyaccount.SecureStoreInstrumentationTest
```

Expected: the boundary script is missing or the instrumentation compile fails because `SecureStoreBridge` does not exist.

- [ ] **Step 3: Implement the Keystore primitive and private ciphertext store**

`SecureStoreBridge.kt` exposes exactly:

```kotlin
@JvmStatic fun put(context: Context, key: String, value: ByteArray): String
@JvmStatic fun getEnvelope(context: Context, key: String): ByteArray
@JvmStatic fun remove(context: Context, key: String): String
```

Validate keys against ASCII `[A-Za-z0-9._-]{1,128}` and values at `0..262144` bytes. Generate a non-exportable 256-bit AES key in provider `AndroidKeyStore`, alias `dailyaccount.session.v1`, purposes encrypt/decrypt, GCM mode, no padding, randomized encryption required, and no user-authentication requirement. For each `put`, generate a 12-byte IV, encrypt with AES/GCM, and persist Base64 of `[version=1][ivLength=12][iv][ciphertext+tag]` in private preferences `dailyaccount_secure_store`; preference keys are SHA-256 of the logical key. `getEnvelope` returns status byte `0` followed by plaintext, byte `1` for absent, or byte `2` followed by a short ASCII diagnostic code for malformed/corrupt/Keystore failure. Clear plaintext temporary arrays in `finally` where mutable. Never log key names, values, ciphertext, exceptions containing content, or aliases beyond the fixed alias.

- [ ] **Step 4: Implement the C++ ISecureStore adapter**

```cpp
class AndroidSecureStore final : public ISecureStore {
public:
    Result<void> put(std::string_view key,
                     const std::vector<std::byte>& value) override;
    Result<std::vector<std::byte>> get(std::string_view key) override;
    Result<void> remove(std::string_view key) override;
};
```

Use `QNativeInterface::QAndroidApplication::context()` and static `QJniObject` calls only in the `.cpp`. Convert Java byte arrays by length, not C strings. Map envelope status `1` to `NotFound`, status `2`, JNI exception, invalid UTF-8 key, and unavailable context to `StorageFailure`; clear JNI exceptions after extracting only a fixed diagnostic code. Instantiate this adapter in `MobileComposition` as an unused platform capability so Stage 4 can consume `ISecureStore` without changing QML. Do not write an auth token in Stage 3.

- [ ] **Step 5: Run focused and cumulative verification**

```bash
cmake -DDA_SOURCE_DIR="$PWD" \
  -DDA_LINK_GRAPH=build/cmake/linux-core/dailyaccount-link-graph.txt \
  -P tests/cmake/android_boundary_contract.cmake
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
gradle -p build/cmake/android-x86_64-debug/android-build \
  connectedDebugAndroidTest \
  -Pandroid.testInstrumentationRunnerArguments.class=local.dailyaccount.SecureStoreInstrumentationTest
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: binary/zero-byte round trip, ciphertext-at-rest, tamper failure, invalid key, oversize rejection, and remove/not-found tests pass on API 28 and API 35; boundary scan passes; app APK and inherited suites remain green.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/apps/android-qml/CMakeLists.txt \
  src/apps/android-qml/mobile_composition.* src/apps/android-qml/bridge/android_secure_store.* \
  src/apps/android-qml/android/build.gradle \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/SecureStoreBridge.kt \
  src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/SecureStoreInstrumentationTest.kt \
  tests/cmake/android_boundary_contract.cmake
git commit -m "feat: add Android Keystore secure store"
```

Run only with explicit authorization; otherwise do not commit.

---

### Task 10: Add Narrow Android File Picker and Text Share Bridges

**Files:**
- Create: `src/apps/android-qml/bridge/android_file_share_bridge.h`
- Create: `src/apps/android-qml/bridge/android_file_share_bridge.cpp`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/DailyAccountActivity.kt`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/FileShareBridge.kt`
- Create: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/FileShareInstrumentationTest.kt`
- Create: `src/apps/android-qml/qml/SettingsPage.qml`
- Modify: `src/apps/android-qml/android/AndroidManifest.xml`
- Modify: `src/apps/android-qml/android/build.gradle`
- Modify: `src/apps/android-qml/qml/Main.qml`
- Modify: `src/apps/android-qml/main.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Modify: `tests/cmake/android_boundary_contract.cmake`
- Test: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/FileShareInstrumentationTest.kt`
- Test: `tests/cmake/android_boundary_contract.cmake`

**Interfaces:**
- Consumes: Android `QtActivity`, `ACTION_OPEN_DOCUMENT`, `ContentResolver`, `ACTION_SEND`, and D-027's 1,048,576-byte import ceiling.
- Produces: `AndroidFileShareBridge::pickTextFile/shareText`; signals carrying raw selected bytes or cancellation/error; no storage permission and no parsing/business write.

- [ ] **Step 1: Write failing intent and byte-boundary instrumentation tests**

```kotlin
@Test
fun pickerUsesSafTextIntentWithoutStoragePermission() {
    val intent = FileShareBridge.createOpenTextIntent()
    assertEquals(Intent.ACTION_OPEN_DOCUMENT, intent.action)
    assertEquals("text/plain", intent.type)
    assertTrue(intent.categories!!.contains(Intent.CATEGORY_OPENABLE))
    val permissions = packageManager.getPackageInfo(packageName,
        PackageManager.GET_PERMISSIONS).requestedPermissions?.toSet().orEmpty()
    assertFalse(permissions.contains(Manifest.permission.READ_EXTERNAL_STORAGE))
    assertFalse(permissions.contains(Manifest.permission.MANAGE_EXTERNAL_STORAGE))
}
```

Also test a synthetic content URI returns exact UTF-8 bytes and display name, reads at most `1,048,577` bytes, reports `FILE_TOO_LARGE_FOR_BRIDGE` on the next byte, cancellation emits no data, share uses `ACTION_SEND`/`text/plain`/`EXTRA_TEXT`, and neither content nor selected URI appears in logcat.

- [ ] **Step 2: Run the focused instrumentation test red**

```bash
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
gradle -p build/cmake/android-x86_64-debug/android-build \
  connectedDebugAndroidTest \
  -Pandroid.testInstrumentationRunnerArguments.class=local.dailyaccount.FileShareInstrumentationTest
```

Expected: Kotlin compilation fails because `FileShareBridge` and `DailyAccountActivity` are absent.

- [ ] **Step 3: Implement one Activity-owned result callback**

`DailyAccountActivity : QtActivity` owns request code `4103`, permits only one pending picker, and overrides `onActivityResult`. `FileShareBridge` exposes:

```kotlin
@JvmStatic fun createOpenTextIntent(): Intent
@JvmStatic fun readPickedText(context: Context, uri: Uri): PickedTextEnvelope
@JvmStatic fun createShareTextIntent(subject: String, text: String): Intent
@JvmStatic external fun nativePicked(requestId: Long, displayName: String, bytes: ByteArray)
@JvmStatic external fun nativeCancelled(requestId: Long)
@JvmStatic external fun nativeFailed(requestId: Long, code: String)
```

Read with `ContentResolver.openInputStream`, a fixed 32 KiB buffer, and a hard stop after `1,048,577` bytes. Do not normalize, parse, classify, retain, or log content in Kotlin. Use `OpenableColumns.DISPLAY_NAME`, capped at 255 UTF-8 bytes after C++ conversion. Do not persist URI permission in Stage 3.

- [ ] **Step 4: Implement queued C++ callbacks and QML-facing capability**

```cpp
class AndroidFileShareBridge final : public QObject {
    Q_OBJECT
public:
    explicit AndroidFileShareBridge(QObject* parent = nullptr);
    Q_INVOKABLE void pickTextFile();
    Q_INVOKABLE bool shareText(const QString& subject, const QString& text);
signals:
    void textFilePicked(QString displayName, QByteArray utf8Bytes);
    void pickCancelled();
    void bridgeFailed(QString code, QString message);
};
```

Use an atomic request counter and one registered JNI callback target. Native callbacks copy byte arrays and use `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`; they never touch QML from the Android callback thread. Reject empty/invalid activity, concurrent pick, display-name overflow, and payload above `1,048,576` with fixed codes. The extra byte exists only to distinguish exact-limit from over-limit input; shared import parsing remains Stage 5.

Export the three callback symbols with their exact JNI names so no runtime string registration is hidden:

```cpp
extern "C" JNIEXPORT void JNICALL
Java_local_dailyaccount_FileShareBridge_nativePicked(
    JNIEnv*, jclass, jlong requestId, jstring displayName, jbyteArray bytes);
extern "C" JNIEXPORT void JNICALL
Java_local_dailyaccount_FileShareBridge_nativeCancelled(
    JNIEnv*, jclass, jlong requestId);
extern "C" JNIEXPORT void JNICALL
Java_local_dailyaccount_FileShareBridge_nativeFailed(
    JNIEnv*, jclass, jlong requestId, jstring code);
```

Register `DailyAccountActivity` in the manifest. `SettingsPage.qml` exposes `选择 TXT 文件` and `分享诊断文本` capability probes with object names `pickTextFileButton` and `shareTextButton`; picked content is not shown or persisted. Main navigation treats settings as an app-shell page, not an accounting module route.

- [ ] **Step 5: Run focused and cumulative verification**

```bash
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
gradle -p build/cmake/android-x86_64-debug/android-build \
  connectedDebugAndroidTest \
  -Pandroid.testInstrumentationRunnerArguments.class=local.dailyaccount.FileShareInstrumentationTest
cmake -DDA_SOURCE_DIR="$PWD" \
  -DDA_LINK_GRAPH=build/cmake/linux-core/dailyaccount-link-graph.txt \
  -P tests/cmake/android_boundary_contract.cmake
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: SAF/share intent, exact-byte, overflow, cancellation, and no-permission tests pass; bridge callbacks run on the Qt thread; no selected content enters logs; Android boundary and inherited tests pass.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add src/apps/android-qml/bridge/android_file_share_bridge.* \
  src/apps/android-qml/android/AndroidManifest.xml \
  src/apps/android-qml/android/build.gradle \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/DailyAccountActivity.kt \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/FileShareBridge.kt \
  src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/FileShareInstrumentationTest.kt \
  src/apps/android-qml/qml/Main.qml src/apps/android-qml/qml/SettingsPage.qml \
  src/apps/android-qml/main.cpp src/apps/android-qml/CMakeLists.txt \
  tests/cmake/android_boundary_contract.cmake
git commit -m "feat: add Android file and share bridges"
```

Without authorization, do not commit.

---

### Task 11: Implement Shared Idempotent Recurring Confirmation and the 90-Day Reminder Planner

**Files:**
- Modify: `src/modules/accounting/application/recurring_service.h`
- Modify: `src/modules/accounting/application/recurring_service.cpp`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.h`
- Modify: `src/modules/accounting/application/accounting_mutation_codec.cpp`
- Create: `src/modules/accounting/application/reminder_planner.h`
- Create: `src/modules/accounting/application/reminder_planner.cpp`
- Modify: `src/modules/accounting/data/sqlite/accounting_unit_of_work.cpp`
- Create: `tests/unit/recurring_confirmation_tests.cpp`
- Create: `tests/unit/reminder_planner_tests.cpp`
- Create: `tests/integration/recurring_confirmation_sqlite_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/recurring_confirmation_tests.cpp`
- Test: `tests/unit/reminder_planner_tests.cpp`
- Test: `tests/integration/recurring_confirmation_sqlite_tests.cpp`

**Interfaces:**
- Consumes: Stage 2 deterministic pending pair, D-025 recurrence/date/event-key rules, `INotificationScheduler`, schema-2 repositories/UoW, and outbox expectations.
- Produces: `ConfirmRecurringOccurrenceCommand`, idempotent `RecurringService::confirmOccurrence`, atomic `CONFIRM_RECURRING_OCCURRENCE` mutation, and `ReminderPlanner::eventsForRule` with inclusive D0+90 semantics.

- [ ] **Step 1: Write failing confirmation and horizon tests**

Use this real-SQLite idempotency case:

```cpp
void duplicateLocalConfirmationPostsTheExistingTransactionExactlyOnce()
{
    RecurringFixture fixture;
    const auto pair = fixture.generate("2026-09");
    const ConfirmRecurringOccurrenceCommand first{
        pair.occurrence.id, UtcInstant{1'788'480'000'000},
        fixture.deviceId(), MutationId::random()};
    const ConfirmRecurringOccurrenceCommand retry{
        pair.occurrence.id, UtcInstant{1'788'480'001'000},
        fixture.deviceId(), MutationId::random()};
    DA_CHECK(fixture.service.confirmOccurrence(first).value().changed);
    DA_CHECK(!fixture.service.confirmOccurrence(retry).value().changed);
    DA_CHECK_EQ(fixture.postedTransactionCount(pair.transaction.id), 1);
    DA_CHECK_EQ(fixture.confirmMutationCount(pair.occurrence.id), 1);
    DA_CHECK_EQ(fixture.transactionIdFor(pair.occurrence.id), pair.transaction.id);
}
```

The planner test uses a deterministic interval rule and asserts event target dates at D0+89 and D0+90 are included, D0+91 is absent, duplicate planning returns byte-identical keys/snapshots, offsets are only `-2/-1/0` according to the rule lead, and the event title/amount snapshot is generated by C++.

- [ ] **Step 2: Run all focused tests red**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target \
  dailyaccount_recurring_confirmation_tests \
  dailyaccount_reminder_planner_tests --parallel 2
```

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_recurring_confirmation_sqlite_tests --parallel 2
```

Expected: unknown targets or compilation failure because confirmation command/planner APIs do not exist.

- [ ] **Step 3: Define the exact confirmation contract**

```cpp
struct ConfirmRecurringOccurrenceCommand {
    RecurringOccurrenceId occurrenceId;
    UtcInstant confirmedAt;
    DeviceId modifiedByDeviceId;
    MutationId mutationId;
};

struct ConfirmedRecurringOccurrence {
    RecurringOccurrence occurrence;
    Transaction transaction;
    bool changed;
};

Result<ConfirmedRecurringOccurrence> RecurringService::confirmOccurrence(
    const ConfirmRecurringOccurrenceCommand& command);
```

Inside one UoW, load the live occurrence and linked transaction. A valid `PENDING` pair changes both statuses to `POSTED`, sets both `updatedAt` and `modifiedByDeviceId`, preserves IDs, amounts, current-period edited fields, `createdAt`, owner, and server revisions, updates transaction then occurrence, and enqueues one `CONFIRM_RECURRING_OCCURRENCE` version-1 mutation with both base revisions. A preexisting valid `POSTED` pair returns `changed=false` and writes no row/outbox even when the retry carries a different mutation ID. Missing, skipped, cancelled, malformed, tombstoned, or split pairs fail without writes. Outbox failure rolls both status changes back.

The payload is canonical JSON containing one complete occurrence and its complete linked transaction after-state. Its expectation array is sorted by `(entityType, entityId)`.

- [ ] **Step 4: Define and implement the shared reminder planner**

```cpp
struct ReminderPlanContext {
    LocalDate d0;
    std::set<std::string> suppressedPeriodKeys;
};

class ReminderPlanner final {
public:
    Result<std::vector<ReminderEvent>> eventsForRule(
        const RecurringRule& rule,
        const ReminderPlanContext& context) const;
};
```

Enumerate canonical periods using D-025's monthly, yearly, or interval grammar without clock/time-zone conversion in Kotlin. Consider due dates far enough outside the window for a lead event to land on D0. Include an event only when `targetLocalDate` is in the inclusive range `[D0, D0+90]`, the rule is live/enabled, the period is within start/end bounds, and its key is not suppressed by a posted/skipped/cancelled occurrence. Emit lead offset `-rule.leadDays` and due offset `0`; sort by target date, rule UUID, period key, then offset. Use exact event key `<rule UUID>:<periodKey>:<offsetDays>`. Build `displaySnapshotJson` as canonical UTF-8 JSON `{"title":"...","amountText":"..."}`, where amount text comes from `formatMoney`; enforce 4,096 bytes and never include note, account, category, user, or token.

- [ ] **Step 5: Run focused and cumulative verification**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_recurring_confirmation_tests
./build/cmake/linux-core/dailyaccount_reminder_planner_tests
./build/cmake/linux-core/dailyaccount_recurring_service_tests
./build/cmake/linux-core/dailyaccount_accounting_rules_tests
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
```

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_recurring_confirmation_sqlite_tests.exe'
ctest --preset windows-desktop -R 'recurring|sqlite_unit_of_work' --output-on-failure
```

```bash
git diff --check
```

Expected: duplicate confirmation posts one existing transaction and one confirmation mutation, rollback leaves the pending pair intact, D0+89/D0+90 pass and D0+91 is absent, all shared tests stay Qt-free, SQLite tests pass, and DAT remains at 22 passes.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/modules/accounting/application/recurring_service.* \
  src/modules/accounting/application/accounting_mutation_codec.* \
  src/modules/accounting/application/reminder_planner.* \
  src/modules/accounting/data/sqlite/accounting_unit_of_work.cpp \
  tests/unit/recurring_confirmation_tests.cpp tests/unit/reminder_planner_tests.cpp \
  tests/integration/recurring_confirmation_sqlite_tests.cpp
git commit -m "feat: confirm recurring occurrences idempotently"
```

Without explicit authorization, do not commit.

---

### Task 12: Implement Persisted Kotlin Reminder Scheduling, Delivery, and Reboot Recovery

**Files:**
- Create: `src/apps/android-qml/bridge/android_notification_scheduler.h`
- Create: `src/apps/android-qml/bridge/android_notification_scheduler.cpp`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderContract.kt`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderStore.kt`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderScheduler.kt`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderReceiver.kt`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderBootReceiver.kt`
- Create: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/ReminderInstrumentationTest.kt`
- Modify: `src/apps/android-qml/android/AndroidManifest.xml`
- Modify: `src/apps/android-qml/android/build.gradle`
- Modify: `src/apps/android-qml/mobile_composition.h`
- Modify: `src/apps/android-qml/mobile_composition.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Modify: `tests/cmake/android_boundary_contract.cmake`
- Test: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/ReminderInstrumentationTest.kt`
- Test: `tests/cmake/android_boundary_contract.cmake`

**Interfaces:**
- Consumes: accepted D-030 alarm API, `INotificationScheduler`, C++ `ReminderEvent`, package lifecycle broadcasts, and Android notification permission/health APIs.
- Produces: durable `ReminderStore`; idempotent `replaceEvents/cancelPeriod/reconcile`; `ReminderReceiver`; `ReminderBootReceiver`; C++ `AndroidNotificationScheduler`; no Qt startup in receivers.

- [ ] **Step 1: Write failing native persistence and idempotency tests**

```kotlin
@Test
fun replacingTheSameEventsTwiceLeavesOneRowAndOnePendingIntent() {
    val context = InstrumentationRegistry.getInstrumentation().targetContext
    val store = ReminderStore(context)
    val event = NativeReminderEvent(
        "11111111-1111-4111-8111-111111111111:2026-12:0",
        "11111111-1111-4111-8111-111111111111", "2026-12", 0,
        LocalDate.of(2026, 12, 2).toEpochDay(), "Asia/Shanghai", "月卡", "99.00")
    assertEquals("", ReminderScheduler.replaceEvents(context, event.ruleId, listOf(event)))
    assertEquals("", ReminderScheduler.replaceEvents(context, event.ruleId, listOf(event)))
    assertEquals(1, store.countByEventKey(event.eventKey))
    assertTrue(ReminderScheduler.pendingIntentExistsForTest(context, event.eventKey))
}
```

Also test transactional replacement removes stale undelivered rule events, cancel-period removes all offsets, delivered metadata does not change accounting state, malformed event JSON is rejected without replacing old rows, local date survives time-zone changes, and health maps permission/scheduling restrictions exactly.

- [ ] **Step 2: Run the native reminder test red**

```bash
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
gradle -p build/cmake/android-x86_64-debug/android-build \
  connectedDebugAndroidTest \
  -Pandroid.testInstrumentationRunnerArguments.class=local.dailyaccount.ReminderInstrumentationTest
```

Expected: Kotlin compilation fails because `NativeReminderEvent`, `ReminderStore`, and `ReminderScheduler` are absent.

- [ ] **Step 3: Implement the native data contract and private SQLite store**

Use the Stage 0-proven shape exactly:

```kotlin
data class NativeReminderEvent(
    val eventKey: String,
    val ruleId: String,
    val periodKey: String,
    val offsetDays: Int,
    val targetEpochDay: Long,
    val timeZoneId: String,
    val displayTitle: String,
    val displayAmountText: String
)
```

`ReminderStore` uses `SQLiteOpenHelper` database `reminders.db`, version 1, and a `STRICT`-compatible table when supported by the accepted Android SQLite version:

```sql
CREATE TABLE reminder_events (
  event_key TEXT PRIMARY KEY,
  rule_id TEXT NOT NULL,
  period_key TEXT NOT NULL,
  offset_days INTEGER NOT NULL CHECK(offset_days IN (-2,-1,0)),
  target_epoch_day INTEGER NOT NULL,
  time_zone_id TEXT NOT NULL,
  display_title TEXT NOT NULL,
  display_amount_text TEXT NOT NULL,
  delivered_at_ms INTEGER,
  scheduled_at_ms INTEGER,
  updated_at_ms INTEGER NOT NULL
)
```

Add indexes `(rule_id, period_key)` and `(target_epoch_day, delivered_at_ms)`. `replaceEvents(ruleId, events)` validates every event belongs to the rule and performs delete-stale plus upsert in one native SQLite transaction. `cancelPeriod` deletes only undelivered rows for that rule/period. Native code never opens `accounting.sqlite`.

- [ ] **Step 4: Implement the exact D-030 scheduling branch**

Derive 09:00 in `timeZoneId`, choosing the first valid instant at or after 09:00 on a DST gap. Use intent action `local.dailyaccount.REMINDER`, data URI `dailyaccount-reminder://event/<percent-encoded-event-key>`, request code `0`, and flags `FLAG_UPDATE_CURRENT | FLAG_IMMUTABLE`.

If D-030 selected `setAndAllowWhileIdle`, production scheduling is exactly:

```kotlin
alarmManager.setAndAllowWhileIdle(
    AlarmManager.RTC_WAKEUP, triggerAtMillis, pendingIntent)
```

If D-030 selected the tested exact-alarm fallback, guard `canScheduleExactAlarms()` on API 31+ and production scheduling is exactly:

```kotlin
if (!alarmManager.canScheduleExactAlarms()) return "SCHEDULING_RESTRICTED"
alarmManager.setExactAndAllowWhileIdle(
    AlarmManager.RTC_WAKEUP, triggerAtMillis, pendingIntent)
```

Implement only the branch selected in accepted D-030. Do not retain runtime switching between unaccepted mechanisms. Past undelivered events schedule an immediate bounded reconciliation notification once; already delivered rows do not reschedule.

- [ ] **Step 5: Implement receiver, rebuild broadcasts, and health**

`ReminderReceiver` loads the current row by event key, confirms it is undelivered, creates channel `recurring_due`, posts with `ic_stat_dailyaccount`, amount text as text, and a deep link containing rule/period IDs. It marks `delivered_at_ms` only after successful post. Opening or dismissing does not call C++, Qt, WorkManager, or any accounting API.

`ReminderBootReceiver` handles `BOOT_COMPLETED`, `TIME_SET`, `TIMEZONE_CHANGED`, and `MY_PACKAGE_REPLACED`, loads undelivered events, and reconciles alarms. Manifest permissions are `RECEIVE_BOOT_COMPLETED` and, on API 33+, `POST_NOTIFICATIONS`; add `SCHEDULE_EXACT_ALARM` only when D-030 selected the exact branch. Receiver classes are plain Kotlin/Android classes and must not reference `QtActivity`, `QtNative`, JNI, or network APIs.

Return health codes `READY`, `PERMISSION_REQUIRED`, `SCHEDULING_RESTRICTED`, or `FORCE_STOPPED_UNTIL_REOPEN`; C++ maps exactly to `NotificationHealthCode`. Force-stop status is set only from the app's next-launch reconciliation diagnostic, not guessed while the package cannot run.

- [ ] **Step 6: Implement the C++ scheduler adapter**

```cpp
class AndroidNotificationScheduler final : public INotificationScheduler {
public:
    Result<void> replaceEvents(
        RecurringRuleId ruleId,
        const std::vector<ReminderEvent>& events) override;
    Result<void> cancelPeriod(
        RecurringRuleId ruleId,
        std::string_view periodKey) override;
    NotificationHealth health() const override;
    Result<void> reconcilePersistedEvents();
};
```

Serialize supplied events to canonical JSON. Convert the already validated C++ `LocalDate` in the adapter with `QDate(1970, 1, 1).daysTo(QDate(year, month, day))`; display title/amount comes from the C++ snapshot, and Kotlin validates transport shape only. JNI errors map to fixed storage/platform codes. `replaceEvents` is called after accounting transactions commit, never from inside a UoW. Instantiate the adapter in `MobileComposition` and reconcile persisted native rows on app launch before QML reports reminder health.

- [ ] **Step 7: Run focused and cumulative verification**

```bash
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
gradle -p build/cmake/android-x86_64-debug/android-build \
  connectedDebugAndroidTest \
  -Pandroid.testInstrumentationRunnerArguments.class=local.dailyaccount.ReminderInstrumentationTest
cmake -DDA_SOURCE_DIR="$PWD" \
  -DDA_LINK_GRAPH=build/cmake/linux-core/dailyaccount-link-graph.txt \
  -P tests/cmake/android_boundary_contract.cmake
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_reminder_planner_tests
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: native persistence/replacement/cancel/health tests pass, the accepted alarm API is the only production branch, receiver boundary scan proves no Qt/network/business dependency, APK builds, and inherited suites remain green.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add src/apps/android-qml/bridge/android_notification_scheduler.* \
  src/apps/android-qml/android/AndroidManifest.xml \
  src/apps/android-qml/android/build.gradle \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderContract.kt \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderStore.kt \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderScheduler.kt \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderReceiver.kt \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderBootReceiver.kt \
  src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/ReminderInstrumentationTest.kt \
  src/apps/android-qml/mobile_composition.* src/apps/android-qml/CMakeLists.txt \
  tests/cmake/android_boundary_contract.cmake
git commit -m "feat: add persisted Android reminders"
```

Do not run the checkpoint without explicit authorization.

---

### Task 13: Add the Minimum Recurring List, Home Card, Reconciliation, and One-Tap Confirmation

**Files:**
- Create: `src/apps/android-qml/models/recurring_list_model.h`
- Create: `src/apps/android-qml/models/recurring_list_model.cpp`
- Create: `src/apps/android-qml/mobile_recurring_facade.h`
- Create: `src/apps/android-qml/mobile_recurring_facade.cpp`
- Create: `src/apps/android-qml/qml/RecurringPage.qml`
- Create: `src/apps/android-qml/qml/components/RecurringCard.qml`
- Modify: `src/apps/android-qml/qml/OverviewPage.qml`
- Modify: `src/apps/android-qml/qml/Main.qml`
- Modify: `src/apps/android-qml/mobile_composition.h`
- Modify: `src/apps/android-qml/mobile_composition.cpp`
- Modify: `src/apps/android-qml/main.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Modify: `tests/integration/mobile_facade_tests.cpp`
- Modify: `tests/integration/mobile_lifecycle_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/integration/mobile_facade_tests.cpp`
- Test: `tests/integration/mobile_lifecycle_tests.cpp`

**Interfaces:**
- Consumes: Task 5 recurring projections/enabled rules, Stage 2 generation, Task 11 confirmation/planner, Task 12 scheduler, active device/clock, and foreground compensation.
- Produces: `RecurringListModel`; `MobileRecurringFacade::refresh/reconcile/confirmOccurrence`; all-unresolved list; due-window home model; notification health; idempotent one-tap posting and period-alarm cancellation.

- [ ] **Step 1: Write the failing home-card confirmation test**

```cpp
void homeCardConfirmationPostsOnceAndCancelsItsNativePeriod()
{
    MobileRecurringFixture fixture;
    const auto ids = fixture.seedPendingDueToday("2026-09", MoneyMinor{9900});
    fixture.facade.refresh("2026-09-04");
    DA_CHECK(fixture.waitForRefresh());
    DA_CHECK_EQ(fixture.facade.homeOccurrences()->rowCount(), 1);
    fixture.facade.confirmOccurrence(QString::fromStdString(ids.occurrenceId.toString()));
    fixture.facade.confirmOccurrence(QString::fromStdString(ids.occurrenceId.toString()));
    DA_CHECK(fixture.waitForConfirmation());
    DA_CHECK_EQ(fixture.postedCount(ids.transactionId), 1);
    DA_CHECK_EQ(fixture.confirmMutationCount(ids.occurrenceId), 1);
    DA_CHECK_EQ(fixture.scheduler.cancelCallsFor(ids.ruleId, "2026-09"), 1);
    DA_CHECK_EQ(fixture.facade.homeOccurrences()->rowCount(), 0);
}
```

Also test prior overdue plus current pending remain separate list rows, home includes every overdue row and pending rows with `0 <= daysUntilDue <= leadDays`, future rows remain list-only, disabled-rule unresolved rows remain visible, notification health error text is actionable, scheduler failure after a committed confirmation does not roll back or duplicate finance state, and foreground reconciliation repairs stale native events.

- [ ] **Step 2: Run the focused facade tests red**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target \
  dailyaccount_mobile_facade_tests dailyaccount_mobile_lifecycle_tests --parallel 2
```

Expected: compilation fails for missing recurring model/facade or the home model remains empty.

- [ ] **Step 3: Define exact recurring model roles and facade API**

`RecurringListModel` exposes roles `occurrenceId`, `ruleId`, `periodKey`, `title`, `note`, `marker`, `amountText`, `effectiveDueOn`, `daysUntilDue`, `persistentStatus`, `displayState`, and `canConfirm`. `amountText` is always `QString`; `daysUntilDue` is non-financial and may be an integer.

```cpp
class MobileRecurringFacade final : public QObject {
    Q_OBJECT
    Q_PROPERTY(RecurringListModel* occurrences READ occurrences CONSTANT)
    Q_PROPERTY(RecurringListModel* homeOccurrences READ homeOccurrences CONSTANT)
    Q_PROPERTY(QString notificationHealthCode READ notificationHealthCode NOTIFY healthChanged)
    Q_PROPERTY(QString notificationHealthMessage READ notificationHealthMessage NOTIFY healthChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    Q_INVOKABLE void refresh(const QString& todayIso);
    Q_INVOKABLE void reconcile(const QString& todayIso);
    Q_INVOKABLE void confirmOccurrence(const QString& occurrenceId);
    void detachModels();
    void stopAcceptingCommands();
    void waitForJobs();
signals:
    void recurringChanged();
    void healthChanged();
    void busyChanged();
    void confirmationSucceeded(QString occurrenceId, QString transactionId);
    void operationFailed(QString code, QString message);
};
```

- [ ] **Step 4: Implement C++ reconciliation and post-commit cancellation**

On launch and each foreground resume, asynchronously list enabled rules and existing occurrence states. For each rule, build `suppressedPeriodKeys` from posted/skipped/cancelled occurrences, call `ReminderPlanner`, deduplicate the returned event `periodKey` values, and call `generateOccurrence` for each missing period with one stable mutation ID per attempt and D-025's 240-occurrence cap. Then call `replaceEvents` with the same plan. The mobile adapter does not parse recurrence grammar or calculate a period/date. Operations already committed before a later failure remain valid and are retried idempotently on the next reconciliation.

`confirmOccurrence` disables that card, creates IDs once, calls shared confirmation, and only after its SQLite commit calls `cancelPeriod`. A cancel failure emits `REMINDER_RECONCILIATION_REQUIRED`, refreshes finance models to the posted truth, and leaves the next launch/resume reconciliation responsible for removing the stale native event. A second tap while pending is ignored; a retry after completion receives `changed=false` and cannot enqueue or cancel twice in the same facade generation.

- [ ] **Step 5: Build the minimum recurring surfaces**

`RecurringPage.qml` displays every unresolved occurrence as one row, including multiple periods of one rule and unresolved rows from disabled rules. It shows name, optional note/marker, decimal amount text, effective date, countdown/overdue wording, persistent/display state, and `确认已支付` when `canConfirm`. Object names are `recurringList`, `recurringEmptyState`, and per-card `confirmRecurringButton`.

`OverviewPage.qml` inserts `homeRecurringList` before recent transactions. Each `RecurringCard` shows the same existing fields and one direct confirmation button without reopening amount/date input. Show notification health banner object `notificationHealthBanner` for `PERMISSION_REQUIRED`, `SCHEDULING_RESTRICTED`, or `FORCE_STOPPED_UNTIL_REOPEN`; its action opens Settings. Do not display a notification as payment confirmation.

Production Stage 3 has an honest empty state and no incomplete rule editor. G3 setup inserts one synthetic schema-valid rule through instrumentation and invokes the same shared `createRule/generateOccurrence` services; Stage 5 owns production rule create/edit, current-period edit, defer, skip, cancel, undo, and import-match UI.

- [ ] **Step 6: Run focused and cumulative verification**

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
& 'build\cmake\windows-desktop\dailyaccount_mobile_facade_tests.exe'
& 'build\cmake\windows-desktop\dailyaccount_mobile_lifecycle_tests.exe'
& 'build\cmake\windows-desktop\dailyaccount_recurring_confirmation_sqlite_tests.exe'
ctest --preset windows-desktop -R 'mobile_|recurring|sqlite_' --output-on-failure
```

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/qml_money_boundary_contract.cmake
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_recurring_confirmation_tests
./build/cmake/linux-core/dailyaccount_reminder_planner_tests
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
git diff --check
```

Expected: list/home filtering and one-tap idempotency pass, finance commit survives scheduler failure, foreground repair passes, every money role remains text, Android packages, and inherited suites remain green.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/apps/android-qml/CMakeLists.txt \
  src/apps/android-qml/models/recurring_list_model.* \
  src/apps/android-qml/mobile_recurring_facade.* \
  src/apps/android-qml/mobile_composition.* src/apps/android-qml/main.cpp \
  src/apps/android-qml/qml/RecurringPage.qml \
  src/apps/android-qml/qml/OverviewPage.qml src/apps/android-qml/qml/Main.qml \
  src/apps/android-qml/qml/components/RecurringCard.qml \
  tests/integration/mobile_facade_tests.cpp tests/integration/mobile_lifecycle_tests.cpp
git commit -m "feat: add recurring home confirmation slice"
```

Without authorization, do not commit.

---

### Task 14: Add Flight-Mode, Restart, and Process-Death Android Instrumentation

**Files:**
- Create: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/OfflineCrudInstrumentationTest.kt`
- Create: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/ProcessDeathInstrumentationTest.kt`
- Create: `tests/android/run_stage3_matrix.sh`
- Create: `tests/android/run_process_death_smoke.sh`
- Modify: `src/apps/android-qml/android/build.gradle`
- Modify: `src/apps/android-qml/android/AndroidManifest.xml`
- Modify: `src/apps/android-qml/main.cpp`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Create: `docs/validation/stage-3/android-runtime-results.json`
- Create: `docs/validation/stage-3/android-offline-results.json`
- Create: `docs/validation/stage-3/android-process-death-results.json`
- Test: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/OfflineCrudInstrumentationTest.kt`
- Test: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/ProcessDeathInstrumentationTest.kt`
- Test: `tests/android/run_stage3_matrix.sh`
- Test: `tests/android/run_process_death_smoke.sh`

**Interfaces:**
- Consumes: stable QML object names, schema-2 app-private database, local profile shell, Android instrumentation/UiAutomator, and atomic entity/outbox behavior.
- Produces: reproducible API 28/API 35/physical-ARM evidence for no-network create/read/edit/delete, exact max-money UI round trip, activity restart, process kill/relaunch, and no partial business/outbox state.

- [ ] **Step 1: Write failing end-to-end instrumentation cases**

The flight-mode case must execute this observable sequence:

```kotlin
@Test
fun flightModeCrudAndMaximumMoneySurviveActivityRestart() {
    launchFreshLocalProfile("G3 Offline")
    setNetworkDisabled()
    openQuickEntry()
    enterText("quickAmountField", "99999999.99")
    tap("quickSaveButton")
    waitForText("99999999.99")
    restartActivity()
    openTransactions()
    tapText("99999999.99")
    replaceText("detailAmountField", "12345678.90")
    tap("detailSaveButton")
    waitForText("12345678.90")
    assertDatabaseCounts(liveTransactions = 1, outbox = 2)
    deleteCurrentTransactionAndConfirm()
    assertDatabaseCounts(liveTransactions = 0, tombstones = 1, outbox = 3)
}
```

The process-death case creates a normal `12.34` expense, waits for local success, records the transaction UUID, executes `am kill local.dailyaccount`, relaunches with `am start -W`, reopens the same profile, and verifies the same UUID/amount and one outbox row. A second case kills immediately after save dispatch and accepts only atomic states `(transactions=0,outbox=0)` or `(transactions=1,outbox=1)`; split counts fail.

- [ ] **Step 2: Run the tests red on API 35**

```bash
: "${DA_API35_SERIAL:?DA_API35_SERIAL must name the accepted API 35 emulator}"
export ANDROID_SERIAL="$DA_API35_SERIAL"
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
gradle -p build/cmake/android-x86_64-debug/android-build \
  connectedDebugAndroidTest \
  -Pandroid.testInstrumentationRunnerArguments.class=local.dailyaccount.OfflineCrudInstrumentationTest,local.dailyaccount.ProcessDeathInstrumentationTest
```

Expected: tests fail on missing helpers/test semantics or missing stable object names before the harness and app diagnostics exist.

- [ ] **Step 3: Implement bounded test diagnostics without a release bypass**

Instrumentation reads app-private SQLite through `InstrumentationRegistry.getInstrumentation().targetContext`; it uses read-only queries for counts/IDs and never mutates finance rows directly. Add a debug-only intent extra `local.dailyaccount.TEST_RESET_PROFILE` honored only when the package has `ApplicationInfo.FLAG_DEBUGGABLE`; release builds ignore it. It deletes only the test-created app data before composition starts and cannot select an arbitrary path. Add this target-scoped definition so the reset branch is absent from non-Debug native binaries:

```cmake
target_compile_definitions(dailyaccount_android PRIVATE
    $<$<CONFIG:Debug>:DA_ANDROID_TEST_HOOKS=1>)
```

Guard all C++ intent-extra handling with `#if defined(DA_ANDROID_TEST_HOOKS)`. Task 15's synthetic recurring setup enters through a separate debug-only composition action and the production shared `createRule/generateOccurrence` services, never direct SQL.

Write `run_stage3_matrix.sh --serial SERIAL --apk APK --result JSON` to verify the target fingerprint/ABI/API against D-020, uninstall for a clean profile, install, and run `SecureStoreInstrumentationTest`, `FileShareInstrumentationTest`, `OfflineCrudInstrumentationTest`, and `ProcessDeathInstrumentationTest`. Capture only whitelisted result fields, restore Wi-Fi/data state in a shell trap, and reject transaction note text/JWT/email patterns in logcat. Use `adb shell svc wifi disable` and `svc data disable` for the offline CRUD window, verify disconnected state, and never rely on host network failure by accident.

`run_process_death_smoke.sh` drives the separate runner process through UiAutomator, uses `am kill` rather than force-stop for the guaranteed process-death case, records pre/post UUID/counts, and writes `atomicState=true` only for matching entity/outbox counts.

- [ ] **Step 4: Run focused and cumulative verification on all mandatory targets**

```bash
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
bash tests/android/run_stage3_matrix.sh \
  --serial "$DA_API28_SERIAL" \
  --apk build/cmake/android-x86_64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  --result /tmp/opencode/g3-api28-runtime.json
bash tests/android/run_stage3_matrix.sh \
  --serial "$DA_API35_SERIAL" \
  --apk build/cmake/android-x86_64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  --result /tmp/opencode/g3-api35-runtime.json
cmake --preset android-arm64-debug
cmake --build --preset android-arm64-debug --target apk --parallel 2
bash tests/android/run_stage3_matrix.sh \
  --serial "$DA_PHYSICAL_ARM64_SERIAL" \
  --apk build/cmake/android-arm64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  --result /tmp/opencode/g3-arm64-runtime.json
bash tests/android/run_process_death_smoke.sh \
  --serial "$DA_API35_SERIAL" \
  --apk build/cmake/android-x86_64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  --result docs/validation/stage-3/android-process-death-results.json
```

Then merge the three sanitized runtime objects into `android-runtime-results.json` and `android-offline-results.json`, retaining serial aliases rather than raw serial numbers. Run inherited checks:

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: every target installs/opens; flight-mode CRUD, exact `99999999.99` round trip, activity restart, `am kill` relaunch, stable UUID, entity/outbox atomicity, and tombstone assertions pass; no network call or sensitive log appears; inherited suites remain green.

- [ ] **Step 5: Use the optional checkpoint only with explicit authorization**

```bash
git add src/apps/android-qml/android/build.gradle \
  src/apps/android-qml/android/AndroidManifest.xml src/apps/android-qml/CMakeLists.txt \
  src/apps/android-qml/main.cpp \
  src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/OfflineCrudInstrumentationTest.kt \
  src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/ProcessDeathInstrumentationTest.kt \
  tests/android/run_stage3_matrix.sh tests/android/run_process_death_smoke.sh \
  docs/validation/stage-3/android-runtime-results.json \
  docs/validation/stage-3/android-offline-results.json \
  docs/validation/stage-3/android-process-death-results.json
git commit -m "test: prove Android offline process recovery"
```

Run only with explicit authorization. APKs, app data, and unredacted logcat remain untracked.

---

### Task 15: Run D0+89/D0+90 Native Reminder and Recurring Confirmation Smokes

**Files:**
- Modify: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/ReminderInstrumentationTest.kt`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/RuntimeDiagnostics.kt`
- Create: `tests/android/run_reminder_smoke.sh`
- Create: `docs/validation/stage-3/android-reminder-results.json`
- Modify: `src/apps/android-qml/main.cpp`
- Modify: `src/apps/android-qml/mobile_composition.h`
- Modify: `src/apps/android-qml/mobile_composition.cpp`
- Test: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/ReminderInstrumentationTest.kt`
- Test: `tests/android/run_reminder_smoke.sh`

**Interfaces:**
- Consumes: accepted D-030 Stage 0 proof, C++ planner boundary vectors, Kotlin persisted scheduler/receivers, recurring QML card, debug-only synthetic rule fixture, and target matrix.
- Produces: Stage 3 wiring evidence for inclusive horizon import, native idempotency, cold-process/offline delivery, normal-reboot rebuild, one-tap local posting, and no Qt initialization by the receiver.

- [ ] **Step 1: Write the failing integrated boundary and confirmation case**

```kotlin
@Test
fun cppBoundaryPlanPersistsD0Plus89And90AndExcludes91() {
    resetProfile()
    seedBoundaryDailyFixture()
    launchAndRunRecurringReconciliation()
    assertNativeTargetDatePersisted("2026-12-02") // D0+89
    assertNativeTargetDatePersisted("2026-12-03") // D0+90
    assertNativeTargetDateAbsent("2026-12-04")    // D0+91
}

@Test
fun dueTodayCardConfirmsOnceAndCancelsItsPeriod() {
    resetProfile()
    seedDueTodayFixture()
    launchAndRunRecurringReconciliation()
    assertHomeRecurringCardCount(1)
    openHomeRecurringCard()
    tap("confirmRecurringButton")
    tap("confirmRecurringButton")
    assertExactlyOnePostedLinkedTransaction()
    assertExactlyOneConfirmationMutation()
    assertNativePeriodHasNoUndeliveredEvent()
}
```

Add native-only tests that replacing the boundary plan twice retains one row per event key, notification display/dismiss leaves occurrence pending, permission denial returns `PERMISSION_REQUIRED`, and app reopen repairs a deliberately missing alarm from persisted data.

- [ ] **Step 2: Run the integrated case red**

```bash
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
gradle -p build/cmake/android-x86_64-debug/android-build \
  connectedDebugAndroidTest \
  -Pandroid.testInstrumentationRunnerArguments.class=local.dailyaccount.ReminderInstrumentationTest
```

Expected: the new integrated test fails before the synthetic fixture/planner-to-native bridge and confirmation observation are wired.

- [ ] **Step 3: Add a safe accelerated reminder smoke mode**

The instrumentation fixture sends debug-only intent action `local.dailyaccount.TEST_SEED_RECURRING` with an enum extra `fixture=BOUNDARY_DAILY` or `fixture=DUE_TODAY`; it accepts no caller-controlled IDs, values, dates, text, or file path. Under `#if defined(DA_ANDROID_TEST_HOOKS)`, expose only this native test surface in `mobile_composition.h`:

```cpp
enum class Stage3RecurringFixture { BoundaryDaily, DueToday };
Result<void> seedStage3RecurringFixtureForInstrumentation(
    Stage3RecurringFixture fixture);
```

`main.cpp` honors the action only in that compile-time branch and only when Android reports the package debuggable, maps the two exact enum strings, and calls this method. The method invokes production shared `createRule/generateOccurrence` services; direct fixture SQL is forbidden. Use these exact synthetic values:

```text
D0: 2026-09-04
zone: Asia/Shanghai
BOUNDARY_DAILY rule: 33333333-3333-4333-8333-333333333333
BOUNDARY_DAILY frequency: INTERVAL_DAYS;DAYS=1
BOUNDARY_DAILY startsOn/nextDueOn: 2026-09-04
BOUNDARY_DAILY leadDays/amount: 1 / 99.00 CNY
DUE_TODAY rule: 44444444-4444-4444-8444-444444444444
DUE_TODAY frequency/period: MONTHLY;DAY=4 / 2026-09
DUE_TODAY startsOn/nextDueOn/amount: 2026-09-04 / 2026-09-04 / 99.00 CNY
```

The daily rule proves target dates D0+89 and D0+90 are included while D0+91 is excluded; the separate monthly rule drives the one-card confirmation smoke. The fixture may call an `internal` Kotlin alarm helper with `triggerAtMillis=now+120000` while retaining the event's real `targetEpochDay` metadata as D0+89 or D0+90. No production JNI/QML method accepts a trigger override, and non-debug builds cannot enable accelerated scheduling.

Define `RuntimeDiagnostics.kt` with `markQtActivityStart(context): Long` and `qtActivityStartCount(context): Long`, backed by app-private preferences `dailyaccount_stage3_diagnostics` and enabled only for a debuggable package. In debug builds, `main.cpp` calls `markQtActivityStart` once after `QGuiApplication` construction. The plain Kotlin receiver never references `RuntimeDiagnostics`, writes, or increments it. The smoke records the count before `am kill`, waits for notification, and requires the count to remain unchanged, proving reminder delivery did not cold-start Qt.

- [ ] **Step 4: Implement the reboot/date/time-zone/package-replacement script**

`run_reminder_smoke.sh --serial SERIAL --apk APK --result JSON` performs these exact drills:

```text
1. Clean install and grant POST_NOTIFICATIONS when API >= 33.
2. Import the C++ D0+89/D0+90 plan twice and assert unique native rows; assert D0+91 absent.
3. Accelerate the D0+89 event to two minutes, disable Wi-Fi/data, run am kill, and observe notification with unchanged qtActivityStartCount.
4. Persist but deliberately do not arm D0+90, issue a normal adb reboot, wait for sys.boot_completed=1, and verify BOOT_COMPLETED rebuilds and delivers the accelerated alarm.
5. Change the emulator time zone, verify targetEpochDay is unchanged and trigger instant is recomputed, then restore the zone.
6. Install the same/newer debug APK with -r and verify MY_PACKAGE_REPLACED reconciliation.
7. Deny notification permission on API 35 and verify health/banner; regrant and reconcile.
8. Force-stop once, record delivery as outside guarantee, reopen, and verify immediate reconciliation succeeds.
9. Seed one due occurrence, confirm twice from the home card, and verify one posted transaction, one confirmation outbox row, and period event cancellation.
```

The script restores network, time zone, and permission state in a trap. It writes booleans/timestamps/event-key hashes only; no rule title, note, transaction UUID, device serial, or raw notification text enters tracked JSON.

- [ ] **Step 5: Run focused and cumulative verification on the mandatory reminder matrix**

```bash
bash tests/android/run_reminder_smoke.sh \
  --serial "$DA_API28_SERIAL" \
  --apk build/cmake/android-x86_64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  --result /tmp/opencode/g3-reminder-api28.json
bash tests/android/run_reminder_smoke.sh \
  --serial "$DA_API35_SERIAL" \
  --apk build/cmake/android-x86_64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  --result /tmp/opencode/g3-reminder-api35.json
bash tests/android/run_reminder_smoke.sh \
  --serial "$DA_PHYSICAL_ARM64_SERIAL" \
  --apk build/cmake/android-arm64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  --result /tmp/opencode/g3-reminder-arm64.json
```

Merge those records into `docs/validation/stage-3/android-reminder-results.json` with `result=PASS`, selected D-030 mechanism, three target aliases/fingerprints, `d0Plus89=true`, `d0Plus90=true`, `d0Plus91Excluded=true`, `offlineColdDelivery=true`, `normalRebootRebuild=true`, `timeZoneRebuild=true`, `packageReplaceRebuild=true`, `permissionHealth=true`, `forceStopRecovery=true`, `qtInitializedByReceiver=false`, `singlePostedTransaction=true`, and `singleConfirmationMutation=true`.

Run cumulative shared checks:

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_reminder_planner_tests
./build/cmake/linux-core/dailyaccount_recurring_confirmation_tests
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: all three targets pass the integrated accelerated/reboot matrix and shared boundary tests. This proves Stage 3 wiring against D-030; the complete production recurring edit/defer/skip/cancel/undo/import/sync reminder workflow remains a Stage 5 gate and is not claimed by G3.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add src/apps/android-qml/main.cpp \
  src/apps/android-qml/mobile_composition.h \
  src/apps/android-qml/mobile_composition.cpp \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/RuntimeDiagnostics.kt \
  src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/ReminderInstrumentationTest.kt \
  tests/android/run_reminder_smoke.sh \
  docs/validation/stage-3/android-reminder-results.json
git commit -m "test: verify Android reminder integration"
```

Without authorization, do not commit.

---

### Task 16: Verify APK Contents, Seal G3 Evidence, and Hand Off to Stage 4

**Files:**
- Create: `tests/android/verify_apk.py`
- Create: `tests/cmake/check_g3.py`
- Create: `tests/cmake/test_check_g3.py`
- Create: `docs/validation/stage-3/linux-core.log`
- Create: `docs/validation/stage-3/windows-mobile-tests.log`
- Create: `docs/validation/stage-3/android-build.log`
- Create: `docs/validation/stage-3/android-apk-results.json`
- Create: `docs/validation/stage-3/source-tree.txt`
- Create: `docs/validation/stage-3/g3-evidence-index.md`
- Create: `docs/validation/stage-3/g3-results.json`
- Create outside tracked source: `artifacts/stage-3/DailyAccount-stage3-arm64-v8a-debug.apk`
- Create outside tracked source: `artifacts/stage-3/DailyAccount-stage3-x86_64-debug.apk`
- Modify: `CMakeLists.txt`
- Test: `tests/android/verify_apk.py`
- Test: `tests/cmake/test_check_g3.py`
- Test: `tests/cmake/check_g3.py`

**Interfaces:**
- Consumes: accepted G2/G0/ADR evidence, every Stage 3 test/result, both packaged ABIs, mandatory device matrix, and final source tree.
- Produces: APK content/security manifest evidence, a machine-readable G3 checker, exact `G3 PASS` output, a human evidence index, and immutable inputs for Stage 4 auth/sync work.

- [ ] **Step 1: Write failing G3 checker tests**

Use Python `unittest` with isolated synthetic trees and include:

```python
def test_rejects_failed_g2(self):
    self.write_json("docs/validation/stage-2/g2-results.json", {"gate": "G2", "result": "FAIL"})
    self.assert_failure("G2 result is not PASS")

def test_rejects_numeric_qml_money(self):
    self.write("src/apps/android-qml/qml/Bad.qml", "property real amount: 1.23")
    self.assert_failure("QML monetary value is numeric")

def test_rejects_split_process_death_state(self):
    self.write_json("docs/validation/stage-3/android-process-death-results.json",
                    {"businessRows": 1, "outboxRows": 0})
    self.assert_failure("process-death business/outbox state is split")

def test_rejects_qt_receiver_dependency(self):
    self.write("src/apps/android-qml/android/src/main/java/local/dailyaccount/ReminderReceiver.kt",
               "import org.qtproject.qt.android.QtNative")
    self.assert_failure("native reminder receiver references Qt")

def test_accepts_complete_g3_fixture(self):
    self.assert_success(
        "G3 PASS: Android API 28-35 offline CRUD, exact money, recurring confirmation, reboot reminder")
```

- [ ] **Step 2: Run checker tests red**

```bash
python3 -m unittest tests/cmake/test_check_g3.py -v
```

Expected: import/file-not-found failure for `tests/cmake/check_g3.py`.

- [ ] **Step 3: Implement APK structural verification**

`verify_apk.py --arm64 APK --x86 APK --aapt2 PATH --json PATH` must inspect `aapt2 dump badging/xmltree` and ZIP entries and require:

```text
package local.dailyaccount
minSdkVersion 28
targetSdkVersion 35
debuggable true for this G3 artifact
arm64-v8a only in the ARM APK
x86_64 only in the emulator APK
Qt Core/Gui/Qml/Quick/QuickControls2/Sql libraries
QSQLITE plugin
DailyAccount native library
DailyAccount.Accounting QML resources and every registered page
DailyAccountActivity
ReminderReceiver and ReminderBootReceiver
RECEIVE_BOOT_COMPLETED and POST_NOTIFICATIONS declarations
no broad storage/contact/SMS/accessibility/account permission
no usesCleartextTraffic=true
no DAT backend/importer symbol or ledger.dat resource
no service-role key, JWT, password, endpoint secret, private email, or raw fixture note
```

Require `SCHEDULE_EXACT_ALARM` exactly when D-030 selected the exact fallback and reject it otherwise. Hash each APK with SHA-256 and write only hashes, sizes, package/version/API/ABI, permissions, component names, and pass booleans.

- [ ] **Step 4: Implement the authoritative G3 checker**

`check_g3.py --root DIR --json PATH` must:

- Require accepted/fresh G2 fields and accepted D-020/D-023/D-025/D-028/D-029/D-030 outcomes.
- Require API 28 x86_64, API 35 x86_64, and physical ARM64 install/start/instrumentation results tied to one source tree.
- Require flight-mode create/list/detail/edit/delete, restart persistence, process-kill persistence, stable transaction UUID, tombstone delete, and entity/outbox atomic-state evidence.
- Require decimal `99999999.99` to persist as `9999999999` and return as the same string; run the QML money scanner and reject numeric monetary role/property types.
- Require local profile isolation, no password/network auth UI, no `IAuthClient`/`ISyncTransport` in the Stage 3 composition graph, and no DAT migration/backend link in Android.
- Require `QAbstractListModel` role names for transaction/category/recurring DTOs and no repository/SQL object exposed to QML.
- Require one pending recurring pair, list/home card visibility, duplicate confirmation yielding one existing posted transaction and one confirmation mutation, and `OccurrenceLinked` protection.
- Require C++ event keys/snapshots, D0+89/D0+90 inclusion, D0+91 exclusion, native unique persistence, accepted D-030 alarm API, offline cold delivery, normal reboot/date/time-zone/package-replacement rebuild, permission health, force-stop reopen recovery, and `qtInitializedByReceiver=false`.
- Require Keystore binary round trip/ciphertext/tamper/remove tests and SAF/share intent/no-storage-permission tests; reject auth tokens because Stage 3 stores none.
- Require deterministic shutdown, model detach, queue drain, no connection warning, no stale post-close signal, and Android reopen after process death.
- Require APK structural checks, package ID/API/ABI/permissions, QSQLITE/QML/native classes, no cleartext allowance, no secrets, and hashes for both artifacts.
- Require Linux CTest `100% tests passed`, direct DAT `22 test(s) passed`, Windows mobile/SQLite CTest zero failures, all Android instrumentation tests passing, `git diff --check` clean, and no tracked generated binary/database/WAL/SHM.
- Write JSON only on success with `gate=G3`, `result=PASS`, `minimumApi=28`, `targetApi=35`, `abis=["arm64-v8a","x86_64"]`, `offlineCrud=true`, `restartPersistence=true`, `processDeathAtomic=true`, `decimalStringMoney=true`, `recurringConfirmationIdempotent=true`, `nativeReminderReboot=true`, `d0Plus90Inclusive=true`, `qtReceiverInitialized=false`, and `failureCount=0`.
- Print exactly `G3 PASS: Android API 28-35 offline CRUD, exact money, recurring confirmation, reboot reminder`.

- [ ] **Step 5: Run focused and cumulative verification while building final APKs**

```bash
set -o pipefail
mkdir -p docs/validation/stage-3 artifacts/stage-3
cmake --preset linux-core 2>&1 | tee docs/validation/stage-3/linux-core.log
cmake --build --preset linux-core --parallel 2 2>&1 | tee -a docs/validation/stage-3/linux-core.log
ctest --preset linux-core --output-on-failure 2>&1 | tee -a docs/validation/stage-3/linux-core.log
./build/cmake/linux-core/dailyaccount_backend_tests 2>&1 | tee -a docs/validation/stage-3/linux-core.log
cmake --preset android-x86_64-debug 2>&1 | tee docs/validation/stage-3/android-build.log
cmake --build --preset android-x86_64-debug --target apk --parallel 2 2>&1 | tee -a docs/validation/stage-3/android-build.log
cmake --preset android-arm64-debug 2>&1 | tee -a docs/validation/stage-3/android-build.log
cmake --build --preset android-arm64-debug --target apk --parallel 2 2>&1 | tee -a docs/validation/stage-3/android-build.log
cp build/cmake/android-x86_64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  artifacts/stage-3/DailyAccount-stage3-x86_64-debug.apk
cp build/cmake/android-arm64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  artifacts/stage-3/DailyAccount-stage3-arm64-v8a-debug.apk
python3 tests/android/verify_apk.py \
  --arm64 artifacts/stage-3/DailyAccount-stage3-arm64-v8a-debug.apk \
  --x86 artifacts/stage-3/DailyAccount-stage3-x86_64-debug.apk \
  --aapt2 "$ANDROID_SDK_ROOT/build-tools/35.0.1/aapt2" \
  --json docs/validation/stage-3/android-apk-results.json
```

On the accepted Windows Qt machine:

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop 2>&1 |
  Tee-Object docs\validation\stage-3\windows-mobile-tests.log
cmake --build --preset windows-desktop --parallel 2 2>&1 |
  Tee-Object -Append docs\validation\stage-3\windows-mobile-tests.log
ctest --preset windows-desktop --output-on-failure 2>&1 |
  Tee-Object -Append docs\validation\stage-3\windows-mobile-tests.log
if ($LASTEXITCODE -ne 0) { throw 'Stage 3 Windows mobile adapter tests failed' }
```

Expected: all cumulative suites pass; both APKs are copied and structurally accepted; Windows still packages/runs its SQLite application; all logs correspond to the same source tree.

- [ ] **Step 6: Record the source tree without altering the real Git index**

```bash
STAGE3_INDEX=/tmp/opencode/dailyaccount-stage3-index
rm -f "$STAGE3_INDEX"
GIT_INDEX_FILE="$STAGE3_INDEX" git read-tree HEAD
GIT_INDEX_FILE="$STAGE3_INDEX" git add -A -- \
  CMakeLists.txt CMakePresets.json cmake src tests docs/validation/stage-3
GIT_INDEX_FILE="$STAGE3_INDEX" git diff --cached --check
GIT_INDEX_FILE="$STAGE3_INDEX" git write-tree > docs/validation/stage-3/source-tree.txt
rm -f "$STAGE3_INDEX"
```

Expected: one tree hash is written; the real index is unchanged; APKs and generated trees are absent from the temporary tree.

- [ ] **Step 7: Write the evidence index and run G3 green**

Create `g3-evidence-index.md` with sections `Gate result`, `G2 prerequisite`, `Accepted Android decisions`, `Source tree`, `Build targets and QML routes`, `Local profile shell`, `Decimal-string boundary`, `Offline overview and quick entry`, `Transaction CRUD`, `Restart and process death`, `Executor shutdown`, `Keystore`, `File picker and share`, `Recurring list and confirmation`, `Native reminder and D0 boundary`, `API/ABI matrix`, `APK contents and hashes`, `Inherited Linux/Windows suites`, `Exceptions`, and `Stage 4 inputs`. Every section names command, UTC time, OS/device alias and fingerprint, exact tool versions, exit code, raw JSON/log, and SHA-256. `Exceptions` lists only the D-029 accepted background-sync mode and the D-030 accepted alarm branch; it does not reclassify Stage 5 scope as a G3 exception.

Run:

```bash
python3 -m unittest tests/cmake/test_check_g3.py -v
python3 tests/cmake/check_g3.py \
  --root . \
  --json docs/validation/stage-3/g3-results.json
git diff --check
git status --short
```

Expected stdout:

```text
G3 PASS: Android API 28-35 offline CRUD, exact money, recurring confirmation, reboot reminder
```

Expected JSON contains every exact success field declared in Step 4 and `failureCount=0`. Status contains only declared Stage 3 files plus recorded unrelated work; no APK, AAB, generated Gradle tree, database, WAL/SHM, Keystore file, private input, token, or device serial is tracked.

- [ ] **Step 8: Request independent G3 review**

The reviewer reruns G2, Linux/Windows cumulative suites, QML/JNI/link boundary scanners, checker unit tests, real G3 checker, APK verifier, API boundary emulator instrumentation, physical ARM smoke, flight-mode CRUD, process kill/relaunch, duplicate recurring confirmation, and normal-reboot reminder drill. The reviewer traces `99999999.99` from QML text through C++ parser/SQLite and back, traces one success and one outbox failure, proves receiver classes contain no Qt/business dependency, and confirms every claimed Stage 5 exclusion remains unimplemented rather than partially exposed.

- [ ] **Step 9: Use the optional final checkpoint only with explicit authorization**

After inspecting `git status`, `git diff`, and recent commits, and only when the user explicitly authorizes a commit:

```bash
git add -A -- CMakeLists.txt CMakePresets.json cmake src tests docs/validation/stage-3
git commit -m "feat: complete Android offline vertical slice"
```

Expected: one reviewed checkpoint containing only source/tests/sanitized evidence. Do not add `artifacts/stage-3/*.apk`, generated build trees, databases, logs with user content, credentials, or unrelated files. Without authorization, leave all verified Stage 3 work uncommitted.

---

## G3 Checklist

- [ ] The accepted and fresh G2 checkers pass exactly; D-020, D-023, D-025, D-028, D-029, and D-030 remain accepted and mutually consistent.
- [ ] `dailyaccount_android` builds through CMake for `x86_64` and `arm64-v8a`, registers QML URI `DailyAccount.Accounting`, and packages API 28/API 35 settings without a DAT/desktop dependency.
- [ ] `registerAccountingMobile` atomically publishes overview, quick-entry, transaction-list, transaction-detail, and recurring routes; navigation accepts registered descriptors only.
- [ ] The local profile shell creates/opens isolated schema-2 profiles without password, remote binding, auth claim, provider adapter, or network dependency; `SIGNED_OUT_RETAINED` still requires Stage 4 online authentication.
- [ ] Android creates and owns its own profile/accounting executors; no database opens on the QML thread, owner/schema checks occur before queries, and profile close/application shutdown detach models and drain/remove all connections without warnings.
- [ ] All QML-facing money inputs/properties/signals/model roles are decimal strings; `99999999.99` persists as `9999999999` minor units and returns byte-for-byte as `99999999.99` after restart/process death.
- [ ] `QObject` facades dispatch blocking work off the GUI thread, return one terminal result on the facade thread, drop stale generations, and expose DTO/list models rather than repositories, SQL handles, or mutable domain objects.
- [ ] Flight-mode overview and quick entry work with an optional category and null optional account; local success occurs only after entity plus outbox commit.
- [ ] Flight-mode transaction list/detail/edit/delete pass, preserve stable UUID and server revision, write tombstones, refresh projections, and keep `OccurrenceLinked` protection.
- [ ] Activity restart and `am kill` preserve committed rows/outbox; a kill racing a command yields only atomic `(0,0)` or `(1,1)` business/outbox state.
- [ ] Android Keystore AES/GCM bridge round-trips arbitrary bytes, stores only ciphertext in private preferences, rejects tampering, removes values, logs no content, and stores no auth token in Stage 3.
- [ ] File selection uses `ACTION_OPEN_DOCUMENT` for text, sharing uses `ACTION_SEND`, no broad storage permission exists, content is bounded/raw/unqueued to C++, and Kotlin performs no accounting/import rule.
- [ ] Shared C++ confirmation changes the deterministic pending occurrence and its existing transaction to `POSTED` in one SQLite/outbox transaction; duplicate taps/retries produce exactly one posted transaction and one confirmation mutation.
- [ ] The recurring list shows separate unresolved periods including disabled-rule leftovers; the home card shows all overdue and in-window occurrences and confirms without re-entering existing values.
- [ ] Shared C++ reminder planning includes D0+89 and D0+90, excludes D0+91 before replenishment, uses stable rule/period/offset keys, formats display money in C++, and suppresses resolved periods.
- [ ] Kotlin persists supplied `ReminderEvent` data idempotently, schedules only with D-030's accepted alarm API, cancels a confirmed period, posts without changing accounting state, and exposes permission/scheduling/force-stop health.
- [ ] `ReminderReceiver` and `ReminderBootReceiver` are Kotlin-only, use no Qt/JNI/business/network path, rebuild after normal reboot/date/time-zone/package replacement, and leave `qtActivityStartCount` unchanged during cold delivery.
- [ ] API 28 x86_64, API 35 x86_64, and physical ARM64 pass clean install/start, offline CRUD, restart/process-death, bridge instrumentation, recurring confirmation, and accelerated D0 boundary/reboot reminder smokes.
- [ ] APK verification confirms package/API/ABI/QML/QSQLITE/native classes, least permissions, no cleartext allowance, no DAT migration/backend, and no embedded secret or private content; both hashes are recorded.
- [ ] Linux core/Qt-free tests, Windows SQLite/mobile adapter tests, the 22-case DAT oracle, Android instrumentation, boundary scans, APK verifier, and `check_g3.py` all pass against one source tree.
- [ ] Full recurring rule management, current-period editing, defer/skip/cancel/undo, import matching, cross-device reminder reconciliation, real auth/session storage, and synchronization remain assigned to Stages 4-5 and are not falsely claimed by G3.
- [ ] `check_g3.py` prints exactly `G3 PASS: Android API 28-35 offline CRUD, exact money, recurring confirmation, reboot reminder`; `git diff --check` is silent; independent review accepts the evidence.
- [ ] No unrelated user change, generated APK/AAB/build tree/database/WAL/SHM, Keystore material, private content, credential, commit, or tag was introduced without explicit authorization.

## Stage 4 Handoff

Stage 4 may begin only when every G3 checkbox is checked and `docs/validation/stage-3/g3-results.json` records `gate=G3`, `result=PASS`, and `failureCount=0`. Its executor must read the architecture, master plan, D-020 through D-030, G0/G1/G2/G3 evidence indexes, all public headers under `src/core/`, `src/platform/`, `src/modules/accounting/`, and `src/apps/android-qml/`, and then execute `docs/superpowers/plans/2026-09-04-stage-4-auth-and-sync.md`; if that child plan is absent or contradicts an accepted decision or G3 interface, repair the Stage 4 plan before production changes.

Stage 4 inherits these immutable inputs:

- Android package/toolchain/API/ABI/QML route identities and the installable G3 APK matrix are fixed; auth/sync additions may not regress offline launch or local CRUD.
- `MobileComposition` already owns isolated profile/module executors, model-detach barriers, facades, `AndroidSecureStore`, and the native scheduler. Stage 4 binds a verified `AuthSession.userId` through D-023 rather than replacing these local boundaries.
- The Stage 3 profile shell is explicitly unauthenticated. Stage 4 adds pre-created-account email/password sign-in, refresh/sign-out, immutable `(providerId, remoteUserId)` binding, and secure token use without making token freshness a prerequisite for local CRUD.
- `AndroidSecureStore` is the only Android session-persistence boundary; provider adapters receive bytes through `ISecureStore` and never call Kotlin/Keystore directly.
- Existing entity/outbox writes, stable UUIDs, preserved `serverRevision`, tombstones, recurring atomic groups, and SQLite query models are synchronization inputs. Stage 4 does not create a second online write path or let QML call transport directly.
- D-029's accepted background mode remains bounded exactly as recorded. If it is `FOREGROUND_COMPENSATION`, Stage 4 may add only the Kotlin due marker plus startup/resume synchronization; it may not claim background convergence. If it is `BACKGROUND_QT_ENABLED`, only the proven cold-process composition subset may run.
- Native reminders remain independent of sync and Qt cold startup. Pulling recurring lifecycle changes must reconcile/cancel/replace supplied events, but a cloud outage or auth failure cannot disable already persisted local reminders.
- Stage 4 must preserve the Stage 3 G3 matrix as a cumulative regression suite, including max-value decimal text, flight-mode CRUD, process-death atomicity, duplicate recurring confirmation, normal reboot delivery, and receiver-no-Qt evidence.
- DAT migration remains Windows-only and read-only. No Android auth/bootstrap path links legacy DAT code or accepts a DAT source.
- Full recurring management and reminder lifecycle acceptance still finish in Stage 5; Stage 4 adds remote atomic change-group convergence without expanding that UI scope.
