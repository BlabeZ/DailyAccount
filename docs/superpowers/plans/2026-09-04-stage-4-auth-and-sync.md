# DailyAccount Stage 4 Authentication and Synchronization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add secure pre-created-user authentication and Supabase-backed local-first synchronization so same-user Windows and Android profiles converge without allowing one tenant to observe another or making local accounting depend on network availability.

**Architecture:** Preserve SQLite as the immediate write/read source. Keep Supabase behind the Stage 1 provider-neutral interfaces, freeze outbox requests before network I/O, expose only three authenticated PostgreSQL RPCs, apply or quarantine whole change groups, and activate bootstrap data only through a validated staging database.

**Tech Stack:** C++17, CMake 3.22.1+, Qt 6.9.3 Core/Network/SQL/Widgets/QML/Quick/Test, QSQLITE, Supabase CLI/Auth/PostgREST, PostgreSQL 15+, PL/pgSQL, pgTAP, Python 3, PowerShell, Android API 28/35, Kotlin 2.0.21, WorkManager 2.10.1, and the D-029-selected Android background mode.

**Source contracts:** `docs/product-architecture.md`, `docs/superpowers/plans/2026-09-04-dailyaccount-v1-master.md`, `docs/superpowers/plans/2026-09-04-stage-0-baseline-and-prototypes.md`, `docs/superpowers/plans/2026-09-04-stage-1-cmake-and-boundaries.md`, `docs/superpowers/plans/2026-09-04-stage-2-sqlite-and-migration.md`, and `docs/superpowers/plans/2026-09-04-stage-3-android-offline.md`.

## Execution Rules

- Run tasks in order. Do not start Task 1 until the entry gate prints its exact PASS line.
- Accepted D-020 through D-030 outcomes override examples here. A contradiction stops execution for review rather than silently changing an accepted contract.
- Keep the Stage 1 signatures in `src/platform/interfaces/auth_client.h`, `src/platform/interfaces/sync_transport.h`, and `src/platform/interfaces/secure_store.h` unchanged.
- Keep domain/application targets free of Qt Network, Qt SQL, provider names, JWT parsing, URLs, and Supabase response types.
- Local commands still commit business state, unchanged confirmed `serverRevision`, local dirty state, and one stable outbox mutation in one SQLite transaction. Network success is never part of that transaction.
- Use protocol version 1 and accounting payload version 1. Cursors, stream epochs, snapshot tokens, and page tokens are opaque strings in client code.
- One mutation is one atomic domain command. A Supabase push RPC handles exactly one mutation in one PostgreSQL transaction; a client batch may invoke that RPC up to 100 times in request order.
- Derive cloud ownership only from `auth.uid()`. Reject `userId`, `user_id`, and `localLedgerOwnerId` anywhere in client payloads.
- Permit production clients to call Supabase Auth plus `POST /rest/v1/rpc/da_sync_push`, `POST /rest/v1/rpc/da_sync_pull`, and `POST /rest/v1/rpc/da_sync_bootstrap`; prohibit direct table REST access.
- Store access/refresh tokens only through `ISecureStore`; never persist passwords. Never ship a service-role key, database password, management token, private endpoint, or test credential file.
- Logs and evidence may contain bounded codes, counts, timings, request IDs, region/plan labels, and pass booleans. They may not contain credentials, emails, full tokens/cursors, request or response bodies, ownership maps, merchant, note, amount, or raw accounting data.
- Every task follows red-green-refactor: add the focused failing test, observe the stated failure, implement the smallest slice, run focused and cumulative verification, then run `git diff --check`.
- Checkpoint commits are optional and may run only after explicit authorization in the implementation session. This planning change grants no commit authorization.
- Preserve unrelated worktree changes and generated-artifact exclusions. Never stash, reset, clean, or stage unrelated paths.

## Stage 4 Entry Gate

- [ ] Read D-020 through D-030, all G0-G3 evidence indexes, and the public headers under `src/core/`, `src/platform/`, `src/modules/accounting/`, and `src/apps/android-qml/`.
- [ ] Re-run G3 into a temporary result without replacing accepted evidence:

```bash
python3 tests/cmake/check_g3.py \
  --root . \
  --json /tmp/opencode/dailyaccount-stage4-g3-recheck.json
```

Expected: `G3 PASS: Android API 28-35 offline CRUD, exact money, recurring confirmation, reboot reminder` and exit code `0`.

- [ ] Verify accepted/fresh G3 and the Supabase branch:

```bash
python3 - <<'PY'
import glob
import json
from pathlib import Path

accepted = json.loads(Path("docs/validation/stage-3/g3-results.json").read_text(encoding="utf-8"))
fresh = json.loads(Path("/tmp/opencode/dailyaccount-stage4-g3-recheck.json").read_text(encoding="utf-8"))
for label, result in (("accepted", accepted), ("fresh", fresh)):
    assert result["gate"] == "G3", label
    assert result["result"] == "PASS", label
    assert result["failureCount"] == 0, label

paths = glob.glob("docs/decisions/D-021-*.md")
assert len(paths) == 1, paths
lines = [line.strip() for line in Path(paths[0]).read_text(encoding="utf-8").splitlines()]
assert "Status: Accepted" in lines
assert "Selected provider: Supabase" in lines
assert any(line.startswith("Selected region:") and line != "Selected region:" for line in lines)
assert any(line.startswith("Selected plan:") and line != "Selected plan:" for line in lines)
print("Stage 4 entry gate: PASS (G3, Supabase)")
PY
```

Expected: exactly `Stage 4 entry gate: PASS (G3, Supabase)`.

If D-021 rejects Supabase, selects another provider, lacks an accepted provider/region/plan, or contains `FAIL_STOP`, stop before modifying `cloud/`, auth, network, sync, profile, or application code. Create and review `docs/superpowers/plans/2026-09-04-stage-4-auth-and-sync-replacement-adapter.md` for the selected provider, covering endpoint paths, identity derivation, migrations, tenant controls, atomic commands, commit-ordered cursors, bootstrap, retention, and conformance commands; replace this plan in the master execution order before continuing.

## Inherited Public Boundary

These Stage 1 methods remain the provider-neutral seam; implementations below add no overload that exposes Supabase or Qt types:

```cpp
class IAuthClient {
public:
    virtual ~IAuthClient() = default;
    virtual Result<AuthSession> signIn(const SignInRequest&) = 0;
    virtual Result<AuthSession> refresh(const RefreshRequest&) = 0;
    virtual Result<void> signOut() = 0;
};

class ISyncTransport {
public:
    virtual ~ISyncTransport() = default;
    virtual Result<PushResponse> push(const PushRequest&) = 0;
    virtual Result<PullResponse> pull(const PullRequest&) = 0;
    virtual Result<BootstrapPage> bootstrap(const BootstrapRequest&) = 0;
};
```

`IRemoteHealthCheck::check()` and `ISecureStore::{put,get,remove}` also remain unchanged. `PushRequest` carries `moduleId`, `streamEpoch`, `deviceId`, and mutations; `PullRequest` carries `moduleId`, `streamEpoch`, opaque `cursor`, and `limit`; `BootstrapRequest` carries `moduleId`, optional snapshot/page tokens, and `limit`.

The representative push wire object is compact canonical JSON. Real `entities` contain the complete after-state or a tombstone and never contain ownership:

```json
{"protocolVersion":1,"moduleId":"accounting","streamEpoch":"33333333-3333-4333-8333-333333333333","deviceId":"22222222-2222-4222-8222-222222222222","mutations":[{"mutationId":"44444444-4444-4444-8444-444444444444","commandType":"UPSERT_TRANSACTION","payloadVersion":1,"expectations":[{"entityType":"transaction","entityId":"11111111-1111-4111-8111-111111111111","baseServerRevision":3}],"payload":{"entities":[{"entityType":"transaction","entityId":"11111111-1111-4111-8111-111111111111","payloadVersion":1,"serverRevision":3,"createdAtMs":1788480000000,"updatedAtMs":1788480001000,"deletedAtMs":null,"modifiedByDeviceId":"22222222-2222-4222-8222-222222222222","after":{"id":"11111111-1111-4111-8111-111111111111","type":"EXPENSE","status":"POSTED","amountMinor":12345,"currency":"CNY","occurredOn":"2026-09-04","tagIds":[]}}]}}]}
```

---

### Task 1: Add Strict Wire Decoding and Accounting Command Encoding

**Files:**
- Create: `src/platform/sync/bounded_json.h`
- Create: `src/platform/sync/bounded_json.cpp`
- Create: `src/platform/sync/sync_wire_codec.h`
- Create: `src/platform/sync/sync_wire_codec.cpp`
- Create: `src/modules/accounting/sync/accounting_sync_codec.h`
- Create: `src/modules/accounting/sync/accounting_sync_codec.cpp`
- Create: `tests/unit/sync_wire_codec_tests.cpp`
- Create: `tests/unit/accounting_sync_codec_tests.cpp`
- Create: `tests/fixtures/sync/accepted-push-v1.json`
- Create: `tests/fixtures/sync/pull-atomic-group-v1.json`
- Modify: `src/core/application/accounting_error.h`
- Modify: `CMakeLists.txt`

**Interfaces and acceptance:**

