# DailyAccount Stage 0 Baseline and Blocking Prototypes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Freeze a reproducible DAT application baseline and resolve every G0 product, cloud, synchronization, SQLite, Windows, and Android uncertainty with reviewable decisions and measured evidence before Stage 1 begins.

**Architecture:** Keep all experiments under `prototypes/stage0/` and all generated evidence under `docs/validation/stage-0/`; no prototype is linked into the shipping application. Decisions are recorded as ADRs, and a final machine-readable gate checks that each blocking prototype passed or selected its predeclared fallback without weakening the product contract silently.

**Tech Stack:** C++17, Qt 6.9.3 Core/Widgets/Quick/QML/SQL/Network, qmake for the frozen Windows baseline, isolated CMake 3.22.1+ prototypes, MinGW-w64 13.1, GCC 11.4+, Python 3 standard library, SQLite through QSQLITE, Supabase CLI/PostgreSQL/pgTAP, Kotlin 2.0.21, Android SDK 35, Build Tools 35.0.1, NDK 27.2.12479018, JDK 17, Gradle 8.10, AGP 8.6.0, AndroidX WorkManager 2.10.1, ADB, and SHA-256 evidence manifests.

**Spec:** `docs/product-architecture.md`; parent plan: `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md`

## Global Constraints

- Execute this plan from a clean, isolated worktree whose selected baseline is one committed revision. Never stash, reset, clean, discard, overwrite, or commit unrelated user changes.
- Do not create a Git commit or tag unless the user explicitly authorizes it in the execution session. Every commit step below is optional and authorization-gated.
- Stage 0 may create ADRs, validation records, sanitized fixtures, disposable test-cloud objects, and isolated code under `prototypes/stage0/`; it must not modify `backend/`, `gui/`, `src/`, `platform/`, `modules/`, the root build graph, or any shipping package source.
- Keep the frozen application at C++17, Qt 6.9.3 Widgets, qmake, and MinGW. A different Qt patch, compiler family, or Windows architecture is not equivalent baseline evidence.
- Keep all monetary values as signed 64-bit integer minor units. Persist and transport no financial amount as `double`, JavaScript `Number`, or QML `int`.
- Preserve `ledger.dat`, `records.dat`, and `categories.dat` as read-only private inputs. Never edit private originals, add them to Git, upload them to a cloud service, or include them in logs.
- Accept only fixtures that pass offline sanitization, structural validation, a private-term denylist scan, and a SHA-256 manifest. Complete raw import text remains local-only.
- Use client-generated UUIDs for synchronizable entities and mutation IDs. `serverRevision` is assigned only by the server; local dirty and in-flight state remains separate.
- Treat each user/module stream independently. A mutation is one atomic command, a pull change group is never split or partly applied, and cursors follow transaction commit order rather than sequence allocation or wall-clock time.
- Use valid TLS certificates only. No prototype or release path may ignore certificate errors, log credentials/JWTs, or embed a Supabase secret/service-role key.
- Use one serial QSQLITE executor per profile/module, unique per-thread connection names, `PRAGMA foreign_keys = ON`, `QSQLITE_BUSY_TIMEOUT=5000`, bounded transactions, and no network wait inside a database transaction.
- Android V1 supports API 28 through API 35. Release evidence requires `arm64-v8a`; emulator evidence also requires `x86_64` at API 28 and API 35.
- WorkManager may fall back to a foreground compensation trigger if cold-process Qt initialization fails. The native 90-day reminder path may not depend on WorkManager, network, cloud push, or cold-process Qt initialization.
- A failed mandatory prototype stops its dependent stage exactly as stated in this plan. Do not infer an untested substitute or lower an acceptance threshold without a reviewed spec and master-plan amendment.
- Every validation document records the command, UTC time, operating system/device, exact tool versions, selected revision, exit code, measured result, artifact paths, and SHA-256 hashes. A prose claim without its raw log or JSON result is not gate evidence.

---

## Deliverables and File Map

The following tracked paths are the complete Stage 0 change surface. Build directories, Android emulator images, private source data, cloud credentials, access tokens, and disposable Supabase project state remain outside Git.

```text
docs/
  product-architecture.md                         # Updated only in Task 14.
  superpowers/plans/
    2026-09-04-dailyaccount-v1-master.md          # Updated only in Task 14.
  decisions/
    D-020-stage-0-baseline-and-toolchains.md
    D-021-cloud-provider-and-conformance.md
    D-022-sync-protocol-v1.md
    D-023-login-and-local-profiles.md
    D-024-currency-and-account-formulas.md
    D-025-recurring-edge-semantics.md
    D-026-conflicts-retention-and-account-deletion.md
    D-027-text-import-contract-and-limits.md
    D-028-qsqlite-threading-and-backup.md
    D-029-android-background-sync.md
    D-030-android-reminder-delivery.md
  validation/
    stage-0/
      baseline.md
      baseline-revision.txt
      baseline-tree.txt
      baseline-source.sha256
      baseline-strict.log
      baseline-sanitizers.log
      windows-build-package.md
      windows-environment.txt
      windows-build.log
      windows-smoke.log
      windows-package-files.txt
      windows-package.sha256
      dat-fixtures.md
      dat-fixtures.sha256
      import-fixtures.md
      import-fixtures.sha256
      sync-model.md
      sync-model.log
      sync-model-results.json
      qsqlite-thread-backup.md
      qsqlite-thread-backup.log
      qsqlite-thread-backup-results.json
      android-toolchain-runtime.md
      android-toolchain.txt
      android-build.log
      android-runtime-results.json
      android-workmanager.md
      android-workmanager-results.json
      android-reminder.md
      android-reminder-results.json
      cloud-supabase-conformance.md
      cloud-local-tests.log
      cloud-conformance-results.json
      decision-consistency.md
      g0-evidence-index.md
      g0-results.json
tests/
  fixtures/
    dat/
      v3-sanitized/ledger.dat
      v3-sanitized/expected.json
      legacy-sanitized/records.dat
      legacy-sanitized/categories.dat
      legacy-sanitized/expected.json
      v3-corrupt-checksum/ledger.dat
      v3-corrupt-checksum/expected-error.txt
    import/
      sample-01-one-line.txt
      sample-01-one-line.expected.json
      sample-02-date-headings.txt
      sample-02-date-headings.expected.json
      sample-03-missing-year.txt
      sample-03-missing-year.expected.json
      sample-04-errors-and-duplicates.txt
      sample-04-errors-and-duplicates.expected.json
      sample-05-recurring-match.txt
      sample-05-recurring-match.expected.json
prototypes/
  stage0/
    dat_fixtures/
      sanitize_dat.py
      validate_dat_fixtures.py
      fixture_probe.cpp
    import_fixtures/
      sanitize_text_samples.py
      validate_text_fixtures.py
    sync_model/
      sync_model.py
      test_sync_model.py
      run_model.py
    qsqlite/
      CMakeLists.txt
      serial_executor.h
      serial_executor.cpp
      qsqlite_probe.cpp
      online_backup_probe.cpp                     # Created only if the declared fallback is selected.
      vendor/sqlite3.c                            # Created only if the declared fallback is selected.
      vendor/sqlite3.h                            # Created only if the declared fallback is selected.
      vendor/README.md                            # Created only if the declared fallback is selected.
    android/
      CMakeLists.txt
      main.cpp
      probe_facade.h
      probe_facade.cpp
      cloud_probe.h
      cloud_probe.cpp
      online_backup_probe.cpp                     # Created only if the declared fallback is selected.
      qml/Main.qml
      android/AndroidManifest.xml
      android/build.gradle
      android/src/main/java/local/dailyaccount/stage0/ProbeWorker.kt
      android/src/main/java/local/dailyaccount/stage0/ProbeStatusStore.kt
      android/src/main/java/local/dailyaccount/stage0/ReminderContract.kt
      android/src/main/java/local/dailyaccount/stage0/ReminderStore.kt
      android/src/main/java/local/dailyaccount/stage0/ReminderScheduler.kt
      android/src/main/java/local/dailyaccount/stage0/ReminderReceiver.kt
      android/src/main/java/local/dailyaccount/stage0/ReminderSystemReceiver.kt
      android/src/androidTest/java/local/dailyaccount/stage0/ReminderSchedulerTest.kt
      scripts/build_android.sh
      scripts/run_runtime_matrix.sh
      scripts/run_reminder_matrix.sh
    cloud/
      supabase/config.toml
      supabase/migrations/202609040001_g0_conformance.sql
      supabase/tests/database/g0_conformance.test.sql
      supabase/seed.sql
      qt_probe/qt_supabase_probe.pro
      qt_probe/main.cpp
      verify_cloud_results.py
    check_g0.py
artifacts/
  stage-0/
    DailyAccount-g0-windows.zip
```

`artifacts/stage-0/DailyAccount-g0-windows.zip` is a reproducibility artifact and may remain outside Git if repository policy excludes binaries; its hash and storage location must still be recorded in `windows-build-package.md`. All prototype executables, APKs, object files, generated Gradle projects, test databases, and cloud credentials stay in ignored build or operating-system temporary directories.

### Evidence Status Vocabulary

Every ADR and validation record uses exactly one terminal result:

- `PASS`: the declared contract and every mandatory assertion passed.
- `PASS_WITH_FALLBACK`: the primary experiment failed, the fallback named in this plan passed, and the ADR selects that fallback.
- `FAIL_STOP`: a mandatory assertion failed and all dependent work is blocked.

Only `PASS` and the specifically allowed `PASS_WITH_FALLBACK` outcomes can contribute to G0. Cloud provider selection, the Windows baseline, QML/QSQLITE/HTTPS runtime, and native reminder delivery require `PASS`; Android cold-process background sync and the QSQLITE backup implementation may use their declared fallback.

---

### Task 1: Freeze the Committed Baseline and Reproduce All 22 Backend Tests

**Files:**
- Create: `docs/decisions/D-020-stage-0-baseline-and-toolchains.md`
- Create: `docs/validation/stage-0/baseline.md`
- Create: `docs/validation/stage-0/baseline-revision.txt`
- Create: `docs/validation/stage-0/baseline-tree.txt`
- Create: `docs/validation/stage-0/baseline-source.sha256`
- Create: `docs/validation/stage-0/baseline-strict.log`
- Create: `docs/validation/stage-0/baseline-sanitizers.log`

**Interfaces:**
- Consumes: the repaired DAT application described by `docs/superpowers/plans/2026-09-03-full-repair.md` and the verified-starting-point section of the master plan.
- Produces: one immutable Git commit hash, a complete tree inventory, strict and sanitizer logs ending in `22 test(s) passed`, and the authoritative desktop/toolchain decision consumed by every later task.

- [ ] **Step 1: Prove the application baseline candidate has no uncommitted source changes**

Run from the repository root:

```bash
ROOT="$(git rev-parse --show-toplevel)"
test "$ROOT" = "$PWD"
BASELINE="$(git rev-parse --verify 'HEAD^{commit}')"
BASELINE_PATHS=(backend gui tests/backend_tests.cpp tests/backend_tests.pro jizhang.pro build/build.bat README.md)
test -z "$(git diff --name-only -- "${BASELINE_PATHS[@]}")"
test -z "$(git diff --cached --name-only -- "${BASELINE_PATHS[@]}")"
test -z "$(git ls-files --others --exclude-standard -- "${BASELINE_PATHS[@]}")"
printf '%s\n' "$BASELINE"
```

Expected: all `test` commands exit `0`; the final command prints one 40-character commit hash. Uncommitted Stage 0 documentation is allowed, but any changed or untracked application/test/build path is `FAIL_STOP`; preserve it and ask the owner to select a committed application baseline. Do not stash, reset, clean, or create a commit automatically.

- [ ] **Step 2: Materialize a detached read-only build worktree at that revision**

Run:

```bash
ROOT="$(git rev-parse --show-toplevel)"
BASELINE="$(git rev-parse --verify 'HEAD^{commit}')"
BASELINE_WORKTREE="/tmp/opencode/dailyaccount-g0-baseline-$BASELINE"
test -d /tmp/opencode
if test -e "$BASELINE_WORKTREE"; then
  test "$(git -C "$BASELINE_WORKTREE" rev-parse HEAD)" = "$BASELINE"
  test -z "$(git -C "$BASELINE_WORKTREE" status --porcelain=v1 --untracked-files=all)"
else
  git worktree add --detach "$BASELINE_WORKTREE" "$BASELINE"
fi
```

Expected: the detached worktree reports the selected hash and an empty status. Existing temporary content with a different hash or any change is `FAIL_STOP`; do not remove it forcibly.

- [ ] **Step 3: Capture the revision and full tracked-tree identity**

Run:

```bash
ROOT="$(git rev-parse --show-toplevel)"
BASELINE="$(git rev-parse --verify 'HEAD^{commit}')"
BASELINE_WORKTREE="/tmp/opencode/dailyaccount-g0-baseline-$BASELINE"
mkdir -p docs/decisions docs/validation/stage-0 artifacts/stage-0
printf '%s\n' "$BASELINE" > docs/validation/stage-0/baseline-revision.txt
git -C "$BASELINE_WORKTREE" ls-tree -r --full-tree HEAD > docs/validation/stage-0/baseline-tree.txt
sha256sum "$BASELINE_WORKTREE"/backend/record.h \
  "$BASELINE_WORKTREE"/backend/category.h "$BASELINE_WORKTREE"/backend/category.cpp \
  "$BASELINE_WORKTREE"/backend/storage.h "$BASELINE_WORKTREE"/backend/storage.cpp \
  "$BASELINE_WORKTREE"/backend/ledger.h "$BASELINE_WORKTREE"/backend/ledger.cpp \
  "$BASELINE_WORKTREE"/gui/main_gui.cpp "$BASELINE_WORKTREE"/gui/mainwindow.h \
  "$BASELINE_WORKTREE"/gui/mainwindow.cpp "$BASELINE_WORKTREE"/tests/backend_tests.cpp \
  "$BASELINE_WORKTREE"/tests/backend_tests.pro "$BASELINE_WORKTREE"/jizhang.pro \
  "$BASELINE_WORKTREE"/build/build.bat "$BASELINE_WORKTREE"/README.md \
  > docs/validation/stage-0/baseline-source.sha256
```

