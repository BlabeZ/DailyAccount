#include "accounting_module.h"
#include "module_registry.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

class TestFailure : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            throw TestFailure(std::string("CHECK failed: ") + #condition + \
                              " at " + __FILE__ + ":" + std::to_string(__LINE__)); \
        } \
    } while (false)

ModuleDescriptor module(std::string id) {
    ModuleDescriptor descriptor;
    descriptor.id = std::move(id);
    descriptor.versionMajor = 1;
    descriptor.syncStreamId = descriptor.id;
    return descriptor;
}

void registerOk(PlatformRegistry& registry, ModuleDescriptor descriptor) {
    const PlatformRegistry::Result result =
        registry.registerModule(std::move(descriptor));
    CHECK(result.ok);
    if (!result.ok) {
        throw TestFailure("registerModule failed: " + result.error);
    }
}

void testAccountingModuleRegistersWithStableIds() {
    PlatformRegistry registry;
    std::string error;
    CHECK(accounting::registerAccountingCore(registry, error));
    CHECK(error.empty());
    CHECK(registry.contains(accounting::kModuleId));
    CHECK(registry.size() == 1);
    const ModuleDescriptor* descriptor = registry.find(accounting::kModuleId);
    CHECK(descriptor != nullptr);
    CHECK(descriptor->id == accounting::kModuleId);
    CHECK(descriptor->syncStreamId == accounting::kSyncStreamId);
    CHECK(descriptor->versionMajor == 1);
    CHECK(descriptor->essential);
    CHECK(registry.moduleIds() == std::vector<std::string>{accounting::kModuleId});
}

void testDuplicateModuleIdIsRejected() {
    PlatformRegistry registry;
    registerOk(registry, module("alpha"));
    const PlatformRegistry::Result duplicate =
        registry.registerModule(module("alpha"));
    CHECK(!duplicate.ok);
    CHECK(duplicate.error.find("duplicate module id") != std::string::npos);
    CHECK(registry.size() == 1);
}

void testDuplicateSyncStreamIdIsRejected() {
    PlatformRegistry registry;
    ModuleDescriptor first = module("alpha");
    registerOk(registry, first);
    ModuleDescriptor second = module("beta");
    second.syncStreamId = "alpha";
    const PlatformRegistry::Result duplicate =
        registry.registerModule(second);
    CHECK(!duplicate.ok);
    CHECK(duplicate.error.find("duplicate sync stream id") != std::string::npos);
    CHECK(duplicate.error.find("alpha") != std::string::npos);
    CHECK(registry.size() == 1);
}

void testEmptyIdsAreRejected() {
    PlatformRegistry registry;
    ModuleDescriptor emptyId = module("alpha");
    emptyId.id.clear();
    CHECK(!registry.registerModule(emptyId).ok);
    ModuleDescriptor emptyStream = module("alpha");
    emptyStream.syncStreamId.clear();
    CHECK(!registry.registerModule(emptyStream).ok);
    CHECK(registry.size() == 0);
}

void testInitOrderHonorsDependencies() {
    PlatformRegistry registry;
    ModuleDescriptor base = module("base");
    ModuleDescriptor middle = module("middle");
    middle.dependencies.push_back("base");
    ModuleDescriptor top = module("top");
    top.dependencies.push_back("middle");
    registerOk(registry, std::move(base));
    registerOk(registry, std::move(middle));
    registerOk(registry, std::move(top));

    const PlatformRegistry::InitOrder order = registry.resolveInitOrder();
    CHECK(order.ok);
    CHECK(order.order.size() == 3);
    std::size_t baseIndex = 0;
    std::size_t middleIndex = 0;
    for (std::size_t i = 0; i < order.order.size(); ++i) {
        if (order.order[i] == "base") baseIndex = i;
        if (order.order[i] == "middle") middleIndex = i;
    }
    CHECK(baseIndex < middleIndex);
}

void testMissingDependencyBreaksInitOrder() {
    PlatformRegistry registry;
    ModuleDescriptor orphan = module("orphan");
    orphan.dependencies.push_back("missing");
    registerOk(registry, std::move(orphan));

    const PlatformRegistry::InitOrder order = registry.resolveInitOrder();
    CHECK(!order.ok);
    CHECK(order.error.find("missing module 'missing'") != std::string::npos);
}

void testCircularDependencyBreaksInitOrder() {
    PlatformRegistry registry;
    ModuleDescriptor first = module("first");
    first.dependencies.push_back("second");
    ModuleDescriptor second = module("second");
    second.dependencies.push_back("first");
    registerOk(registry, std::move(first));
    registerOk(registry, std::move(second));

    const PlatformRegistry::InitOrder order = registry.resolveInitOrder();
    CHECK(!order.ok);
    CHECK(order.error.find("circular dependency") != std::string::npos);
    CHECK(order.error.find("first") != std::string::npos);
    CHECK(order.error.find("second") != std::string::npos);
}

void testLookupRejectsUnknownModules() {
    PlatformRegistry registry;
    registerOk(registry, module("alpha"));
    CHECK(registry.find("beta") == nullptr);
    CHECK(registry.moduleIds().size() == 1);
    CHECK(registry.moduleIds()[0] == "alpha");
}

void testRegistrationIsTransactionalOnConflict() {
    PlatformRegistry registry;
    registerOk(registry, module("alpha"));
    ModuleDescriptor beta = module("beta");
    beta.syncStreamId = "alpha";
    CHECK(!registry.registerModule(beta).ok);
    registerOk(registry, module("gamma"));
    CHECK(registry.size() == 2);
    CHECK(registry.contains("alpha"));
    CHECK(registry.contains("gamma"));
    CHECK(!registry.contains("beta"));
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, void (*)()>> tests = {
        {"accounting module registers with stable ids",
         testAccountingModuleRegistersWithStableIds},
        {"duplicate module id is rejected", testDuplicateModuleIdIsRejected},
        {"duplicate sync stream id is rejected",
         testDuplicateSyncStreamIdIsRejected},
        {"empty ids are rejected", testEmptyIdsAreRejected},
        {"init order honors dependencies", testInitOrderHonorsDependencies},
        {"missing dependency breaks init order",
         testMissingDependencyBreaksInitOrder},
        {"circular dependency breaks init order",
         testCircularDependencyBreaksInitOrder},
        {"lookup rejects unknown modules", testLookupRejectsUnknownModules},
        {"registration is transactional on conflict",
         testRegistrationIsTransactionalOnConflict},
    };

    int failures = 0;
    for (const auto& [name, test] : tests) {
        try {
            test();
            std::cout << "PASS: " << name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "FAIL: " << name << " - " << error.what() << '\n';
        }
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    std::cout << tests.size() << " test(s) passed\n";
    return 0;
}