```cpp
class SyncWireCodec final {
public:
    Result<DecodedPush> decodePush(const PushResponse&) const;
    Result<DecodedPull> decodePull(const PullResponse&) const;
    Result<DecodedBootstrap> decodeBootstrap(const BootstrapPage&) const;
};

class AccountingSyncCodec final {
public:
    Result<std::string> encode(
        const OutboxMutation&,
        const std::vector<LocalWireEntitySnapshot>&) const;
    Result<void> validateGroup(const WireChangeGroup&) const;
};
```

- `WireChangeGroup` contains `changeGroupId`, opaque `cursorAfter`, `commandType`, `payloadVersion`, and complete `entities`.
- Reject bodies over 4,194,304 bytes, nesting over 32, duplicate keys, invalid UTF-8, trailing bytes, floating/negative/overflow revisions, malformed UUIDs, unknown required enums, ownership keys at any depth, duplicate identities, and command/entity cardinality mismatches.
- Encode canonical UTF-8 with sorted identities and keys, exact 64-bit integers, payload version 1, and a maximum mutation body of 1,048,576 bytes.

- [ ] **Write the failing tests**

```cpp
void duplicateKeysAndOwnershipAreRejected()
{
    SyncWireCodec wire;
    DA_CHECK(!wire.decodePush(PushResponse{
        "{\"protocolVersion\":1,\"protocolVersion\":1}"}).hasValue());

    AccountingSyncCodec accounting;
    const auto encoded = accounting.encode(validOutbox(), validSnapshots());
    DA_CHECK(encoded.hasValue());
    DA_CHECK(encoded.value().find("userId") == std::string::npos);
    DA_CHECK(encoded.value().find("localLedgerOwnerId") == std::string::npos);
}
```

- [ ] **Run red**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target \
  dailyaccount_sync_wire_codec_tests \
  dailyaccount_accounting_sync_codec_tests --parallel 2
```

Expected red: non-zero because the codec targets or headers do not exist.

- [ ] **Implement the minimum slice**

Add a standard-C++ bounded parser that preserves unsigned 64-bit values and a codec for `ACCEPTED`, `CONFLICT`, `REJECTED`, `STALE_EPOCH`, and `UPGRADE_REQUIRED`. Validate all command-specific aggregate shapes, including recurring occurrence/transaction pairs, before exposing decoded values or network results.

- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_sync_wire_codec_tests
./build/cmake/linux-core/dailyaccount_accounting_sync_codec_tests
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected green: strict/canonical cases pass, Linux remains Qt-free, CTest has zero failures, and the DAT oracle prints `22 test(s) passed`.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add CMakeLists.txt src/core/application/accounting_error.h \
  src/platform/sync/bounded_json.h src/platform/sync/bounded_json.cpp \
  src/platform/sync/sync_wire_codec.h src/platform/sync/sync_wire_codec.cpp \
  src/modules/accounting/sync/accounting_sync_codec.h \
  src/modules/accounting/sync/accounting_sync_codec.cpp \
  tests/unit/sync_wire_codec_tests.cpp tests/unit/accounting_sync_codec_tests.cpp \
  tests/fixtures/sync/accepted-push-v1.json tests/fixtures/sync/pull-atomic-group-v1.json
git commit -m "feat: define strict synchronization wire codec"
```

---

### Task 2: Implement Hardened Qt Network, Supabase Auth, and RPC-Only Transport

**Files:**
- Create: `src/platform/network/network_contract.h`
- Create: `src/platform/network/qt_network_executor.h`
- Create: `src/platform/network/qt_network_executor.cpp`
- Create: `src/platform/network/redacted_network_log.h`
- Create: `src/platform/network/redacted_network_log.cpp`
- Create: `src/platform/providers/supabase/supabase_config.h`
- Create: `src/platform/providers/supabase/supabase_config.cpp`
- Create: `src/platform/providers/supabase/supabase_auth_client.h`
- Create: `src/platform/providers/supabase/supabase_auth_client.cpp`
- Create: `src/platform/providers/supabase/supabase_remote_health_check.h`
- Create: `src/platform/providers/supabase/supabase_remote_health_check.cpp`
- Create: `src/platform/providers/supabase/supabase_sync_transport.h`
- Create: `src/platform/providers/supabase/supabase_sync_transport.cpp`
- Create: `tests/integration/qt_network_executor_tests.cpp`
- Create: `tests/integration/supabase_auth_client_tests.cpp`
- Create: `tests/integration/supabase_sync_transport_tests.cpp`
- Create: `tests/cmake/network_redaction_contract.cmake`
- Create: `tests/cmake/no_direct_cloud_dml_contract.cmake`
- Modify: `cmake/DailyAccountOptions.cmake`
- Modify: `CMakePresets.json`
- Modify: `CMakeLists.txt`

**Interfaces and acceptance:**

```cpp
class QtNetworkExecutor final : public QObject {
public:
    Result<HttpResponse> execute(const HttpRequest&);
    void cancelAll();
    Result<void> drainAndClose();
};

class SupabaseAuthClient final : public IAuthClient {
public:
    Result<AuthSession> signIn(const SignInRequest&) override;
    Result<AuthSession> refresh(const RefreshRequest&) override;
    Result<void> signOut() override;
};

class SupabaseSyncTransport final : public ISyncTransport {
public:
    Result<PushResponse> push(const PushRequest&) override;
    Result<PullResponse> pull(const PullRequest&) override;
    Result<BootstrapPage> bootstrap(const BootstrapRequest&) override;
};
```

- Own one `QNetworkAccessManager` on its network thread. Accept only HTTPS and the configured host; verify peers, reject every redirect, and provide 10,000 ms connect, 20,000 ms transfer, and 30,000 ms absolute deadlines.
- Cap auth requests at 65,536 bytes, sync mutation requests at 1,048,576 bytes, and responses at 4,194,304 bytes. Cancellation must release replies without GUI-thread blocking.
- Auth uses `/auth/v1/token?grant_type=password`, `/auth/v1/token?grant_type=refresh_token`, `/auth/v1/logout`, and `/auth/v1/health`. Validate `user.id`, bearer type, integral expiry, and refresh subject equality before replacing tokens.
- Sync uses only the three RPC paths. A multi-mutation `push` sends one serial RPC per mutation and aggregates validated results in original order; partial response loss returns failure so frozen requests can be replayed.

- [ ] **Write the failing tests**

```cpp
void threeMutationsUseThreeTransactions()
{
    SupabaseTransportFixture fixture;
    fixture.enqueueAcceptedPush(3);
    DA_CHECK(fixture.transport.push(validPushRequest(3)).hasValue());
    DA_CHECK_EQ(fixture.requests().size(), std::size_t{3});
    for (const auto& request : fixture.requests()) {
        DA_CHECK_EQ(request.path, "/rest/v1/rpc/da_sync_push");
        DA_CHECK_EQ(request.mutationCount(), std::size_t{1});
    }
}
```

Also test invalid TLS without an ignore path, cleartext/cross-host rejection, all three deadlines, oversize abort, malformed auth/sync JSON, refresh rotation, 401/403/429/5xx mapping, and that logs never receive headers, URLs, bodies, tokens, passwords, emails, or accounting values.

- [ ] **Run red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target `
  dailyaccount_qt_network_executor_tests `
  dailyaccount_supabase_auth_client_tests `
  dailyaccount_supabase_sync_transport_tests --parallel 2
```

Expected red: unknown targets or missing network/provider headers.

- [ ] **Implement the minimum slice**

Create `DA_BUILD_NETWORK`, enabled only for Windows/Android presets, and link Qt Network only into `dailyaccount_qt_network` and `dailyaccount_supabase_adapter`. Runtime configuration carries one validated project base URL and publishable key. Add exactly `apikey`, bearer authorization where required, JSON content headers, and `Content-Profile: api` for RPCs. Do not parse JWTs to establish identity and do not internally retry with changed request bytes.

- [ ] **Run green and cumulative checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop -R "qt_network|supabase_(auth|sync)" --output-on-failure
ctest --preset windows-desktop --output-on-failure
```

```bash
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/network_redaction_contract.cmake
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/no_direct_cloud_dml_contract.cmake
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected green: timeout/TLS/size/cancel/auth/RPC tests pass, scanners are silent, Windows is green, Linux does not link Qt Network, and DAT remains at 22 passes.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add CMakeLists.txt CMakePresets.json cmake/DailyAccountOptions.cmake \
  src/platform/network/network_contract.h \
  src/platform/network/qt_network_executor.h \
  src/platform/network/qt_network_executor.cpp \
  src/platform/network/redacted_network_log.h \
  src/platform/network/redacted_network_log.cpp \
  src/platform/providers/supabase/supabase_config.h \
  src/platform/providers/supabase/supabase_config.cpp \
  src/platform/providers/supabase/supabase_auth_client.h \
  src/platform/providers/supabase/supabase_auth_client.cpp \
  src/platform/providers/supabase/supabase_remote_health_check.h \
  src/platform/providers/supabase/supabase_remote_health_check.cpp \
  src/platform/providers/supabase/supabase_sync_transport.h \
  src/platform/providers/supabase/supabase_sync_transport.cpp \
  tests/integration/qt_network_executor_tests.cpp \
  tests/integration/supabase_auth_client_tests.cpp \
  tests/integration/supabase_sync_transport_tests.cpp \
  tests/cmake/network_redaction_contract.cmake \
  tests/cmake/no_direct_cloud_dml_contract.cmake
