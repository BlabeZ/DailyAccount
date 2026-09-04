#ifndef MODULE_DESCRIPTOR_H
#define MODULE_DESCRIPTOR_H

#include <cstdint>
#include <string>
#include <vector>

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

inline const char* capabilityToName(PlatformCapability capability) {
    switch (capability) {
    case PlatformCapability::Accounts:
        return "accounts";
    case PlatformCapability::DataLocations:
        return "data-locations";
    case PlatformCapability::SyncScheduling:
        return "sync-scheduling";
    case PlatformCapability::SecureStorage:
        return "secure-storage";
    case PlatformCapability::Notifications:
        return "notifications";
    case PlatformCapability::SettingsAndTheme:
        return "settings-and-theme";
    case PlatformCapability::LoggingAndDiagnostics:
        return "logging-and-diagnostics";
    case PlatformCapability::ImportExportBackup:
        return "import-export-backup";
    }
    return "unknown";
}

struct ModuleDescriptor {
    std::string id;
    std::uint32_t versionMajor = 0;
    std::uint32_t versionMinor = 0;
    std::uint32_t versionPatch = 0;
    std::vector<std::string> dependencies;
    std::vector<PlatformCapability> requiredCapabilities;
    std::string syncStreamId;
    int databaseSchemaVersion = 0;
    bool essential = false;
};

#endif // MODULE_DESCRIPTOR_H