Expected: `baseline-revision.txt` has one line, `baseline-tree.txt` contains every tracked path, and `baseline-source.sha256` contains 15 lines with no missing-file error.

- [ ] **Step 4: Reconcile the frozen tree with the master plan inventory**

Compare `baseline-tree.txt` with `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md` lines 48-69. Record every already-present CMake, registry, SQLite, Android, or synchronization path in `baseline.md`; do not remove it. If the committed tree no longer matches the master's verified starting point, mark the evidence `FAIL_STOP` until Task 14 updates that inventory to the actual frozen revision and confirms that no Stage 1 artifact is being credited as G0 evidence without its own test.

- [ ] **Step 5: Compile and run the strict baseline suite**

Run:

```bash
set -o pipefail
ROOT="$(git rev-parse --show-toplevel)"
BASELINE="$(cat docs/validation/stage-0/baseline-revision.txt)"
BASELINE_WORKTREE="/tmp/opencode/dailyaccount-g0-baseline-$BASELINE"
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror \
  -I"$BASELINE_WORKTREE/backend" \
  "$BASELINE_WORKTREE/tests/backend_tests.cpp" \
  "$BASELINE_WORKTREE/backend/category.cpp" \
  "$BASELINE_WORKTREE/backend/storage.cpp" \
  "$BASELINE_WORKTREE/backend/ledger.cpp" \
  -o /tmp/opencode/dailyaccount_g0_backend_tests \
  2>&1 | tee "$ROOT/docs/validation/stage-0/baseline-strict.log"
/tmp/opencode/dailyaccount_g0_backend_tests \
  2>&1 | tee -a "$ROOT/docs/validation/stage-0/baseline-strict.log"
```

Expected: compile exit code `0`, test exit code `0`, and the last test summary is exactly `22 test(s) passed`.

- [ ] **Step 6: Repeat the suite under AddressSanitizer and UndefinedBehaviorSanitizer**

Run:

```bash
set -o pipefail
ROOT="$(git rev-parse --show-toplevel)"
BASELINE="$(cat docs/validation/stage-0/baseline-revision.txt)"
BASELINE_WORKTREE="/tmp/opencode/dailyaccount-g0-baseline-$BASELINE"
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror \
  -fsanitize=address,undefined -fno-omit-frame-pointer \
  -I"$BASELINE_WORKTREE/backend" \
  "$BASELINE_WORKTREE/tests/backend_tests.cpp" \
  "$BASELINE_WORKTREE/backend/category.cpp" \
  "$BASELINE_WORKTREE/backend/storage.cpp" \
  "$BASELINE_WORKTREE/backend/ledger.cpp" \
  -o /tmp/opencode/dailyaccount_g0_backend_tests_sanitized \
  2>&1 | tee "$ROOT/docs/validation/stage-0/baseline-sanitizers.log"
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  /tmp/opencode/dailyaccount_g0_backend_tests_sanitized \
  2>&1 | tee -a "$ROOT/docs/validation/stage-0/baseline-sanitizers.log"
```

Expected: exit code `0`, exactly `22 test(s) passed`, no `AddressSanitizer` error, no runtime-error report, and no leak summary. A sanitizer failure is `FAIL_STOP`; do not establish a weaker non-sanitized baseline.

- [ ] **Step 7: Record the baseline and selected toolchain decision**

Create `D-020-stage-0-baseline-and-toolchains.md` with `Status: Accepted`, the exact revision, C++17, Qt 6.9.3, Windows x86_64 MinGW-w64 13.1, Linux GCC 11.4+, CMake floor 3.22.1, and the Android versions stated in this plan's Tech Stack. Create `baseline.md` with sections `Result`, `Revision`, `Tree reconciliation`, `Linux environment`, `Strict test`, `Sanitizer test`, `Evidence hashes`, and `Stop conditions`; set `Result` to `PASS` only after both logs pass.

- [ ] **Step 8: Hash the baseline evidence**

Run:

```bash
sha256sum docs/validation/stage-0/baseline-revision.txt \
  docs/validation/stage-0/baseline-tree.txt \
  docs/validation/stage-0/baseline-source.sha256 \
  docs/validation/stage-0/baseline-strict.log \
  docs/validation/stage-0/baseline-sanitizers.log \
  >> docs/validation/stage-0/baseline.md
```

Expected: five parseable SHA-256 lines are appended and `baseline.md` names the same commit as `baseline-revision.txt`.

- [ ] **Step 9: Use the optional baseline checkpoint only with explicit authorization**

If and only if the user authorizes a commit in the execution session, run:

```bash
git add docs/decisions/D-020-stage-0-baseline-and-toolchains.md \
  docs/validation/stage-0/baseline.md \
  docs/validation/stage-0/baseline-revision.txt \
  docs/validation/stage-0/baseline-tree.txt \
  docs/validation/stage-0/baseline-source.sha256 \
  docs/validation/stage-0/baseline-strict.log \
  docs/validation/stage-0/baseline-sanitizers.log
git commit -m "docs: freeze stage 0 baseline evidence"
```

Expected: one commit containing only the listed files. Without authorization, leave the verified files uncommitted.

---

### Task 2: Build, Package, and Smoke-Test the Frozen Windows Qt Application

**Files:**
- Create: `docs/validation/stage-0/windows-build-package.md`
- Create: `docs/validation/stage-0/windows-environment.txt`
- Create: `docs/validation/stage-0/windows-build.log`
- Create: `docs/validation/stage-0/windows-smoke.log`
- Create: `docs/validation/stage-0/windows-package-files.txt`
- Create: `docs/validation/stage-0/windows-package.sha256`
- Create: `artifacts/stage-0/DailyAccount-g0-windows.zip`

**Interfaces:**
- Consumes: the exact hash from `baseline-revision.txt`, `build/build.bat`, Qt 6.9.3 `mingw_64`, and MinGW-w64 13.1.
- Produces: a clean-device-style package manifest, archive hash, and process-start evidence for that same revision.

- [ ] **Step 1: Open a Windows checkout at the exact frozen revision**

Run in one PowerShell session from the Windows coordination worktree. Build only from a detached temporary worktree:

```powershell
$RepoRoot = (git rev-parse --show-toplevel).Trim()
$EvidenceRoot = Join-Path $RepoRoot 'docs\validation\stage-0'
$ArtifactRoot = Join-Path $RepoRoot 'artifacts\stage-0'
$Baseline = (Get-Content (Join-Path $EvidenceRoot 'baseline-revision.txt') -Raw).Trim()
$SourceRoot = Join-Path $env:TEMP "DailyAccount-G0-Windows-$Baseline"
if (Test-Path $SourceRoot) {
  if ((git -C $SourceRoot rev-parse HEAD).Trim() -ne $Baseline) { throw "Existing detached worktree has a different revision" }
  if (git -C $SourceRoot status --porcelain=v1 --untracked-files=all) { throw "Existing detached worktree is not clean" }
} else {
  git worktree add --detach $SourceRoot $Baseline
  if ($LASTEXITCODE -ne 0) { throw "Could not create detached Windows baseline worktree" }
}
Set-Location $SourceRoot
```

Expected: `$SourceRoot` is at the exact hash with empty status. A different or dirty source tree is `FAIL_STOP`; do not force-remove it.

- [ ] **Step 2: Verify and capture the exact Windows compiler and Qt kit**

Run:

```powershell
$env:QT_DIR = 'D:\tools\Qt\6.9.3\mingw_64'
$env:MINGW_DIR = 'D:\tools\mingw64\bin'
$qtVersion = & "$env:QT_DIR\bin\qmake.exe" -query QT_VERSION
$compilerVersion = & "$env:MINGW_DIR\g++.exe" -dumpfullversion
if ($qtVersion.Trim() -ne '6.9.3') { throw "Qt must be 6.9.3" }
if ($compilerVersion.Trim() -ne '13.1.0') { throw "MinGW-w64 GCC must be 13.1.0" }
@(
  "baseline=$Baseline"
  "qt=$($qtVersion.Trim())"
  "qmake=$env:QT_DIR\bin\qmake.exe"
  "compiler=$($compilerVersion.Trim())"
  "mingw=$env:MINGW_DIR"
  "os=$([System.Environment]::OSVersion.VersionString)"
) | Set-Content -Encoding utf8 (Join-Path $EvidenceRoot 'windows-environment.txt')
```

Expected: all path checks succeed and the evidence records Qt `6.9.3` and compiler `13.1.0`. If these exact tools are absent, install that kit and rerun; do not substitute MSVC or another Qt patch for baseline evidence.

- [ ] **Step 3: Run the repository Windows build and packaging script**

Run:

```powershell
cmd /d /c "build\build.bat > `"$EvidenceRoot\windows-build.log`" 2>&1"
if ($LASTEXITCODE -ne 0) { throw "build.bat failed" }
$buildLog = Get-Content (Join-Path $EvidenceRoot 'windows-build.log') -Raw
if ($buildLog -notmatch '22 test\(s\) passed') { throw "Windows backend suite did not report 22 passing tests" }
if ($buildLog -notmatch 'Build and tests succeeded') { throw "Windows package completion marker is absent" }
```

Expected: `build\dist\DailyAccount.exe` exists, the Windows backend suite reports exactly 22 passing tests, and `windeployqt` exits successfully.

- [ ] **Step 4: Verify mandatory deployed files**

Run:

```powershell
$required = @(
  'build\dist\DailyAccount.exe',
  'build\dist\Qt6Core.dll',
  'build\dist\Qt6Gui.dll',
  'build\dist\Qt6Widgets.dll',
  'build\dist\platforms\qwindows.dll'
)
$missing = $required | Where-Object { -not (Test-Path $_ -PathType Leaf) }
if ($missing) { throw "Missing deployed files: $($missing -join ', ')" }
Get-ChildItem build\dist -File -Recurse |
  Sort-Object FullName |
  ForEach-Object { $_.FullName.Substring((Resolve-Path build\dist).Path.Length + 1) } |
  Set-Content -Encoding utf8 (Join-Path $EvidenceRoot 'windows-package-files.txt')
```

Expected: no missing files and a non-empty recursive package manifest.

- [ ] **Step 5: Start the packaged executable against an isolated data directory**

Run:

```powershell
$smokeRoot = Join-Path $env:TEMP 'DailyAccount-G0-Smoke'
Remove-Item $smokeRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item $smokeRoot -ItemType Directory | Out-Null
$env:APPDATA = Join-Path $smokeRoot 'AppData'
$env:LOCALAPPDATA = Join-Path $smokeRoot 'LocalAppData'
$process = Start-Process (Resolve-Path 'build\dist\DailyAccount.exe') -PassThru
Start-Sleep -Seconds 8
if ($process.HasExited) { throw "Packaged application exited during the 8-second smoke window with code $($process.ExitCode)" }
"PASS process_id=$($process.Id) alive_after_seconds=8 data_root=$smokeRoot" |
  Set-Content -Encoding utf8 (Join-Path $EvidenceRoot 'windows-smoke.log')
Stop-Process -Id $process.Id
Remove-Item $smokeRoot -Recurse -Force
```

Expected: the GUI process remains alive for eight seconds, no real `%APPDATA%` ledger is opened, and the smoke log begins with `PASS`.

- [ ] **Step 6: Archive and hash the Windows baseline package**

Run:

```powershell
New-Item $ArtifactRoot -ItemType Directory -Force | Out-Null
$Archive = Join-Path $ArtifactRoot 'DailyAccount-g0-windows.zip'
Remove-Item $Archive -Force -ErrorAction SilentlyContinue
Compress-Archive -Path build\dist\* -DestinationPath $Archive -CompressionLevel Optimal
Get-FileHash $Archive -Algorithm SHA256 |
  ForEach-Object { "$($_.Hash.ToLower())  artifacts/stage-0/DailyAccount-g0-windows.zip" } |
  Set-Content -Encoding ascii (Join-Path $EvidenceRoot 'windows-package.sha256')