git commit -m "feat: add hardened Supabase Qt adapters"
```

---

### Task 3: Protect Sessions and Bind Local Profiles Immutably

**Files:**
- Create: `src/platform/auth/session_manager.h`
- Create: `src/platform/auth/session_manager.cpp`
- Create: `src/platform/auth/profile_binding_service.h`
- Create: `src/platform/auth/profile_binding_service.cpp`
- Create: `src/platform/profile/migrations/003_remote_binding_jobs.sql`
- Create: `src/apps/desktop-widgets/windows_credential_store.h`
- Create: `src/apps/desktop-widgets/windows_credential_store.cpp`
- Create: `tests/unit/session_manager_tests.cpp`
- Create: `tests/integration/profile_binding_tests.cpp`
- Modify: `src/platform/profile/profile_store.h`
- Modify: `src/platform/profile/profile_store.cpp`
- Modify: `src/modules/accounting/data/sqlite/accounting_database.cpp`
- Modify: `CMakeLists.txt`

**Interfaces and acceptance:**

```cpp
class SessionManager final {
public:
    Result<SessionSnapshot> restore(ProfileId);
    Result<AuthSession> signIn(ProfileId, const SignInRequest&);
    Result<std::string> accessTokenForSync(ProfileId, UtcInstant);
    Result<void> signOut(ProfileId);
};

class ProfileBindingService final {
public:
    Result<BoundProfile> bind(
        ProfileId, const AuthSession&, std::string maskedEmail, UtcInstant);
    Result<void> recover(ProfileId, UtcInstant);
};
```

- Store envelope version 1 at `auth.supabase.<profile-uuid>.session.v1` through `ISecureStore`, using Android's inherited `AndroidSecureStore` and a new Windows Credential Manager adapter. A representative test envelope is:

```json
{"formatVersion":1,"providerId":"supabase","profileId":"99999999-9999-4999-8999-999999999999","remoteUserId":"aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa","accessToken":"test-access-token","refreshToken":"test-refresh-token","expiresAtMs":1788483600000}
```

- First login and reopening `SIGNED_OUT_RETAINED` require online authentication. Bind exactly once to `(providerId="supabase", AuthSession.userId)`; reject rebinding, duplicate same-user profiles on one installation, and wrong-owner opens before business queries.
- Refresh once when within 60 seconds of expiry. A transient/offline refresh failure pauses sync but keeps an already authenticated `ACTIVE` profile and local CRUD available. Invalid grant or changed subject requires reauthentication without deleting local data.
- Sign-out removes secure credentials and implements D-023's separate keep-local and delete-local choices through the existing profile switch/drain barrier.

- [ ] **Write the failing tests**

```cpp
void expiredSessionOfflinePausesOnlySync()
{
    SessionFixture fixture;
    fixture.persist(expiredSession());
    fixture.setNetworkAvailable(false);
    DA_CHECK(!fixture.manager.accessTokenForSync(
        fixture.profileId(), fixture.now()).hasValue());
    DA_CHECK_EQ(fixture.profileState(), LocalProfileState::Active);
    DA_CHECK(fixture.localCrudEnabled());
}
```

Add crash checks after binding `PREPARED`, accounting identity write, and profile completion; add ten-call single-flight refresh, atomic token rotation, corrupt secure envelope, wrong subject, keep/delete sign-out, and Credential Manager byte/zeroization tests.

- [ ] **Run red**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target dailyaccount_session_manager_tests --parallel 2
```

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_profile_binding_tests --parallel 2
```

Expected red: unknown targets or missing session/binding files.

- [ ] **Implement the minimum slice**

Use `remote_binding_jobs(profile_id,provider_id,remote_user_id,state,started_at_ms,updated_at_ms)` with states `PREPARED`, `ACCOUNTING_BOUND`, and `COMPLETED`. Disable commands, detach models, drain the module executor, set `database_identity.remote_user_id` only when null/equal, reopen and owner-check, then complete the profile record. Recovery may complete only matching identities. Empty profiles remain `INITIALIZING`; existing profiles return `ACTIVE` with bootstrap pending.

- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core -R "session_manager|platform_contract" --output-on-failure
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
```

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop -R "profile_binding|profile_store|supabase_auth" --output-on-failure
ctest --preset windows-desktop --output-on-failure
```

```bash
git diff --check
```

Expected green: secure rotation/binding crash cases pass, wrong users fail before reads, offline expiry leaves local CRUD enabled, and cumulative suites pass.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add CMakeLists.txt src/platform/auth/session_manager.h \
  src/platform/auth/session_manager.cpp \
  src/platform/auth/profile_binding_service.h \
  src/platform/auth/profile_binding_service.cpp \
  src/platform/profile/profile_store.h src/platform/profile/profile_store.cpp \
  src/platform/profile/migrations/003_remote_binding_jobs.sql \
  src/apps/desktop-widgets/windows_credential_store.h \
  src/apps/desktop-widgets/windows_credential_store.cpp \
  src/modules/accounting/data/sqlite/accounting_database.cpp \
  tests/unit/session_manager_tests.cpp tests/integration/profile_binding_tests.cpp
git commit -m "feat: protect sessions and bind profiles"
```

---

### Task 4: Add Local Sync Schema, Frozen Outbox Retry, and Push Coordination

**Files:**
- Create: `src/platform/sync/sync_store.h`
- Create: `src/platform/sync/sync_coordinator.h`
- Create: `src/platform/sync/sync_coordinator.cpp`
- Create: `src/modules/accounting/data/sqlite/sqlite_sync_store.h`
- Create: `src/modules/accounting/data/sqlite/sqlite_sync_store.cpp`
- Create: `src/modules/accounting/data/sqlite/migrations/003_sync.sql`
- Create: `tests/integration/sqlite_sync_schema_tests.cpp`
- Create: `tests/integration/sqlite_outbox_state_tests.cpp`
- Create: `tests/unit/sync_coordinator_tests.cpp`
- Create: `tests/support/fake_sync_transport.h`
- Modify: `src/modules/accounting/data/sqlite/accounting_migration_runner.h`
- Modify: `src/modules/accounting/data/sqlite/accounting_migration_runner.cpp`
- Modify: `src/modules/accounting/data/sqlite/accounting_unit_of_work.cpp`
- Modify: `src/modules/accounting/accounting_module.cpp`
- Modify: `CMakeLists.txt`

**Interfaces and acceptance:**

```cpp
class ISyncStore {
public:
    virtual ~ISyncStore() = default;
    virtual Result<void> recoverInFlight(UtcInstant) = 0;
    virtual Result<std::vector<FrozenOutboxMutation>> freezeNextBatch(
        std::string_view moduleId, DeviceId, std::uint32_t limit, UtcInstant) = 0;
    virtual Result<void> markRetry(
        MutationId, UtcInstant retryAt, std::string_view errorCode) = 0;
    virtual Result<void> acknowledge(
        const WireMutationResult&, std::string_view currentEpoch) = 0;
    virtual Result<void> recordPushConflict(
        const FrozenOutboxMutation&, const WireMutationResult&, UtcInstant) = 0;
};

class SyncCoordinator final {
public:
    Result<PushCycleSummary> pushOnce(const SyncRunContext&);
    void cancel();
};
```

- `003_sync.sql` is the only accounting migration changed for Stage 4. Rebuild `outbox` transactionally while retaining every schema-2 row/local sequence and add frozen epoch, device, expectations, payload, request JSON, SHA-256 digest, first/in-flight times, attempt count, retry time, and bounded error code.
- Add `sync_stream_state`, `sync_conflicts`, `sync_conflict_entities`, `remote_deletion_guards`, and `sync_diagnostics`. A representative stream table is:

```sql
CREATE TABLE sync_stream_state (
  module_id TEXT PRIMARY KEY CHECK(module_id = 'accounting'),
  stream_epoch TEXT,
  cursor TEXT,
  status TEXT NOT NULL CHECK(status IN ('UNCONFIGURED','IDLE','PENDING','SYNCING','OFFLINE','FAILED_RETRYABLE','AUTHENTICATION_REQUIRED','CONFLICT','BOOTSTRAP_REQUIRED','BOOTSTRAPPING','UPGRADE_REQUIRED')),
  last_success_at_ms INTEGER,
  next_retry_at_ms INTEGER,
  last_error_code TEXT
) STRICT;
```

- Freeze canonical bytes before changing a row to `IN_FLIGHT`. Process death, timeout, cancellation, or lost acknowledgement returns it to `UNSENT` without changing mutation ID/digest/bytes. An edit during flight creates a later row.
- Select at most 100 due rows in local-sequence order and do not batch overlapping entity identity sets. Network I/O occurs after the SQLite freeze transaction commits.
- Current-epoch `ACCEPTED` deletes only the matching row, applies assigned revisions, and rebases only unfrozen later rows. Old-epoch accepted replay deletes only that exact row and enters bootstrap. Conflict/rejection/version errors retain payloads.

- [ ] **Write the failing ACK-loss test**

```cpp
void lostAckRetriesIdenticalFrozenBytes()
{
    SyncFixture fixture;
    fixture.localCreate();
    fixture.transport().acceptThenDropResponse();
    DA_CHECK(!fixture.coordinator().pushOnce(fixture.context()).hasValue());
    const auto first = fixture.transport().lastRequestBytes();
    fixture.restart();
    fixture.transport().replayAcceptedResult();
    DA_CHECK(fixture.coordinator().pushOnce(fixture.context()).hasValue());
    DA_CHECK_EQ(fixture.transport().lastRequestBytes(), first);
    DA_CHECK_EQ(fixture.serverEntityCount(), 1);
    DA_CHECK_EQ(fixture.outboxCount(), 0);
}
```

