#include "category.h"

#include <algorithm>
#include <cctype>

namespace {

bool validRecordType(RecordType type) {
    return type == RecordType::EXPENSE || type == RecordType::INCOME;
}

} // namespace

CategoryManager::CategoryManager()
    : m_presetExpense{
          "饮食", "交通", "房租", "水电气", "通讯", "购物", "娱乐",
          "医疗", "教育", "居住", "服饰美容", "人情往来", "其他"}
    , m_presetIncome{
          "工资", "奖金", "稿费", "视频收益", "投资收益", "兼职",
          "个人转账", "退款", "礼金", "其他"} {}

std::vector<std::string> CategoryManager::getCategories(RecordType type) const {
    if (!validRecordType(type)) return {};
    const auto& preset = type == RecordType::EXPENSE ? m_presetExpense : m_presetIncome;
    const auto& custom = type == RecordType::EXPENSE ? m_customExpense : m_customIncome;

    std::vector<std::string> result;
    result.reserve(preset.size() + custom.size());
    result.insert(result.end(), preset.begin(), preset.end());
    result.insert(result.end(), custom.begin(), custom.end());
    return result;
}

bool CategoryManager::hasValidContent(const std::string& name) {
    if (name.empty()) return false;
    return std::any_of(name.begin(), name.end(), [](unsigned char character) {
        return !std::isspace(character);
    });
}

bool CategoryManager::addCustomCategory(RecordType type, const std::string& name) {
    if (name.size() > 256) return false;
    return addStoredCategory(type, name);
}

bool CategoryManager::addStoredCategory(RecordType type, const std::string& name) {
    if (!validRecordType(type) || !hasValidContent(name) ||
        categoryExists(type, name)) {
        return false;
    }
    auto& custom = type == RecordType::EXPENSE ? m_customExpense : m_customIncome;
    custom.push_back(name);
    return true;
}

bool CategoryManager::removeCustomCategory(RecordType type, const std::string& name) {
    if (!validRecordType(type) || isPreset(type, name)) return false;

    const auto& inUse = type == RecordType::EXPENSE ? m_inUseExpense : m_inUseIncome;
    if (inUse.find(name) != inUse.end()) return false;

    auto& custom = type == RecordType::EXPENSE ? m_customExpense : m_customIncome;
    const auto it = std::find(custom.begin(), custom.end(), name);
    if (it == custom.end()) return false;
    custom.erase(it);
    return true;
}

void CategoryManager::clearCustomCategories() {
    m_customExpense.clear();
    m_customIncome.clear();
    m_inUseExpense.clear();
    m_inUseIncome.clear();
}

bool CategoryManager::isPreset(RecordType type, const std::string& name) const {
    if (!validRecordType(type)) return false;
    const auto& preset = type == RecordType::EXPENSE ? m_presetExpense : m_presetIncome;
    return std::find(preset.begin(), preset.end(), name) != preset.end();
}

bool CategoryManager::categoryExists(RecordType type, const std::string& name) const {
    if (!validRecordType(type)) return false;
    const auto categories = getCategories(type);
    return std::find(categories.begin(), categories.end(), name) != categories.end();
}

void CategoryManager::setInUseCategories(
    const std::set<std::string>& expenseCategories,
    const std::set<std::string>& incomeCategories) {
    m_inUseExpense = expenseCategories;
    m_inUseIncome = incomeCategories;
}

std::set<std::string> CategoryManager::getInUseCategories(RecordType type) const {
    if (!validRecordType(type)) return {};
    return type == RecordType::EXPENSE ? m_inUseExpense : m_inUseIncome;
}

std::vector<std::pair<RecordType, std::string>>
CategoryManager::getCustomCategories() const {
    std::vector<std::pair<RecordType, std::string>> result;
    result.reserve(m_customExpense.size() + m_customIncome.size());
    for (const auto& category : m_customExpense) {
        result.emplace_back(RecordType::EXPENSE, category);
    }
    for (const auto& category : m_customIncome) {
        result.emplace_back(RecordType::INCOME, category);
    }
    return result;
}