```

Expected: one ZIP file and one 64-hex-character SHA-256 line. Preserve the archive at the location recorded in the validation document even if binaries are not tracked.

- [ ] **Step 7: Write the Windows validation record**

Return to the coordination worktree with `Set-Location $RepoRoot`. Create `windows-build-package.md` with `Result: PASS`, the frozen revision, machine/OS, exact kit paths and versions, build command, `22 test(s) passed`, package-file assertions, smoke duration, archive location, and archive hash. A build-only result without the isolated startup smoke is `FAIL_STOP`.

- [ ] **Step 8: Use the optional Windows evidence checkpoint only with explicit authorization**

If and only if authorized, stage the six `docs/validation/stage-0/windows-*` files. Add the ZIP only if repository binary policy explicitly permits it, then run:

```bash
git commit -m "docs: record Windows baseline package evidence"
```

Expected: no build directory or private application data is staged. Without authorization, do not commit.

---

### Task 3: Produce Sanitized V3 and Legacy DAT Migration Fixtures

**Files:**
- Create: `prototypes/stage0/dat_fixtures/sanitize_dat.py`
- Create: `prototypes/stage0/dat_fixtures/validate_dat_fixtures.py`
- Create: `prototypes/stage0/dat_fixtures/fixture_probe.cpp`
- Create: `tests/fixtures/dat/v3-sanitized/ledger.dat`
- Create: `tests/fixtures/dat/v3-sanitized/expected.json`
- Create: `tests/fixtures/dat/legacy-sanitized/records.dat`
- Create: `tests/fixtures/dat/legacy-sanitized/categories.dat`
- Create: `tests/fixtures/dat/legacy-sanitized/expected.json`
- Create: `tests/fixtures/dat/v3-corrupt-checksum/ledger.dat`
- Create: `tests/fixtures/dat/v3-corrupt-checksum/expected-error.txt`
- Create: `docs/validation/stage-0/dat-fixtures.md`
- Create: `docs/validation/stage-0/dat-fixtures.sha256`

**Interfaces:**
- Consumes: private local DAT copies through `DA_PRIVATE_V3_DIR`, optional private legacy copies through `DA_PRIVATE_LEGACY_DIR`, and a non-repository denylist through `DA_PRIVATE_TERMS_FILE`.
- Produces: deterministic privacy-safe fixtures plus expected record count, income total, expense total, date range, category set, format, and source-kind metadata. It never emits a hash of a private source file into the repository.

- [ ] **Step 1: Implement a strictly offline DAT sanitizer**

Implement `sanitize_dat.py` with this CLI:

```text
sanitize_dat.py --v3-source-dir DIR --legacy-source-dir DIR --private-terms FILE --output-root DIR
```

The script must expose three exact modes: V3 sanitization with `--v3-source-dir`, optional legacy sanitization with `--legacy-source-dir`, synthetic legacy generation with `--generate-legacy-edge-fixture`, and checksum corruption with `--make-checksum-corruption`. Every mode requires `--output-root`; private-input modes also require `--private-terms`. The sanitizer must parse `#DAILYACCOUNT_V3`, `NEXT_ID`, `CATEGORY`, `RECORD`, and `END` rows; percent-decode and re-encode text; validate and recompute the FNV-1a 64-bit checksum used by `backend/storage.cpp`; and parse legacy paired rows without rewriting either input. Apply these deterministic transformations:

- Preserve record IDs, record types, duplicate relationships, empty subcategories, and `nextId` ordering.
- Map each distinct non-preset category consistently to `Custom-Income-001` or `Custom-Expense-001` style names; map subcategories to `Subcategory-001` style names.
- Map each distinct note consistently to `Note-001`, each merchant-like token to `Merchant-001`, and retain delimiter/newline escaping behavior.
- Replace each distinct positive amount by `10000 + 137 * rank` minor units, where ranks start at `1` in sorted original-amount order; equal original values remain equal and every replacement stays below `9,999,999,999`.
- Shift each date by 28 calendar years when the result remains at or below year 9999, otherwise shift it back 28 years. This preserves leap-year and weekday relationships without preserving the real year.
- Write only sanitized values and sanitized summary totals. Keep all processing local and use Python's standard library only.

Expected: malformed input, checksum mismatch, an incomplete legacy pair, an empty denylist, or a path below the repository root is rejected before output is written.

- [ ] **Step 2: Implement independent fixture validation and privacy checks**

Implement `validate_dat_fixtures.py` to reject invalid UTF-8, emails, URL credentials, Chinese mainland mobile-number patterns, bank-card-like digit runs of 12-19 digits, absolute home paths, any case-insensitive term from `DA_PRIVATE_TERMS_FILE`, duplicate IDs, invalid dates, non-positive/out-of-range amounts, inconsistent category references, wrong row counts, or a wrong V3 checksum. It must compare each valid fixture to its `expected.json` and print one `PASS` line per fixture.

- [ ] **Step 3: Implement a current-parser fixture probe**

Implement `fixture_probe.cpp` as a Qt-free CLI that accepts one fixture directory, loads it through `StorageManager`, and prints a single JSON object containing `format`, `recordCount`, `incomeMinor`, `expenseMinor`, `minimumDate`, `maximumDate`, and sorted categories. Exit `0` only on a valid load; exit non-zero and print `StorageManager::lastError()` for invalid data.

- [ ] **Step 4: Generate a sanitized V3 fixture from a private local copy**

Run only on the trusted machine holding the private data:

```bash
: "${DA_PRIVATE_V3_DIR:?DA_PRIVATE_V3_DIR must name the private V3 directory}"
: "${DA_PRIVATE_TERMS_FILE:?DA_PRIVATE_TERMS_FILE must name the private denylist}"
python3 prototypes/stage0/dat_fixtures/sanitize_dat.py \
  --v3-source-dir "$DA_PRIVATE_V3_DIR" \
  --private-terms "$DA_PRIVATE_TERMS_FILE" \
  --output-root tests/fixtures/dat
```

Expected: `v3-sanitized/ledger.dat` and `expected.json` are created without modifying the source. If no valid private V3 source is available, mark `FAIL_STOP`; Stage 2 DAT migration may not rely only on synthetic data.

- [ ] **Step 5: Generate or construct the legacy paired fixture**

If `DA_PRIVATE_LEGACY_DIR` contains a valid paired `records.dat` and `categories.dat`, rerun the sanitizer with `--legacy-source-dir "$DA_PRIVATE_LEGACY_DIR"`. If no private legacy pair exists, use `sanitize_dat.py --generate-legacy-edge-fixture` to create a clearly marked synthetic pair containing a category with parentheses, a category containing a pipe-delimited note tail, income and expense rows, duplicate amount values, a leap-day date, and a missing-catalog category recovered from a record. `legacy-sanitized/expected.json` must set `sourceKind` to either `sanitized-private` or `synthetic-edge`; both routes preserve the existing parser's semantics.

Expected: both legacy files load together, neither loads as a valid standalone source, and no private file is copied.

- [ ] **Step 6: Create a deterministic checksum-corruption fixture**

Run:

```bash
python3 prototypes/stage0/dat_fixtures/sanitize_dat.py \
  --make-checksum-corruption tests/fixtures/dat/v3-sanitized/ledger.dat \
  --output-root tests/fixtures/dat
```

Expected: the corrupt fixture changes one encoded note byte without changing the `END` checksum and `expected-error.txt` contains exactly `checksum mismatch`.

- [ ] **Step 7: Validate fixtures with both validators**

Run:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Ibackend \
  prototypes/stage0/dat_fixtures/fixture_probe.cpp \
  backend/category.cpp backend/storage.cpp backend/ledger.cpp \
  -o /tmp/opencode/dailyaccount_dat_fixture_probe
python3 prototypes/stage0/dat_fixtures/validate_dat_fixtures.py \
  --root tests/fixtures/dat --private-terms "$DA_PRIVATE_TERMS_FILE"
/tmp/opencode/dailyaccount_dat_fixture_probe tests/fixtures/dat/v3-sanitized
/tmp/opencode/dailyaccount_dat_fixture_probe tests/fixtures/dat/legacy-sanitized
if /tmp/opencode/dailyaccount_dat_fixture_probe tests/fixtures/dat/v3-corrupt-checksum; then exit 1; fi
```

Expected: two validator `PASS` results, two valid JSON summaries matching `expected.json`, and the corrupt fixture fails specifically with `checksum mismatch`.

- [ ] **Step 8: Record provenance without private identifiers and hash tracked fixtures**

Create `dat-fixtures.md` with `Result: PASS`, sanitizer version, source kind, transformation rules, fixture coverage, validation commands, expected summaries, and an explicit statement that private paths, private hashes, and original values were not retained. Then run:

```bash
sha256sum tests/fixtures/dat/v3-sanitized/ledger.dat \
  tests/fixtures/dat/v3-sanitized/expected.json \
  tests/fixtures/dat/legacy-sanitized/records.dat \
  tests/fixtures/dat/legacy-sanitized/categories.dat \
  tests/fixtures/dat/legacy-sanitized/expected.json \
  tests/fixtures/dat/v3-corrupt-checksum/ledger.dat \
  tests/fixtures/dat/v3-corrupt-checksum/expected-error.txt \
  > docs/validation/stage-0/dat-fixtures.sha256
```

Expected: seven hashes and no private source metadata.

- [ ] **Step 9: Use the optional fixture checkpoint only with explicit authorization**

If and only if authorized, stage the three `prototypes/stage0/dat_fixtures/` files, the three fixture directories, and the two DAT validation files, then run:

```bash
git commit -m "test: add sanitized DAT migration fixtures"
```

Expected: private sources and the denylist remain untracked and unstaged. Without authorization, do not commit.

---

### Task 4: Decide V1 Login Identity, Local Profiles, and Account Switching

**Files:**
- Create: `docs/decisions/D-023-login-and-local-profiles.md`

**Interfaces:**
- Consumes: `IAuthClient`, `AuthSession`, per-user directories, bootstrap rules, and the local threat boundary from the master plan and architecture sections 6.4, 14.9, and 15.
- Produces: a closed V1 identity/profile state machine for Stage 1 interfaces and Stage 4 authentication.

- [ ] **Step 1: Record the selected login identifier and provisioning flow**

Write `D-023-login-and-local-profiles.md` with `Status: Accepted` and these selected outcomes:

- V1 login is case-insensitive email plus password through `IAuthClient::signIn`; no username mapping, public sign-up, email-verification UI, password-reset UI, or family administrator role is exposed in the client.
- The deployment maintainer pre-creates and confirms 2-3 accounts in the selected provider. Management credentials never enter a client build.
- The UI stores only a user-chosen display label and a masked email for profile selection. Passwords are never persisted; access and refresh tokens use the platform secure store.
- First login and every login after explicit sign-out require network access. An expired access token pauses sync but does not close an already-open local profile.

- [ ] **Step 2: Define immutable local and remote identity bindings**

Record that `ProfileId` is a random local UUID and each profile directory stores immutable `ownerProfileId`; successful first login binds exactly one `(providerId, remoteUserId)` to that profile. The same remote user may not bind to two local profile directories on one installation, an existing profile may not be rebound to another remote user, and unowned DAT data is never bound without an explicit profile selection and migration confirmation.

- [ ] **Step 3: Define the profile lifecycle and switch barrier**

Use exactly these states: `LOCAL_UNBOUND`, `INITIALIZING`, `ACTIVE`, `SIGNED_OUT_RETAINED`, `RECOVERY_READ_ONLY`, and `LOCAL_DELETE_PENDING`. Only `ACTIVE` accepts normal writes and sync. A switch performs this ordered barrier: disable commands, detach UI models, drain the profile/module executors, destroy all queries and database handles on their owning threads, remove connection names, clear sensitive view models, replace secure-session context, open and owner-check the destination databases, bind fresh models, then re-enable commands.

- [ ] **Step 4: Define sign-out and local deletion behavior**

Record two explicit sign-out choices: `Sign out and keep local copy` moves to `SIGNED_OUT_RETAINED`; `Sign out and delete local copy` clears secure tokens, closes all databases, atomically renames the profile directory to a deletion-staging name, recursively removes it, and removes the profile index entry. Neither choice changes cloud data. Reopening a retained profile requires an online sign-in whose verified `remoteUserId` matches the immutable binding.

- [ ] **Step 5: Add an acceptance matrix to the ADR**

Include allow/deny cases for first login offline, first login online, token expiry offline, transient refresh failure, explicit sign-out, wrong remote user attempting to open a retained profile, account switch while writes are queued, recovery profile access, and local-copy deletion. Expected results must state that local CRUD remains available only for a previously authenticated `ACTIVE` profile, no old profile row is displayed during a switch, and wrong-owner opens fail before any query.

- [ ] **Step 6: Validate that the ADR has one closed decision**

Run:

```bash
python3 -c "from pathlib import Path; p=Path('docs/decisions/D-023-login-and-local-profiles.md').read_text(); required=['Status: Accepted','email','LOCAL_UNBOUND','INITIALIZING','ACTIVE','SIGNED_OUT_RETAINED','RECOVERY_READ_ONLY','LOCAL_DELETE_PENDING','ownerProfileId','remoteUserId']; assert all(x in p for x in required)"
```

Expected: exit code `0`. Missing identity, state, or switch-barrier language blocks Stage 1 profile interfaces.

- [ ] **Step 7: Use the optional decision checkpoint only with explicit authorization**

If and only if authorized, run:

```bash
git add docs/decisions/D-023-login-and-local-profiles.md
git commit -m "docs: decide login and local profile semantics"
```

Without authorization, do not commit.

---

### Task 5: Freeze CNY Scope, Money Limits, Account Balances, and Analytics Formulas

**Files:**
- Create: `docs/decisions/D-024-currency-and-account-formulas.md`

**Interfaces:**
- Consumes: `MoneyMinor`, `CurrencyCode`, `Transaction`, `Account`, transaction constraints, and monthly analysis rules from architecture sections 9, 10, and 13.
- Produces: exact equations and test vectors for Stage 1 domain types and Stage 5 account/analytics behavior.

- [ ] **Step 1: Select the V1 currency and amount boundary**

Record `Status: Accepted` and select CNY-only writes for V1. Every transaction/account still persists ISO 4217 `CNY`; create, edit, import, and sync commands reject any other code with `InvalidArgument`. Individual transaction, refund, transfer, recurring expected amount, and absolute opening-balance magnitude are `1..9,999,999,999` minor units except that a zero opening balance is valid. Aggregates use checked signed 64-bit arithmetic and fail rather than wrap.

- [ ] **Step 2: Define the signed account balance equation**

Record this exact equation for posted, non-deleted transactions whose `occurredOn` is on or after `openingBalanceOn`:

```text
bookBalanceMinor = openingBalanceMinor
                 + income posted to account
                 + refunds posted to account
                 + transfers received by account
                 - expenses posted to account
                 - transfers sent by account