- [ ] **Run red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target `
  dailyaccount_sqlite_sync_schema_tests `
  dailyaccount_sqlite_outbox_state_tests --parallel 2
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target dailyaccount_sync_coordinator_tests --parallel 2
```

Expected red: schema/targets are absent or compilation fails at `sync_store.h`.

- [ ] **Implement the minimum slice**

Run every store method on the profile/module serial executor and in one bounded SQLite transaction. On one 401, refresh once and replay identical bytes. Retry transient errors with full jitter, base 5 seconds, exponent capped at 8, local cap 15 minutes, and server `Retry-After` cap 30 minutes. Never acknowledge a missing, duplicate, reordered, or wrong-mutation result.

- [ ] **Run green and cumulative checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop -R "sqlite_(sync|outbox)|unit_of_work" --output-on-failure
ctest --preset windows-desktop --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_sync_coordinator_tests
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected green: schema 2 upgrades losslessly to 3; crash/ACK-loss/edit-in-flight/rebase cases pass; all inherited suites remain green.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add CMakeLists.txt src/platform/sync/sync_store.h \
  src/platform/sync/sync_coordinator.h src/platform/sync/sync_coordinator.cpp \
  src/modules/accounting/accounting_module.cpp \
  src/modules/accounting/data/sqlite/accounting_migration_runner.h \
  src/modules/accounting/data/sqlite/accounting_migration_runner.cpp \
  src/modules/accounting/data/sqlite/accounting_unit_of_work.cpp \
  src/modules/accounting/data/sqlite/sqlite_sync_store.h \
  src/modules/accounting/data/sqlite/sqlite_sync_store.cpp \
  src/modules/accounting/data/sqlite/migrations/003_sync.sql \
  tests/support/fake_sync_transport.h tests/unit/sync_coordinator_tests.cpp \
  tests/integration/sqlite_sync_schema_tests.cpp \
  tests/integration/sqlite_outbox_state_tests.cpp
git commit -m "feat: add frozen local synchronization state"
```

---

### Task 5: Create the Default-Deny Supabase Schema and Tenant Boundary

**Files:**
- Create: `cloud/supabase/config.toml`
- Create: `cloud/supabase/seed.sql`
- Create: `cloud/supabase/migrations/20260904010000_stage4_private_schema.sql`
- Create: `cloud/supabase/tests/database/0000_helpers.sql`
- Create: `cloud/supabase/tests/database/0001_permissions.test.sql`
- Create: `cloud/supabase/scripts/provision_precreated_users.py`
- Create: `cloud/supabase/scripts/verify_remote_settings.py`

**Interfaces and acceptance:**

- Configure project ID `dailyaccount-stage4`, API/DB/Studio/Inbucket ports `54321/54322/54323/54324`, sign-up disabled, 3,600-second JWT expiry, API schema `api`, and maximum 1,000 rows.
- In `dailyaccount_private`, create `user_accounts`, `protocol_versions`, `stream_state`, `cursor_positions`, `accounting_entities`, `mutation_results`, `change_groups`, `snapshot_sessions`, `snapshot_entities`, and `snapshot_pages`. Every user-bearing primary/index path starts with `(user_id,module_id)` and all ownership columns are immutable.
- An admin-created confirmed Auth user triggers creation of one `ACTIVE` account and one `accounting` stream. V1 exposes no client sign-up, confirmation, reset, or provisioning endpoint.
- Force RLS on every user-bearing table, revoke all table/sequence/function defaults from `public`, `anon`, and `authenticated`, and later grant `authenticated` only schema `api` usage plus execution of the three RPCs.

Representative policy/grant shape:

```sql
ALTER TABLE dailyaccount_private.accounting_entities ENABLE ROW LEVEL SECURITY;
ALTER TABLE dailyaccount_private.accounting_entities FORCE ROW LEVEL SECURITY;
CREATE POLICY accounting_entities_own_rows
ON dailyaccount_private.accounting_entities
FOR ALL TO authenticated
USING ((SELECT auth.uid()) = user_id)
WITH CHECK ((SELECT auth.uid()) = user_id);
REVOKE ALL ON ALL TABLES IN SCHEMA dailyaccount_private FROM public, anon, authenticated;
REVOKE ALL ON ALL SEQUENCES IN SCHEMA dailyaccount_private FROM public, anon, authenticated;
GRANT USAGE ON SCHEMA api TO authenticated;
```

- [ ] **Write the failing permission tests**

Cover anonymous denial; no direct authenticated SELECT/INSERT/UPDATE/DELETE; no sequence use; RLS enabled and forced on every tenant table; user A policy never matching B; no public function execute; only three eventual authenticated RPC grants; initialization for a pre-created user; and tenant-leading indexes.

Create the local `config.toml`, seed harness, and pgTAP file in this step so the red run reaches permission assertions rather than failing during Supabase startup; do not add the private-schema migration yet.

- [ ] **Run red**

```bash
supabase start --workdir cloud
supabase db reset --workdir cloud
supabase test db --workdir cloud --file supabase/tests/database/0001_permissions.test.sql
```

Expected red: pgTAP fails because Stage 4 schemas, relations, policies, and grants do not exist.

- [ ] **Implement the minimum slice**

Use schema-qualified objects and fixed `search_path=''` for every security-definer function. The maintainer provisioning script reads service credentials and a 2-3-user input file from outside the repository, sets `email_confirm:true`, and prints only request ID, masked account label, returned UUID, and result code. `verify_remote_settings.py` emits only plan/region/boolean settings and rejects a mismatch with D-021.

- [ ] **Run green and cumulative checks**

```bash
supabase db reset --workdir cloud
supabase db lint --workdir cloud --level warning --fail-on error
supabase test db --workdir cloud --file supabase/tests/database/0001_permissions.test.sql
supabase test db --workdir cloud
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/no_direct_cloud_dml_contract.cmake
supabase stop --workdir cloud
git diff --check
```

Expected green: permission tests have no skip/failure, direct DML is denied, sign-up is disabled, and only maintainer-side pre-creation can initialize accounts.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add cloud/supabase/config.toml cloud/supabase/seed.sql \
  cloud/supabase/migrations/20260904010000_stage4_private_schema.sql \
  cloud/supabase/tests/database/0000_helpers.sql \
  cloud/supabase/tests/database/0001_permissions.test.sql \
  cloud/supabase/scripts/provision_precreated_users.py \
  cloud/supabase/scripts/verify_remote_settings.py
git commit -m "feat: add default-deny Supabase schema"
```

---

### Task 6: Implement Atomic Idempotent Push and Complete Change Groups

**Files:**
- Create: `cloud/supabase/migrations/20260904020000_stage4_atomic_push.sql`
- Create: `cloud/supabase/tests/database/0002_atomic_push.test.sql`

**Interface and acceptance:**

```sql
CREATE FUNCTION api.da_sync_push(p_request jsonb)
RETURNS jsonb
LANGUAGE plpgsql
SECURITY DEFINER
SET search_path = '';
REVOKE ALL ON FUNCTION api.da_sync_push(jsonb) FROM public, anon;
GRANT EXECUTE ON FUNCTION api.da_sync_push(jsonb) TO authenticated;
```

- The wrapper obtains `auth.uid()` and rejects an absent subject before tenant-table access. It accepts exact protocol/module/epoch/device keys and exactly one mutation.
- In one transaction, lock `(auth.uid(),'accounting')` in `stream_state` before idempotency, epoch, revisions, entities, cursor, group, and result work.
- Check `(user,module,mutationId)` before epoch/version validation. Same ID and digest returns the stored result; same ID with another digest returns `IDEMPOTENCY_KEY_REUSED` without writes.
- Lock affected entity rows in sorted identity order. Base 0 creates revision 1; exact existing base advances by one; any mismatch returns all current affected states as one conflict and changes no entity/cursor/group.
- Validate complete command shapes. Recurring generation/confirmation writes both occurrence and transaction or neither; a duplicate period returns canonical IDs without an orphan.
- An accepted changing command commits every entity, one new opaque cursor token, one complete `change_groups` row, and one mutation result. SQL failure rolls all of them back.

- [ ] **Write the failing pgTAP tests**

Cover same-digest replay, changed-digest rejection, base revision create/update/conflict, tombstone, forged ownership, malformed scalar/UUID/date/money, aggregate second-row rollback, recurring race/canonical IDs, one revision per entity, one unsplit group, and all-or-none cursor/result writes.

- [ ] **Run red**

```bash
supabase start --workdir cloud
supabase db reset --workdir cloud
supabase test db --workdir cloud --file supabase/tests/database/0002_atomic_push.test.sql
```

Expected red: non-zero because `api.da_sync_push(jsonb)` is absent.

- [ ] **Implement the minimum slice**

Use a static command dispatcher with no dynamic table names. Validate expectation/entity identity-set equality and full after-state before writes. Never accept client ownership or client-assigned next revisions. Store mutation results and groups for at least 210 natural days.

- [ ] **Run green and cumulative checks**

```bash
supabase db reset --workdir cloud
supabase test db --workdir cloud --file supabase/tests/database/0002_atomic_push.test.sql
supabase test db --workdir cloud
supabase db lint --workdir cloud --level warning --fail-on error
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/no_direct_cloud_dml_contract.cmake
supabase stop --workdir cloud
git diff --check
```

Expected green: replay/revision/aggregate/race cases pass, each accepted command has one complete group, and direct table DML remains denied.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add cloud/supabase/migrations/20260904020000_stage4_atomic_push.sql \
  cloud/supabase/tests/database/0002_atomic_push.test.sql
git commit -m "feat: add atomic Supabase push"
```

