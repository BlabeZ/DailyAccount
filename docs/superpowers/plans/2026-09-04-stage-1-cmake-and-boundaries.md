# DailyAccount Stage 1 CMake and Architecture Boundaries Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the provisional build graph with a tested CMake build and stable C++17 application, platform, synchronization, reminder, and compile-time module boundaries while preserving all 22 DAT regressions and the current Windows Qt Widgets behavior.

**Architecture:** Keep `backend/` and `gui/` operational as an explicitly named legacy DAT vertical slice, rather than presenting them as the target accounting domain. Add dependency-inverted standard-C++ contracts under `src/`, use three explicit module registration entry points, and prove that shared targets cannot acquire UI, SQL, network, or platform implementation dependencies. Retire qmake only after the same revision builds, tests, packages, and starts through CMake on the frozen Windows toolchain.

**Tech Stack:** C++17, CMake 3.22.1+, CTest, GCC 11.4+ on Linux, Qt 6.9.3 Widgets with MinGW-w64 13.1 on Windows, Python 3 standard library for gate validation, PowerShell for Windows package smoke tests, and the existing Qt-free DAT backend.

**Spec:** `docs/product-architecture.md`; parent plan: `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md`; prerequisite evidence: `docs/validation/stage-0/g0-evidence-index.md`

## Global Constraints

- Begin no implementation task until the G0 entry procedure below prints exactly `G0 PASS: 11 decisions, 10 blocking validations, 22 baseline tests` and `docs/validation/stage-0/g0-results.json` records `gate=G0`, `result=PASS`, and `failureCount=0`.
- Read D-020 through D-030 before implementation. Their accepted revision, tool versions, provider boundary, CNY limits, profile state machine, recurrence rules, retention periods, QSQLITE mechanism, Android background mode, and reminder mechanism override assumptions in this plan when Stage 0 has updated the architecture accordingly.
- Preserve all unrelated worktree changes. Never stash, reset, clean, discard, or overwrite user work.
- Do not create a Git commit unless the user explicitly authorizes it in the execution session. Every commit step is optional and authorization-gated.
- Keep C++17 as the shared-language floor and CMake 3.22.1 as the minimum accepted CMake version.
- Keep Qt at exactly 6.9.3 and the accepted Windows compiler at MinGW-w64 13.1 for G1 package evidence.
- Keep all monetary values as signed 64-bit integer minor units. No shared contract may expose a financial amount as `double`, QML `int`, or JavaScript `Number`.
- Use client-generated UUIDs for synchronizable identities. Existing local integer IDs remain confined to the legacy DAT target and the future read-only importer.
- Domain, application, platform-interface, synchronization-contract, and core-registry targets must not link Qt Widgets, Qt Quick, QML, Qt SQL, Qt Network, JNI, Android APIs, Windows APIs, or a cloud-provider SDK.
- The application layer depends on repository and platform abstractions; repository, transport, notification, desktop, and mobile adapters depend inward on those abstractions.
- Module registration is explicit. Do not use global constructors, linker-section discovery, static self-registration, runtime plugins, plugin directories, ABI loading, or a plugin marketplace.
- Keep `registerAccountingCore(PlatformRegistry&)`, `registerAccountingDesktop(DesktopRegistry&)`, and `registerAccountingMobile(MobileRegistry&)` as separate entry points. Shared registration must not return or mention `QWidget`, QML, or Android types.
- Preserve `ledger.dat`, `ledger.dat.bak`, `records.dat`, and `categories.dat` behavior throughout Stage 1. This stage does not create SQLite schemas, open QSQLITE connections, migrate DAT data, or stop DAT writes.
- Do not create Android application/QML pages in this stage. `MobileRegistry` is only a standard-C++ route-description contract consumed by Stage 3.
- Do not implement authentication providers, cloud transports, synchronization algorithms, native reminder adapters, production repositories, or production unit-of-work adapters in this stage.
- Every behavior-changing task follows red-green-refactor: write a focused failing test, observe the specified failure, implement the smallest declared behavior, run focused and cumulative suites, and inspect the diff.
- Every task ends with `git diff --check`, the Linux core cumulative preset, and the unchanged `dailyaccount_backend_tests` summary of exactly `22 test(s) passed`. Windows-only tasks additionally run the Windows desktop preset.

---

## G0 Entry Gate

These checks happen before Task 1. A missing file, changed count, unsupported fallback, failed hash, or checker error stops Stage 1 without modifying production code.

- [ ] **Verify that all required G0 artifacts exist**

Run from the repository root:

```bash
test -f docs/validation/stage-0/g0-results.json
test -f docs/validation/stage-0/g0-evidence-index.md
for number in 020 021 022 023 024 025 026 027 028 029 030; do
  test -f "docs/decisions/D-${number}-"*.md
done
test -f prototypes/stage0/check_g0.py
```

Expected: every command exits `0`. If the shell cannot expand exactly one ADR for any number, stop and repair G0 evidence rather than selecting an outcome in Stage 1.

- [ ] **Re-run the authoritative G0 checker without replacing accepted evidence**

```bash
: "${DA_PRIVATE_TERMS_FILE:?DA_PRIVATE_TERMS_FILE must name the external private-term denylist}"
python3 prototypes/stage0/check_g0.py \
  --root . \
  --private-terms "$DA_PRIVATE_TERMS_FILE" \
  --json /tmp/opencode/dailyaccount-stage1-g0-recheck.json
```

Expected stdout:

```text
G0 PASS: 11 decisions, 10 blocking validations, 22 baseline tests
```

- [ ] **Compare the accepted and fresh machine-readable outcomes**

```bash
python3 - <<'PY'
import json
from pathlib import Path

accepted = json.loads(Path("docs/validation/stage-0/g0-results.json").read_text(encoding="utf-8"))
fresh = json.loads(Path("/tmp/opencode/dailyaccount-stage1-g0-recheck.json").read_text(encoding="utf-8"))
keys = ("gate", "result", "decisionCount", "blockingValidationCount", "baselineTestCount", "failureCount")
expected = ("G0", "PASS", 11, 10, 22, 0)
assert tuple(accepted[key] for key in keys) == expected
assert tuple(fresh[key] for key in keys) == expected
print("Stage 1 entry gate: PASS")
PY
```

Expected: `Stage 1 entry gate: PASS` and exit code `0`.

- [ ] **Read the accepted decisions and preserve the starting worktree**

```bash
git status --short
git diff --check
```

Read `docs/product-architecture.md`, the parent plan, `g0-evidence-index.md`, and D-020 through D-030. Record pre-existing changed and untracked paths in the execution notes. In particular, treat any pre-existing `CMakeLists.txt`, `platform/`, `modules/`, or `tests/registry_tests.*` content as provisional input that must pass the tasks below; it is not G1 evidence by existence alone.

---

## Deliverables and File Map

Stage 1 owns the following change surface. Generated build trees and deployed DLLs remain untracked.

```text
CMakeLists.txt
CMakePresets.json
README.md
cmake/
  DailyAccountOptions.cmake
  DailyAccountTests.cmake
  DailyAccountWarnings.cmake
build/
  build.bat
  build-cmake.bat                         # Temporary parity runner, deleted after qmake retirement.
src/
  core/
    domain/
      result.h
      uuid.h
      uuid.cpp
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
    notifications/
      reminder_event.h
    sync/
      sync_contract.h
    modules/
      module_descriptor.h
      platform_registry.h
      platform_registry.cpp
      desktop_registry.h
      desktop_registry.cpp
      mobile_registry.h
      mobile_registry.cpp
  modules/
    accounting/
      domain/
        entity_metadata.h
        transaction.h
        account.h
        category.h
        recurring.h
        import.h
      application/
        accounting_repositories.h
        accounting_service.h
        accounting_service.cpp
      accounting_module.h
      accounting_module.cpp
      accounting_desktop_module.cpp
      accounting_mobile_module.cpp
  apps/
    desktop-widgets/
      register_modules.h
      register_modules.cpp
tests/
  support/
    test_harness.h
    temporary_directory.h
  unit/
    test_support_tests.cpp
    result_tests.cpp
    uuid_tests.cpp
    date_money_tests.cpp
    accounting_contract_tests.cpp
    accounting_application_tests.cpp
    platform_contract_tests.cpp
  registry_tests.cpp
  platform_ui_registry_tests.cpp
  widgets/
    widgets_contract_tests.cpp
  cmake/
    build_graph_contract.cmake
    architecture_boundary_contract.cmake
    check_g1.py
    test_check_g1.py
  windows/
    verify_widgets_package.ps1
docs/
  validation/
    stage-1/
      linux-core.log
      linux-sanitizers.log
      linux-core-results.json
      architecture-results.json
      windows-cmake.log
      windows-ctest.log
      windows-package-files.txt
      windows-package.sha256
      windows-cmake-results.json
      windows-parity-results.json
      source-tree.txt
      g1-evidence-index.md
      g1-results.json
```

The following provisional or qmake paths are removed only in the tasks that name them:

```text
platform/module_descriptor.h
platform/module_registry.h
platform/module_registry.cpp
modules/accounting/accounting_module.h
modules/accounting/accounting_module.cpp
jizhang.pro
tests/backend_tests.pro
tests/registry_tests.pro
```

### Target Graph

The final Stage 1 graph is:

```text
dailyaccount_core_domain
  <- dailyaccount_accounting_domain
  <- dailyaccount_platform_interfaces

dailyaccount_accounting_domain
  <- dailyaccount_accounting_storage_contracts
  <- dailyaccount_accounting_application
  <- dailyaccount_accounting_sync

dailyaccount_platform_interfaces
  <- dailyaccount_platform_modules
  <- dailyaccount_desktop_registry
  <- dailyaccount_mobile_registry

dailyaccount_platform_modules + dailyaccount_accounting_domain
  <- dailyaccount_accounting_module

dailyaccount_desktop_registry + dailyaccount_accounting_module
  <- dailyaccount_accounting_desktop_registration
  <- dailyaccount_desktop_composition

dailyaccount_legacy_backend
  <- dailyaccount_legacy_widgets
  <- dailyaccount_desktop
```

`dailyaccount_accounting_sqlite` is intentionally absent until Stage 2 supplies a production repository and unit-of-work adapter. `dailyaccount_android` is intentionally absent until Stage 3 supplies Qt Quick/QML sources. The contract targets `dailyaccount_accounting_storage_contracts`, `dailyaccount_accounting_sync`, and `dailyaccount_mobile_registry` preserve those extension points without claiming that later-stage implementations exist.

---

### Task 1: Establish CMake Presets, Strict Warnings, and the Legacy DAT Target

**Files:**
- Create: `CMakePresets.json`
- Create: `cmake/DailyAccountOptions.cmake`
- Create: `cmake/DailyAccountWarnings.cmake`
- Create: `tests/cmake/build_graph_contract.cmake`
- Modify: `CMakeLists.txt`
- Test: `tests/cmake/build_graph_contract.cmake`
- Preserve unchanged: `backend/record.h`, `backend/category.*`, `backend/storage.*`, `backend/ledger.*`, `tests/backend_tests.cpp`

**Interfaces:**
- Consumes: C++17, CMake 3.22.1+, GCC 11.4+, the frozen 22-test DAT baseline, and the accepted Qt/MinGW paths from D-020.
- Produces: options `DA_BUILD_DESKTOP` and `DA_ENABLE_SANITIZERS`; function `da_enable_strict_warnings(target)`; presets `linux-core`, `linux-core-sanitized`, and `windows-desktop`; target `dailyaccount_legacy_backend`; executable `dailyaccount_backend_tests`.

- [ ] **Step 1: Write the failing build-graph contract**

Create `tests/cmake/build_graph_contract.cmake` with exact textual assertions:

```cmake
if(NOT DEFINED DA_SOURCE_DIR)
    message(FATAL_ERROR "DA_SOURCE_DIR is required")
endif()

file(READ "${DA_SOURCE_DIR}/CMakeLists.txt" root_cmake)
foreach(required
        "cmake_minimum_required(VERSION 3.22.1)"
        "include(DailyAccountOptions)"
        "include(DailyAccountWarnings)"
        "add_library(dailyaccount_legacy_backend"
        "target_link_libraries(dailyaccount_backend_tests PRIVATE dailyaccount_legacy_backend)")
    string(FIND "${root_cmake}" "${required}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "missing build contract: ${required}")
    endif()
endforeach()

string(FIND "${root_cmake}" "add_compile_options(" global_warnings)
if(NOT global_warnings EQUAL -1)
    message(FATAL_ERROR "warnings must be target-scoped")
endif()

string(FIND "${root_cmake}" "dailyaccount_accounting_domain STATIC\n    backend/" mislabeled_legacy)
if(NOT mislabeled_legacy EQUAL -1)
    message(FATAL_ERROR "legacy DAT sources are mislabeled as the target domain")
endif()
```

- [ ] **Step 2: Run the contract and observe the current mismatch**

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/build_graph_contract.cmake
```

Expected: non-zero exit with `missing build contract: cmake_minimum_required(VERSION 3.22.1)` against the provisional root file.

- [ ] **Step 3: Add target-scoped options and warnings**

Implement `cmake/DailyAccountOptions.cmake`:

```cmake
option(DA_BUILD_DESKTOP "Build the Qt Widgets desktop application" OFF)
option(DA_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(DA_ENABLE_SANITIZERS AND MSVC)
    message(FATAL_ERROR "DA_ENABLE_SANITIZERS is supported only by GCC and Clang in Stage 1")
endif()
```

Implement `cmake/DailyAccountWarnings.cmake`:

```cmake
function(da_enable_strict_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    if(DA_ENABLE_SANITIZERS)
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    endif()
endfunction()
```

- [ ] **Step 4: Replace the provisional root graph with an honest legacy target**

Use this foundation in `CMakeLists.txt` and keep the provisional registry target temporarily until Task 9 relocates it:

```cmake
cmake_minimum_required(VERSION 3.22.1)
project(DailyAccount VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(DailyAccountOptions)
include(DailyAccountWarnings)
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(dailyaccount_legacy_backend STATIC
    backend/category.cpp
    backend/storage.cpp
    backend/ledger.cpp)
target_include_directories(dailyaccount_legacy_backend PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/backend")
da_enable_strict_warnings(dailyaccount_legacy_backend)

add_executable(dailyaccount_backend_tests tests/backend_tests.cpp)
target_link_libraries(dailyaccount_backend_tests PRIVATE dailyaccount_legacy_backend)
da_enable_strict_warnings(dailyaccount_backend_tests)
add_test(NAME dailyaccount_backend_tests COMMAND dailyaccount_backend_tests)

# Preserve the already-written registry experiment until Task 9 replaces it.
add_library(dailyaccount_platform_interfaces INTERFACE)
target_include_directories(dailyaccount_platform_interfaces INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/accounting")
add_library(dailyaccount_accounting_module STATIC
    platform/module_registry.cpp
    modules/accounting/accounting_module.cpp)
target_link_libraries(dailyaccount_accounting_module PUBLIC
    dailyaccount_platform_interfaces)
target_include_directories(dailyaccount_accounting_module PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules/accounting")
da_enable_strict_warnings(dailyaccount_accounting_module)

add_executable(dailyaccount_registry_tests tests/registry_tests.cpp)
target_link_libraries(dailyaccount_registry_tests PRIVATE
    dailyaccount_accounting_module)
da_enable_strict_warnings(dailyaccount_registry_tests)
add_test(NAME dailyaccount_registry_tests COMMAND dailyaccount_registry_tests)

add_test(NAME dailyaccount_build_graph_contract
    COMMAND "${CMAKE_COMMAND}"
        -DDA_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/build_graph_contract.cmake)
```

Do not rename files under `backend/` or alter their behavior. Their CMake target name communicates that DAT remains the migration baseline, not the target repository/domain implementation.

- [ ] **Step 5: Add reproducible configure, build, and test presets**

Create schema version 3 `CMakePresets.json` with these preset names and values:

```json
{
  "version": 3,
  "cmakeMinimumRequired": { "major": 3, "minor": 22, "patch": 1 },
  "configurePresets": [
    {
      "name": "linux-core",
      "generator": "Unix Makefiles",
      "binaryDir": "${sourceDir}/build/cmake/linux-core",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "DA_BUILD_DESKTOP": "OFF",
        "DA_ENABLE_SANITIZERS": "OFF"
      },
      "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Linux" }
    },
    {
      "name": "linux-core-sanitized",
      "inherits": "linux-core",
      "binaryDir": "${sourceDir}/build/cmake/linux-core-sanitized",
      "cacheVariables": { "DA_ENABLE_SANITIZERS": "ON" }
    },
    {
      "name": "windows-desktop",
      "generator": "MinGW Makefiles",
      "binaryDir": "${sourceDir}/build/cmake/windows-desktop",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "CMAKE_PREFIX_PATH": "$env{QT_DIR}",
        "DA_BUILD_DESKTOP": "ON",
        "DA_ENABLE_SANITIZERS": "OFF"
      },
      "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows" }
    }
  ],
  "buildPresets": [
    { "name": "linux-core", "configurePreset": "linux-core" },
    { "name": "linux-core-sanitized", "configurePreset": "linux-core-sanitized" },
    { "name": "windows-desktop", "configurePreset": "windows-desktop" }
  ],
  "testPresets": [
    { "name": "linux-core", "configurePreset": "linux-core", "output": { "outputOnFailure": true } },
    { "name": "linux-core-sanitized", "configurePreset": "linux-core-sanitized", "output": { "outputOnFailure": true } },
    { "name": "windows-desktop", "configurePreset": "windows-desktop", "output": { "outputOnFailure": true } }
  ]
}
```

- [ ] **Step 6: Run the focused and cumulative green checks**

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/build_graph_contract.cmake
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: the contract exits `0`, configure/build exit `0`, CTest reports `100% tests passed`, and the direct legacy suite ends with exactly `22 test(s) passed`.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt CMakePresets.json cmake/DailyAccountOptions.cmake \
  cmake/DailyAccountWarnings.cmake tests/cmake/build_graph_contract.cmake
git commit -m "build: establish cmake legacy baseline"
```

Without authorization, leave the verified changes uncommitted.

---

### Task 2: Extract Reusable Qt-Free Test Support

**Files:**
- Create: `cmake/DailyAccountTests.cmake`
- Create: `tests/support/test_harness.h`
- Create: `tests/support/temporary_directory.h`
- Create: `tests/unit/test_support_tests.cpp`
- Modify: `tests/backend_tests.cpp`
- Modify: `tests/registry_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/test_support_tests.cpp`

**Interfaces:**
- Consumes: `da_enable_strict_warnings(target)` and existing test main functions.
- Produces: `dailyaccount::test::Failure`, `DA_CHECK(expression)`, `DA_CHECK_EQ(actual, expected)`, `dailyaccount::test::runTests(std::initializer_list<TestCase>)`, `TemporaryDirectory::path()`, `writeText(path, content)`, and `readText(path)`; CMake function `da_add_core_test(target source)`.

- [ ] **Step 1: Change one test to include the absent support API**

Create `tests/unit/test_support_tests.cpp` first:

```cpp
#include "support/test_harness.h"
#include "support/temporary_directory.h"

#include <filesystem>

using dailyaccount::test::TemporaryDirectory;

void temporaryDirectoryOwnsAndRemovesItsPath()
{
    std::filesystem::path path;
    {
        TemporaryDirectory directory("support");
        path = directory.path();
        DA_CHECK(std::filesystem::is_directory(path));
        dailyaccount::test::writeText(path / "value.txt", "exact\n");
        DA_CHECK_EQ(dailyaccount::test::readText(path / "value.txt"), "exact\n");
    }
    DA_CHECK(!std::filesystem::exists(path));
}

int main()
{
    return dailyaccount::test::runTests({
        {"temporary directory owns and removes its path", temporaryDirectoryOwnsAndRemovesItsPath},
    });
}
```

- [ ] **Step 2: Verify the new test fails because support is absent**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Itests \
  tests/unit/test_support_tests.cpp -o /tmp/opencode/dailyaccount_test_support_tests
```

Expected: compilation fails with `support/test_harness.h: No such file or directory`.

- [ ] **Step 3: Implement the shared harness and temporary directory**

`tests/support/test_harness.h` must provide this public surface and print the same summary format used by the current suites:

```cpp
namespace dailyaccount::test {

class Failure : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct TestCase {
    std::string name;
    std::function<void()> function;
};

void check(bool condition, const char* expression, const char* file, int line);
int runTests(std::initializer_list<TestCase> tests);

}

#define DA_CHECK(expression) \
    ::dailyaccount::test::check(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
#define DA_CHECK_EQ(actual, expected) DA_CHECK((actual) == (expected))
```

`runTests` prints `PASS: <name>` or `FAIL: <name> - <message>`, returns `1` if any test failed, and otherwise prints `<count> test(s) passed` and returns `0`. `TemporaryDirectory` has `explicit TemporaryDirectory(std::string_view suite = "core")`, is non-copyable, creates a collision-resistant child of `std::filesystem::temp_directory_path()`, removes only that child in its destructor, and throws `Failure` on creation, read, or write failure.

- [ ] **Step 4: Refactor existing suites without changing test cases or counts**

Replace the duplicate `TestFailure`, `CHECK`, runner, `TempDirectory`, `writeText`, and `readText` definitions in `tests/backend_tests.cpp` and `tests/registry_tests.cpp` with:

```cpp
#include "support/test_harness.h"
#include "support/temporary_directory.h"

#define CHECK(expression) DA_CHECK(expression)
using TempDirectory = dailyaccount::test::TemporaryDirectory;
using dailyaccount::test::readText;
using dailyaccount::test::writeText;
```

Make each `main` return `dailyaccount::test::runTests` with an initializer list containing every existing named case. Keep all 22 backend test names and all 9 provisional registry test names unchanged in this task.

- [ ] **Step 5: Add the CMake test helper and support test target**

Implement `cmake/DailyAccountTests.cmake`:

```cmake
function(da_add_core_test target source)
    add_executable(${target} ${source})
    target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/tests")
    da_enable_strict_warnings(${target})
    add_test(NAME ${target} COMMAND ${target})
endfunction()
```

Include it from the root and replace repeated executable/warning/test setup. Link `dailyaccount_backend_tests` to `dailyaccount_legacy_backend`, then add `dailyaccount_test_support_tests` from `tests/unit/test_support_tests.cpp`.

- [ ] **Step 6: Run focused and cumulative tests**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_test_support_tests
./build/cmake/linux-core/dailyaccount_backend_tests
./build/cmake/linux-core/dailyaccount_registry_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: support reports `1 test(s) passed`, backend reports exactly `22 test(s) passed`, provisional registry reports exactly `9 test(s) passed`, and CTest reports no failures.

- [ ] **Step 7: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt cmake/DailyAccountTests.cmake tests/support \
  tests/unit/test_support_tests.cpp tests/backend_tests.cpp tests/registry_tests.cpp
git commit -m "test: share core test support"
```

Without authorization, do not commit.

---

### Task 3: Add the Shared Result and Error Contract

**Files:**
- Create: `src/core/domain/result.h`
- Create: `src/core/application/accounting_error.h`
- Create: `tests/unit/result_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/result_tests.cpp`

**Interfaces:**
- Consumes: standard C++17 only.
- Produces: `AccountingErrorCode`, `AccountingError`, `Result<T>`, and `Result<void>` exactly as consumed by all later Stage 1 contracts.

- [ ] **Step 1: Write success, failure, and invalid-access tests**

Use these assertions in `tests/unit/result_tests.cpp`:

```cpp
#include "core/domain/result.h"
#include "support/test_harness.h"

using namespace dailyaccount;

void valueResultExposesOnlyItsValue()
{
    const auto result = Result<int>::success(42);
    DA_CHECK(result.hasValue());
    DA_CHECK_EQ(result.value(), 42);
}

void failureResultExposesStructuredError()
{
    const auto result = Result<int>::failure(
        {AccountingErrorCode::InvalidArgument, "amount is invalid"});
    DA_CHECK(!result.hasValue());
    DA_CHECK_EQ(result.error().code, AccountingErrorCode::InvalidArgument);
    DA_CHECK_EQ(result.error().message, "amount is invalid");
}

void voidResultPreservesFailure()
{
    const auto result = Result<void>::failure(
        {AccountingErrorCode::StorageFailure, "write failed"});
    DA_CHECK(!result.hasValue());
    DA_CHECK_EQ(result.error().code, AccountingErrorCode::StorageFailure);
}
```

Also test that `value()` on a failure and `error()` on a success throw `std::logic_error` with `Result has no value` and `Result has no error`, respectively.

- [ ] **Step 2: Verify compilation fails before the contract exists**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/unit/result_tests.cpp -o /tmp/opencode/dailyaccount_result_tests
```

Expected: compilation fails with `core/domain/result.h: No such file or directory`.

- [ ] **Step 3: Implement the exact result contract**

Define these names in `src/core/domain/result.h`:

```cpp
namespace dailyaccount {

enum class AccountingErrorCode {
    InvalidArgument,
    NotFound,
    DomainConstraint,
    OccurrenceLinked,
    RefundLimitExceeded,
    DuplicateImport,
    RecurringCatchUpLimitExceeded,
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
    bool hasValue() const noexcept;
    const T& value() const;
    T& value();
    const AccountingError& error() const;

private:
    explicit Result(std::variant<T, AccountingError> state);
    std::variant<T, AccountingError> state_;
};

template <>
class Result<void> {
public:
    static Result success();
    static Result failure(AccountingError error);
    bool hasValue() const noexcept;
    const AccountingError& error() const;

private:
    explicit Result(std::optional<AccountingError> error);
    std::optional<AccountingError> error_;
};

}
```

Keep the implementation inline in the header. `src/core/application/accounting_error.h` contains only `#pragma once` and `#include "core/domain/result.h"`; this preserves the master-plan include path without making domain code depend outward on application code.

- [ ] **Step 4: Add the core target and focused test**

Add `dailyaccount_core_domain` as a static library containing the later `uuid.cpp`; until Task 4 creates that source, use an `INTERFACE` library, link the result test to it, and switch it to `STATIC` in Task 4. Its public include directory is exactly `${PROJECT_SOURCE_DIR}/src`.

- [ ] **Step 5: Run focused and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_result_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: result tests report `5 test(s) passed`, backend reports `22 test(s) passed`, and CTest has zero failures.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/core/domain/result.h \
  src/core/application/accounting_error.h tests/unit/result_tests.cpp
git commit -m "feat: add structured result contract"
```

Without authorization, do not commit.

---

### Task 4: Add Typed UUIDv4 and UUIDv5 Identities

**Files:**
- Create: `src/core/domain/uuid.h`
- Create: `src/core/domain/uuid.cpp`
- Create: `tests/unit/uuid_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/uuid_tests.cpp`

**Interfaces:**
- Consumes: `Result<T>` and `AccountingErrorCode::InvalidArgument`.
- Produces: non-interchangeable `StrongUuid<Tag>` aliases, canonical parsing/string formatting, random RFC 4122 version-4 IDs, deterministic RFC 4122 version-5 IDs, equality, ordering, and `uuidV5<OutputTag>(namespaceId, name)` for cross-tag deterministic identities.

- [ ] **Step 1: Write canonical, random, type-safety, and RFC-vector tests**

The focused suite must include:

```cpp
using DnsId = dailyaccount::StrongUuid<struct DnsIdTag>;
using OutputId = dailyaccount::StrongUuid<struct OutputIdTag>;

void parsesOnlyCanonicalUuidText()
{
    const auto parsed = DnsId::parse("6ba7b810-9dad-11d1-80b4-00c04fd430c8");
    DA_CHECK(parsed.hasValue());
    DA_CHECK_EQ(parsed.value().toString(), "6ba7b810-9dad-11d1-80b4-00c04fd430c8");
    DA_CHECK(!DnsId::parse("6BA7B810-9DAD-11D1-80B4-00C04FD430C8").hasValue());
    DA_CHECK(!DnsId::parse("not-a-uuid").hasValue());
}

void uuidV5MatchesRfcVector()
{
    const auto dns = DnsId::parse("6ba7b810-9dad-11d1-80b4-00c04fd430c8").value();
    const auto generated = dailyaccount::uuidV5<OutputIdTag>(dns, "www.widgets.com");
    DA_CHECK_EQ(generated.toString(), "21f7f8de-8051-5b89-8680-0195ef798b6a");
}

static_assert(!std::is_convertible_v<dailyaccount::UserId, dailyaccount::TransactionId>);
```

Also test UUIDv4 version/variant bits, equal UUIDv5 inputs, unequal UUIDv5 names, and all aliases listed below.

- [ ] **Step 2: Verify the focused test fails before implementation**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/unit/uuid_tests.cpp src/core/domain/uuid.cpp \
  -o /tmp/opencode/dailyaccount_uuid_tests
```

Expected: compilation fails because `uuid.h` and `uuid.cpp` do not exist.

- [ ] **Step 3: Implement the typed public surface**

`src/core/domain/uuid.h` must expose:

```cpp
namespace dailyaccount {

template <typename Tag>
class StrongUuid {
public:
    static Result<StrongUuid> parse(std::string_view text);
    static StrongUuid random();
    static StrongUuid v5(const StrongUuid& namespaceId, std::string_view name);
    std::string toString() const;
    friend bool operator==(const StrongUuid& left, const StrongUuid& right) noexcept
    {
        return left.bytes_ == right.bytes_;
    }
    friend bool operator!=(const StrongUuid& left, const StrongUuid& right) noexcept
    {
        return !(left == right);
    }
    friend bool operator<(const StrongUuid& left, const StrongUuid& right);

private:
    explicit StrongUuid(std::array<std::uint8_t, 16> bytes);
    std::array<std::uint8_t, 16> bytes_;
    template <typename OutputTag, typename NamespaceTag>
    friend StrongUuid<OutputTag> uuidV5(
        const StrongUuid<NamespaceTag>&, std::string_view);
};

template <typename OutputTag, typename NamespaceTag>
StrongUuid<OutputTag> uuidV5(
    const StrongUuid<NamespaceTag>& namespaceId,
    std::string_view name);

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

}
```

Put hex decoding, canonical validation, random-byte generation, SHA-1, version/variant bit setting, and formatting in non-template helpers implemented by `uuid.cpp`. Reject braces, uppercase hex, missing hyphens, non-hex bytes, and noncanonical lengths. UUIDv4 randomness is for collision resistance, not secret generation.

- [ ] **Step 4: Convert `dailyaccount_core_domain` to a compiled library**

```cmake
add_library(dailyaccount_core_domain STATIC src/core/domain/uuid.cpp)
target_include_directories(dailyaccount_core_domain PUBLIC "${PROJECT_SOURCE_DIR}/src")
da_enable_strict_warnings(dailyaccount_core_domain)
```

Link result and UUID tests to this target.

- [ ] **Step 5: Run focused and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_uuid_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: UUID tests report `6 test(s) passed`, backend reports `22 test(s) passed`, and CTest has no failures.

- [ ] **Step 6: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/core/domain/uuid.h src/core/domain/uuid.cpp \
  tests/unit/uuid_tests.cpp
git commit -m "feat: add typed uuid identities"
```

Without authorization, do not commit.

---

### Task 5: Add Exact Money, Currency, Date, and Time Values

**Files:**
- Create: `src/core/domain/money.h`
- Create: `src/core/domain/date_time.h`
- Create: `tests/unit/date_money_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/date_money_tests.cpp`

**Interfaces:**
- Consumes: D-024's accepted CNY-only write limit and `Result<T>`.
- Produces: `MoneyMinor`, `CurrencyCode`, `LocalDate`, `LocalTime`, `UtcInstant`, `parseCurrencyCode`, `makeLocalDate`, `makeLocalTime`, `validatePositiveAmount`, `checkedAddMoney`, and `checkedSubtractMoney`.

- [ ] **Step 1: Write exact-boundary tests**

Cover these named cases in `tests/unit/date_money_tests.cpp`:

```cpp
void cnyIsAcceptedAndOtherOrMalformedCodesAreRejected();
void positiveAmountsAcceptOneAnd9999999999();
void positiveAmountsRejectZeroNegativeAnd10000000000();
void checkedMoneyArithmeticRejectsSignedOverflow();
void localDateAccepts01000101And99991231();
void localDateUsesGregorianLeapYearRules();
void localDateRejectsImpossibleDates();
void localTimeAccepts235959AndRejects240000();
void utcInstantPreservesNegativeAndPositiveEpochMilliseconds();
void valueTypesUseNoFloatingPointFields();
```

The CNY test must compare `CurrencyCode{{'C', 'N', 'Y'}}`; arithmetic tests must exercise both `INT64_MAX + 1` and `INT64_MIN - 1`.

- [ ] **Step 2: Verify the focused test fails before implementation**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/unit/date_money_tests.cpp -o /tmp/opencode/dailyaccount_date_money_tests
```

Expected: compilation fails because `core/domain/money.h` is absent.

- [ ] **Step 3: Implement the exact standard-C++ value contracts**

Use these public declarations:

```cpp
namespace dailyaccount {

using MoneyMinor = std::int64_t;
inline constexpr MoneyMinor kMaximumV1AmountMinor = 9'999'999'999;

struct CurrencyCode {
    std::array<char, 3> value;
    friend bool operator==(const CurrencyCode& left, const CurrencyCode& right) noexcept
    {
        return left.value == right.value;
    }
};

Result<CurrencyCode> parseCurrencyCode(std::string_view text);
Result<MoneyMinor> validatePositiveAmount(MoneyMinor amount);
Result<MoneyMinor> checkedAddMoney(MoneyMinor left, MoneyMinor right);
Result<MoneyMinor> checkedSubtractMoney(MoneyMinor left, MoneyMinor right);

struct LocalDate {
    std::int32_t year;
    std::uint8_t month;
    std::uint8_t day;
    friend bool operator==(const LocalDate& left, const LocalDate& right) noexcept
    {
        return left.year == right.year && left.month == right.month && left.day == right.day;
    }
};

struct LocalTime {
    std::uint8_t hour;
    std::uint8_t minute;
    std::uint8_t second;
    friend bool operator==(const LocalTime& left, const LocalTime& right) noexcept
    {
        return left.hour == right.hour && left.minute == right.minute && left.second == right.second;
    }
};

struct UtcInstant {
    std::int64_t epochMilliseconds;
    friend bool operator==(const UtcInstant& left, const UtcInstant& right) noexcept
    {
        return left.epochMilliseconds == right.epochMilliseconds;
    }
};

Result<LocalDate> makeLocalDate(std::int32_t year, std::uint8_t month, std::uint8_t day);
Result<LocalTime> makeLocalTime(std::uint8_t hour, std::uint8_t minute, std::uint8_t second);

}
```

Implement these functions inline. Accept years `100..9999`; apply Gregorian divisibility rules; accept only `CNY` for V1 writes; never convert through floating point.

- [ ] **Step 4: Add and run the focused CMake target**

Link `dailyaccount_date_money_tests` to `dailyaccount_core_domain`, then run:

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_date_money_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: value tests report `10 test(s) passed`, backend reports `22 test(s) passed`, and CTest has zero failures.

- [ ] **Step 5: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/core/domain/money.h src/core/domain/date_time.h \
  tests/unit/date_money_tests.cpp
git commit -m "feat: add exact shared value types"
```

Without authorization, do not commit.

---

### Task 6: Freeze the Accounting Domain Data Contracts

**Files:**
- Create: `src/modules/accounting/domain/entity_metadata.h`
- Create: `src/modules/accounting/domain/transaction.h`
- Create: `src/modules/accounting/domain/account.h`
- Create: `src/modules/accounting/domain/category.h`
- Create: `src/modules/accounting/domain/recurring.h`
- Create: `src/modules/accounting/domain/import.h`
- Create: `tests/unit/accounting_contract_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/accounting_contract_tests.cpp`

**Interfaces:**
- Consumes: `StrongUuid` aliases, `MoneyMinor`, `CurrencyCode`, date/time values, D-024, D-025, and D-027.
- Produces: standard-C++ accounting entity contracts with server revision separate from local dirty/in-flight state; no persistence or UI behavior.

- [ ] **Step 1: Write compile-time and value-preservation tests**

Tests must prove:

```cpp
static_assert(std::is_same_v<decltype(dailyaccount::Transaction::amountMinor), dailyaccount::MoneyMinor>);
static_assert(std::is_same_v<decltype(dailyaccount::EntityMetadata::serverRevision), std::uint64_t>);
static_assert(!std::is_convertible_v<dailyaccount::TransactionId, dailyaccount::AccountId>);
static_assert(std::is_same_v<decltype(dailyaccount::RecurringOccurrence::transactionId),
                             std::optional<dailyaccount::TransactionId>>);
```

Add runtime tests constructing an expense, transfer, refund, recurring occurrence, import item, and provenance relation. Assert every assigned field round-trips unchanged, including an amount above 32-bit range, `serverRevision=0`, a tombstone, `periodKey`, `fingerprint`, and `detachedAt`.

- [ ] **Step 2: Verify compilation fails before domain headers exist**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/unit/accounting_contract_tests.cpp src/core/domain/uuid.cpp \
  -o /tmp/opencode/dailyaccount_accounting_contract_tests
```

Expected: compilation fails at the first missing `modules/accounting/domain/` include.

- [ ] **Step 3: Implement metadata and transaction declarations**

Use the master-plan fields exactly:

```cpp
namespace dailyaccount {

struct EntityMetadata {
    UserId userId;
    UtcInstant createdAt;
    UtcInstant updatedAt;
    std::uint64_t serverRevision;
    std::optional<UtcInstant> deletedAt;
    DeviceId modifiedByDeviceId;
};

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

}
```

- [ ] **Step 4: Implement account, category, and tag declarations**

Use these declarations and keep all fields as public standard-C++ data:

```cpp
namespace dailyaccount {

enum class AccountType { Cash, BankCard, ElectronicWallet, Credit, Other };

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

enum class CategoryApplicability { Income, Expense, Both };

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

}
```

- [ ] **Step 5: Implement recurring declarations**

```cpp
namespace dailyaccount {

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
    std::uint8_t leadDays;
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

}
```

The enum retains `MoveToNextMonth` for wire/schema compatibility, while D-025 allows only `ClampToLastDay` in V1 commands.

- [ ] **Step 6: Implement import and provenance declarations**

Use these complete declarations:

```cpp
namespace dailyaccount {

enum class ImportBatchStatus { Draft, Committed, Discarded };
enum class ImportConfidence { High, NeedsReview, Invalid };

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

enum class ProvenanceKind { TextImport, BillImport, Recurring };

struct TransactionProvenance {
    UserId userId;
    TransactionId transactionId;
    ProvenanceKind kind;
    std::string externalKey;
    std::uint32_t sourceVersion;
    std::optional<UtcInstant> detachedAt;
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

}
```

No implementation of `IPaymentSignalSource` is created in V1, and this contract has no method that posts a transaction.

- [ ] **Step 7: Add the accounting domain target and run tests**

Create `dailyaccount_accounting_domain` as an `INTERFACE` target that exposes `src` and links only `dailyaccount_core_domain`. Then run:

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_accounting_contract_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: contract tests report `6 test(s) passed`, backend reports `22 test(s) passed`, and CTest reports no failures.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/modules/accounting/domain \
  tests/unit/accounting_contract_tests.cpp
git commit -m "feat: define accounting domain contracts"
```

Without authorization, do not commit.

---

### Task 7: Add Repository, Unit-of-Work, and Application Service Boundaries

**Files:**
- Create: `src/core/application/accounting_unit_of_work.h`
- Create: `src/modules/accounting/application/accounting_repositories.h`
- Create: `src/modules/accounting/application/accounting_service.h`
- Create: `src/modules/accounting/application/accounting_service.cpp`
- Create: `tests/unit/accounting_application_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/accounting_application_tests.cpp`

**Interfaces:**
- Consumes: all Task 6 entities and `Result<void>`.
- Produces: row-level repository interfaces, `AccountingRepositories`, `IAccountingUnitOfWork::execute`, `IAccountingService::findTransaction`, and its unit-of-work-backed `AccountingService` implementation; future SQLite code implements repository contracts without changing application callers.

- [ ] **Step 1: Write a fake-unit-of-work contract test**

Create fakes entirely in `tests/unit/accounting_application_tests.cpp`. The test must verify that:

```cpp
class FakeUnitOfWork final : public dailyaccount::IAccountingUnitOfWork {
public:
    dailyaccount::Result<void> execute(
        const std::function<dailyaccount::Result<void>(dailyaccount::AccountingRepositories&)>& operation) override;

    int executeCalls = 0;
    bool operationEntered = false;
};
```

`AccountingService::findTransaction` calls the unit of work exactly once, asks only `repositories.transactions.findById(id)`, returns the found/absent result unchanged, and returns an injected `StorageFailure` unchanged. Add compile-time assertions that `IAccountingService` is abstract and `AccountingService` is not constructible without an `IAccountingUnitOfWork&`.

- [ ] **Step 2: Verify the focused test fails before interfaces exist**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/unit/accounting_application_tests.cpp \
  src/modules/accounting/application/accounting_service.cpp \
  src/core/domain/uuid.cpp -o /tmp/opencode/dailyaccount_application_tests
```

Expected: compilation fails at `core/application/accounting_unit_of_work.h`.

- [ ] **Step 3: Define the unit-of-work aggregate**

Implement `src/core/application/accounting_unit_of_work.h` exactly:

```cpp
namespace dailyaccount {

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

}
```

- [ ] **Step 4: Define exact row-level repository surfaces**

In `accounting_repositories.h`, define a reusable standard-C++ base and the named interfaces:

```cpp
template <typename Entity, typename Id>
class IRepository {
public:
    virtual ~IRepository() = default;
    virtual Result<std::optional<Entity>> findById(const Id& id) = 0;
    virtual Result<void> insert(const Entity& entity) = 0;
    virtual Result<void> update(const Entity& entity) = 0;
    virtual Result<void> markDeleted(const Id& id, UtcInstant deletedAt) = 0;
};

class ITransactionRepository : public IRepository<Transaction, TransactionId> {};
class IAccountRepository : public IRepository<Account, AccountId> {};
class ICategoryRepository : public IRepository<Category, CategoryId> {};
class ITagRepository : public IRepository<Tag, TagId> {};

class IRecurringRepository {
public:
    virtual ~IRecurringRepository() = default;
    virtual Result<std::optional<RecurringRule>> findRule(const RecurringRuleId& id) = 0;
    virtual Result<std::optional<RecurringOccurrence>> findOccurrence(
        const RecurringOccurrenceId& id) = 0;
    virtual Result<void> insertRule(const RecurringRule& rule) = 0;
    virtual Result<void> updateRule(const RecurringRule& rule) = 0;
    virtual Result<void> insertOccurrence(const RecurringOccurrence& occurrence) = 0;
    virtual Result<void> updateOccurrence(const RecurringOccurrence& occurrence) = 0;
};

class IImportRepository {
public:
    virtual ~IImportRepository() = default;
    virtual Result<std::optional<ImportBatch>> findBatch(const ImportBatchId& id) = 0;
    virtual Result<std::optional<ImportItem>> findItem(const ImportItemId& id) = 0;
    virtual Result<void> insertBatch(const ImportBatch& batch) = 0;
    virtual Result<void> insertItem(const ImportItem& item) = 0;
    virtual Result<void> updateItem(const ImportItem& item) = 0;
};

struct OutboxMutation {
    MutationId mutationId;
    std::string commandType;
    std::uint32_t payloadVersion;
    std::string payloadJson;
};

class IOutboxRepository {
public:
    virtual ~IOutboxRepository() = default;
    virtual Result<void> enqueue(const OutboxMutation& mutation) = 0;
};
```

These methods are callable only through references supplied inside `IAccountingUnitOfWork::execute`; no implementation is added here.

- [ ] **Step 5: Implement the minimal application service**

```cpp
namespace dailyaccount {

class IAccountingService {
public:
    virtual ~IAccountingService() = default;
    virtual Result<std::optional<Transaction>> findTransaction(
        const TransactionId& id) = 0;
};

class AccountingService final : public IAccountingService {
public:
    explicit AccountingService(IAccountingUnitOfWork& unitOfWork);
    Result<std::optional<Transaction>> findTransaction(
        const TransactionId& id) override;

private:
    IAccountingUnitOfWork& unitOfWork_;
};

}
```

The `.cpp` constructor stores the reference. `findTransaction` captures one `Result<std::optional<Transaction>>`, calls `unitOfWork_.execute` once, assigns the repository result inside that callback, propagates its error through the callback, and returns the captured value only after the unit of work succeeds. Repository references never appear in the public service API. Write commands and their complete validation arrive in their owning later tasks rather than exposing a generic callback that could bypass application rules.

- [ ] **Step 6: Add separate storage-contract and application targets**

Create `dailyaccount_accounting_storage_contracts` as an `INTERFACE` target linking `dailyaccount_accounting_domain`. Create compiled `dailyaccount_accounting_application` from `accounting_service.cpp`, link it to the storage-contract target, and apply strict warnings.

- [ ] **Step 7: Run focused and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_accounting_application_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: application tests report `3 test(s) passed`, backend reports `22 test(s) passed`, and CTest has zero failures.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/core/application/accounting_unit_of_work.h \
  src/modules/accounting/application tests/unit/accounting_application_tests.cpp
git commit -m "feat: add accounting application boundaries"
```

Without authorization, do not commit.

---

### Task 8: Add Provider-Independent Platform, Sync, and Reminder Contracts

**Files:**
- Create: `src/platform/sync/sync_contract.h`
- Create: `src/platform/notifications/reminder_event.h`
- Create: `src/platform/interfaces/auth_client.h`
- Create: `src/platform/interfaces/sync_transport.h`
- Create: `src/platform/interfaces/secure_store.h`
- Create: `src/platform/interfaces/notification_scheduler.h`
- Create: `src/platform/interfaces/module_database_locator.h`
- Create: `tests/unit/platform_contract_tests.cpp`
- Modify: `CMakeLists.txt`
- Test: `tests/unit/platform_contract_tests.cpp`

**Interfaces:**
- Consumes: G0's selected provider boundary, typed IDs, dates, `Result<T>`, and recurring IDs.
- Produces: `IAuthClient`, `ISyncTransport`, `IRemoteHealthCheck`, `ISecureStore`, `INotificationScheduler`, `IModuleDatabaseLocator`, and provider-neutral sync/reminder value contracts.

- [ ] **Step 1: Write compile-only fakes and value tests**

Create one final fake for each interface and verify exact request values reach it. The suite must test a 64-bit `baseServerRevision`, opaque cursor and epoch strings, mutation payload preservation, secure-store bytes containing zero, reminder offsets `-2`, `-1`, and `0`, and all four notification health codes. Include:

```cpp
static_assert(std::is_abstract_v<dailyaccount::IAuthClient>);
static_assert(std::is_abstract_v<dailyaccount::ISyncTransport>);
static_assert(std::is_abstract_v<dailyaccount::INotificationScheduler>);
static_assert(!std::is_constructible_v<dailyaccount::PushRequest, double>);
```

- [ ] **Step 2: Verify compilation fails before platform contracts exist**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/unit/platform_contract_tests.cpp src/core/domain/uuid.cpp \
  -o /tmp/opencode/dailyaccount_platform_contract_tests
```

Expected: compilation fails at `platform/interfaces/auth_client.h`.

- [ ] **Step 3: Define synchronization values without provider types**

Implement these values in `sync_contract.h` using only standard C++ and typed IDs:

```cpp
namespace dailyaccount {

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

}
```

- [ ] **Step 4: Define auth, transport, and health interfaces**

Use these signatures:

```cpp
namespace dailyaccount {

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

struct RemoteHealth {
    bool reachable;
    std::string diagnosticCode;
};

class IRemoteHealthCheck {
public:
    virtual ~IRemoteHealthCheck() = default;
    virtual Result<RemoteHealth> check() = 0;
};

}
```

No URL, provider SDK object, JWT parser, or Qt network type appears in these headers.

- [ ] **Step 5: Define secure-store, database-location, and reminder interfaces**

Use these complete declarations; scheduling accepts precomputed events and never derives financial rules:

```cpp
namespace dailyaccount {

class ISecureStore {
public:
    virtual ~ISecureStore() = default;
    virtual Result<void> put(
        std::string_view key,
        const std::vector<std::byte>& value) = 0;
    virtual Result<std::vector<std::byte>> get(std::string_view key) = 0;
    virtual Result<void> remove(std::string_view key) = 0;
};

struct ModuleDatabaseLocation {
    ProfileId ownerProfileId;
    std::string moduleId;
    std::filesystem::path databasePath;
};

class IModuleDatabaseLocator {
public:
    virtual ~IModuleDatabaseLocator() = default;
    virtual Result<ModuleDatabaseLocation> locate(
        ProfileId ownerProfileId,
        std::string_view moduleId) const = 0;
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

}
```

- [ ] **Step 6: Add platform and synchronization contract targets**

Create `dailyaccount_platform_interfaces` as an `INTERFACE` target linking only `dailyaccount_core_domain` and exposing `src`. Create `dailyaccount_accounting_sync` as an `INTERFACE` target linking `dailyaccount_platform_interfaces` and `dailyaccount_accounting_domain`; it contains contracts only at G1.

- [ ] **Step 7: Run focused and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_platform_contract_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: platform tests report `5 test(s) passed`, backend reports `22 test(s) passed`, and CTest reports no failures.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/platform/interfaces src/platform/sync \
  src/platform/notifications tests/unit/platform_contract_tests.cpp
git commit -m "feat: define platform and sync contracts"
```

Without authorization, do not commit.

---

### Task 9: Replace the Provisional Registry with a Deterministic Core Registry

**Files:**
- Create: `src/platform/modules/module_descriptor.h`
- Create: `src/platform/modules/platform_registry.h`
- Create: `src/platform/modules/platform_registry.cpp`
- Create: `src/modules/accounting/accounting_module.h`
- Create: `src/modules/accounting/accounting_module.cpp`
- Modify: `tests/registry_tests.cpp`
- Modify: `tests/registry_tests.pro`
- Modify: `CMakeLists.txt`
- Delete: `platform/module_descriptor.h`
- Delete: `platform/module_registry.h`
- Delete: `platform/module_registry.cpp`
- Delete: `modules/accounting/accounting_module.h`
- Delete: `modules/accounting/accounting_module.cpp`
- Test: `tests/registry_tests.cpp`

**Interfaces:**
- Consumes: `Result<T>` and the platform capabilities represented by Task 8 interfaces.
- Produces: owned `ModuleDescriptor` values; deterministic `PlatformRegistry::resolveInitializationOrder`; explicit `registerAccountingCore(PlatformRegistry&)`; stable module and stream ID `accounting`.

- [ ] **Step 1: Rewrite the registry suite against the target API**

Keep the nine existing behavioral cases and add two cases: registration order independence and missing capability rejection. Use:

```cpp
PlatformRegistry registry;
DA_CHECK(registerAccountingCore(registry).hasValue());
const auto capabilities = registry.validateRequiredCapabilities({
    PlatformCapability::Accounts,
    PlatformCapability::DataLocations,
    PlatformCapability::SyncScheduling,
    PlatformCapability::SecureStorage,
    PlatformCapability::Notifications,
    PlatformCapability::SettingsAndTheme,
    PlatformCapability::LoggingAndDiagnostics,
    PlatformCapability::ImportExportBackup,
});
DA_CHECK(capabilities.hasValue());
const auto order = registry.resolveInitializationOrder();
DA_CHECK(order.hasValue());
DA_CHECK_EQ(order.value(), std::vector<std::string>{"accounting"});
```

The deterministic test registers the same DAG in opposite orders and expects identical lexicographically tie-broken output. Duplicate module ID, duplicate stream ID, empty IDs, missing dependency, cycle, missing capability, unknown lookup, and failed-registration atomicity assertions must inspect exact diagnostic substrings.

- [ ] **Step 2: Verify the rewritten suite fails against the provisional API**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/registry_tests.cpp platform/module_registry.cpp \
  modules/accounting/accounting_module.cpp src/core/domain/uuid.cpp \
  -o /tmp/opencode/dailyaccount_registry_tests
```

Expected: compilation fails because the provisional registry has no `resolveInitializationOrder(capabilities)` returning `dailyaccount::Result` and the `src/` headers are absent.

- [ ] **Step 3: Define the complete owned module descriptor**

```cpp
namespace dailyaccount {

enum class PlatformCapability {
    Accounts,
    DataLocations,
    SyncScheduling,
    SecureStorage,
    Notifications,
    SettingsAndTheme,
    LoggingAndDiagnostics,
    ImportExportBackup
};

struct ModuleVersion {
    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
};

struct ModuleDescriptor {
    std::string id;
    ModuleVersion version;
    std::vector<std::string> dependencies;
    std::vector<PlatformCapability> requiredCapabilities;
    std::string databaseFileName;
    std::uint32_t databaseSchemaVersion;
    std::string syncStreamId;
    std::vector<std::string> backgroundTaskIds;
    std::vector<std::string> exporterIds;
    bool essential;
};

}
```

`databaseFileName` is metadata only; no database is opened in Stage 1.

- [ ] **Step 4: Implement deterministic validation and ordering**

Expose:

```cpp
class PlatformRegistry {
public:
    Result<void> registerModule(ModuleDescriptor descriptor);
    const ModuleDescriptor* find(std::string_view moduleId) const noexcept;
    bool contains(std::string_view moduleId) const noexcept;
    std::vector<std::string> moduleIds() const;
    std::size_t size() const noexcept;
    Result<void> validateRequiredCapabilities(
        const std::vector<PlatformCapability>& availableCapabilities) const;
    Result<std::vector<std::string>> resolveInitializationOrder() const;

private:
    std::vector<ModuleDescriptor> modules_;
};
```

Reject empty module ID, stream ID, database file name, or zero schema version and duplicate module or stream IDs during registration. `validateRequiredCapabilities` rejects every unavailable declared capability. `resolveInitializationOrder` rejects missing dependencies and cycles. Copy/move the descriptor into registry-owned storage only after registration validation succeeds. Kahn ordering uses a sorted ready set and sorted dependent lists so registration order cannot affect output. Diagnostics include all involved stable IDs.

- [ ] **Step 5: Implement the accounting core manifest**

`accounting_module.h` declares all three registration functions using forward declarations, although Task 10 implements the two UI-specific functions:

```cpp
namespace dailyaccount {

class PlatformRegistry;
class DesktopRegistry;
class MobileRegistry;

inline constexpr std::string_view kAccountingModuleId = "accounting";
inline constexpr std::string_view kAccountingSyncStreamId = "accounting";
inline constexpr std::string_view kAccountingDesktopRouteId = "accounting.main";
inline constexpr std::string_view kAccountingMobileRouteId = "accounting.overview";

ModuleDescriptor accountingModuleDescriptor();
Result<void> registerAccountingCore(PlatformRegistry& registry);
Result<void> registerAccountingDesktop(DesktopRegistry& registry);
Result<void> registerAccountingMobile(MobileRegistry& registry);

}
```

The descriptor version is `1.0.0`, dependencies are empty, all eight capabilities are required, database file is `accounting.sqlite`, schema version is `1`, stream ID is `accounting`, background task IDs are `accounting.sync` and `accounting.recurring`, exporter IDs are `accounting.csv` and `accounting.json`, and `essential=true`.

- [ ] **Step 6: Promote the new targets, update temporary qmake coverage, and remove duplicates**

Create compiled targets `dailyaccount_platform_modules` and `dailyaccount_accounting_module`. Update `tests/registry_tests.pro` include/source paths to `../src` and the new `.cpp` files so qmake parity remains available. Once direct and CMake tests pass, delete the five provisional files at their old roots; do not retain forwarding copies.

- [ ] **Step 7: Run focused and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_registry_tests
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: registry reports `11 test(s) passed`, backend reports `22 test(s) passed`, CTest has no failures, and no source includes the deleted root-level registry headers.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add -A CMakeLists.txt src/platform/modules src/modules/accounting \
  platform modules/accounting tests/registry_tests.cpp tests/registry_tests.pro
git commit -m "feat: add deterministic module registry"
```

Without authorization, do not commit.

---

### Task 10: Separate Desktop and Mobile Registration and Enforce Boundaries

**Files:**
- Create: `src/platform/modules/desktop_registry.h`
- Create: `src/platform/modules/desktop_registry.cpp`
- Create: `src/platform/modules/mobile_registry.h`
- Create: `src/platform/modules/mobile_registry.cpp`
- Create: `src/modules/accounting/accounting_desktop_module.cpp`
- Create: `src/modules/accounting/accounting_mobile_module.cpp`
- Create: `tests/platform_ui_registry_tests.cpp`
- Create: `tests/cmake/architecture_boundary_contract.cmake`
- Modify: `CMakeLists.txt`
- Test: `tests/platform_ui_registry_tests.cpp`
- Test: `tests/cmake/architecture_boundary_contract.cmake`

**Interfaces:**
- Consumes: accounting route constants, `Result<T>`, and the core registry.
- Produces: desktop-only `DesktopPageFactory`, `DesktopRegistry`; standard-C++ QML description `MobileRouteDescriptor`, `MobileRegistry`; explicit accounting desktop/mobile registration implementations; generated target-link evidence.

- [ ] **Step 1: Write platform-separation tests**

Create five tests:

```cpp
void desktopRegistrationRequiresAPreboundWidgetFactory();
void desktopRegistrationCreatesOnlyTheAccountingMainRoute();
void mobileRegistrationCreatesOnlyTheAccountingOverviewRoute();
void duplicateDesktopAndMobileRoutesAreRejectedTransactionally();
void desktopAndMobileRegistriesHaveNoImplicitConversion();
```

The desktop test binds `accounting.main` to a non-empty `std::function<QWidget*(QWidget*)>` returning `nullptr` without dereferencing Qt types, then calls `registerAccountingDesktop`. The mobile test expects module `accounting`, route `accounting.overview`, QML URI `DailyAccount.Accounting`, and QML type `OverviewPage`. Add static assertions that neither registry is constructible from the other.

- [ ] **Step 2: Verify the focused suite fails because platform registries are absent**

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Isrc -Itests \
  tests/platform_ui_registry_tests.cpp src/core/domain/uuid.cpp \
  src/platform/modules/platform_registry.cpp \
  src/modules/accounting/accounting_module.cpp \
  -o /tmp/opencode/dailyaccount_platform_ui_registry_tests
```

Expected: compilation fails at `platform/modules/desktop_registry.h`.

- [ ] **Step 3: Implement the desktop-only factory registry**

`desktop_registry.h` forward declares global `class QWidget;` and exposes:

```cpp
namespace dailyaccount {

using DesktopPageFactory = std::function<QWidget*(QWidget* parent)>;

struct DesktopPageDescriptor {
    std::string moduleId;
    std::string routeId;
    std::string navigationLabel;
};

class DesktopRegistry {
public:
    Result<void> bindFactory(
        std::string moduleId,
        std::string routeId,
        DesktopPageFactory factory);
    Result<void> registerPage(DesktopPageDescriptor descriptor);
    const DesktopPageDescriptor* find(std::string_view routeId) const noexcept;
    Result<QWidget*> createPage(std::string_view routeId, QWidget* parent) const;
    std::size_t size() const noexcept;
};

}
```

Reject empty IDs, empty factories, duplicate binding keys, duplicate routes, and registration without a matching prebound factory. `createPage` rejects unknown routes and a factory that returns null. Store descriptors and factories by value.

- [ ] **Step 4: Implement the mobile route registry**

```cpp
namespace dailyaccount {

struct MobileRouteDescriptor {
    std::string moduleId;
    std::string routeId;
    std::string qmlUri;
    std::string qmlType;
};

class MobileRegistry {
public:
    Result<void> registerRoute(MobileRouteDescriptor descriptor);
    const MobileRouteDescriptor* find(std::string_view routeId) const noexcept;
    std::size_t size() const noexcept;
};

}
```

Reject empty fields and duplicate route IDs. This file contains no Qt include and does not instantiate a QML engine.

- [ ] **Step 5: Implement separate accounting entry points**

`registerAccountingDesktop` calls `registerPage({"accounting", "accounting.main", "Accounting"})`; it succeeds only after the application target binds a Widgets factory. `registerAccountingMobile` calls `registerRoute({"accounting", "accounting.overview", "DailyAccount.Accounting", "OverviewPage"})`. Compile each `.cpp` into a separate target so mobile linkage cannot pull desktop symbols and desktop linkage cannot pull mobile symbols.

- [ ] **Step 6: Write an executable architecture-boundary scanner**

`architecture_boundary_contract.cmake` receives `DA_SOURCE_DIR` and `DA_LINK_GRAPH`. Recursively scan these roots:

```cmake
set(shared_roots
    "${DA_SOURCE_DIR}/src/core"
    "${DA_SOURCE_DIR}/src/modules/accounting/domain"
    "${DA_SOURCE_DIR}/src/modules/accounting/application"
    "${DA_SOURCE_DIR}/src/platform/interfaces"
    "${DA_SOURCE_DIR}/src/platform/sync"
    "${DA_SOURCE_DIR}/src/platform/notifications")
set(forbidden_source_tokens
    "<QWidget" "<QtWidgets" "<QQuick" "<QtQuick" "<QQml" "<QtQml"
    "<QSql" "<QtSql" "<QNetwork" "<QtNetwork" "jni.h" "windows.h"
    "supabase" "QSQLITE")
set(forbidden_links
    "Qt6::Widgets" "Qt6::Quick" "Qt6::Qml" "Qt6::Sql" "Qt6::Network")
```

Fail with the exact file/token or target/link pair. Also scan `accounting_module.h`, `accounting_module.cpp`, and `module_descriptor.h` for `QWidget`, `QQml`, and `QQuick`. Permit `QWidget` only under `desktop_registry.*`, `accounting_desktop_module.cpp`, `src/apps/desktop-widgets/`, `gui/`, and Widgets tests.

Generate `${CMAKE_BINARY_DIR}/dailyaccount-link-graph.txt` from target `LINK_LIBRARIES` and `INTERFACE_LINK_LIBRARIES`, and register the scanner as `dailyaccount_architecture_boundary_contract` in CTest:

```cmake
set(DA_BOUNDARY_TARGETS
    dailyaccount_core_domain
    dailyaccount_accounting_domain
    dailyaccount_accounting_storage_contracts
    dailyaccount_accounting_application
    dailyaccount_accounting_sync
    dailyaccount_platform_interfaces
    dailyaccount_platform_modules
    dailyaccount_accounting_module)
set(DA_LINK_GRAPH_CONTENT "")
foreach(target IN LISTS DA_BOUNDARY_TARGETS)
    get_target_property(direct_links ${target} LINK_LIBRARIES)
    get_target_property(interface_links ${target} INTERFACE_LINK_LIBRARIES)
    if(direct_links MATCHES "-NOTFOUND$")
        set(direct_links "")
    endif()
    if(interface_links MATCHES "-NOTFOUND$")
        set(interface_links "")
    endif()
    string(APPEND DA_LINK_GRAPH_CONTENT
        "${target}|${direct_links}|${interface_links}\n")
endforeach()
file(GENERATE
    OUTPUT "${CMAKE_BINARY_DIR}/dailyaccount-link-graph.txt"
    CONTENT "${DA_LINK_GRAPH_CONTENT}")
add_test(NAME dailyaccount_architecture_boundary_contract
    COMMAND "${CMAKE_COMMAND}"
        -DDA_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DDA_LINK_GRAPH=${CMAKE_BINARY_DIR}/dailyaccount-link-graph.txt
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cmake/architecture_boundary_contract.cmake)
```

- [ ] **Step 7: Run focused and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_platform_ui_registry_tests
ctest --preset linux-core -R 'registry|architecture' --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
ctest --preset linux-core --output-on-failure
git diff --check
```

Expected: platform UI registry reports `5 test(s) passed`, both registry suites and the architecture scanner pass, backend reports `22 test(s) passed`, and cumulative CTest has zero failures.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt src/platform/modules/desktop_registry.* \
  src/platform/modules/mobile_registry.* \
  src/modules/accounting/accounting_desktop_module.cpp \
  src/modules/accounting/accounting_mobile_module.cpp \
  tests/platform_ui_registry_tests.cpp tests/cmake/architecture_boundary_contract.cmake
git commit -m "feat: separate platform module registration"
```

Without authorization, do not commit.

---

### Task 11: Build the Existing Widgets Application Through Explicit Composition

**Files:**
- Create: `src/apps/desktop-widgets/register_modules.h`
- Create: `src/apps/desktop-widgets/register_modules.cpp`
- Create: `tests/widgets/widgets_contract_tests.cpp`
- Modify: `gui/main_gui.cpp`
- Modify: `gui/mainwindow.cpp`
- Modify: `jizhang.pro`
- Modify: `CMakeLists.txt`
- Test: `tests/widgets/widgets_contract_tests.cpp`

**Interfaces:**
- Consumes: the legacy `Ledger`, existing `MainWindow`, `PlatformRegistry`, `DesktopRegistry`, `registerAccountingCore`, and `registerAccountingDesktop`.
- Produces: `registerDesktopModules(PlatformRegistry&, DesktopRegistry&, Ledger&)`; targets `dailyaccount_legacy_widgets`, `dailyaccount_desktop_composition`, and `dailyaccount_desktop`; installable `DailyAccount.exe`.

- [ ] **Step 1: Write a Widgets contract test before creating its target**

The test creates an isolated `TemporaryDirectory`, `StorageManager`, and `Ledger`, then constructs the desktop composition under `QT_QPA_PLATFORM=offscreen`. Add stable object names only for test observation. Assert:

```cpp
DA_CHECK_EQ(window->minimumWidth(), 1100);
DA_CHECK_EQ(window->minimumHeight(), 700);
auto* stack = window->findChild<QStackedWidget*>("mainStack");
DA_CHECK(stack != nullptr);
DA_CHECK_EQ(stack->count(), 5);
DA_CHECK_EQ(stack->currentIndex(), 0);
auto* second = window->findChild<QPushButton*>("navButton1");
DA_CHECK(second != nullptr);
QTest::mouseClick(second, Qt::LeftButton);
DA_CHECK_EQ(stack->currentIndex(), 1);
```

Also assert that core registration resolves to `accounting`, the desktop registry has one route, and creating `accounting.main` returns a `MainWindow` whose title is non-empty.

- [ ] **Step 2: Verify the Windows configure/build is red before composition exists**

Run on the accepted Windows machine:

```powershell
$env:QT_DIR = 'D:\tools\Qt\6.9.3\mingw_64'
$env:MINGW_DIR = 'D:\tools\mingw64\bin'
$env:Path = "$env:QT_DIR\bin;$env:MINGW_DIR;$env:Path"
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_widgets_contract_tests
```

Expected: build exits non-zero with `No rule to make target 'dailyaccount_widgets_contract_tests'` or the generator-equivalent unknown-target diagnostic.

- [ ] **Step 3: Implement explicit desktop composition**

Expose:

```cpp
namespace dailyaccount {

Result<void> registerDesktopModules(
    PlatformRegistry& platformRegistry,
    DesktopRegistry& desktopRegistry,
    Ledger& ledger);

}
```

Implementation order is exact:

```cpp
auto core = registerAccountingCore(platformRegistry);
if (!core.hasValue()) return core;

auto factory = desktopRegistry.bindFactory(
    std::string(kAccountingModuleId),
    std::string(kAccountingDesktopRouteId),
    [&ledger](QWidget* parent) -> QWidget* { return new MainWindow(ledger, parent); });
if (!factory.hasValue()) return factory;

auto desktop = registerAccountingDesktop(desktopRegistry);
if (!desktop.hasValue()) return desktop;

const auto order = platformRegistry.resolveInitializationOrder();
if (!order.hasValue()) return Result<void>::failure(order.error());
return Result<void>::success();
```

The legacy desktop does not call `validateRequiredCapabilities`, because Stage 1 has defined contracts but has not supplied auth, sync, secure-store, notification, or database-location adapters. Those capabilities become available only when later application containers bind concrete implementations. No static initializer performs registration.

- [ ] **Step 4: Wire the current entry point without changing DAT startup behavior**

Keep all existing data-directory, lock, migration, backup, and `Ledger::load` code in place. Replace only the final direct window construction:

```cpp
dailyaccount::PlatformRegistry platformRegistry;
dailyaccount::DesktopRegistry desktopRegistry;
const auto registered = dailyaccount::registerDesktopModules(
    platformRegistry, desktopRegistry, ledger);
if (!registered.hasValue()) {
    QMessageBox::critical(nullptr, "Startup failed",
        QString::fromStdString(registered.error().message));
    return 1;
}

const auto created = desktopRegistry.createPage(
    dailyaccount::kAccountingDesktopRouteId, nullptr);
if (!created.hasValue()) {
    QMessageBox::critical(nullptr, "Startup failed",
        QString::fromStdString(created.error().message));
    return 1;
}

std::unique_ptr<QWidget> window(created.value());
window->show();
return app.exec();
```

Set `m_stackedWidget->setObjectName("mainStack")` and each of the five functional navigation buttons to `navButton0` through `navButton4` in `mainwindow.cpp`; make no visual, copy, storage, or interaction change.

- [ ] **Step 5: Split CMake GUI code into library, composition, executable, and test**

Under `if(DA_BUILD_DESKTOP)`, require `find_package(Qt6 6.9.3 EXACT REQUIRED COMPONENTS Widgets Test)`. Put every current GUI source except `main_gui.cpp` into `dailyaccount_legacy_widgets`; put `register_modules.cpp` into `dailyaccount_desktop_composition`; put only `main_gui.cpp` into `dailyaccount_desktop`. Enable `AUTOMOC`/`AUTOUIC` per target, preserve `flowpage.ui`, output name `DailyAccount`, Windows `WIN32_EXECUTABLE`, and `user32`. Add:

```cmake
install(TARGETS dailyaccount_desktop RUNTIME DESTINATION .)
set_tests_properties(dailyaccount_widgets_contract_tests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
```

Keep Qt links private to Widgets targets. Update `jizhang.pro` with `INCLUDEPATH += src tests`, the new core/registry/composition sources, and the same application entry point so qmake remains a working parity producer until Task 13.

- [ ] **Step 6: Run the Windows CMake tests**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop --output-on-failure
& 'build\cmake\windows-desktop\dailyaccount_backend_tests.exe'
```

Expected: configure and build succeed, Widgets contract passes under the offscreen platform, all CTest tests pass, and the direct backend output ends with `22 test(s) passed`.

- [ ] **Step 7: Re-run the Linux core graph**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected: Linux does not search for Qt, all core tests pass, and backend reports `22 test(s) passed`.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add CMakeLists.txt jizhang.pro gui/main_gui.cpp gui/mainwindow.cpp \
  src/apps/desktop-widgets tests/widgets/widgets_contract_tests.cpp
git commit -m "feat: compose widgets app through registries"
```

Without authorization, do not commit.

---

### Task 12: Prove Windows CMake Package Parity Before Removing qmake

**Files:**
- Create: `build/build-cmake.bat`
- Create: `tests/windows/verify_widgets_package.ps1`
- Create: `docs/validation/stage-1/windows-cmake.log`
- Create: `docs/validation/stage-1/windows-ctest.log`
- Create: `docs/validation/stage-1/windows-package-files.txt`
- Create: `docs/validation/stage-1/windows-package.sha256`
- Create: `docs/validation/stage-1/windows-cmake-results.json`
- Create: `docs/validation/stage-1/windows-parity-results.json`
- Test: `tests/windows/verify_widgets_package.ps1`
- Preserve until green: `build/build.bat`, `jizhang.pro`, `tests/backend_tests.pro`, `tests/registry_tests.pro`

**Interfaces:**
- Consumes: accepted Qt/MinGW kit, the G0 Windows package result, current qmake package command, CMake `windows-desktop` preset, and `DailyAccount.exe` install rule.
- Produces: isolated CMake package `build/dist-cmake`, file/hash evidence, an eight-second startup smoke result, and a machine-readable parity result authorizing CP-01.

- [ ] **Step 1: Write the package verifier before producing a package**

`verify_widgets_package.ps1` accepts mandatory `-PackageRoot`, `-ResultPath`, and `-Source` parameters. It verifies these files:

```powershell
$required = @(
  'DailyAccount.exe',
  'Qt6Core.dll',
  'Qt6Gui.dll',
  'Qt6Widgets.dll',
  'platforms\qwindows.dll'
)
```

It creates an isolated working directory plus `APPDATA` and `LOCALAPPDATA` children under `$env:TEMP`, starts the package with that empty directory as `-WorkingDirectory`, waits eight seconds, fails if the process exits, terminates it, removes the isolated data, and writes JSON containing `result`, `source`, `aliveAfterSeconds`, `requiredFiles`, and `exitCode`. `result` is `PASS` only when all assertions pass.

- [ ] **Step 2: Run the verifier against an absent CMake package**

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests\windows\verify_widgets_package.ps1 `
  -PackageRoot build\dist-cmake `
  -ResultPath $env:TEMP\dailyaccount-missing-package.json `
  -Source cmake
```

Expected: non-zero exit with `Missing package file: DailyAccount.exe`.

- [ ] **Step 3: Implement the temporary CMake package runner**

`build/build-cmake.bat` must run from any working directory and execute this sequence with error checks after every command:

```bat
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop --output-on-failure
cmake --install build\cmake\windows-desktop --prefix build\dist-cmake
"%QT_DIR%\bin\windeployqt.exe" --release --compiler-runtime --no-translations build\dist-cmake\DailyAccount.exe
```

Default `QT_DIR` and `MINGW_DIR` remain the D-020 values, add both `bin` directories to `PATH`, remove only `build\cmake\windows-desktop` and `build\dist-cmake`, and print `CMake build, tests, and package succeeded` only after deployment succeeds.

- [ ] **Step 4: Build and smoke the qmake parity side on the same revision**

```powershell
$Revision = (git rev-parse HEAD).Trim()
cmd /d /c "build\build.bat"
if ($LASTEXITCODE -ne 0) { throw 'qmake parity build failed' }
powershell -NoProfile -ExecutionPolicy Bypass -File tests\windows\verify_widgets_package.ps1 `
  -PackageRoot build\dist `
  -ResultPath $env:TEMP\dailyaccount-qmake-smoke.json `
  -Source qmake
```

Expected: qmake still reports exactly `22 test(s) passed`, packages all five required files, and its smoke JSON records `PASS` on the current revision.

- [ ] **Step 5: Build, test, deploy, and smoke the CMake side**

```powershell
New-Item docs\validation\stage-1 -ItemType Directory -Force | Out-Null
cmd /d /c "build\build-cmake.bat > docs\validation\stage-1\windows-cmake.log 2>&1"
if ($LASTEXITCODE -ne 0) { throw 'CMake package build failed' }
ctest --preset windows-desktop --output-on-failure |
  Tee-Object docs\validation\stage-1\windows-ctest.log
if ($LASTEXITCODE -ne 0) { throw 'Windows CTest failed' }
powershell -NoProfile -ExecutionPolicy Bypass -File tests\windows\verify_widgets_package.ps1 `
  -PackageRoot build\dist-cmake `
  -ResultPath docs\validation\stage-1\windows-cmake-results.json `
  -Source cmake
```

Expected: all commands exit `0`; CTest has zero failures; `windows-cmake-results.json` records `PASS`, source `cmake`, and `aliveAfterSeconds=8`.

- [ ] **Step 6: Record package manifest, hash, and parity result**

```powershell
$Revision = (git rev-parse HEAD).Trim()
$root = (Resolve-Path build\dist-cmake).Path
Get-ChildItem $root -File -Recurse | Sort-Object FullName |
  ForEach-Object { $_.FullName.Substring($root.Length + 1) } |
  Set-Content -Encoding utf8 docs\validation\stage-1\windows-package-files.txt
$hash = (Get-FileHash build\dist-cmake\DailyAccount.exe -Algorithm SHA256).Hash.ToLower()
"$hash  build/dist-cmake/DailyAccount.exe" |
  Set-Content -Encoding ascii docs\validation\stage-1\windows-package.sha256
$qmake = Get-Content $env:TEMP\dailyaccount-qmake-smoke.json -Raw | ConvertFrom-Json
$cmake = Get-Content docs\validation\stage-1\windows-cmake-results.json -Raw | ConvertFrom-Json
if ($qmake.result -ne 'PASS' -or $cmake.result -ne 'PASS') { throw 'package parity smoke failed' }
@{
  gate = 'CP-01'
  result = 'PASS'
  revision = $Revision
  qmakeSmoke = $qmake.result
  cmakeSmoke = $cmake.result
  legacyBackendTests = 22
  widgetsContract = 'PASS'
} | ConvertTo-Json | Set-Content -Encoding utf8 docs\validation\stage-1\windows-parity-results.json
```

Expected: a non-empty recursive manifest, one lowercase SHA-256 line, and parity JSON with `gate=CP-01`, `result=PASS`, and `legacyBackendTests=22`. Binary hashes need not match because build systems may produce different layouts; behavior, tests, package requirements, and startup must match.

- [ ] **Step 7: Run the Linux cumulative gate and inspect all parity inputs**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
python3 -m json.tool docs/validation/stage-1/windows-cmake-results.json >/dev/null
python3 -m json.tool docs/validation/stage-1/windows-parity-results.json >/dev/null
git diff --check
```

Expected: Linux CTest passes, backend reports `22 test(s) passed`, both JSON files parse, and diff check is silent. If parity is not `PASS`, retain qmake and stop Task 13.

- [ ] **Step 8: Use the optional checkpoint only with explicit authorization**

```bash
git add build/build-cmake.bat tests/windows/verify_widgets_package.ps1 \
  docs/validation/stage-1/windows-cmake.log \
  docs/validation/stage-1/windows-ctest.log \
  docs/validation/stage-1/windows-package-files.txt \
  docs/validation/stage-1/windows-package.sha256 \
  docs/validation/stage-1/windows-cmake-results.json \
  docs/validation/stage-1/windows-parity-results.json
git commit -m "test: prove cmake windows package parity"
```

Do not stage deployed binaries or build trees. Without authorization, do not commit.

---

### Task 13: Retire qmake, Seal G1 Evidence, and Hand Off to Stage 2

**Files:**
- Create: `tests/cmake/check_g1.py`
- Create: `tests/cmake/test_check_g1.py`
- Create: `docs/validation/stage-1/linux-core.log`
- Create: `docs/validation/stage-1/linux-sanitizers.log`
- Create: `docs/validation/stage-1/linux-core-results.json`
- Create: `docs/validation/stage-1/architecture-results.json`
- Create: `docs/validation/stage-1/source-tree.txt`
- Create: `docs/validation/stage-1/g1-evidence-index.md`
- Create: `docs/validation/stage-1/g1-results.json`
- Modify: `build/build.bat`
- Modify: `README.md`
- Modify: `CMakeLists.txt`
- Delete: `build/build-cmake.bat`
- Delete: `jizhang.pro`
- Delete: `tests/backend_tests.pro`
- Delete: `tests/registry_tests.pro`
- Test: `tests/cmake/test_check_g1.py`
- Test: `tests/cmake/check_g1.py`

**Interfaces:**
- Consumes: accepted G0 result, all Stage 1 test targets, CP-01 parity JSON, Windows CMake smoke/package evidence, and Linux normal/sanitized presets.
- Produces: CMake-only `build/build.bat`, documented Linux/Windows commands, machine-readable `G1 PASS`, evidence index, and immutable Stage 2 contract inputs.

- [ ] **Step 1: Unit-test the G1 checker with synthetic repository fixtures**

Use Python `unittest` and `tempfile.TemporaryDirectory` in `test_check_g1.py`. Build a minimal fake root containing all required PASS JSON and expected source markers. Test these exact failures independently:

```python
def test_rejects_failed_g0(self):
    self.write_json("docs/validation/stage-0/g0-results.json", {"gate": "G0", "result": "FAIL_STOP"})
    self.assert_failure("G0 result is not PASS")

def test_rejects_remaining_qmake_file(self):
    self.write("jizhang.pro", "TEMPLATE = app\n")
    self.assert_failure("qmake file remains: jizhang.pro")

def test_rejects_non_cmake_build_script(self):
    self.write("build/build.bat", "qmake jizhang.pro\n")
    self.assert_failure("build/build.bat is not CMake-only")

def test_rejects_missing_windows_parity(self):
    self.write_json("docs/validation/stage-1/windows-parity-results.json", {"result": "FAIL"})
    self.assert_failure("Windows parity result is not PASS")

def test_accepts_complete_g1_fixture(self):
    self.assert_success("G1 PASS: Linux core, Windows desktop, 22 legacy regressions, CMake-only build")
```

- [ ] **Step 2: Run the checker tests red before implementation**

```bash
python3 -m unittest tests/cmake/test_check_g1.py -v
```

Expected: import or file-not-found failure for `tests/cmake/check_g1.py`.

- [ ] **Step 3: Implement the machine-readable G1 checker**

`check_g1.py --root DIR --json PATH` must:

- Require G0 JSON `PASS` with 11 decisions, 10 blocking validations, 22 tests, and zero failures.
- Perform the G0 check first and the qmake-retirement check second, before reading Stage 1 evidence, so the pre-retirement red run has one deterministic failure reason.
- Require Stage 1 Linux, architecture, Windows CMake, and Windows parity JSON results to be `PASS` and all to name the same base Git revision recorded by the evidence index. Require final Linux and final Windows results to name the exact source tree from `source-tree.txt`; CP-01 remains historical evidence from immediately before qmake deletion.
- Require the Linux normal and sanitizer logs to contain `100% tests passed` and exactly `22 test(s) passed`, with no sanitizer diagnostic.
- Require Windows CTest to report no failures, the package manifest to contain all five mandatory files, and its SHA-256 file to parse.
- Require all final targets named in this plan and reject `dailyaccount_accounting_sqlite` and `dailyaccount_android` before their owning stages.
- Require the three explicit accounting registration symbols and reject static self-registration patterns.
- Require the architecture result to report no Qt UI/SQL/Network token or link in shared targets.
- Reject every tracked `*.pro` file, `qmake`, or `mingw32-make` reference in `build/build.bat` and `README.md`.
- Reject tracked build trees, deployed DLLs, executables, credentials, provider secrets, and changes outside the declared Stage 1 surface, except pre-existing unrelated paths recorded in the evidence index.
- Write JSON only after all checks pass, with `gate=G1`, `result=PASS`, `legacyBackendTestCount=22`, `failureCount=0`, and SHA-256 hashes of the four prerequisite result JSON files.
- Print exactly `G1 PASS: Linux core, Windows desktop, 22 legacy regressions, CMake-only build` on success.

- [ ] **Step 4: Make the checker tests green, then observe the real qmake failure**

```bash
python3 -m unittest tests/cmake/test_check_g1.py -v
python3 tests/cmake/check_g1.py \
  --root . --json /tmp/opencode/dailyaccount-pre-retirement-g1.json
```

Expected: all synthetic tests pass; the real invocation exits non-zero and includes `qmake file remains: jizhang.pro`. If it fails first on missing Linux evidence, create that evidence in Step 6, rerun, and still require the qmake-specific red result before deletion.

- [ ] **Step 5: Replace the public Windows build command and remove qmake inputs**

Replace `build/build.bat` with the validated Task 12 CMake sequence, changing the install destination back to `build\dist` and retaining the final marker `Build and tests succeeded`. It must configure `windows-desktop`, build, run CTest, install, run `windeployqt --release --compiler-runtime --no-translations`, and fail on every non-zero command. Then delete `build/build-cmake.bat`, all three `.pro` files, and no C++ source.

Update `README.md` to state CMake 3.22.1+, Qt 6.9.3, MinGW-w64 13.1, Windows `build\build.bat`, Linux `cmake --preset linux-core`, `cmake --build --preset linux-core`, and `ctest --preset linux-core`. Keep the DAT storage and migration sections unchanged and explicitly state that SQLite migration begins in Stage 2.

- [ ] **Step 6: Capture fresh Linux normal, sanitizer, and architecture evidence**

```bash
set -o pipefail
mkdir -p docs/validation/stage-1
STAGE1_INDEX=/tmp/opencode/dailyaccount-stage1-index
rm -f "$STAGE1_INDEX"
GIT_INDEX_FILE="$STAGE1_INDEX" git read-tree HEAD
GIT_INDEX_FILE="$STAGE1_INDEX" git add -A -- \
  CMakeLists.txt CMakePresets.json README.md cmake build src backend gui tests jizhang.pro
GIT_INDEX_FILE="$STAGE1_INDEX" git write-tree > docs/validation/stage-1/source-tree.txt
rm -f "$STAGE1_INDEX"
cmake --preset linux-core 2>&1 | tee docs/validation/stage-1/linux-core.log
cmake --build --preset linux-core --parallel 2 2>&1 | tee -a docs/validation/stage-1/linux-core.log
ctest --preset linux-core --output-on-failure 2>&1 | tee -a docs/validation/stage-1/linux-core.log
./build/cmake/linux-core/dailyaccount_backend_tests 2>&1 | tee -a docs/validation/stage-1/linux-core.log

cmake --preset linux-core-sanitized 2>&1 | tee docs/validation/stage-1/linux-sanitizers.log
cmake --build --preset linux-core-sanitized --parallel 2 2>&1 | tee -a docs/validation/stage-1/linux-sanitizers.log
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
  ctest --preset linux-core-sanitized --output-on-failure \
  2>&1 | tee -a docs/validation/stage-1/linux-sanitizers.log
```

Expected: `source-tree.txt` contains one 40-hex Git tree ID without changing the real index; both presets report `100% tests passed`; the normal log includes exactly `22 test(s) passed`; and the sanitizer log contains no AddressSanitizer, LeakSanitizer, or runtime-error report. Record compiler/CMake versions, base revision, source tree, commands, exit codes, and `result=PASS` in `linux-core-results.json`. Record scanner command, source tree, checked roots/targets, forbidden token/link sets, and `result=PASS` in `architecture-results.json`.

- [ ] **Step 7: Re-run the final CMake-only Windows package**

```powershell
$Stage1Index = Join-Path $env:TEMP 'dailyaccount-stage1-index'
Remove-Item $Stage1Index -Force -ErrorAction SilentlyContinue
$env:GIT_INDEX_FILE = $Stage1Index
git read-tree HEAD
git add -A -- CMakeLists.txt CMakePresets.json README.md cmake build src backend gui tests jizhang.pro
$WindowsSourceTree = (git write-tree).Trim()
Remove-Item Env:GIT_INDEX_FILE
Remove-Item $Stage1Index -Force
$ExpectedSourceTree = (Get-Content docs\validation\stage-1\source-tree.txt -Raw).Trim()
if ($WindowsSourceTree -ne $ExpectedSourceTree) { throw 'Windows and Linux source trees differ' }
cmd /d /c "build\build.bat"
if ($LASTEXITCODE -ne 0) { throw 'final CMake build failed' }
powershell -NoProfile -ExecutionPolicy Bypass -File tests\windows\verify_widgets_package.ps1 `
  -PackageRoot build\dist `
  -ResultPath docs\validation\stage-1\windows-cmake-results.json `
  -Source cmake-final
$WindowsResult = Get-Content docs\validation\stage-1\windows-cmake-results.json -Raw | ConvertFrom-Json
$WindowsResult | Add-Member -NotePropertyName baseRevision -NotePropertyValue (git rev-parse HEAD).Trim()
$WindowsResult | Add-Member -NotePropertyName sourceTree -NotePropertyValue $WindowsSourceTree
$WindowsResult | ConvertTo-Json | Set-Content -Encoding utf8 docs\validation\stage-1\windows-cmake-results.json
ctest --preset windows-desktop --output-on-failure |
  Tee-Object docs\validation\stage-1\windows-ctest.log
if ($LASTEXITCODE -ne 0) { throw 'final Windows CTest failed' }
```

Expected: `build\dist\DailyAccount.exe` is deployed, the process remains alive for eight seconds with isolated app-data paths, all Windows tests pass, and the refreshed JSON records `PASS`, the same base revision, and the same source tree as Linux evidence.

- [ ] **Step 8: Write the evidence index and run the real G1 checker green**

Create `g1-evidence-index.md` with sections `Gate result`, `Base revision and source tree`, `G0 prerequisite`, `Linux core`, `Linux sanitizers`, `Architecture boundaries`, `Windows CMake`, `CP-01 parity`, `qmake retirement`, `Preserved DAT behavior`, `Deferred stage-owned targets`, `Artifact hashes`, `Exceptions`, and `Stage 2 inputs`. Every section names its command, result file/log, UTC execution time, machine/tool versions, exit code, and SHA-256. `Exceptions` is `None` unless a G0-approved bounded fallback directly applies.

Run:

```bash
python3 tests/cmake/check_g1.py \
  --root . \
  --json docs/validation/stage-1/g1-results.json
git diff --check
git status --short
```

Expected stdout:

```text
G1 PASS: Linux core, Windows desktop, 22 legacy regressions, CMake-only build
```

Expected JSON: `gate` is `G1`, `result` is `PASS`, `legacyBackendTestCount` is `22`, and `failureCount` is `0`. Status contains only declared Stage 1 changes plus previously recorded unrelated work; no build tree, executable, DLL, private data, or secret is tracked.

- [ ] **Step 9: Request independent G1 review**

The reviewer reruns both Linux presets, the architecture scanner, checker unit tests, the real G1 checker, the Windows preset/CTest/package smoke, and direct legacy backend tests. The reviewer traces every target link, confirms all three registration entry points are explicit and platform-separated, confirms DAT behavior and source files remain intact, and confirms SQLite/Android production work did not enter Stage 1. Any unsupported evidence changes G1 to failure until corrected.

- [ ] **Step 10: Use the optional final checkpoint only with explicit authorization**

After inspecting `git status`, `git diff`, and recent commits, and only with explicit authorization:

```bash
git add -A -- CMakeLists.txt CMakePresets.json README.md cmake build src tests \
  docs/validation/stage-1 jizhang.pro
git commit -m "build: complete cmake and boundary migration"
```

Expected: the commit contains only declared Stage 1 changes and deletions. Without authorization, leave all reviewed files uncommitted.

---

## G1 Checklist

- [ ] The authoritative G0 checker still prints exactly `G0 PASS: 11 decisions, 10 blocking validations, 22 baseline tests` with accepted hashes and no reopened decision.
- [ ] CMake minimum is 3.22.1, shared C++ is C++17, Qt is exactly 6.9.3 on Windows, and strict warnings are target-scoped and fatal.
- [ ] `linux-core`, `linux-core-sanitized`, and `windows-desktop` configure, build, and test through named presets.
- [ ] `dailyaccount_legacy_backend` honestly contains the unchanged DAT implementation; `dailyaccount_accounting_domain` contains only target data contracts.
- [ ] The direct legacy executable still ends with exactly `22 test(s) passed` under normal Linux, sanitized Linux, and Windows CMake builds.
- [ ] `Result`, typed UUIDv4/UUIDv5, CNY minor-unit money, Gregorian date/time, metadata, accounting entities, import/provenance, repository, and unit-of-work contracts pass focused tests.
- [ ] Auth, secure store, database location, sync transport, remote health, notification health, and reminder events are provider-independent standard-C++ interfaces.
- [ ] Domain/application/shared targets contain and link no Widgets, Quick, QML, SQL, Network, JNI, Windows API, or provider SDK implementation.
- [ ] Registry ownership, duplicate module ID, duplicate stream ID, empty fields, missing dependency, cycle, unavailable capability, transactional rejection, and deterministic order tests pass.
- [ ] The app explicitly calls `registerAccountingCore` and `registerAccountingDesktop`; `registerAccountingMobile` is independently tested without creating an Android UI.
- [ ] Desktop registry accepts only prebound `QWidget` factories, mobile registry accepts only QML route descriptions, and neither platform type leaks into the core manifest.
- [ ] Existing Windows data-directory, locking, DAT load/save, legacy migration, backup recovery, five-page navigation, dimensions, and package startup behavior remain unchanged.
- [ ] CP-01 records qmake and CMake package smoke success on one revision before qmake files are removed.
- [ ] `build/build.bat` is CMake-only, the repository contains no tracked `.pro` file, and README build commands use CMake.
- [ ] Windows package contains `DailyAccount.exe`, Qt Core/Gui/Widgets DLLs, and `platforms/qwindows.dll`, and starts for eight seconds with isolated app data.
- [ ] `dailyaccount_accounting_sqlite` and `dailyaccount_android` do not exist yet; no production database, cloud, sync, notification, or Android UI adapter was introduced.
- [ ] `check_g1.py` prints exactly `G1 PASS: Linux core, Windows desktop, 22 legacy regressions, CMake-only build`, `git diff --check` is silent, and independent review accepts the evidence.
- [ ] No unrelated user change, generated build output, private DAT/text input, credential, commit, or tag was modified without explicit authorization.

## Stage 2 Handoff

Stage 2 may begin only when every G1 checkbox is checked and `docs/validation/stage-1/g1-results.json` records `PASS`. Its executor must read the architecture, parent plan, D-020 through D-030, `g0-evidence-index.md`, `g1-evidence-index.md`, and all headers under `src/core`, `src/platform`, and `src/modules/accounting` before changing persistence.

Stage 2 inherits these immutable inputs:

- `dailyaccount_legacy_backend` and the sanitized G0 DAT fixtures are read-only migration sources and regression oracles.
- `StrongUuid`, money/date values, entity metadata, accounting data contracts, repository interfaces, and `IAccountingUnitOfWork` are the API to implement rather than replace informally.
- The production target name is `dailyaccount_accounting_sqlite`; it implements repository and unit-of-work contracts behind Qt SQL without exposing `QSqlDatabase` to domain or application targets.
- Every SQLite business write and outbox mutation shares one local transaction; Stage 2 follows the QSQLITE executor and backup mechanism selected by D-028.
- DAT source files are never rewritten or deleted. Windows continues DAT writes until Stage 2's validated atomic activation point; after the first accepted SQLite business write, it never silently falls back to DAT.
- `dailyaccount_android`, cloud transport implementations, native reminder adapters, and full synchronization remain owned by later stages.

If Stage 2 needs to change a Stage 1 public signature, stop first and update the architecture, parent plan, this plan's G1 evidence, and the Stage 2 plan in one reviewed contract change. Do not hide an interface break inside a persistence implementation.