```

Pending transactions, unassigned transactions, and tombstones contribute zero. Assigning an account to a transaction before that account's `openingBalanceOn` is rejected. A transfer affects both accounts atomically, requires different CNY accounts, and contributes zero to income/expense analytics.

- [ ] **Step 3: Define credit-account presentation and reconciliation**

For `AccountType::Credit`, store the same signed balance as every other account: an amount owed is negative. The creation UI accepts positive `amount owed` and stores its negation. Display `amount owed = max(0, -bookBalanceMinor)` and `overpayment = max(0, bookBalanceMinor)`. A credit expense decreases the signed balance, a refund or payment transfer into the credit account increases it. A manually entered actual balance uses the same signed convention; `differenceMinor = actualBalanceMinor - bookBalanceMinor`. V1 reports the difference and never creates an automatic adjustment transaction.

- [ ] **Step 4: Define monthly income, net expense, surplus, and category-share formulas**

Use the current profile time zone and `occurredOn` to select a natural month. Define:

```text
incomeMinor = sum(POSTED INCOME)
grossExpenseMinor = sum(POSTED EXPENSE)
refundMinor = sum(POSTED REFUND occurring in the month)
netExpenseMinor = grossExpenseMinor - refundMinor
surplusMinor = incomeMinor - grossExpenseMinor + refundMinor
categoryNetMinor[c] = expense in c - refunds linked to expenses in c
chartBasisMinor[c] = max(categoryNetMinor[c], 0)
chartDenominatorMinor = sum(chartBasisMinor)
```

Transfers and pending transactions contribute zero. If the chart denominator is zero, every share is zero and no pie/donut slices render. Otherwise allocate 10,000 basis points by largest remainder from `chartBasisMinor`; deterministic ties sort by stable category ID. Negative category net values remain visible in the list as net refunds but receive zero chart basis. The card, chart, list, and drill-down consume one query result containing gross, refund, net, chart basis, and basis points.

- [ ] **Step 5: Put executable arithmetic vectors in the ADR**

Include these exact expected vectors:

| Case | Inputs in minor units | Expected |
| --- | --- | --- |
| Asset account | opening `100000`, expense `12345`, income `50000`, transfer out `20000`, transfer in `7000`, refund `2345` | `bookBalanceMinor=127000` |
| Transfer destination | opening `0`, transfer in `20000`, transfer out `7000` | `bookBalanceMinor=13000` |
| Credit account | opening owed `80000` stored `-80000`, expense `10000`, refund `2500`, payment transfer in `30000` | signed `-57500`, displayed owed `57500` |
| Monthly analytics | income `200000`, food expense `80000`, transport expense `20000`, food refund `30000` | net expense `70000`, surplus `130000`, food `7143` bp, transport `2857` bp |
| Negative category net | food expense `1000`, food refund `2000`, transport expense `3000` | total net `2000`, food chart `0` bp, transport `10000` bp |

- [ ] **Step 6: Validate equations and required constants**

Run:

```bash
python3 -c "from pathlib import Path; p=Path('docs/decisions/D-024-currency-and-account-formulas.md').read_text(); required=['Status: Accepted','CNY','9,999,999,999','bookBalanceMinor','differenceMinor','netExpenseMinor','chartBasisMinor','10,000','127000','-57500','7143','2857']; assert all(x in p for x in required)"
```

Expected: exit code `0`. Stage 1 may define types after this passes; Stage 5 may not invent a different account sign or percentage denominator.

- [ ] **Step 7: Use the optional formula checkpoint only with explicit authorization**

If and only if authorized, run:

```bash
git add docs/decisions/D-024-currency-and-account-formulas.md
git commit -m "docs: freeze currency and account formulas"
```

Without authorization, do not commit.

---

### Task 6: Resolve Recurring-Date, Lifecycle, Reminder, and Undo Edge Semantics

**Files:**
- Create: `docs/decisions/D-025-recurring-edge-semantics.md`

**Interfaces:**
- Consumes: `RecurringRule`, `RecurringOccurrence`, deterministic UUIDv5 identities, and reminder events from architecture sections 9.6, 9.7, and 11.
- Produces: deterministic recurrence grammar, date calculations, lifecycle commands, and import/provenance behavior used by Stages 3 and 5.

- [ ] **Step 1: Select the supported recurrence grammar**

Record `Status: Accepted` and accept exactly these canonical `frequencySpec` forms in V1:

```text
MONTHLY;DAY=1..31
YEARLY;MONTH=1..12;DAY=1..31
INTERVAL_DAYS;DAYS=1..365
```

`startsOn` anchors interval cycles; monthly and yearly rules preserve their requested ordinal rather than using the previous clamped occurrence as the next anchor. Invalid combinations are rejected at the boundary.

- [ ] **Step 2: Select short-month and leap-year behavior**

Select `ClampToLastDay` as the only V1 short-month policy. A monthly day 31 produces January 31, February 28 or 29, and March 31 without drift. A yearly February 29 rule produces February 28 in non-leap years and returns to February 29 in leap years. Monthly `periodKey` is `YYYY-MM`, yearly is `YYYY`, and interval-day keys are `D` followed by a zero-padded eight-digit zero-based cycle index such as `D00000000`.

- [ ] **Step 3: Freeze time-zone, horizon, and reminder-key rules**

Every rule stores a valid IANA time-zone ID, defaulting once from the profile at creation. Period keys and target local dates use that stored zone and do not change when the device zone changes. The default lead is one day; users may select one or two days, and every occurrence also receives a due-day event. The stable event key is `rule UUID:periodKey:offsetDays`, where offsets are `-1`, `-2`, or `0`. Native scheduling targets 09:00 in the rule zone, selecting the first valid instant at or after 09:00 on that local date; delivery acceptance remains the target natural day, not an exact minute.

- [ ] **Step 4: Define generation, catch-up, and edit behavior**

At rule creation and every startup, foreground resume, rule edit, and successful sync, generate every missing period through `D0 + 90 days`. Previous unresolved periods never block later periods. One calculation accepts at most 240 new occurrences per rule; exceeding that bound returns `RecurringCatchUpLimitExceeded` and writes nothing. Rule edits affect only not-yet-generated periods; generated occurrences keep their snapshot. Archiving stops future generation and requires an explicit choice to keep, skip, or cancel each current pending occurrence.

- [ ] **Step 5: Define atomic occurrence lifecycle commands**

Record these transitions:

- Generate: create deterministic occurrence and pending transaction IDs in one transaction.
- Confirm: `PENDING -> POSTED` and pending transaction `PENDING -> POSTED` in one idempotent command.
- Defer: retain `PENDING`, retain the same transaction ID, replace `deferredUntil`, cancel old events, and create events based on the deferred date.
- Skip or cancel: set the terminal status, tombstone the pending transaction, clear `transactionId`, and cancel remaining period events in one command.
- Overdue: derive from effective due date less than the current local date; never persist a separate status and never auto-post.
- Generic transaction deletion: reject any live occurrence-linked transaction with `OccurrenceLinked`.

- [ ] **Step 6: Decide how a mistaken confirmation is undone**

Define `UndoRecurringConfirmation` as the only route from `POSTED` back to `PENDING`. With no linked import item, it atomically restores the same transaction to `PENDING`, preserves its ID and edited fields, restores the occurrence to `PENDING`, and reschedules future events. If an import item is linked, the UI requires an explicit `Detach import match and return to pending` confirmation; that command clears `ImportItem.transactionId`, marks the structured provenance relation with `detachedAt`, preserves the import item and local raw text for review, then restores the same occurrence/transaction IDs to pending. Posted occurrences cannot be skipped, cancelled, or generically deleted until this undo command succeeds.

- [ ] **Step 7: Add deterministic edge vectors**

Include expected vectors for January 31 across leap/non-leap February, February 29 yearly behavior, a 14-day interval across year-end, `D0+89`, `D0+90`, and excluded `D0+91` reminder events, two devices generating the same period, an overdue prior month plus current pending month, defer then confirm, archive with an unresolved occurrence, duplicate confirm, and undo with/without import provenance.

- [ ] **Step 8: Validate the closed recurring contract**

Run:

```bash
python3 -c "from pathlib import Path; p=Path('docs/decisions/D-025-recurring-edge-semantics.md').read_text(); required=['Status: Accepted','ClampToLastDay','MONTHLY;DAY=1..31','YEARLY;MONTH=1..12;DAY=1..31','INTERVAL_DAYS;DAYS=1..365','D00000000','240','D0 + 90','UndoRecurringConfirmation','detachedAt','OccurrenceLinked']; assert all(x in p for x in required)"
```

Expected: exit code `0`.

- [ ] **Step 9: Use the optional recurring decision checkpoint only with explicit authorization**

If and only if authorized, run:

```bash
git add docs/decisions/D-025-recurring-edge-semantics.md
git commit -m "docs: resolve recurring edge semantics"
```

Without authorization, do not commit.

---

### Task 7: Decide Conflict Resolution, Retention Windows, and Cloud Account Deletion

**Files:**
- Create: `docs/decisions/D-026-conflicts-retention-and-account-deletion.md`

**Interfaces:**
- Consumes: sync conflict groups, tombstones, cursor expiry, bootstrap, profile lifecycle, and cloud deletion requirements from architecture sections 14.7, 14.10, 15, and 16.4.
- Produces: fixed retention numbers, conflict actions, and an idempotent administrator-operated deletion state machine.

- [ ] **Step 1: Freeze offline and server-retention windows**

Record `Status: Accepted` and select a 180-natural-day maximum supported incremental-offline interval. Retain tombstones, change-log positions, and mutation idempotency results for at least 210 natural days after their server commit. A cursor before `minValidCursor` receives `CURSOR_EXPIRED`; an old stream epoch receives `STALE_EPOCH`; both routes preserve local outbox/conflicts and enter reviewed re-bootstrap. Resolved conflict payloads are reduced to non-sensitive audit metadata after 30 days.

- [ ] **Step 2: Freeze single-entity conflict interaction**

Display local and server versions side by side with entity ID, revision, changed fields, deletion state, and source device. Offer exactly `Keep local`, `Keep server`, and `Edit merged copy`; never field-merge automatically. `Keep local` or a manual merge creates a new mutation against the latest server revision. `Keep server` archives the local conflicting payload in the conflict record until resolution ACK. Delete-versus-edit is always explicit and never treats deletion as automatically newer.

- [ ] **Step 3: Freeze atomic-group conflict behavior**

If any affected entity in a pulled group is dirty, conflicted, or would violate aggregate rules, persist the complete remote group, relevant local snapshots, and outbox references in one transaction; mark every affected entity isolated; then advance the cursor only after that durable quarantine. Resolve recurring, import-match, and refund groups as one aggregate command. Replay later related groups in server order after resolution. No UI action may accept only one entity from a group.

- [ ] **Step 4: Freeze account-deletion states and authorization**

Cloud deletion is maintainer-operated in V1 and uses `ACTIVE -> DELETING -> DELETED` with a stable client-visible request UUID. The user must reauthenticate, type the account email, and acknowledge that offline device copies cannot be remotely erased; the client offers JSON/CSV export before the request. Entering `DELETING` revokes new sessions and writes, drains already-started module transactions, invalidates snapshot tokens, deletes module rows/change logs/idempotency rows, removes the auth identity last, and retains only a deletion receipt containing request ID, actor ID, timestamps, outcome, and no ledger content.

- [ ] **Step 5: Freeze deletion timing and backup disclosure**

Set a 24-hour operational completion target for active cloud rows and a hard 30-natural-day cap for deleted content remaining in provider-managed backups. The selected provider must document and fit that cap; otherwise cloud conformance fails. Deletion is not user-reversible after `DELETING` is accepted. Keep the non-sensitive deletion receipt for 365 days. V1 exposes no operation that retains an account/profile while clearing the entire ledger.

- [ ] **Step 6: Add conflict and deletion acceptance vectors**

Include same-entity edit/edit, edit/delete, recurring occurrence/transaction group conflict, conflict-resolution ACK loss, resolution conflicting again, 179/180/181-day reconnect, 209/210/211-day retention boundaries, duplicate deletion request, deletion during active sync, deletion during bootstrap, server failure after each deletion step, and an offline device reopening after cloud deletion. Each vector must state preserved data and next state.

- [ ] **Step 7: Validate fixed numbers and state names**

Run:

```bash
python3 -c "from pathlib import Path; p=Path('docs/decisions/D-026-conflicts-retention-and-account-deletion.md').read_text(); required=['Status: Accepted','180','210','30','365','ACTIVE -> DELETING -> DELETED','Keep local','Keep server','Edit merged copy','CURSOR_EXPIRED','STALE_EPOCH','24-hour']; assert all(x in p for x in required)"
```

Expected: exit code `0`. Cloud selection must later prove the backup-retention cap.

- [ ] **Step 8: Use the optional conflict/deletion checkpoint only with explicit authorization**

If and only if authorized, run:

```bash
git add docs/decisions/D-026-conflicts-retention-and-account-deletion.md
git commit -m "docs: decide conflicts retention and account deletion"
```

Without authorization, do not commit.

---

### Task 8: Collect Sanitized Import Samples and Freeze Input Limits

**Files:**
- Create: `docs/decisions/D-027-text-import-contract-and-limits.md`
- Create: `prototypes/stage0/import_fixtures/sanitize_text_samples.py`
- Create: `prototypes/stage0/import_fixtures/validate_text_fixtures.py`
- Create: `tests/fixtures/import/sample-01-one-line.txt`
- Create: `tests/fixtures/import/sample-01-one-line.expected.json`
- Create: `tests/fixtures/import/sample-02-date-headings.txt`
- Create: `tests/fixtures/import/sample-02-date-headings.expected.json`
- Create: `tests/fixtures/import/sample-03-missing-year.txt`
- Create: `tests/fixtures/import/sample-03-missing-year.expected.json`
- Create: `tests/fixtures/import/sample-04-errors-and-duplicates.txt`
- Create: `tests/fixtures/import/sample-04-errors-and-duplicates.expected.json`
- Create: `tests/fixtures/import/sample-05-recurring-match.txt`
- Create: `tests/fixtures/import/sample-05-recurring-match.expected.json`
- Create: `docs/validation/stage-0/import-fixtures.md`
- Create: `docs/validation/stage-0/import-fixtures.sha256`

**Interfaces:**
- Consumes: at least three private typed-note excerpts through `DA_PRIVATE_IMPORT_SAMPLE_DIR` and the same external denylist used for DAT sanitization.
- Produces: five deterministic UTF-8 fixture families, expected parse candidates, normalization version 1, and hard input/field limits for `IImportParser`.

- [ ] **Step 1: Freeze normalization version 1**

Record `Status: Accepted` and define normalization in this exact order: reject input over the byte limit; strict UTF-8 decode; remove one leading UTF-8 BOM; reject NUL; convert CRLF and lone CR to LF; normalize Unicode to NFC; remove trailing ASCII space/tab from each physical line; preserve leading whitespace and blank-line count; ensure one final LF for hashing. Compute `batchHash = SHA-256("dailyaccount-import-batch-v1\n" + normalized UTF-8 bytes)`. Compute each item fingerprint as `SHA-256("dailyaccount-import-item-v1\n" + exact normalized item UTF-8 bytes)`. Hashes are duplicate hints, never global unique identities.

- [ ] **Step 2: Freeze input and field limits**

Put these exact limits in D-027:

| Boundary | Limit and result |
| --- | --- |
| Clipboard or TXT bytes | `1,048,576`; reject before decode when exceeded |
| Physical lines | `10,000`; reject the batch when exceeded |
| Candidate items | `5,000`; reject the batch when exceeded |
| One physical line | `4,096` UTF-8 bytes after normalization; mark item invalid |
| One assembled item | `16,384` UTF-8 bytes; mark item invalid |
| Merchant | `256` UTF-8 bytes; require correction when exceeded |
| Note | `2,048` UTF-8 bytes; require correction when exceeded |
| Unmapped category token | `128` UTF-8 bytes; require correction when exceeded |
| Amount | `1..9,999,999,999` CNY minor units; reject zero, overflow, and over-precision |

Only clipboard text and `.txt` files are accepted. Complete raw input stays local, is excluded from logs/sync/cloud backup/ordinary JSON export, and is retained through confirmation unless the user selected immediate post-commit removal.

- [ ] **Step 3: Freeze parse confidence and direction rules**

An explicit income/expense word sets direction when consistent. A leading negative amount alone may propose `EXPENSE` but remains `NeedsReview`; a positive amount without direction remains `NeedsReview`; contradictory sign and word is `Invalid`. Missing date, missing amount, missing direction, absent default year, unknown category, excessive field, invalid UTF-8, and overflow never become `High`. No candidate becomes `POSTED` before explicit batch confirmation.

- [ ] **Step 4: Implement offline text sanitization**

Implement `sanitize_text_samples.py` to read only local files, preserve line grouping and format classes, shift dates by 28 years, map merchants/notes/categories to stable synthetic tokens, remap distinct amounts with the DAT rank formula, and write no original value or source path. Require at least three source excerpts and at least 50 candidate-like lines in total. If that minimum is unavailable, return non-zero and set the validation result to `FAIL_STOP`; parser implementation in Stage 5 remains blocked.

- [ ] **Step 5: Generate the five fixture families**

Run:

```bash
: "${DA_PRIVATE_IMPORT_SAMPLE_DIR:?DA_PRIVATE_IMPORT_SAMPLE_DIR must name the private sample directory}"
: "${DA_PRIVATE_TERMS_FILE:?DA_PRIVATE_TERMS_FILE must name the private denylist}"
python3 prototypes/stage0/import_fixtures/sanitize_text_samples.py \
  --source-dir "$DA_PRIVATE_IMPORT_SAMPLE_DIR" \
  --private-terms "$DA_PRIVATE_TERMS_FILE" \
  --output-dir tests/fixtures/import