---

### Task 7: Implement Commit-Ordered Pull, Stable Bootstrap, and Compatibility Retention

**Files:**
- Create: `cloud/supabase/migrations/20260904030000_stage4_pull_and_bootstrap.sql`
- Create: `cloud/supabase/migrations/20260904040000_stage4_retention_and_compatibility.sql`
- Create: `cloud/supabase/tests/database/0003_pull_bootstrap.test.sql`
- Create: `cloud/supabase/tests/database/0004_retention_compatibility.test.sql`
- Create: `tests/sync/run_commit_order_probe.sh`

**Interfaces and acceptance:**

```sql
api.da_sync_pull(p_request jsonb) RETURNS jsonb
api.da_sync_bootstrap(p_request jsonb) RETURNS jsonb
dailyaccount_private.da_run_retention_at(p_now timestamptz) RETURNS jsonb
```

Representative PostgREST bodies:

```json
{"p_request":{"protocolVersion":1,"moduleId":"accounting","streamEpoch":"33333333-3333-4333-8333-333333333333","cursor":"55555555-5555-4555-8555-555555555555","limit":100}}
```

```json
{"p_request":{"protocolVersion":1,"moduleId":"accounting","snapshotToken":null,"pageToken":null,"limit":100}}
```

- Pull resolves opaque cursors only inside the authenticated user/module/epoch. Return groups by locked stream commit position, never split a group, and let the first group exceed the entity target. Empty pages repeat the input cursor.
- A bootstrap start materializes current live rows plus retained tombstones at one stream high-water cursor. Persist user/module/epoch-bound snapshot and page tokens for 24 hours; identical page retries return identical entities and next token.
- Protocol version below the minimum or unsupported payload returns `UPGRADE_REQUIRED` without cursor movement. Unknown old epoch returns `STALE_EPOCH`; a retained-epoch cursor below `min_valid_cursor` returns `CURSOR_EXPIRED`.
- Preserve incremental service for 180 natural days and tombstones, cursor positions/groups, and mutation results for at least 210 natural days. Physical tombstone cleanup rotates the stream epoch and preserves live rows.
- An existing same-ID/same-digest result is replayed before stale-epoch/version checks. A first-seen old-epoch mutation is rejected and retained client-side.

- [ ] **Write the failing tests**

Cover commit order under reversed transaction completion, opaque/cross-user cursor denial, whole-group limit behavior, empty-page stability, stable high-water pages, interruption/retry, token identity/epoch/expiry, retained tombstones, protocol 0/2, payload 0/2, 179/180/181-day reconnects, 209/210/211-day cleanup, old accepted replay, stale unseen mutation, and epoch rotation.

- [ ] **Run red**

```bash
supabase start --workdir cloud
supabase db reset --workdir cloud
supabase test db --workdir cloud --file supabase/tests/database/0003_pull_bootstrap.test.sql
supabase test db --workdir cloud --file supabase/tests/database/0004_retention_compatibility.test.sql
```

Expected red: pull/bootstrap/retention functions are absent.

- [ ] **Implement the minimum slice**

Grant authenticated execution only on `da_sync_pull` and `da_sync_bootstrap`; keep retention private/service-only and schedule it once daily. Allocate cursor positions under the same stream lock as push. The concurrency probe must hold stream A, start B, commit A then B, and observe pull order A then B regardless of mutation UUID ordering.

- [ ] **Run green and cumulative checks**

```bash
supabase db reset --workdir cloud
supabase test db --workdir cloud --file supabase/tests/database/0003_pull_bootstrap.test.sql
supabase test db --workdir cloud --file supabase/tests/database/0004_retention_compatibility.test.sql
DA_SUPABASE_DB_URL='postgresql://postgres:postgres@127.0.0.1:54322/postgres' \
  bash tests/sync/run_commit_order_probe.sh
supabase test db --workdir cloud
supabase db lint --workdir cloud --level warning --fail-on error
supabase stop --workdir cloud
git diff --check
```

Expected green: the probe prints `COMMIT ORDER PASS: A,B`; pages are stable/whole; all boundary-day, epoch, tombstone, and version tests pass.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add cloud/supabase/migrations/20260904030000_stage4_pull_and_bootstrap.sql \
  cloud/supabase/migrations/20260904040000_stage4_retention_and_compatibility.sql \
  cloud/supabase/tests/database/0003_pull_bootstrap.test.sql \
  cloud/supabase/tests/database/0004_retention_compatibility.test.sql \
  tests/sync/run_commit_order_probe.sh
git commit -m "feat: add pull bootstrap and retention RPCs"
```

---

### Task 8: Apply or Quarantine Whole Pull Groups and Resolve Conflicts

**Files:**
- Create: `src/modules/accounting/sync/accounting_change_applier.h`
- Create: `src/modules/accounting/sync/accounting_change_applier.cpp`
- Create: `src/modules/accounting/sync/accounting_conflict_projector.h`
- Create: `src/modules/accounting/sync/accounting_conflict_projector.cpp`
- Create: `src/platform/sync/conflict_resolution_service.h`
- Create: `src/platform/sync/conflict_resolution_service.cpp`
- Create: `tests/integration/pull_change_group_tests.cpp`
- Create: `tests/unit/conflict_resolution_tests.cpp`
- Modify: `src/platform/sync/sync_coordinator.h`
- Modify: `src/platform/sync/sync_coordinator.cpp`
- Modify: `src/modules/accounting/data/sqlite/sqlite_sync_store.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_sync_store.cpp`
- Modify: `CMakeLists.txt`

**Interfaces and acceptance:**

```cpp
class IChangeGroupApplier {
public:
    virtual ~IChangeGroupApplier() = default;
    virtual Result<ChangeGroupApplyResult> applyOrQuarantine(
        const WireChangeGroup&, UtcInstant) = 0;
};

enum class ConflictResolutionKind { KeepLocal, KeepServer, EditMergedCopy };

class ConflictResolutionService final {
public:
    Result<void> resolve(const ResolveConflictCommand&);
    Result<void> acceptResolutionAck(const WireMutationResult&);
};
```

- Decode a full pull page before SQLite work. In one `BEGIN IMMEDIATE`, process groups in cursor order and update the page cursor only after every group is applied or durably quarantined.
- Apply all entities only when every affected local entity is clean, unreferenced by outbox, and outside open conflicts. Unknown remote tombstones become deletion guards.
- If any member is dirty, intersects a conflict, or a valid remote group cannot be applied because its combination with local state violates an aggregate invariant, persist the complete remote group, complete local snapshots, exact outbox references, and every identity; isolate all members. Apply none of that group.
- A malformed wire object, unsupported payload, wrong identity/cardinality set, or internally invalid remote aggregate fails the page with no local write and no cursor movement; it is not converted into a user-resolvable conflict.
- Later intersecting groups remain quarantined behind the earliest conflict; unrelated later groups may apply. A transaction failure restores the old cursor and all rows.
- Resolution identity sets must equal the root group. Keep local, Keep server, or Edit merged copy becomes one `RESOLVE_CHANGE_GROUP` mutation. Preserve both sides until accepted ACK; a second server conflict appends the newer remote version. Reduce resolved sensitive payloads to bounded audit hashes after 30 days.

- [ ] **Write the failing whole-group test**

```cpp
void dirtyPairQuarantinesBothEntitiesAndMovesCursorAtomically()
{
    PullFixture fixture;
    fixture.seedDirtyPendingPair();
    fixture.returnRemotePostedPair();
    DA_CHECK(fixture.coordinator().pullOnce(fixture.context()).hasValue());
    DA_CHECK_EQ(fixture.localPairStatus(), "PENDING/PENDING");
    DA_CHECK_EQ(fixture.conflictEntityCount(), 2);
    DA_CHECK_EQ(fixture.cursor(), fixture.remoteCursorAfter());
}
```

Also test second-entity rollback, malformed group with no cursor movement, aggregate partial resolution rejection, all three choices, resolution ACK loss, conflict-again behavior, related replay order, and post-commit-only reminder reconciliation.

- [ ] **Run red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_pull_change_group_tests --parallel 2
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target dailyaccount_conflict_resolution_tests --parallel 2
```

Expected red: applier/resolution targets or `pullOnce` do not exist.

- [ ] **Implement the minimum slice**

Add `SyncCoordinator::pullOnce`, page-transaction hooks in `SqliteSyncStore`, typed accounting validation through `AccountingSyncCodec`, and immutable conflict projections. Reminder schedules/cancellations run only after the SQLite commit and cannot roll back synced data.

