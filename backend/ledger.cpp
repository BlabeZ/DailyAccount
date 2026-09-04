#include "ledger.h"

#include <algorithm>
#include <climits>
#include <map>
#include <set>
#include <utility>

namespace {

constexpr std::int64_t kMaxNextId = static_cast<std::int64_t>(INT_MAX) + 1;

bool validRecordType(RecordType type) {
    return type == RecordType::INCOME || type == RecordType::EXPENSE;
}

bool calculateTotals(const std::vector<Record>& records, Money& income,
                     Money& expense, std::string& error) {
    income = 0;
    expense = 0;
    for (const auto& record : records) {
        Money& total = record.type == RecordType::INCOME ? income : expense;
        Money updated = 0;
        if (!checkedAddMoney(total, record.amountCents, updated)) {
            error = "金额合计超出可表示范围。";
            return false;
        }
        total = updated;
    }
    return true;
}

void updateInUseCategories(CategoryManager& categories,
                           const std::vector<Record>& records) {
    std::set<std::string> expenseCategories;
    std::set<std::string> incomeCategories;
    for (const auto& record : records) {
        if (record.type == RecordType::EXPENSE) {
            expenseCategories.insert(record.category);
        } else {
            incomeCategories.insert(record.category);
        }
    }
    categories.setInUseCategories(expenseCategories, incomeCategories);
}

bool validateRecord(const Record& record, const CategoryManager& categories,
                    std::string& error) {
    if (!isValidIsoDate(record.date)) {
        error = "记录日期无效，应为 0100-01-01 至 9999-12-31 的 YYYY-MM-DD 格式。";
        return false;
    }
    if (!validRecordType(record.type)) {
        error = "记录类型无效。";
        return false;
    }
    if (record.amountCents <= 0 || record.amountCents > kMaxRecordAmountCents) {
        error = "记录金额必须大于 0 且不超过 99,999,999.99。";
        return false;
    }
    if (record.category.empty() ||
        !categories.categoryExists(record.type, record.category)) {
        error = "记录分类不存在。";
        return false;
    }
    return true;
}

bool splitKnownLegacySubcategory(Record& record,
                                 const CategoryManager& categories) {
    if (record.type != RecordType::EXPENSE ||
        categories.categoryExists(record.type, record.category)) {
        return false;
    }

    static const std::map<std::string, std::set<std::string>> known = {
        {"饮食", {"早饭", "午饭", "晚饭", "夜宵", "小吃", "聚餐", "其他"}},
        {"交通", {"公交", "地铁", "打车", "共享单车", "火车", "飞机", "其他"}},
    };

    for (const auto& [parent, children] : known) {
        const std::string prefix = parent + "(";
        if (record.category.size() <= prefix.size() ||
            record.category.compare(0, prefix.size(), prefix) != 0 ||
            record.category.back() != ')') {
            continue;
        }
        const std::string child = record.category.substr(
            prefix.size(), record.category.size() - prefix.size() - 1);
        if (children.find(child) == children.end()) continue;
        record.category = parent;
        record.subcategory = child;
        return true;
    }
    return false;
}

StoredData makeStoredData(const std::vector<Record>& records,
                           const CategoryManager& categories,
                           std::int64_t nextId) {
    StoredData data;
    data.records = records;
    data.customCategories = categories.getCustomCategories();
    std::set<std::pair<RecordType, std::string>> catalog(
        data.customCategories.begin(), data.customCategories.end());
    for (const auto& record : records) {
        const auto entry = std::make_pair(record.type, record.category);
        if (catalog.insert(entry).second) {
            data.customCategories.push_back(entry);
        }
    }
    data.nextId = nextId;
    data.format = StorageFormat::V3;
    return data;
}

} // namespace

Ledger::Ledger(LedgerStorage& storage)
    : m_storage(storage) {}

bool Ledger::load() {
    StoredData loaded;
    if (!m_storage.load(loaded)) {
        m_lastError = m_storage.lastError();
        return false;
    }

    CategoryManager candidateCategories;
    std::set<std::pair<RecordType, std::string>> catalogRows;
    for (const auto& [type, name] : loaded.customCategories) {
        if (!validRecordType(type) || !catalogRows.emplace(type, name).second ||
            (!candidateCategories.isPreset(type, name) &&
             !candidateCategories.addStoredCategory(type, name))) {
            m_lastError = "存储中包含无效或重复的自定义分类：" + name;
            return false;
        }
    }

    std::set<int> ids;
    int maxId = 0;
    for (auto& record : loaded.records) {
        if (loaded.format == StorageFormat::Legacy) {
            splitKnownLegacySubcategory(record, candidateCategories);
            if (!candidateCategories.categoryExists(record.type, record.category) &&
                !candidateCategories.addStoredCategory(record.type, record.category)) {
                m_lastError = "旧数据中包含无法恢复的分类：" + record.category;
                return false;
            }
        }
        if (!validateRecord(record, candidateCategories, m_lastError)) return false;
        if (record.id <= 0 || !ids.insert(record.id).second) {
            m_lastError = "记录 ID 必须为不重复的正整数。";
            return false;
        }
        maxId = std::max(maxId, record.id);
    }
    if (loaded.nextId < 1 || loaded.nextId > kMaxNextId ||
        loaded.nextId <= maxId) {
        m_lastError = "存储中的下一个记录 ID 无效。";
        return false;
    }

    Money income = 0;
    Money expense = 0;
    if (!calculateTotals(loaded.records, income, expense, m_lastError)) return false;
    updateInUseCategories(candidateCategories, loaded.records);

    m_records.swap(loaded.records);
    m_categoryManager = std::move(candidateCategories);
    m_nextId = loaded.nextId;
    m_totalIncome = income;
    m_totalExpense = expense;
    m_loadedLegacyFormat = loaded.format == StorageFormat::Legacy;
    m_lastError.clear();
    return true;
}