```

Expected fixture coverage:

- `sample-01-one-line`: full date, amount, direction, optional category/note on one line.
- `sample-02-date-headings`: one date heading followed by multiple transactions, blank lines, and CRLF-origin evidence.
- `sample-03-missing-year`: month/day values requiring one explicit batch default year.
- `sample-04-errors-and-duplicates`: malformed amount/date, contradictory direction, exact duplicate item, duplicate batch content, unknown category, delimiter text, and Unicode.
- `sample-05-recurring-match`: one unique high-confidence occurrence match, two ambiguous matches, one amount outside tolerance, and one already-matched occurrence.

- [ ] **Step 6: Implement and run fixture validation**

`validate_text_fixtures.py` must apply normalization version 1, enforce every limit, scan generic privacy patterns and every denylist term, compare fixture hashes and expected candidate states, assert at least 50 total candidate-like lines, and generate an oversized input in memory rather than storing a megabyte fixture. Run:

```bash
python3 prototypes/stage0/import_fixtures/validate_text_fixtures.py \
  --root tests/fixtures/import --private-terms "$DA_PRIVATE_TERMS_FILE"
```

Expected: exactly five fixture-family `PASS` lines, an oversized-input rejection at byte `1,048,577`, an item-count rejection at `5,001`, and no privacy match.

- [ ] **Step 7: Record and hash import evidence**

Create `import-fixtures.md` with `Result: PASS`, sample counts, covered syntax, normalization rules, hard limits, privacy scan result, and the statement that every expected JSON file contains sanitized candidates only. Run:

```bash
sha256sum tests/fixtures/import/*.txt tests/fixtures/import/*.expected.json \
  > docs/validation/stage-0/import-fixtures.sha256
```

Expected: ten hash lines sorted by shell expansion and no private source hash.

- [ ] **Step 8: Use the optional import checkpoint only with explicit authorization**

If and only if authorized, stage D-027, the two import-fixture scripts, ten fixture files, and two import validation files, then run:

```bash
git commit -m "test: add sanitized text import contract fixtures"
```

Without authorization, do not commit.

---

### Task 9: Model and Falsify the Synchronization Protocol Before Transport Code

**Files:**
- Create: `docs/decisions/D-022-sync-protocol-v1.md`
- Create: `prototypes/stage0/sync_model/sync_model.py`
- Create: `prototypes/stage0/sync_model/test_sync_model.py`
- Create: `prototypes/stage0/sync_model/run_model.py`
- Create: `docs/validation/stage-0/sync-model.md`
- Create: `docs/validation/stage-0/sync-model.log`
- Create: `docs/validation/stage-0/sync-model-results.json`

**Interfaces:**
- Consumes: the mutation/pull/bootstrap contracts in architecture section 14 and the 180/210-day decisions in D-026.
- Produces: an executable, provider-neutral reference state machine and an accepted V1 protocol ADR for cloud conformance and Stage 4.

- [ ] **Step 1: Define the model's immutable wire records**

Implement these Python interfaces with frozen dataclasses and JSON-canonical SHA-256 request digests:

```python
from collections.abc import Sequence
from dataclasses import dataclass

@dataclass(frozen=True)
class RevisionExpectation:
    entity_type: str
    entity_id: str
    base_server_revision: int

@dataclass(frozen=True)
class Mutation:
    mutation_id: str
    command_type: str
    payload_version: int
    expectations: Sequence[RevisionExpectation]
    affected_entities: Sequence[str]
    payload: dict[str, object]

@dataclass(frozen=True)
class ChangeGroup:
    change_group_id: str
    commit_cursor: int
    command_type: str
    payload_version: int
    entities: Sequence[dict[str, object]]
```

Implement these exact callable signatures:

```text
ModelServer.push(user_id: str, module_id: str, epoch: str, mutations: Sequence[Mutation]) -> list[dict[str, object]]
ModelServer.pull(user_id: str, module_id: str, epoch: str, cursor: int, limit: int) -> dict[str, object]
ModelServer.bootstrap(user_id: str, module_id: str, snapshot_token: str | None, page_token: str | None, limit: int) -> dict[str, object]
ModelClient.edit(mutation: Mutation) -> None
ModelClient.sync_once(server: ModelServer) -> None
ModelClient.resolve_conflict(conflict_id: str, resolution: dict[str, object]) -> None
```

Use integer model cursors internally only to prove ordering; D-022 requires real transports to expose them as opaque strings.

- [ ] **Step 2: Implement server ordering, idempotency, and atomic groups**

For each `(user_id, module_id)`, model one stream lock, epoch, next commit cursor, entity revisions, idempotency results, and ordered change groups. Under the stream lock, check an existing `(user,module,mutationId)` digest before epoch/version/revision validation; return the original complete result for the same digest and `IDEMPOTENCY_KEY_REUSED` for a different digest. Allocate the cursor at commit, update all affected entities, save the result, and append one complete group atomically.

- [ ] **Step 3: Implement client outbox and quarantine states**

Model `UNSENT`, immutable `IN_FLIGHT`, acknowledged, dirty entity, quarantined entity, conflict group, cursor, epoch, and bootstrap staging states. An edit during in-flight creates a later unsent mutation. Pull applies a whole group or stores the whole group plus local snapshots and outbox references, then advances the cursor in that same model transaction. Historical ACK from an old epoch clears only its matching outbox row and triggers bootstrap without applying its entity payload.

- [ ] **Step 4: Add the 18 named protocol tests**

Implement exactly these tests:

```text
test_commit_cursor_follows_commit_order
test_idempotent_retry_returns_original_result
test_reused_mutation_id_with_changed_digest_is_rejected
test_lost_ack_does_not_duplicate
test_edit_while_in_flight_survives_first_ack
test_revision_conflict_preserves_both_payloads
test_delete_update_conflict_preserves_local_update
test_atomic_group_is_applied_or_quarantined_whole
test_cursor_advances_only_after_apply_or_durable_quarantine
test_pull_page_never_splits_change_group
test_occurrence_race_returns_canonical_ids_without_orphan
test_import_batch_retry_returns_original_transactions
test_bootstrap_resume_uses_same_snapshot
test_local_bootstrap_replays_late_outbox_before_activation
test_cursor_expiry_requires_bootstrap
test_old_epoch_ack_clears_only_matching_outbox_then_bootstraps
test_unsupported_payload_does_not_advance_cursor
test_module_stream_failure_does_not_rollback_another_module
```

- [ ] **Step 5: Add deterministic adversarial interleavings**

`run_model.py` must execute the 18 tests and then 10,000 seeded schedules using seed `20260904`. Each schedule interleaves two clients, lost responses, duplicate sends, edit-during-flight, reversed server-transaction completion, page failures, epoch rotation, and bootstrap interruption. Assert that accepted mutations are never lost, an entity revision never decreases, a cursor never skips an un-applied/un-quarantined group, atomic groups never split, and a mutation ID never creates two results.

- [ ] **Step 6: Run the model and capture machine-readable evidence**

Run:

```bash
set -o pipefail
python3 prototypes/stage0/sync_model/run_model.py \
  --seed 20260904 --schedules 10000 \
  --json docs/validation/stage-0/sync-model-results.json \
  2>&1 | tee docs/validation/stage-0/sync-model.log
```

Expected: `Ran 18 tests`, `OK`, `10000 adversarial schedules passed`, exit code `0`, and JSON fields `seed: 20260904`, `scheduleCount: 10000`, `failedInvariantCount: 0`.

- [ ] **Step 7: Write the accepted synchronization ADR**

Create D-022 with `Status: Accepted` and freeze these choices: full command-after-state payloads rather than JSON Patch; one atomic mutation result per command; entity-scoped server revisions; stream-scoped opaque commit cursors; same-ID/same-digest result replay; same-ID/different-digest rejection; complete-group pull pagination; whole-group local quarantine; 100-mutation push target limit; resumable authenticated snapshot bootstrap; 180-day incremental-offline support; 210-day tombstone/change/idempotency retention; and independent module streams.

- [ ] **Step 8: Write the synchronization validation record**

Create `sync-model.md` with `Result: PASS`, model limitations, command and environment, all 18 scenario names, schedule seed/count, JSON/log hashes, and a traceability table to architecture sections 14.2-14.12. Any failed invariant is `FAIL_STOP` for cloud RPC design and Stage 4; fix the model or revise the ADR explicitly, never code around it.

- [ ] **Step 9: Use the optional protocol-model checkpoint only with explicit authorization**

If and only if authorized, stage D-022, the three model files, and the three sync validation files, then run:

```bash
git commit -m "test: model the synchronization protocol"
```

Without authorization, do not commit.

---

### Task 10: Prove QSQLITE Thread Ownership, Busy Bounds, Drain, Backup, and Restore

**Files:**
- Create: `docs/decisions/D-028-qsqlite-threading-and-backup.md`
- Create: `prototypes/stage0/qsqlite/CMakeLists.txt`
- Create: `prototypes/stage0/qsqlite/serial_executor.h`
- Create: `prototypes/stage0/qsqlite/serial_executor.cpp`
- Create: `prototypes/stage0/qsqlite/qsqlite_probe.cpp`
- Create on declared fallback: `prototypes/stage0/qsqlite/online_backup_probe.cpp`
- Create on declared fallback: `prototypes/stage0/qsqlite/vendor/sqlite3.c`
- Create on declared fallback: `prototypes/stage0/qsqlite/vendor/sqlite3.h`
- Create on declared fallback: `prototypes/stage0/qsqlite/vendor/README.md`
- Create on declared fallback: `prototypes/stage0/android/online_backup_probe.cpp`
- Create: `docs/validation/stage-0/qsqlite-thread-backup.md`
- Create: `docs/validation/stage-0/qsqlite-thread-backup.log`
- Create: `docs/validation/stage-0/qsqlite-thread-backup-results.json`

**Interfaces:**
- Consumes: Qt 6.9.3 Core/SQL, one temporary profile/module database, and the thread/backup constraints in architecture sections 6.3, 16.3, and 19.2.
- Produces: a tested connection lifecycle and exactly one selected consistent-backup mechanism for Stage 2.

- [ ] **Step 1: Define the prototype executor boundary**

Implement this isolated interface:

```cpp
struct ProbeResult {
    bool success;
    QString code;
    QString detail;
};

using SqlOperation = std::function<ProbeResult(QSqlDatabase&)>;

class SerialSqlExecutor final : public QObject {
    Q_OBJECT
public:
    explicit SerialSqlExecutor(QString profileId, QString moduleId,
                               QString databasePath, QObject* parent = nullptr);
    QFuture<ProbeResult> enqueue(SqlOperation operation);
    QFuture<ProbeResult> backupTo(QString temporaryBackupPath);
    ProbeResult drainAndClose();
};
```

The worker thread creates, opens, uses, closes, and removes its own uniquely named connection. The name format is `da/profileId/moduleId/workerSequence`. No `QSqlDatabase` or `QSqlQuery` leaves that thread. `drainAndClose()` rejects new jobs, runs queued jobs, destroys every query/database handle, closes the connection, calls `removeDatabase`, and then stops the thread.

- [ ] **Step 2: Apply and assert the selected connection pragmas**

Before open, set `QSQLITE_BUSY_TIMEOUT=5000`. After open, execute and read back `foreign_keys=ON`, `journal_mode=WAL`, and `synchronous=FULL`. Create a `STRICT` probe table with primary key, `CHECK(amount_minor > 0 AND amount_minor <= 9999999999)`, `UNIQUE`, and foreign-key constraints. Treat inability to enable any mandatory setting as probe failure.

- [ ] **Step 3: Implement the primary `VACUUM INTO` backup path**

Queue backup behind all prior writes on the same executor. Remove a stale generated `.partial` path, execute `VACUUM INTO` with the generated destination path safely bound or escaped by a dedicated filename-expression function, open the result read-only with a new validation-only connection on its own thread, require `PRAGMA integrity_check` to return `ok`, compare schema version/row count/sum, close validation handles, then atomically rename `.partial` to the final backup path. Never copy an active database or its WAL.

- [ ] **Step 4: Add the complete QSQLITE probe matrix**

`qsqlite_probe.cpp` must run and report these cases:

```text
qsqlite_driver_present
connection_created_used_removed_on_worker_thread
foreign_keys_reject_orphan
strict_and_check_reject_wrong_values
eight_producers_serialize_8000_writes
second_writer_times_out_between_4500_and_6500_ms
queued_backup_contains_all_prior_and_no_later_write
vacuum_into_backup_integrity_is_ok
interrupted_partial_backup_is_rejected_and_cleaned
restore_validation_rejects_corruption
drain_before_profile_switch_preserves_all_commits
close_emits_no_qsqldatabase_still_in_use_warning
```

The busy test holds a write transaction on a second connection and proves the UI/event-loop heartbeat continues while the worker reaches its bounded timeout.

- [ ] **Step 5: Build and run the probe with Qt 6.9.3 on Windows**

Run in a Qt/MinGW PowerShell:

```powershell
& "$env:QT_DIR\bin\qt-cmake.bat" -S prototypes\stage0\qsqlite -B build\stage0-qsqlite -G "MinGW Makefiles"
cmake --build build\stage0-qsqlite --parallel
& build\stage0-qsqlite\qsqlite_probe.exe --json docs\validation\stage-0\qsqlite-thread-backup-results.json 2>&1 |
  Tee-Object docs\validation\stage-0\qsqlite-thread-backup.log
if ($LASTEXITCODE -ne 0) { throw "QSQLITE probe failed" }
```

Expected: 12 named cases pass, `QSQLITE` is listed, the observed SQLite library version is recorded, integrity is `ok`, 8,000 writes are present, busy duration is 4,500-6,500 ms, and there is no connection-in-use warning.

- [ ] **Step 6: Exercise the declared backup fallback only if `VACUUM INTO` fails on a target**

If and only if the Qt-bundled SQLite on Windows or Android rejects or corrupts `VACUUM INTO`, create `online_backup_probe.cpp` for desktop and Android plus `vendor/sqlite3.c`, `vendor/sqlite3.h`, and `vendor/README.md` from the official SQLite source version embedded by Qt 6.9.3. Record the source URL, archive SHA-256, SQLite version, public-domain notice, and extraction command in `vendor/README.md`. Queue `sqlite3_backup_init`, repeated bounded `sqlite3_backup_step`, and `sqlite3_backup_finish` on the serial executor; validate/rename the same `.partial` artifact and never share the QSQLITE connection handle.

Run the desktop fallback with:

```powershell
cmake --build build\stage0-qsqlite --target qsqlite_online_backup_probe --parallel
& build\stage0-qsqlite\qsqlite_online_backup_probe.exe --json docs\validation\stage-0\qsqlite-thread-backup-results.json 2>&1 |
  Tee-Object docs\validation\stage-0\qsqlite-thread-backup.log
if ($LASTEXITCODE -ne 0) { throw "SQLite Online Backup probe failed" }
```

Build the Android fallback through `build_android.sh --backup-mechanism online-api` and rerun Task 11's runtime matrix. Select it only if both targets pass all consistency/interruption/restore assertions. Record `PASS_WITH_FALLBACK` and exact source/version/license provenance in D-028. If neither mechanism passes on both Windows and Android, set `FAIL_STOP`; Stage 2 persistence and Stage 6 backup work cannot begin.

- [ ] **Step 7: Record the selected mechanism and lifecycle**

Create D-028 with one selected mechanism, connection-name format, pragmas, 5,000 ms timeout, queue/drain sequence, `.partial` validation/rename protocol, restore staging validation, and the rule that backup/restore never shares a connection across threads. Create `qsqlite-thread-backup.md` with result, tool/SQLite versions, all 12 cases, measured busy duration, log/JSON hashes, and Android cross-check required by Task 11. D-028 is accepted for G0 only after Task 11 records the same mechanism passing on Android.

- [ ] **Step 8: Use the optional QSQLITE checkpoint only with explicit authorization**

If and only if authorized, stage D-028, the four QSQLITE prototype files, and the three validation files, then run:

```bash
git commit -m "test: prove QSQLITE threading and backup"
```

Without authorization, do not commit.

---

### Task 11: Freeze the Android Matrix and Prototype QML, 64-bit Money, QSQLITE, HTTPS, and WorkManager

**Files:**
- Create: `docs/decisions/D-029-android-background-sync.md`
- Modify if Android selects the backup fallback: `docs/decisions/D-028-qsqlite-threading-and-backup.md`
- Modify: `docs/validation/stage-0/qsqlite-thread-backup.md`
- Create: `prototypes/stage0/android/CMakeLists.txt`
- Create: `prototypes/stage0/android/main.cpp`
- Create: `prototypes/stage0/android/probe_facade.h`
- Create: `prototypes/stage0/android/probe_facade.cpp`
- Create: `prototypes/stage0/android/cloud_probe.h`
- Create: `prototypes/stage0/android/cloud_probe.cpp`
- Create: `prototypes/stage0/android/qml/Main.qml`
- Create: `prototypes/stage0/android/android/AndroidManifest.xml`
- Create: `prototypes/stage0/android/android/build.gradle`
- Create: `prototypes/stage0/android/android/src/main/java/local/dailyaccount/stage0/ProbeWorker.kt`
- Create: `prototypes/stage0/android/android/src/main/java/local/dailyaccount/stage0/ProbeStatusStore.kt`
- Create: `prototypes/stage0/android/scripts/build_android.sh`
- Create: `prototypes/stage0/android/scripts/run_runtime_matrix.sh`
- Create: `docs/validation/stage-0/android-toolchain-runtime.md`
- Create: `docs/validation/stage-0/android-toolchain.txt`
- Create: `docs/validation/stage-0/android-build.log`
- Create: `docs/validation/stage-0/android-runtime-results.json`
- Create: `docs/validation/stage-0/android-workmanager.md`
- Create: `docs/validation/stage-0/android-workmanager-results.json`

**Interfaces:**
- Consumes: D-020 tool versions, D-028 QSQLITE settings, a valid HTTPS probe endpoint, one local self-signed endpoint, and disposable provider test credentials supplied outside Git.
- Produces: an installable non-production APK and a selected background-sync mode without changing the native reminder contract.

- [ ] **Step 1: Freeze and install the exact Android toolchain**

Use JDK 17, SDK Platform 35, Build Tools 35.0.1, Platform Tools, CMake 3.22.1, NDK 27.2.12479018, Qt 6.9.3 `android_arm64_v8a` and `android_x86_64`, Gradle 8.10, AGP 8.6.0, Kotlin 2.0.21, and WorkManager 2.10.1. Run:

```bash
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
yes | "$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager" --licenses
"$ANDROID_SDK_ROOT/cmdline-tools/latest/bin/sdkmanager" \
  "platform-tools" "platforms;android-35" "build-tools;35.0.1" \
  "cmake;3.22.1" "ndk;27.2.12479018" "emulator" \
  "system-images;android-28;google_apis;x86_64" \
  "system-images;android-35;google_apis;x86_64"
```

Expected: all packages install successfully. Capture `java -version`, `cmake --version`, `adb version`, `sdkmanager --list_installed`, both Qt kit versions, Gradle, AGP, Kotlin, and WorkManager coordinates in `android-toolchain.txt`.

- [ ] **Step 2: Define the mandatory device/API matrix**

Create two clean AVDs named `DailyAccount_API28_x86_64` and `DailyAccount_API35_x86_64`, plus use at least one physical `arm64-v8a` device running an API from 28 through 35. Record manufacturer/model, API, ABI, build fingerprint, notification permission state, battery-optimization state, and network class for each. Missing either boundary emulator or a physical ARM64 device is `FAIL_STOP` for Android runtime/reminder evidence.

- [ ] **Step 3: Implement the isolated Qt Quick probe**

Use `qt_add_executable`, `qt_add_qml_module`, Qt Core/Quick/QML/SQL/Network, and package ID `local.dailyaccount.stage0`. `Main.qml` displays `qml-ok`, accepts money only as decimal text, and passes `99999999.99` to C++ without JavaScript numeric conversion. `ProbeFacade` must parse it to exactly `9,999,999,999`, persist it through QSQLITE, close/reopen, and return the same decimal string. QML performs no financial arithmetic.

- [ ] **Step 4: Add Android QSQLITE and HTTPS assertions**

On a worker owned connection, print available drivers, require `QSQLITE`, apply D-028 pragmas, create one strict table, insert/read the 64-bit amount, run `VACUUM INTO` or D-028's selected fallback, and validate the backup. `CloudProbe` uses `QNetworkAccessManager` with a 10-second connection timeout and 20-second transfer timeout, validates JSON type/shape, succeeds against the valid endpoint, and fails against the local self-signed endpoint. There is no certificate-ignore signal handler.

- [ ] **Step 5: Add a WorkManager cold-process experiment**

`ProbeWorker` is a Kotlin `CoroutineWorker` scheduled once with network required. It records start/end/process IDs in app-private `ProbeStatusStore`, then invokes one narrow JNI entry that attempts to initialize the minimum Qt/C++ runtime, open the probe QSQLITE database on an owned thread, and complete one valid HTTPS health request. The worker returns retry on transient network failure and failure on initialization/schema errors; it never performs a shipping sync.

- [ ] **Step 6: Build both emulator and physical-device APK variants**

Run:

```bash
export QT_ANDROID_ARM64="$HOME/Qt/6.9.3/android_arm64_v8a"
export QT_ANDROID_X86_64="$HOME/Qt/6.9.3/android_x86_64"
bash prototypes/stage0/android/scripts/build_android.sh \
  2>&1 | tee docs/validation/stage-0/android-build.log
```

`build_android.sh` must configure isolated CMake directories with minimum API 28, compile/target API 35, package one `x86_64` debug APK and one `arm64-v8a` debug APK, and print each APK SHA-256. Expected: both APKs build and contain Qt Quick, QML, SQL, Network, the QSQLITE plugin, Kotlin classes, and WorkManager 2.10.1.

- [ ] **Step 7: Run QML, money, QSQLITE, restart, backup, and TLS tests on every target**

Run `run_runtime_matrix.sh` once per AVD and physical device. The script must install with `adb install -r`, start with `am start -W`, assert `qml-ok` from UI automation, execute the C++ probe, use `am kill local.dailyaccount.stage0`, reopen, and verify the database value/backup. It must record valid HTTPS success, self-signed TLS rejection, no token/content in logcat, and one result object per device in `android-runtime-results.json`.

Expected: every target reports `qml=true`, `moneyMinor=9999999999`, `moneyText="99999999.99"`, `qsqlite=true`, `restartPersistence=true`, `backupIntegrity="ok"`, `validHttps=true`, and `invalidCertificateRejected=true`. If only the selected `VACUUM INTO` path fails, execute Task 10 Step 6, update D-028 and its validation record, and rerun the complete Android matrix before recording a result. Any other failure is `FAIL_STOP`; raising the minimum API or dropping ARM64 requires explicit product approval and amendments before rerun.

- [ ] **Step 8: Run the cold-process WorkManager branch**

Schedule the probe for two minutes, return to the launcher, run `adb shell am kill local.dailyaccount.stage0`, wait up to five minutes without reopening the activity, and read `ProbeStatusStore` with `run-as`. Repeat on API 28, API 35, and the physical device while online, then repeat once offline to verify that the network constraint leaves work queued without invoking Qt or losing the due marker.

Expected primary result: a different process ID starts, JNI/Qt initializes, QSQLITE and HTTPS complete, and WorkManager records success on every online target. If this primary result fails on any mandatory target, execute the declared fallback: WorkManager remains a Kotlin-only `sync due` marker with bounded retry, and app startup/foreground resume performs all Qt/C++ sync and recurring catch-up. The fallback is acceptable as `PASS_WITH_FALLBACK` because synchronization remains local-first and foreground-compensated; it may not schedule reminders or claim background convergence.

- [ ] **Step 9: Record the selected background mode and runtime evidence**

Create D-029 with either `BACKGROUND_QT_ENABLED` and the passing matrix, or `FOREGROUND_COMPENSATION` and the failed cold-init evidence plus Kotlin-only trigger contract. Create `android-toolchain-runtime.md` and `android-workmanager.md` with terminal results, matrix, commands, hashes, timings, and limitations. WorkManager failure without the exact fallback evidence is `FAIL_STOP`.

- [ ] **Step 10: Use the optional Android runtime checkpoint only with explicit authorization**

If and only if authorized, stage D-029, the Android prototype files created in this task, and the seven Android runtime/WorkManager evidence files, then run:

```bash
git commit -m "test: prove Android Qt runtime boundaries"
```

Do not stage APKs, generated Gradle trees, credentials, or app-private databases. Without authorization, do not commit.

---

### Task 12: Prove Native Local 90-Day Reminder Delivery and Rebuild Paths

**Files:**
- Create: `docs/decisions/D-030-android-reminder-delivery.md`
- Create: `prototypes/stage0/android/android/src/main/java/local/dailyaccount/stage0/ReminderContract.kt`
- Create: `prototypes/stage0/android/android/src/main/java/local/dailyaccount/stage0/ReminderStore.kt`
- Create: `prototypes/stage0/android/android/src/main/java/local/dailyaccount/stage0/ReminderScheduler.kt`
- Create: `prototypes/stage0/android/android/src/main/java/local/dailyaccount/stage0/ReminderReceiver.kt`
- Create: `prototypes/stage0/android/android/src/main/java/local/dailyaccount/stage0/ReminderSystemReceiver.kt`
- Create: `prototypes/stage0/android/android/src/androidTest/java/local/dailyaccount/stage0/ReminderSchedulerTest.kt`
- Modify: `prototypes/stage0/android/android/AndroidManifest.xml`
- Create: `prototypes/stage0/android/scripts/run_reminder_matrix.sh`
- Create: `docs/validation/stage-0/android-reminder.md`
- Create: `docs/validation/stage-0/android-reminder-results.json`

**Interfaces:**
- Consumes: reminder keys/dates from D-025 and the mandatory target matrix from Task 11.
- Produces: an Android-native scheduler that persists supplied events and displays notifications without deriving finance rules, starting Qt, or requiring network access.

- [ ] **Step 1: Implement the native reminder contract and persistence**

Use this Kotlin data contract:

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

`ReminderStore` uses Android `SQLiteOpenHelper` in app-private storage with a unique `event_key`, delivery state, and minimum display snapshot. `replaceEvents(ruleId, events)` and `cancelPeriod(ruleId, periodKey)` are transactional/idempotent. The native layer does not create an occurrence/transaction or calculate recurrence periods.

- [ ] **Step 2: Implement the day-level AlarmManager primary path**

For each supplied event, derive 09:00 in its IANA zone and schedule a unique immutable `PendingIntent` with `AlarmManager.setAndAllowWhileIdle(AlarmManager.RTC_WAKEUP, triggerAtMillis, pendingIntent)`. Use the event key in the intent data URI so hash collisions cannot alias events. `ReminderReceiver` loads the current event by key, posts a notification, and marks only delivery metadata. Dismissing/opening the notification does not mutate accounting state.

- [ ] **Step 3: Implement all rebuild and health paths**

`ReminderSystemReceiver` handles `BOOT_COMPLETED`, `TIME_SET`, `TIMEZONE_CHANGED`, and `MY_PACKAGE_REPLACED` by loading persisted undelivered events and replacing alarms. App launch performs the same reconciliation. Report `Ready`, `PermissionRequired`, `SchedulingRestricted`, or `ForceStoppedUntilReopen`; Android 13+ notification denial and exact-alarm restrictions must be visible rather than reported as healthy. A user force-stop remains an explicit platform exception, and the next launch immediately rebuilds alarms.

- [ ] **Step 4: Add deterministic 90-day boundary instrumentation tests**

With an injected calculation clock, assert all of the following:

- Events at `D0+89` and `D0+90` are persisted and scheduled; `D0+91` is excluded until the window is replenished.
- Replacing the same rule/events twice creates no duplicate row or `PendingIntent`.
- Confirm, skip, and cancel remove all untriggered events for the period.
- Defer removes old events and installs only the new effective-date events.
- A time-zone change preserves target local dates and recomputes instants.
- A date change, normal reboot, and package replacement rebuild from persisted events.
- Delivery/dismissal never changes a probe occurrence's pending state.
- Notification denial returns `PermissionRequired` and posts nothing.

- [ ] **Step 5: Run real cold-process, offline, and normal-reboot delivery drills**

`run_reminder_matrix.sh` must install the APK and run separate accelerated drills for events representing `D0+89` and `D0+90`; each drill schedules one actual alarm two minutes ahead, disables network, kills the app process without force-stop, and proves the notification arrives without any Qt initialization marker. Repeat with an event persisted but not scheduled, perform a normal emulator/device reboot, and prove `BOOT_COMPLETED` rebuilds and delivers it. Repeat time-zone change and package-upgrade rebuild on API 28, API 35, and the physical ARM64 device.

Expected: each mandatory target reports boundary inclusion, idempotency, offline delivery, reboot rebuild, time-zone rebuild, upgrade rebuild, cancellation/replacement, permission health, and `qtInitializedByReminder=false`.

- [ ] **Step 6: Exercise force-stop recovery explicitly**

Schedule one debug event, run `adb shell am force-stop local.dailyaccount.stage0`, and record that delivery is not promised while stopped. Relaunch the activity, require immediate reconciliation, and verify a newly scheduled accelerated event delivers. This is a documented operating-system exception, not a failed normal-reboot test.

- [ ] **Step 7: Use the exact-alarm fallback only if day-level alarms miss a target date**

If `setAndAllowWhileIdle` misses the target natural day on any mandatory target under normal conditions, test `setExactAndAllowWhileIdle` with the platform's exact-alarm access and the same persistent rebuild logic. Select it only if policy eligibility is documented, granted/denied health is visible, and every matrix test passes; record `PASS_WITH_FALLBACK`. If neither path meets day-level delivery, set `FAIL_STOP`, reject D-017 as a release promise, and stop before Stage 1 until architecture sections 11.3, 19.6, 21.4, decision D-017, and the master G3/G5 gates are revised and approved.

- [ ] **Step 8: Record the selected reminder mechanism and evidence**

Create D-030 with the selected alarm API, 09:00 target, 90-day inclusive boundary, persisted-event schema, rebuild broadcasts, permission/force-stop semantics, and measured device matrix. Create `android-reminder.md` with `Result: PASS` or the allowed fallback result, exact test commands, durations, device fingerprints, notification IDs/timestamps, JSON/log hashes, and a statement that the receiver loaded no Qt runtime and used no network.

- [ ] **Step 9: Use the optional reminder checkpoint only with explicit authorization**

If and only if authorized, stage D-030, the seven reminder prototype changes, the reminder script, and two validation files, then run:

```bash
git commit -m "test: prove native Android reminder delivery"
```

Without authorization, do not commit.

---

### Task 13: Test Supabase Against the Cloud and Protocol Conformance Gate

**Files:**
- Create: `docs/decisions/D-021-cloud-provider-and-conformance.md`
- Create: `prototypes/stage0/cloud/supabase/config.toml`
- Create: `prototypes/stage0/cloud/supabase/migrations/202609040001_g0_conformance.sql`
- Create: `prototypes/stage0/cloud/supabase/tests/database/g0_conformance.test.sql`
- Create: `prototypes/stage0/cloud/supabase/seed.sql`
- Create: `prototypes/stage0/cloud/qt_probe/qt_supabase_probe.pro`
- Create: `prototypes/stage0/cloud/qt_probe/main.cpp`
- Create: `prototypes/stage0/cloud/verify_cloud_results.py`
- Create: `docs/validation/stage-0/cloud-supabase-conformance.md`
- Create: `docs/validation/stage-0/cloud-local-tests.log`
- Create: `docs/validation/stage-0/cloud-conformance-results.json`

**Interfaces:**
- Consumes: D-022 protocol semantics, D-023 email login, D-026 retention/deletion constraints, Windows Qt Network, the Android `CloudProbe`, and disposable users A/B.
- Produces: one accepted Supabase region/plan with measured Windows/Android evidence, or `FAIL_STOP` with no provider-specific production directory.

- [ ] **Step 1: Define the isolated Supabase conformance schema**

Create prototype-only tables `g0_stream_state`, `g0_entities`, `g0_idempotency`, `g0_change_groups`, `g0_snapshot`, and `g0_deletion_requests`, all scoped by immutable `user_id` and module ID. Create RPCs `g0_sync_push(jsonb)`, `g0_sync_pull(text,text,integer)`, and `g0_sync_bootstrap(text,text,text,integer)`. Revoke anonymous/authenticated direct DML on internal tables; enable RLS as defense in depth; derive user identity from `auth.uid()`; lock one stream-state row before idempotency/epoch/revision checks and commit-cursor allocation.

- [ ] **Step 2: Add local pgTAP conformance assertions**

Write at least these 30 named assertions: anonymous denial; A own read/write through RPC; A cannot read/insert/update/delete B; forged `user_id` ignored/rejected; direct table DML denied; same mutation/digest replay; mutation-ID reuse rejected; revision conflict; tombstone write; atomic two-entity mutation; rollback on second-entity failure; commit-order cursor under reversed transaction completion; complete-group pull; no split at limit; empty-page cursor stability; payload upgrade rejection; old accepted mutation replay before epoch check; stale new mutation rejection; cursor expiry; snapshot identity binding; snapshot module binding; snapshot epoch binding; snapshot expiry; resumable page; stable high-water cursor; deletion blocks new sessions/writes; duplicate deletion request; deletion resume after injected failure; no secret role exposed; and business-table indexes include `user_id`.

- [ ] **Step 3: Run the local Supabase suite**

Run:

```bash
supabase start --workdir prototypes/stage0/cloud
supabase db reset --workdir prototypes/stage0/cloud
set -o pipefail
supabase test db --workdir prototypes/stage0/cloud \
  2>&1 | tee docs/validation/stage-0/cloud-local-tests.log
supabase stop --workdir prototypes/stage0/cloud
```

Expected: all 30 named pgTAP assertions pass, no skipped assertion, and no internal table is writable through ordinary client REST DML.

- [ ] **Step 4: Build the Windows Qt Auth/RPC probe**

`qt_supabase_probe` uses Qt 6.9.3 Core/Network only and accepts endpoint, publishable key, disposable email/password, network label, request count, and JSON output path at runtime. It signs in, validates the authenticated subject, refreshes the session, calls health and no-op RPC 30 times, exercises push/pull/bootstrap, verifies malformed JSON/type errors, signs out, and redacts tokens/passwords/transaction payloads. It must reject invalid TLS without an override.

Build with the frozen Windows Qt kit:

```powershell
New-Item build\stage0-cloud-qt -ItemType Directory -Force | Out-Null
Push-Location build\stage0-cloud-qt
& "$env:QT_DIR\bin\qmake.exe" ..\..\prototypes\stage0\cloud\qt_probe\qt_supabase_probe.pro CONFIG+=release
& "$env:MINGW_DIR\mingw32-make.exe"
Pop-Location
```

Expected: a Qt 6.9.3 MinGW probe executable with no provider SDK dependency.

- [ ] **Step 5: Provision disposable candidate projects and test accounts safely**

Provision test-only Supabase projects in Singapore and Tokyo when both regions are available. Create two confirmed email/password users through the provider dashboard or a maintainer-only script executed outside the client. Supply runtime values through `DA_G0_SUPABASE_URL`, `DA_G0_SUPABASE_PUBLISHABLE_KEY`, `DA_G0_USER_A_EMAIL`, `DA_G0_USER_A_PASSWORD`, `DA_G0_USER_B_EMAIL`, and `DA_G0_USER_B_PASSWORD`; keep service-role/database credentials outside the repository and delete disposable users/projects after evidence capture.

- [ ] **Step 6: Run tenant, protocol, export, deletion, and latency tests on real cloud projects**

Deploy the prototype migration to each candidate region and run the same isolation/protocol assertions remotely. From Windows home broadband, Android Wi-Fi, and Android mobile data, perform 30 health requests and 30 authenticated no-op RPC requests per region/network. Run user A/B allow/deny probes, same/different idempotency digest, reversed concurrent commit, bootstrap resume, dump/export, and deletion-resume drills. Save only status codes, error codes, region, network class, request counts, p50/p95 milliseconds, and pass/fail booleans in `cloud-conformance-results.json`.

Mandatory thresholds are at least 29 successes out of 30 per request class, health p95 at or below 1,500 ms, authenticated RPC p95 at or below 2,000 ms, zero TLS validation bypasses, complete A/B isolation, exact D-022 semantics, tested export, active-row deletion within 24 hours, provider-backup deletion cap at or below 30 days, and recurring monthly cost at or below CNY 250 for the 2-3-account deployment.

- [ ] **Step 7: Re-run Qt/C++ provider access from Android**

Use Task 11's `CloudProbe` against each still-eligible region on the physical ARM64 device over Wi-Fi and mobile data. It must sign in through HTTPS, call the typed probe RPC, refresh once, reject malformed response JSON, reject the self-signed endpoint, and emit only redacted timing/result JSON. Expected: the same availability and p95 thresholds as the Windows probe.

- [ ] **Step 8: Apply the deterministic provider/region decision rule**

Accept Supabase only if every mandatory security, protocol, backup/export/deletion, budget, Windows, and Android assertion passes. If both Singapore and Tokyo pass, select Singapore unless Tokyo's combined authenticated-RPC p95 is at least 20 percent lower across the three target networks; if only one passes, select that region. Record the exact region, provider plan, measured monthly price, backup retention, terms review date, project migration/export procedure, and API boundaries `IAuthClient`, `ISyncTransport`, and `IRemoteHealthCheck` in D-021.

If one region fails reachability, retain its evidence and test the other. If both fail or Supabase misses any non-network conformance requirement, set `FAIL_STOP`, do not create `cloud/supabase/`, and stop Stage 4 planning. The only next cloud action is a separately reviewed provider-reselection plan; Stage 1 also remains blocked by the master plan's ordered G0 gate until a managed provider passes the same D-022/D-026 suite.

- [ ] **Step 9: Verify evidence redaction and write the conformance record**

Run:

```bash
python3 prototypes/stage0/cloud/verify_cloud_results.py \
  --results docs/validation/stage-0/cloud-conformance-results.json \
  --require-networks windows-home android-wifi android-mobile \
  --health-p95-ms 1500 --rpc-p95-ms 2000 --minimum-successes 29
```

Expected: `SUPABASE CONFORMANCE PASS`, no JWT-shaped string, password, service-role key, email address, free-form transaction text, or full endpoint credential in tracked evidence. Create `cloud-supabase-conformance.md` with `Result: PASS`, selected region/plan, exact commands, all thresholds, RLS/RPC test count, measured results, cost/terms/backup/export/deletion evidence, and hashes.

- [ ] **Step 10: Use the optional cloud decision checkpoint only with explicit authorization**

If and only if authorized and Supabase passed, stage D-021, prototype SQL/tests/probes, and the three cloud evidence files, then run:

```bash
git commit -m "test: validate Supabase protocol conformance"
```

Do not stage credentials, generated local Supabase state, access tokens, database dumps, or disposable project identifiers that grant access. Without authorization, do not commit.

---

### Task 14: Reconcile Decisions, Seal G0 Evidence, and Hand Off to Stage 1

**Files:**
- Create: `prototypes/stage0/check_g0.py`
- Modify: `docs/product-architecture.md`
- Modify: `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md`
- Create: `docs/validation/stage-0/decision-consistency.md`
- Create: `docs/validation/stage-0/g0-evidence-index.md`
- Create: `docs/validation/stage-0/g0-results.json`

**Interfaces:**
- Consumes: D-020 through D-030 and every Stage 0 validation output.
- Produces: one machine-checked `G0 PASS` index and exact immutable inputs for `docs/superpowers/plans/2026-09-04-stage-1-cmake-and-boundaries.md`.

- [ ] **Step 1: Update the architecture decision status from measured outcomes**

Use the selected ADR outcomes to update `docs/product-architecture.md`: freeze the actual cloud provider/region, Android versions/matrix, WorkManager mode, reminder mechanism/SLA, CNY-only presentation, account formulas, short-month policy, login identifier/profile switch behavior, conflict UI, 180/210-day windows, import limits, QSQLITE mechanism, and 30-day deleted-backup cap. Mark D-011 and D-017 accepted only when their mandatory evidence passed; append D-020 through D-030 to section 24. Remove each resolved item from section 23 by replacing it with a link to its ADR, without weakening any already-confirmed requirement.

- [ ] **Step 2: Reconcile the master plan with the frozen baseline and toolchains**

Update the master's Tech Stack and Verified Starting Point with D-020's exact versions/revision and Task 1's actual tree inventory. If CMake/module files already existed in the frozen revision, identify them as pre-existing inputs requiring Stage 1 review rather than claiming they were absent or deleting them. Update G0 links to the evidence index and selected fallback, while leaving G1-G6 acceptance criteria unchanged unless a separately approved spec amendment requires it.

- [ ] **Step 3: Write a decision consistency matrix**

Create `decision-consistency.md` with one row for each D-020 through D-030, its selected outcome, validating command/result path, affected architecture sections, affected master-plan sections, and dependent stage. Add explicit rows proving that WorkManager fallback does not weaken D-017, CNY-only writes retain the currency field, account deletion remains distinct from local deletion/sign-out, and Supabase internals remain behind the three provider interfaces.

- [ ] **Step 4: Implement the G0 evidence checker**

`check_g0.py` must:

- Require all 11 ADR files and exactly one accepted selected outcome in each.
- Require `PASS` for baseline, Windows, DAT, import, sync model, Android runtime, native reminder, and cloud conformance.
- Allow `PASS_WITH_FALLBACK` only for Android WorkManager and QSQLITE backup, and require the corresponding selected fallback text/evidence.
- Parse both baseline logs for exactly `22 test(s) passed` and reject sanitizer diagnostics.
- Verify every SHA-256 manifest against current tracked fixture/evidence bytes and verify the Windows archive if its recorded storage path is present.
- Require sync seed `20260904`, 10,000 schedules, 18 tests, and zero invariant failures.
- Require Android API 28/API 35 x86_64 plus one physical ARM64 result, exact 64-bit money round trip, QSQLITE, backup integrity, valid HTTPS, invalid-certificate rejection, reminder `D0+89`/`D0+90`, excluded `D0+91`, offline reboot delivery, and no Qt reminder initialization.
- Require cloud isolation, protocol, network, latency, budget, export, deletion, and backup-retention thresholds.
- Scan tracked Stage 0 logs/JSON/markdown for JWT-shaped values, passwords, service-role keys, private denylist terms when the external denylist is supplied, emails in evidence, and raw transaction/import content.
- Require the architecture and master plan to name the same revision, provider, region, versions, retention numbers, and selected fallbacks as the ADRs.

- [ ] **Step 5: Generate the final gate result**

Run:

```bash
python3 prototypes/stage0/check_g0.py \
  --root . \
  --private-terms "$DA_PRIVATE_TERMS_FILE" \
  --json docs/validation/stage-0/g0-results.json
```

Expected stdout:

```text
G0 PASS: 11 decisions, 10 blocking validations, 22 baseline tests
```

Expected JSON: `gate` is `G0`, `result` is `PASS`, `decisionCount` is `11`, `blockingValidationCount` is `10`, `baselineTestCount` is `22`, and `failureCount` is `0`. Any other output prevents handoff.

- [ ] **Step 6: Create the human-readable G0 evidence index**

Create `g0-evidence-index.md` with the frozen revision, Windows ZIP hash/location, all ADR links, ten blocking validation links, Android matrix, selected provider/region/plan, selected QSQLITE backup path, selected WorkManager mode, selected reminder API, fixture hashes, G0 checker command/output, and `Gate result: PASS`. Include an `Exceptions` section containing only accepted, bounded fallbacks; if none were used, state `None`.

- [ ] **Step 7: Run final non-mutating repository checks**

Run:

```bash
git diff --check
git status --short
git diff -- docs/product-architecture.md \
  docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md \
  docs/decisions docs/validation/stage-0 prototypes/stage0 tests/fixtures
```

Expected: `git diff --check` is silent; status contains only the declared Stage 0 paths and any unrelated pre-existing user paths, which remain untouched; the diff contains no shipping implementation change, credential, private data, generated build output, or APK.

- [ ] **Step 8: Request independent gate review**

The reviewer must reproduce the strict/sanitizer tests, inspect Windows artifact hash and smoke record, rerun pure-Python validators/model/checker, compare Android raw JSON with device fingerprints, inspect pgTAP allow/deny coverage, and trace every D-020 through D-030 outcome to architecture/master text. Any unsupported assertion changes `Gate result` to `FAIL_STOP` until corrected.

- [ ] **Step 9: Use the optional final Stage 0 checkpoint only with explicit authorization**

If and only if the user authorizes a commit after independent review, inspect `git status`, `git diff`, and recent commits, stage only the declared Stage 0 files, and run:

```bash
git commit -m "docs: accept stage 0 baseline and prototypes"
```

Expected: one reviewed checkpoint with no secrets, private sources, build trees, APKs, or unrelated changes. Without authorization, leave all verified Stage 0 work uncommitted.

---

## G0 Checklist

- [ ] The baseline is one clean committed revision and both strict and ASan/UBSan runs report exactly `22 test(s) passed`.
- [ ] Qt 6.9.3/MinGW-w64 13.1 builds, packages, and starts the frozen Windows application from that same revision; the archive and manifest hashes are recorded.
- [ ] Sanitized V3 and legacy DAT fixtures load through the current parser, preserve migration invariants, reject checksum corruption, and contain no private term or identifier.
- [ ] At least three sanitized typed-note sources and 50 candidate-like lines cover all five import fixture families; normalization version 1 and every byte/item/field limit are fixed.
- [ ] D-023 fixes email login, immutable local/remote profile binding, offline-session semantics, sign-out choices, and the account-switch drain barrier.
- [ ] D-024 fixes CNY-only V1 writes, amount bounds, asset/credit balance signs, transfer/refund treatment, reconciliation difference, and monthly category basis-point formulas.
- [ ] D-025 fixes recurrence grammar, clamped month-end/leap behavior, stable keys, catch-up bound, lifecycle atomicity, reminder keys, and mistaken-confirmation undo/provenance behavior.
- [ ] D-026 fixes manual conflict resolution, whole-group quarantine, 180-day offline support, 210-day server retention, and the active/deleting/deleted account workflow with a 30-day backup cap.
- [ ] The sync reference model passes 18 named tests and 10,000 seed-`20260904` adversarial schedules with zero invariant failures.
- [ ] QSQLITE proves per-thread connection ownership, 5,000 ms busy bounds, queue drain/removal, constraints, consistent backup, interrupted-backup cleanup, and validated restore on Windows and Android.
- [ ] Android toolchain versions are frozen; API 28/API 35 x86_64 and physical ARM64 pass QML startup, exact 64-bit decimal-string round trip, QSQLITE/restart/backup, valid HTTPS, and invalid-certificate rejection.
- [ ] D-029 selects either proven cold-process Qt WorkManager execution or the tested Kotlin-only due marker plus startup/foreground compensation, without claiming guaranteed background convergence.
- [ ] Native Android reminders persist and deliver `D0+89` and `D0+90` events offline across normal reboot, exclude `D0+91` before replenishment, rebuild for time/time-zone/upgrade changes, expose permission health, and initialize no Qt runtime.
- [ ] Supabase passes local and remote tenant/protocol tests, Windows/Android target-network thresholds, export/deletion/backup requirements, and the CNY 250 monthly cap; D-021 names one region and plan.
- [ ] `docs/product-architecture.md`, the master plan, all 11 ADRs, and all evidence agree on revision, versions, provider, formulas, limits, retention windows, and selected fallbacks.
- [ ] `check_g0.py` prints exactly `G0 PASS: 11 decisions, 10 blocking validations, 22 baseline tests`, `git diff --check` is silent, and independent review accepts the evidence.
- [ ] No production code, private source data, credential, generated build tree, APK, unrelated user change, commit, or tag was introduced without explicit authorization.

## Stage 1 Handoff

Stage 1 may begin only when every G0 checkbox is checked and `docs/validation/stage-0/g0-results.json` says `PASS`. The Stage 1 executor must read `docs/product-architecture.md`, `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md`, `docs/validation/stage-0/g0-evidence-index.md`, D-020 through D-030, and then execute `docs/superpowers/plans/2026-09-04-stage-1-cmake-and-boundaries.md`; if that child plan is absent or contradicts a selected G0 outcome, stop and repair the Stage 1 plan before touching production code. Stage 1 inherits the frozen 22-test baseline, Windows package hash, exact toolchain matrix, CNY/formula contracts, profile state machine, provider-neutral sync interfaces/model, selected provider boundary, QSQLITE executor/backup mechanism, Android background mode, and native reminder contract without reopening those decisions informally.