- [ ] **Run green and cumulative checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop -R "pull_change_group|conflict|sqlite_outbox" --output-on-failure
ctest --preset windows-desktop --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core -R "conflict_resolution|sync_coordinator" --output-on-failure
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected green: every group is entirely applied or entirely quarantined, cursor atomicity and three resolution choices pass, and no version is silently discarded.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add CMakeLists.txt src/platform/sync/conflict_resolution_service.h \
  src/platform/sync/conflict_resolution_service.cpp \
  src/platform/sync/sync_coordinator.h src/platform/sync/sync_coordinator.cpp \
  src/modules/accounting/sync/accounting_change_applier.h \
  src/modules/accounting/sync/accounting_change_applier.cpp \
  src/modules/accounting/sync/accounting_conflict_projector.h \
  src/modules/accounting/sync/accounting_conflict_projector.cpp \
  src/modules/accounting/data/sqlite/sqlite_sync_store.h \
  src/modules/accounting/data/sqlite/sqlite_sync_store.cpp \
  tests/integration/pull_change_group_tests.cpp \
  tests/unit/conflict_resolution_tests.cpp
git commit -m "feat: quarantine and resolve complete sync groups"
```

---

### Task 9: Bootstrap Empty and Existing Profiles Through Validated Staging

**Files:**
- Create: `src/platform/sync/bootstrap_coordinator.h`
- Create: `src/platform/sync/bootstrap_coordinator.cpp`
- Create: `tests/integration/bootstrap_coordinator_tests.cpp`
- Modify: `src/platform/profile/migrations/003_remote_binding_jobs.sql`
- Modify: `src/platform/profile/profile_store.h`
- Modify: `src/platform/profile/profile_store.cpp`
- Modify: `src/platform/database/atomic_file_activation.h`
- Modify: `src/platform/database/atomic_file_activation.cpp`
- Modify: `src/modules/accounting/data/sqlite/sqlite_sync_store.h`
- Modify: `src/modules/accounting/data/sqlite/sqlite_sync_store.cpp`
- Modify: `CMakeLists.txt`

**Interfaces and acceptance:**

```cpp
enum class BootstrapMode { Empty, ExistingData };

class BootstrapCoordinator final {
public:
    Result<BootstrapProgress> begin(
        ProfileId, std::string_view moduleId, BootstrapMode, UtcInstant);
    Result<BootstrapProgress> resume(
        ProfileId, std::string_view moduleId, UtcInstant);
    Result<void> discard(ProfileId, std::string_view moduleId);
};
```

- Persist job mode/state, immutable subject, staging/safety paths, `bootstrapStartSeq`, token hashes, stream epoch, high-water cursor, and timestamps in `module_bootstrap_jobs`. Store raw snapshot/page tokens only under `bootstrap.supabase.<profile-uuid>.accounting.v1` in `ISecureStore`.
- Empty mode requires no business/outbox/conflict rows and keeps profile `INITIALIZING`, with writes blocked, until all pages are committed to schema-3 staging, owner/FK/integrity/version checks pass, and activation/reopen succeeds.
- Existing mode records `bootstrapStartSeq=MAX(outbox.local_sequence)`, snapshots the live DB, and permits normal local writes during download. Merge remote-only clean rows, retain local-only dirty rows/outbox, coalesce byte-equivalent rows, and quarantine differing live/delete collisions.
- At final activation, hold a short module write barrier, drain the executor, replay every row and affected entity after `bootstrapStartSeq` into staging in local order, validate, atomically replace, reopen, then release queued commands. Failure leaves the old active DB usable and all local/outbox/conflict data preserved.
- `CURSOR_EXPIRED` and `STALE_EPOCH` enter the same reviewed existing-data path; no error deletes the local DB.

- [ ] **Write the failing existing-data test**

```cpp
void existingBootstrapReplaysEditsAfterStartSequence()
{
    BootstrapFixture fixture(BootstrapMode::ExistingData);
    fixture.seedLocalRowAndOutbox();
    fixture.beginFirstPage();
    const auto start = fixture.bootstrapStartSeq();
    fixture.editWhileDownloading();
    DA_CHECK(fixture.maxOutboxSequence() > start);
    DA_CHECK(fixture.finish().hasValue());
    DA_CHECK(fixture.activeDatabaseContainsLateEdit());
    DA_CHECK(fixture.activeDatabaseContainsEveryOutboxRow());
}
```

Add interruption tests after job creation, each page, ready state, replacement, and reopen; token corruption/expiry; empty-write blocking; high-water delta pull; activation validation failure; and discard/restart.

- [ ] **Run red**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_bootstrap_coordinator_tests --parallel 2
```

Expected red: unknown target or missing `bootstrap_coordinator.h`.

- [ ] **Implement the minimum slice**

Use same-directory staging and safety files. On Windows use `ReplaceFileW` with write-through semantics; on POSIX use same-filesystem rename plus parent-directory `fsync`. Remove continuation secrets and safety data only after the activated database reopens and passes immutable owner/schema checks.

- [ ] **Run green and cumulative checks**

```powershell
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop -R "bootstrap|profile_binding|atomic_file" --output-on-failure
ctest --preset windows-desktop --output-on-failure
```

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
git diff --check
```

Expected green: both modes resume safely, empty writes remain blocked, late edits/outbox/conflicts survive existing-data activation, and replacement is all-old or all-new.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add CMakeLists.txt src/platform/sync/bootstrap_coordinator.h \
  src/platform/sync/bootstrap_coordinator.cpp src/platform/profile \
  src/platform/database/atomic_file_activation.h \
  src/platform/database/atomic_file_activation.cpp \
  src/modules/accounting/data/sqlite/sqlite_sync_store.h \
  src/modules/accounting/data/sqlite/sqlite_sync_store.cpp \
  tests/integration/bootstrap_coordinator_tests.cpp
git commit -m "feat: bootstrap profiles through validated staging"
```

---

### Task 10: Add Per-Module Scheduling, Status, Client Composition, and Android Triggers

**Files:**
- Create: `src/platform/sync/sync_scheduler.h`
- Create: `src/platform/sync/sync_scheduler.cpp`
- Create: `src/platform/sync/sync_status.h`
- Create: `src/platform/sync/sync_diagnostics.h`
- Create: `src/platform/sync/sync_diagnostics.cpp`
- Create: `src/apps/desktop-widgets/desktop_auth_controller.h`
- Create: `src/apps/desktop-widgets/desktop_auth_controller.cpp`
- Create: `src/apps/desktop-widgets/sync_status_widget.h`
- Create: `src/apps/desktop-widgets/sync_status_widget.cpp`
- Create: `src/apps/android-qml/mobile_auth_facade.h`
- Create: `src/apps/android-qml/mobile_auth_facade.cpp`
- Create: `src/apps/android-qml/mobile_sync_facade.h`
- Create: `src/apps/android-qml/mobile_sync_facade.cpp`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/SyncDueStore.kt`
- Create: `src/apps/android-qml/android/src/main/java/local/dailyaccount/SyncDueWorker.kt`
- Create: `tests/unit/sync_scheduler_tests.cpp`
- Create: `tests/integration/sync_status_ui_tests.cpp`
- Create: `src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/AuthSyncInstrumentationTest.kt`
- Modify: `src/apps/desktop-widgets/desktop_composition.cpp`
- Modify: `src/apps/android-qml/mobile_composition.cpp`
- Modify: `src/apps/android-qml/qml/LoginPage.qml`
- Modify: `src/apps/android-qml/qml/SettingsPage.qml`
- Modify: `src/apps/android-qml/android/AndroidManifest.xml`
- Modify: `src/apps/android-qml/android/build.gradle`
- Modify: `src/apps/android-qml/CMakeLists.txt`
- Modify: `tests/cmake/android_boundary_contract.cmake`
- Modify: `CMakeLists.txt`

**Interfaces and acceptance:**

```cpp
enum class SyncTrigger {
    Login, Startup, Foreground, LocalMutation, Manual, NetworkRestored, BackgroundDue
};

enum class ModuleSyncState {
    Synced, Pending, Syncing, Offline, FailedRetryable, Conflict,
    AuthenticationRequired, Bootstrapping, UpgradeRequired
};

class SyncScheduler final {
public:
    Result<void> registerModule(SyncModuleBinding);
    Result<void> trigger(std::string_view moduleId, SyncTrigger);
    Result<void> cancelAndDrain();
    ModuleSyncStatus status(std::string_view moduleId) const;
};
```

- Serialize one run per module, coalesce triggers, debounce local edits for 2 seconds, and bound each run to ten push batches and ten pull pages. Modules have independent cursor/outbox/conflict/retry/status state.
- Publish `Synced`, `Local changes waiting`, `Syncing`, `Offline; local accounting is available`, `Sync failed; retry available`, `Conflict requires review`, `Sign in again to sync`, `Preparing cloud data`, and `App update required for sync`.
- Diagnostics retain at most 200 safe events per module and expose only module/state/count/time/error/request-ID fields. UI uses controllers/facades only and never calls transport or handles tokens.
- Add email/password sign-in for pre-created users, immediate password-field clearing, keep/delete sign-out, manual sync, conflict navigation, and the D-023 profile switch barrier. Expose no registration, confirmation, or reset action.
- Preserve `android:usesCleartextTraffic="false"`, add only Internet and D-029-required WorkManager integration, and retain all G3 reminder components. Under `FOREGROUND_COMPENSATION`, the worker writes only a due/time/attempt marker and starts no Qt/JNI/database/auth code. Under `BACKGROUND_QT_ENABLED`, call only the proven bounded cold composition path. Both modes sync on startup/resume and leave native reminders independent.

- [ ] **Write the failing scheduler/UI/device tests**

Test trigger coalescing, one-run-at-a-time, module independence, retry/manual bypass, profile-switch cancellation, every status string, diagnostic redaction, first login, offline expired session with CRUD, process-death session/outbox recovery, wrong-user retained profile, and exactly the selected D-029 behavior.

- [ ] **Run red**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target dailyaccount_sync_scheduler_tests --parallel 2
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --target dailyaccount_sync_status_ui_tests --parallel 2
```