bool Ledger::save() {
    if (!saveTo(m_storage)) return false;
    m_loadedLegacyFormat = false;
    return true;
}

bool Ledger::saveTo(LedgerStorage& storage) {
    const StoredData data = makeStoredData(m_records, m_categoryManager, m_nextId);
    if (!storage.save(data)) {
        m_lastError = storage.lastError();
        return false;
    }
    m_lastError.clear();
    return true;
}

bool Ledger::persistCandidate(std::vector<Record> records,
                              CategoryManager categories,
                              std::int64_t nextId) {
    std::set<int> ids;
    int maxId = 0;
    for (const auto& record : records) {
        if (!validateRecord(record, categories, m_lastError)) return false;
        if (record.id <= 0 || !ids.insert(record.id).second) {
            m_lastError = "记录 ID 必须为不重复的正整数。";
            return false;
        }
        maxId = std::max(maxId, record.id);
    }
    if (nextId < 1 || nextId > kMaxNextId || nextId <= maxId) {
        m_lastError = "下一个记录 ID 无效。";
        return false;
    }

    Money income = 0;
    Money expense = 0;
    if (!calculateTotals(records, income, expense, m_lastError)) return false;
    updateInUseCategories(categories, records);

    const StoredData data = makeStoredData(records, categories, nextId);
    if (!m_storage.save(data)) {
        m_lastError = m_storage.lastError();
        return false;
    }

    m_records.swap(records);
    m_categoryManager = std::move(categories);
    m_nextId = nextId;
    m_totalIncome = income;
    m_totalExpense = expense;
    m_loadedLegacyFormat = false;
    m_lastError.clear();
    return true;
}

bool Ledger::addRecord(const Record& record) {
    if (m_nextId > INT_MAX) {
        m_lastError = "记录 ID 已耗尽，无法继续添加。";
        return false;
    }
    if (!validateRecord(record, m_categoryManager, m_lastError)) return false;

    std::vector<Record> records = m_records;
    Record added = record;
    added.id = static_cast<int>(m_nextId);
    records.push_back(std::move(added));
    return persistCandidate(std::move(records), m_categoryManager, m_nextId + 1);
}

bool Ledger::updateRecord(int id, const Record& updated) {
    if (!validateRecord(updated, m_categoryManager, m_lastError)) return false;

    std::vector<Record> records = m_records;
    const auto found = std::find_if(records.begin(), records.end(),
        [id](const Record& record) { return record.id == id; });
    if (found == records.end()) {
        m_lastError = "未找到要修改的记录。";
        return false;
    }

    Record replacement = updated;
    replacement.id = id;
    *found = std::move(replacement);
    return persistCandidate(std::move(records), m_categoryManager, m_nextId);
}

bool Ledger::deleteRecord(int id) {
    std::vector<Record> records = m_records;
    const auto found = std::find_if(records.begin(), records.end(),
        [id](const Record& record) { return record.id == id; });
    if (found == records.end()) {
        m_lastError = "未找到要删除的记录。";
        return false;
    }
    records.erase(found);
    return persistCandidate(std::move(records), m_categoryManager, m_nextId);
}

std::optional<Record> Ledger::findRecord(int id) const {
    const auto found = std::find_if(m_records.begin(), m_records.end(),
        [id](const Record& record) { return record.id == id; });
    return found == m_records.end() ? std::nullopt
                                    : std::optional<Record>(*found);
}

bool Ledger::addCustomCategory(RecordType type, const std::string& name) {
    if (!validRecordType(type)) {
        m_lastError = "分类类型无效。";
        return false;
    }
    CategoryManager categories = m_categoryManager;
    if (!categories.addCustomCategory(type, name)) {
        m_lastError = "分类名称已存在、为空或过长。";
        return false;
    }
    return persistCandidate(m_records, std::move(categories), m_nextId);
}

