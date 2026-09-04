#ifndef CATEGORY_H
#define CATEGORY_H

#include "record.h"

#include <set>
#include <string>
#include <utility>
#include <vector>

class CategoryManager {
public:
    CategoryManager();

    std::vector<std::string> getCategories(RecordType type) const;
    bool addCustomCategory(RecordType type, const std::string& name);
    bool addStoredCategory(RecordType type, const std::string& name);
    bool removeCustomCategory(RecordType type, const std::string& name);
    void clearCustomCategories();

    bool isPreset(RecordType type, const std::string& name) const;
    bool categoryExists(RecordType type, const std::string& name) const;

    void setInUseCategories(const std::set<std::string>& expenseCategories,
                            const std::set<std::string>& incomeCategories);
    std::set<std::string> getInUseCategories(RecordType type) const;

    std::vector<std::pair<RecordType, std::string>> getCustomCategories() const;

private:
    static bool hasValidContent(const std::string& name);

    std::vector<std::string> m_presetExpense;
    std::vector<std::string> m_presetIncome;
    std::vector<std::string> m_customExpense;
    std::vector<std::string> m_customIncome;
    std::set<std::string> m_inUseExpense;
    std::set<std::string> m_inUseIncome;
};

#endif // CATEGORY_H