Expected red: scheduler/status/facade targets are absent.

- [ ] **Implement the minimum slice**

Compose `SessionManager`, `SupabaseSyncTransport`, `SyncCoordinator`, and `SyncScheduler` per active profile/module. Dispatch blocking work off UI threads and drop stale profile-generation callbacks. Make status transitions durable enough to survive restart without persisting secrets or accounting content.

- [ ] **Run green and cumulative checks**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
ctest --preset linux-core -R "sync_scheduler|sync_coordinator|session_manager" --output-on-failure
ctest --preset linux-core --output-on-failure
./build/cmake/linux-core/dailyaccount_backend_tests
```

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop -R "sync_status|profile_binding|desktop" --output-on-failure
ctest --preset windows-desktop --output-on-failure
```

```bash
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/network_redaction_contract.cmake
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/android_boundary_contract.cmake
git diff --check
```

Expected green: status/login/profile-switch tests pass, Android implements exactly one D-029 branch, offline CRUD and G3 reminder behavior remain available, and scans are silent.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add CMakeLists.txt src/platform/sync/sync_scheduler.h \
  src/platform/sync/sync_scheduler.cpp src/platform/sync/sync_status.h \
  src/platform/sync/sync_diagnostics.h src/platform/sync/sync_diagnostics.cpp \
  src/apps/desktop-widgets/desktop_auth_controller.h \
  src/apps/desktop-widgets/desktop_auth_controller.cpp \
  src/apps/desktop-widgets/sync_status_widget.h \
  src/apps/desktop-widgets/sync_status_widget.cpp \
  src/apps/desktop-widgets/desktop_composition.cpp \
  src/apps/android-qml/mobile_auth_facade.h \
  src/apps/android-qml/mobile_auth_facade.cpp \
  src/apps/android-qml/mobile_sync_facade.h \
  src/apps/android-qml/mobile_sync_facade.cpp \
  src/apps/android-qml/mobile_composition.cpp \
  src/apps/android-qml/qml/LoginPage.qml \
  src/apps/android-qml/qml/SettingsPage.qml \
  src/apps/android-qml/android/AndroidManifest.xml \
  src/apps/android-qml/android/build.gradle \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/SyncDueStore.kt \
  src/apps/android-qml/android/src/main/java/local/dailyaccount/SyncDueWorker.kt \
  src/apps/android-qml/android/src/androidTest/java/local/dailyaccount/AuthSyncInstrumentationTest.kt \
  src/apps/android-qml/CMakeLists.txt \
  tests/unit/sync_scheduler_tests.cpp tests/integration/sync_status_ui_tests.cpp \
  tests/cmake/android_boundary_contract.cmake
git commit -m "feat: compose authentication and sync status"
```

---

### Task 11: Prove Tenant Isolation and Same-User Two-Device Convergence

**Files:**
- Create: `tests/sync/run_fake_transport_matrix.py`
- Create: `tests/sync/run_supabase_conformance.py`
- Create: `tests/sync/run_two_device_e2e.ps1`
- Create: `tests/sync/two_device_sync_tests.cpp`
- Create: `tests/android/run_stage4_matrix.sh`
- Create: `docs/validation/stage-4/cloud-remote-results.json`
- Create: `docs/validation/stage-4/fake-transport-results.json`
- Create: `docs/validation/stage-4/two-device-results.json`
- Create: `docs/validation/stage-4/android-auth-sync-results.json`
- Modify: `CMakeLists.txt`

**Interfaces and acceptance:**

- `run_supabase_conformance.py` signs in disposable pre-created users A/B, exercises only the three RPCs, and proves anonymous/B cannot access A through forged ownership, A cursor/snapshot tokens, or direct GET/POST/PATCH/DELETE table endpoints.
- `two_device_sync_tests.cpp` uses two real schema-3 databases with fake transport; the PowerShell runner repeats the same-user flow on Windows plus Android against the selected remote project.
- Required scenarios are offline create/edit/delete convergence, stable identity/revisions, lost ACK replay, edit during flight, edit/edit and edit/delete conflict resolution, recurring pair atomicity/canonical generation, pull interruption, both bootstrap modes, cursor expiry, epoch change, unsupported payload retention, independent module failure, and user A/B isolation.
- The fake matrix runs 10,000 schedules at seed `20260904` and proves no accepted mutation loss, duplicate entity, partial aggregate, earlier ACK deletion of a later edit, or discarded blocked payload.
- Evidence JSON is sanitized before write; credentials come only from environment and never appear in command arguments, logs, or result files.

- [ ] **Write the failing system test**

```cpp
void lostAckThenSecondDevicePullsOneAcceptedRevision()
{
    TwoDeviceFixture fixture;
    fixture.deviceA().createOffline();
    fixture.server().acceptThenDropAck();
    fixture.deviceA().syncTwice();
    fixture.deviceB().sync();
    DA_CHECK_EQ(fixture.server().entityCount(), 1);
    DA_CHECK_EQ(fixture.deviceB().entityCount(), 1);
    DA_CHECK_EQ(fixture.server().revision(), std::uint64_t{1});
}
```

- [ ] **Run red**

```bash
cmake --preset linux-core
cmake --build --preset linux-core --target dailyaccount_two_device_sync_tests --parallel 2
python3 tests/sync/run_fake_transport_matrix.py \
  --seed 20260904 --schedules 10000 \
  --json /tmp/opencode/dailyaccount-stage4-fake-results.json
```

Expected red: the target and matrix runner are absent.

- [ ] **Implement the local and remote runners**

The remote runner requires `DA_SUPABASE_URL`, `DA_SUPABASE_PUBLISHABLE_KEY`, and A/B credentials from environment. It writes only counts, HTTP/error codes, timings, selected region/plan, and booleans. The two-device runner signs both clients into A, performs offline create/edit/delete and conflict/bootstrap/ACK-loss cases, switches one client to B to prove isolation, then reauthenticates A before reopening A's retained profile.

- [ ] **Run green locally and remotely**

```bash
mkdir -p docs/validation/stage-4
supabase start --workdir cloud
supabase db reset --workdir cloud
supabase test db --workdir cloud
python3 tests/sync/run_fake_transport_matrix.py \
  --seed 20260904 --schedules 10000 \
  --json docs/validation/stage-4/fake-transport-results.json
cmake --preset linux-core
cmake --build --preset linux-core --parallel 2
./build/cmake/linux-core/dailyaccount_two_device_sync_tests \
  --transport fake --seed 20260904 \
  --json /tmp/opencode/dailyaccount-two-device-fake.json
supabase stop --workdir cloud
```

```bash
: "${SUPABASE_ACCESS_TOKEN:?required}"
: "${SUPABASE_DB_PASSWORD:?required}"
: "${DA_SUPABASE_PROJECT_REF:?required}"
: "${DA_SUPABASE_SERVICE_ROLE_KEY:?required}"
: "${DA_PRECREATED_USERS_FILE:?required}"
supabase link --workdir cloud --project-ref "$DA_SUPABASE_PROJECT_REF"
supabase migration list --workdir cloud --linked
supabase db push --workdir cloud --linked --include-all
python3 cloud/supabase/scripts/provision_precreated_users.py \
  --input "$DA_PRECREATED_USERS_FILE" --count-min 2 --count-max 3
python3 cloud/supabase/scripts/verify_remote_settings.py \
  --project-ref "$DA_SUPABASE_PROJECT_REF" \
  --json /tmp/opencode/dailyaccount-stage4-remote-settings.json
python3 tests/sync/run_supabase_conformance.py \
  --result docs/validation/stage-4/cloud-remote-results.json
```

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests\sync\run_two_device_e2e.ps1 `
  -DesktopDriver build\cmake\windows-desktop\dailyaccount_two_device_sync_tests.exe `
  -AndroidApk build\cmake\android-x86_64-debug\android-build\build\outputs\apk\debug\android-build-debug.apk `
  -AndroidSerial $env:DA_ANDROID_API35_SERIAL `
  -ResultPath docs\validation\stage-4\two-device-results.json
```

Expected green: 10,000 schedules report zero invariant failures; all named two-device cases pass; own-user RPCs work; anonymous, cross-user, forged-owner, token-reuse, and direct-DML attempts reveal no A/B row.

- [ ] **Run Android and inherited G3 checks**

```bash
cmake --preset android-x86_64-debug
cmake --build --preset android-x86_64-debug --target apk --parallel 2
cmake --preset android-arm64-debug
cmake --build --preset android-arm64-debug --target apk --parallel 2
bash tests/android/run_stage4_matrix.sh \
  --api28-serial "$DA_ANDROID_API28_SERIAL" \
  --api35-serial "$DA_ANDROID_API35_SERIAL" \
  --arm64-serial "$DA_ANDROID_ARM64_SERIAL" \
  --x86-apk build/cmake/android-x86_64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  --arm64-apk build/cmake/android-arm64-debug/android-build/build/outputs/apk/debug/android-build-debug.apk \
  --result docs/validation/stage-4/android-auth-sync-results.json
python3 tests/cmake/check_g3.py \
  --root . --json /tmp/opencode/dailyaccount-stage4-final-g3.json
git diff --check
```

