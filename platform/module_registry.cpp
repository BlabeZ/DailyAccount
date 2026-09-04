#include "module_registry.h"

#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <utility>

namespace {

bool isEmptyId(const std::string& value) {
    return value.empty();
}

std::string joinNames(const std::vector<std::string>& names) {
    std::ostringstream output;
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (i != 0) output << ", ";
        output << names[i];
    }
    return output.str();
}

} // namespace

PlatformRegistry::Result PlatformRegistry::registerModule(
    const ModuleDescriptor& descriptor) {
    Result failure;
    failure.ok = false;

    if (isEmptyId(descriptor.id)) {
        failure.error = "module id must not be empty";
        return failure;
    }
    if (isEmptyId(descriptor.syncStreamId)) {
        failure.error = "sync stream id of module '" + descriptor.id +
                        "' must not be empty";
        return failure;
    }

    std::string conflict;
    if (conflictsWithRegisteredModule(descriptor, conflict)) {
        failure.error = "registration rejected for module '" + descriptor.id +
                        "' (version " + formatVersion(descriptor) + "): " +
                        conflict;
        return failure;
    }

    m_modules.push_back(descriptor);
    Result success;
    success.ok = true;
    return success;
}

bool PlatformRegistry::contains(const std::string& moduleId) const {
    return find(moduleId) != nullptr;
}

const ModuleDescriptor* PlatformRegistry::find(
    const std::string& moduleId) const {
    for (const ModuleDescriptor& module : m_modules) {
        if (module.id == moduleId) {
            return &module;
        }
    }
    return nullptr;
}

std::vector<std::string> PlatformRegistry::moduleIds() const {
    std::vector<std::string> ids;
    ids.reserve(m_modules.size());
    for (const ModuleDescriptor& module : m_modules) {
        ids.push_back(module.id);
    }
    return ids;
}

PlatformRegistry::InitOrder PlatformRegistry::resolveInitOrder() const {
    InitOrder result;

    std::set<std::string> registered;
    for (const ModuleDescriptor& module : m_modules) {
        registered.insert(module.id);
    }

    for (const ModuleDescriptor& module : m_modules) {
        for (const std::string& dependency : module.dependencies) {
            if (registered.find(dependency) == registered.end()) {
                result.ok = false;
                result.error = "module '" + module.id +
                               "' depends on missing module '" + dependency +
                               "'";
                return result;
            }
        }
    }

    std::map<std::string, std::vector<std::string>> dependents;
    std::map<std::string, std::size_t> unsatisfiedDependencyCount;
    for (const ModuleDescriptor& module : m_modules) {
        unsatisfiedDependencyCount[module.id] = module.dependencies.size();
        for (const std::string& dependency : module.dependencies) {
            dependents[dependency].push_back(module.id);
        }
    }

    std::vector<std::string> ready;
    for (const auto& entry : unsatisfiedDependencyCount) {
        if (entry.second == 0) {
            ready.push_back(entry.first);
        }
    }

    std::set<std::string> initialized;
    while (!ready.empty()) {
        std::string moduleId = std::move(ready.back());
        ready.pop_back();
        if (initialized.find(moduleId) != initialized.end()) {
            continue;
        }
        initialized.insert(moduleId);
        result.order.push_back(moduleId);
        for (const std::string& dependent : dependents[moduleId]) {
            const auto count = unsatisfiedDependencyCount.find(dependent);
            if (count != unsatisfiedDependencyCount.end() &&
                count->second > 0) {
                --count->second;
                if (count->second == 0) {
                    ready.push_back(dependent);
                }
            }
        }
    }

    if (result.order.size() != m_modules.size()) {
        std::vector<std::string> remaining;
        for (const ModuleDescriptor& module : m_modules) {
            if (initialized.find(module.id) == initialized.end()) {
                remaining.push_back(module.id);
            }
        }
        result.ok = false;
        result.error = "circular dependency among modules: " +
                       joinNames(remaining);
        return result;
    }

    result.ok = true;
    return result;
}

std::string PlatformRegistry::formatVersion(
    const ModuleDescriptor& descriptor) {
    return std::to_string(descriptor.versionMajor) + "." +
           std::to_string(descriptor.versionMinor) + "." +
           std::to_string(descriptor.versionPatch);
}

bool PlatformRegistry::conflictsWithRegisteredModule(
    const ModuleDescriptor& candidate,
    std::string& conflictDescription) const {
    for (const ModuleDescriptor& existing : m_modules) {
        if (existing.id == candidate.id) {
            conflictDescription = "duplicate module id (already registered as "
                                  "version " +
                                  formatVersion(existing) + ")";
            return true;
        }
        if (existing.syncStreamId == candidate.syncStreamId) {
            conflictDescription = "duplicate sync stream id '" +
                                  candidate.syncStreamId + "' owned by module '" +
                                  existing.id + "'";
            return true;
        }
    }
    return false;
}