bool Ledger::removeCustomCategory(RecordType type, const std::string& name) {
    if (!validRecordType(type)) {
        m_lastError = "分类类型无效。";
        return false;
    }
    if (m_categoryManager.isPreset(type, name)) {
        m_lastError = "预设分类不可删除。";
        return false;
    }
    const auto inUse = m_categoryManager.getInUseCategories(type);
    if (inUse.find(name) != inUse.end()) {
        m_lastError = "该分类正在被记录使用。";
        return false;
    }

    CategoryManager categories = m_categoryManager;
    if (!categories.removeCustomCategory(type, name)) {
        m_lastError = "未找到要删除的自定义分类。";
        return false;
    }
    return persistCandidate(m_records, std::move(categories), m_nextId);
}

bool Ledger::clearAllData() {
    CategoryManager categories;
    return persistCandidate({}, std::move(categories), 1);
}

std::vector<Record> Ledger::getRecordsByDate(const std::string& date) const {
    std::vector<Record> result;
    for (const auto& record : m_records) {
        if (record.date == date) result.push_back(record);
    }
    return result;
}

std::vector<Record> Ledger::getRecordsByDateRange(
    const std::string& startDate, const std::string& endDate) const {
    std::vector<Record> result;
    if (startDate > endDate) return result;
    for (const auto& record : m_records) {
        if (record.date >= startDate && record.date <= endDate) {
            result.push_back(record);
        }
    }
    return result;
}

std::vector<Record> Ledger::getRecordsByCategory(
    RecordType type, const std::string& category) const {
    std::vector<Record> result;
    for (const auto& record : m_records) {
        if (record.type == type && record.category == category) {
            result.push_back(record);
        }
    }
    return result;
}

std::vector<std::string> Ledger::getUniqueDates() const {
    std::set<std::string> dates;
    for (const auto& record : m_records) dates.insert(record.date);
    return {dates.rbegin(), dates.rend()};
}

DaySummary Ledger::getDaySummary(const std::string& date) const {
    DaySummary summary;
    summary.date = date;
    for (const auto& record : m_records) {
        if (record.date != date) continue;
        if (record.type == RecordType::INCOME) {
            summary.totalIncome += record.amountCents;
        } else {
            summary.totalExpense += record.amountCents;
        }
        ++summary.recordCount;
    }
    return summary;
}

std::vector<DaySummary> Ledger::getDailySummaries() const {
    std::map<std::string, DaySummary> summaries;
    for (const auto& record : m_records) {
        auto& summary = summaries[record.date];
        summary.date = record.date;
        if (record.type == RecordType::INCOME) {
            summary.totalIncome += record.amountCents;
        } else {
            summary.totalExpense += record.amountCents;
        }
        ++summary.recordCount;
    }

    std::vector<DaySummary> result;
    for (const auto& [date, summary] : summaries) {
        static_cast<void>(date);
        result.push_back(summary);
    }
    std::sort(result.begin(), result.end(),
        [](const DaySummary& left, const DaySummary& right) {
            return left.date > right.date;
        });
    return result;
}

std::vector<MonthSummary> Ledger::getMonthlySummaries() const {
    std::map<std::string, MonthSummary> summaries;
    for (const auto& record : m_records) {
        const std::string month = record.date.substr(0, 7);
        auto& summary = summaries[month];
        summary.month = month;
        if (record.type == RecordType::INCOME) {
            summary.totalIncome += record.amountCents;
        } else {
            summary.totalExpense += record.amountCents;
        }
    }

    std::vector<MonthSummary> result;
    for (const auto& [month, summary] : summaries) {
        static_cast<void>(month);
        result.push_back(summary);
    }
    std::sort(result.begin(), result.end(),
        [](const MonthSummary& left, const MonthSummary& right) {
            return left.month > right.month;
        });
    return result;
}

std::vector<CategorySummary> Ledger::categorySummaries(
    RecordType type, const std::string* startDate,
    const std::string* endDate) const {
    std::map<std::string, CategorySummary> summaries;
    Money total = 0;
    for (const auto& record : m_records) {
        if (record.type != type) continue;
        if (startDate && endDate &&
            (record.date < *startDate || record.date > *endDate)) {
            continue;
        }
        auto& summary = summaries[record.category];
        summary.category = record.category;
        summary.type = type;
        summary.totalAmount += record.amountCents;
        ++summary.count;
        total += record.amountCents;
    }

    std::vector<CategorySummary> result;
    for (auto& [category, summary] : summaries) {
        static_cast<void>(category);
        summary.percentage = total > 0
            ? static_cast<double>(summary.totalAmount) /
                  static_cast<double>(total) * 100.0
            : 0.0;
        result.push_back(summary);
    }
    std::sort(result.begin(), result.end(),
        [](const CategorySummary& left, const CategorySummary& right) {
            return left.totalAmount > right.totalAmount;
        });
    return result;
}

std::vector<CategorySummary> Ledger::getCategorySummaries(RecordType type) const {
    return categorySummaries(type, nullptr, nullptr);
}

std::vector<CategorySummary> Ledger::getCategorySummaries(
    RecordType type, const std::string& startDate,
    const std::string& endDate) const {
    if (startDate > endDate) return {};
    return categorySummaries(type, &startDate, &endDate);
}