Expected green: API 28 x86_64, API 35 x86_64, and physical ARM64 pass Stage 4 plus inherited flight-mode CRUD, exact money, process-death, recurring, reboot reminder, and receiver-no-Qt checks; G3 prints its exact PASS line.

- [ ] **Conditional checkpoint**

Only after explicit authorization:

```bash
git add CMakeLists.txt tests/sync/run_fake_transport_matrix.py \
  tests/sync/run_supabase_conformance.py tests/sync/run_two_device_e2e.ps1 \
  tests/sync/two_device_sync_tests.cpp tests/android/run_stage4_matrix.sh \
  docs/validation/stage-4/cloud-remote-results.json \
  docs/validation/stage-4/fake-transport-results.json \
  docs/validation/stage-4/two-device-results.json \
  docs/validation/stage-4/android-auth-sync-results.json
git commit -m "test: prove tenant isolation and device convergence"
```

---

### Task 12: Seal the G4 Authentication and Sync Gate

**Files:**
- Create: `tests/cmake/stage4_boundary_contract.cmake`
- Create: `tests/cmake/check_g4.py`
- Create: `tests/cmake/test_check_g4.py`
- Create: `docs/validation/stage-4/cloud-local.log`
- Create: `docs/validation/stage-4/linux-core.log`
- Create: `docs/validation/stage-4/windows-auth-sync.log`
- Create: `docs/validation/stage-4/source-tree.txt`
- Create: `docs/validation/stage-4/g4-evidence-index.md`
- Create: `docs/validation/stage-4/g4-results.json`
- Modify: `CMakeLists.txt`

**Gate contract:**

- Require accepted/fresh G3; accepted Supabase region/plan; schema 3; protocol/payload version 1; strict Qt TLS/limits/redaction; protected sessions; immutable profile binding; and local CRUD during sync/auth outage.
- Require forced RLS/default deny, exactly three authenticated sync RPC grants, `auth.uid()` tenancy, no client table DML, same-digest replay, atomic revisions/groups, commit-ordered opaque cursors, whole-group quarantine/resolution, both bootstraps, 180/210-day boundaries, safe epoch/tombstone/version behavior, and module status.
- Require local/remote A/B isolation, 10,000 deterministic schedules, same-user Windows/Android convergence, API 28/API 35/ARM64 matrices, Linux/Windows CTest, DAT 22, fresh G3, source scanners, and one recorded source tree.
- Reject credentials, private endpoints, sensitive accounting content, generated databases/APKs/build trees, and unrecorded unrelated paths from evidence.

- [ ] **Write the failing checker tests**

```python
def test_rejects_partial_group(self):
    self.write_json("docs/validation/stage-4/conflict-results.json", {
        "remoteEntityCount": 2,
        "persistedEntityCount": 1,
    })
    self.assert_failure("atomic change group was split")

def test_accepts_complete_fixture(self):
    self.assert_success(
        "G4 PASS: Supabase auth isolation, atomic sync, bootstrap, and two-device convergence")
```

- [ ] **Run red**

```bash
python3 -m unittest tests/cmake/test_check_g4.py -v
```

Expected red: import/file-not-found failure for `tests/cmake/check_g4.py`.

- [ ] **Implement the checker and boundary scan**

`stage4_boundary_contract.cmake` rejects provider/Qt leakage, direct table URLs, certificate-ignore calls, service credentials in shipping source, sensitive diagnostic parameters, Android cleartext/broad permissions, and Qt/JNI/network references in reminder receivers. `check_g4.py --root DIR --json PATH` verifies every result above and writes JSON only after all checks pass.

The successful result is exactly:

```json
{"gate":"G4","result":"PASS","provider":"Supabase","localSchemaVersion":3,"protocolVersion":1,"payloadVersion":1,"precreatedUserCount":2,"directClientDml":false,"crossTenantIsolation":true,"idempotentAckLoss":true,"commitOrderedCursor":true,"wholeGroupConflict":true,"emptyBootstrap":true,"existingDataBootstrap":true,"twoDeviceConvergence":true,"offlineCrudPreserved":true,"failureCount":0}
```

`precreatedUserCount` may be 2 or 3; all other fields and values are exact.

- [ ] **Run final green verification**

```bash
set -o pipefail
mkdir -p docs/validation/stage-4
supabase start --workdir cloud
supabase db reset --workdir cloud
supabase db lint --workdir cloud --level warning --fail-on error
supabase test db --workdir cloud 2>&1 | tee docs/validation/stage-4/cloud-local.log
supabase stop --workdir cloud
cmake --preset linux-core 2>&1 | tee docs/validation/stage-4/linux-core.log
cmake --build --preset linux-core --parallel 2 2>&1 | tee -a docs/validation/stage-4/linux-core.log
ctest --preset linux-core --output-on-failure 2>&1 | tee -a docs/validation/stage-4/linux-core.log
./build/cmake/linux-core/dailyaccount_backend_tests 2>&1 | tee -a docs/validation/stage-4/linux-core.log
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/network_redaction_contract.cmake
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/no_direct_cloud_dml_contract.cmake
cmake -DDA_SOURCE_DIR="$PWD" -P tests/cmake/stage4_boundary_contract.cmake
STAGE4_INDEX=/tmp/opencode/dailyaccount-stage4-index
rm -f "$STAGE4_INDEX"
GIT_INDEX_FILE="$STAGE4_INDEX" git read-tree HEAD
GIT_INDEX_FILE="$STAGE4_INDEX" git add -A -- \
  CMakeLists.txt CMakePresets.json cmake cloud/supabase src tests docs/validation/stage-4
GIT_INDEX_FILE="$STAGE4_INDEX" git rm --cached --ignore-unmatch \
  docs/validation/stage-4/source-tree.txt
GIT_INDEX_FILE="$STAGE4_INDEX" git diff --cached --check
GIT_INDEX_FILE="$STAGE4_INDEX" git write-tree \
  > docs/validation/stage-4/source-tree.txt
rm -f "$STAGE4_INDEX"
python3 -m unittest tests/cmake/test_check_g4.py -v
python3 tests/cmake/check_g4.py \
  --root . --json docs/validation/stage-4/g4-results.json
git diff --check
git status --short
```

On the accepted Windows host, also run:

```powershell
$env:QT_QPA_PLATFORM = 'offscreen'
cmake --preset windows-desktop
cmake --build --preset windows-desktop --parallel 2
ctest --preset windows-desktop --output-on-failure 2>&1 |
  Tee-Object docs\validation\stage-4\windows-auth-sync.log
if ($LASTEXITCODE -ne 0) { throw 'Stage 4 Windows CTest failed' }
```

Expected green: cloud tests have no skip/failure; Linux and Windows have zero CTest failures; DAT prints `22 test(s) passed`; all Stage 4 device/system results pass; scanners and `git diff --check` are silent; the checker prints exactly:

```text
G4 PASS: Supabase auth isolation, atomic sync, bootstrap, and two-device convergence
```

- [ ] **Record evidence and obtain independent review**

`g4-evidence-index.md` records each command, UTC time, OS/device alias, exact tool versions, exit code, sanitized result path, source-tree identity, and SHA-256. The reviewer reruns G3/G4, pgTAP, A/B/direct-DML denial, ACK-loss, commit ordering, both bootstraps, whole-group conflict resolution, boundary-day/epoch/version cases, two-device flow, and source scanners.

- [ ] **Conditional final checkpoint**

After inspecting `git status --short`, `git diff`, and `git log --oneline -10`, and only after explicit authorization, commit the gate files separately from the preceding implementation checkpoints:

```bash
git add CMakeLists.txt tests/cmake/stage4_boundary_contract.cmake \
  tests/cmake/check_g4.py tests/cmake/test_check_g4.py \
  docs/validation/stage-4/cloud-local.log \
  docs/validation/stage-4/linux-core.log \
  docs/validation/stage-4/windows-auth-sync.log \
  docs/validation/stage-4/source-tree.txt \
  docs/validation/stage-4/g4-evidence-index.md \
  docs/validation/stage-4/g4-results.json
git commit -m "test: seal G4 authentication and sync evidence"
```

Expected: only reviewed Stage 4 source, tests, and sanitized evidence are committed. Credentials, linked Supabase state, APKs, databases, WAL/SHM files, build trees, device serials, and unrelated files remain untracked.

## Stage 5 Handoff

Stage 5 may begin only after every Task 12 check passes, independent review accepts the evidence, and `docs/validation/stage-4/g4-results.json` records `gate=G4`, `result=PASS`, `provider=Supabase`, `localSchemaVersion=3`, `protocolVersion=1`, `wholeGroupConflict=true`, `twoDeviceConvergence=true`, and `failureCount=0`.

Stage 5 inherits the unchanged provider seam, immutable profile subject binding, schema/protocol/payload version independence, stable local command/outbox atomicity, server-assigned entity revisions, commit-ordered opaque cursors, whole-group apply/quarantine/resolution, both bootstrap paths, ACK-loss retry, retention/epoch/version behavior, per-module scheduler/status, Android background-mode limits, and native reminder independence. New accounting workflows must use these paths rather than direct cloud DML or a second online write path.
