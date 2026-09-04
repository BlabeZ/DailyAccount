#ifndef ACCOUNTING_MODULE_H
#define ACCOUNTING_MODULE_H

#include "module_descriptor.h"

#include <string>

class PlatformRegistry;

namespace accounting {

constexpr const char* kModuleId = "accounting";
constexpr const char* kSyncStreamId = "accounting";

ModuleDescriptor createModuleDescriptor();

bool registerAccountingCore(PlatformRegistry& registry, std::string& error);

} // namespace accounting

#endif // ACCOUNTING_MODULE_H
