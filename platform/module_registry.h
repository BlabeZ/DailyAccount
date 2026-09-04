#ifndef MODULE_REGISTRY_H
#define MODULE_REGISTRY_H

#include "module_descriptor.h"

#include <cstddef>
#include <string>
#include <vector>

class PlatformRegistry {
public:
    struct Result {
        bool ok = false;
        std::string error;
    };

    Result registerModule(const ModuleDescriptor& descriptor);
    bool contains(const std::string& moduleId) const;
    const ModuleDescriptor* find(const std::string& moduleId) const;
    std::vector<std::string> moduleIds() const;
    std::size_t size() const { return m_modules.size(); }

    struct InitOrder {
        bool ok = false;
        std::vector<std::string> order;
        std::string error;
    };

    InitOrder resolveInitOrder() const;

private:
    static std::string formatVersion(const ModuleDescriptor& descriptor);
    bool conflictsWithRegisteredModule(const ModuleDescriptor& candidate,
                                       std::string& conflictDescription) const;

    std::vector<ModuleDescriptor> m_modules;
};

#endif // MODULE_REGISTRY_H
