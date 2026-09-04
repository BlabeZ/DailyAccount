#include "accounting_module.h"

#include "module_registry.h"

#include <utility>

namespace accounting {

ModuleDescriptor createModuleDescriptor() {
    ModuleDescriptor descriptor;
    descriptor.id = kModuleId;
    descriptor.versionMajor = 1;
    descriptor.versionMinor = 0;
    descriptor.versionPatch = 0;
    descriptor.requiredCapabilities = {
        PlatformCapability::Accounts,
        PlatformCapability::DataLocations,
        PlatformCapability::SyncScheduling,
        PlatformCapability::SecureStorage,
        PlatformCapability::Notifications,
        PlatformCapability::SettingsAndTheme,
        PlatformCapability::LoggingAndDiagnostics,
        PlatformCapability::ImportExportBackup
    };
    descriptor.syncStreamId = kSyncStreamId;
    descriptor.essential = true;
    return descriptor;
}

bool registerAccountingCore(PlatformRegistry& registry, std::string& error) {
    const PlatformRegistry::Result result =
        registry.registerModule(createModuleDescriptor());
    if (!result.ok) {
        error = result.error;
    }
    return result.ok;
}

} // namespace accounting
