/*
 * ===========================================================================
 * 文件名称: ledger.cpp
 * 所属模块: backend
 * 功能描述: Ledger 类实现 —— 流水记录的增删改查和统计汇总。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#include "ledger.h"
#include <algorithm>
#include <map>
#include <set>

Ledger::Ledger(StorageManager& storage, CategoryManager& catMan)
    : m_storage(storage), m_catMan(catMan) {}

void Ledger::load() {
    m_records = m_storage.loadRecords();
    recalculateTotals();
    refreshInUseCategories();
}

void Ledger::save() {
    m_storage.saveRecords(m_records);
    m_storage.saveCategories(m_catMan);
}

int Ledger::getNextId() const {
    return m_storage.getNextId();
}

Record* Ledger::addRecord(const Record& t) {
    Record newT = t;
    newT.id = m_storage.getNextId();
    m_records.push_back(newT);
    m_storage.setNextId(newT.id + 1);
    recalculateTotals();
    refreshInUseCategories();
    save();
    return &m_records.back();
}

bool Ledger::updateRecord(int id, const Record& updated) {
    for (auto& t : m_records) {
        if (t.id == id) {
            t.date     = updated.date;
            t.type     = updated.type;
            t.amount   = updated.amount;
            t.category = updated.category;
            t.note     = updated.note;
            recalculateTotals();
            refreshInUseCategories();
            save();
            return true;
        }
    }
    return false;
}

bool Ledger::deleteRecord(int id) {
    auto it = std::find_if(m_records.begin(), m_records.end(),
        [id](const Record& t) { return t.id == id; });
    if (it != m_records.end()) {
        m_records.erase(it);
        recalculateTotals();
        refreshInUseCategories();
        save();
        return true;
    }
    return false;
}

Record* Ledger::findRecord(int id) {
    for (auto& t : m_records) {
        if (t.id == id) return &t;
    }
    return nullptr;
}

// 按日期查询（不再按时序排列，因为已无 time 字段）
std::vector<Record> Ledger::getRecordsByDate(const std::string& date) const {
    std::vector<Record> result;
    for (const auto& t : m_records) {
        if (t.date == date) result.push_back(t);
    }
    return result;
}

std::vector<Record> Ledger::getRecordsByDateRange(
    const std::string& startDate, const std::string& endDate) const {
    std::vector<Record> result;
    for (const auto& t : m_records) {
        if (t.date >= startDate && t.date <= endDate) result.push_back(t);
    }
    return result;
}

std::vector<Record> Ledger::getRecordsByCategory(
    RecordType type, const std::string& category) const {
    std::vector<Record> result;
    for (const auto& t : m_records) {
        if (t.type == type && t.category == category) result.push_back(t);
    }
    return result;
}

std::vector<std::string> Ledger::getUniqueDates() const {
    std::set<std::string> dates;
    for (const auto& t : m_records) {
        dates.insert(t.date);
    }
    std::vector<std::string> result(dates.rbegin(), dates.rend());
    return result;
}

DaySummary Ledger::getDaySummary(const std::string& date) const {
    DaySummary summary;
    summary.date = date;
    for (const auto& t : m_records) {
        if (t.date == date) {
            if (t.type == RecordType::INCOME)
                summary.totalIncome += t.amount;
            else
                summary.totalExpense += t.amount;
            summary.recordCount++;
        }
    }
    return summary;
}

std::vector<DaySummary> Ledger::getDailySummaries() const {
    std::map<std::string, DaySummary> map;
    for (const auto& t : m_records) {
        auto& ds = map[t.date];
        ds.date = t.date;
        if (t.type == RecordType::INCOME)
            ds.totalIncome += t.amount;
        else
            ds.totalExpense += t.amount;
        ds.recordCount++;
    }
    std::vector<DaySummary> result;
    for (auto& [date, ds] : map) result.push_back(ds);
    std::sort(result.begin(), result.end(),
        [](const DaySummary& a, const DaySummary& b) { return a.date > b.date; });
    return result;
}

std::vector<MonthSummary> Ledger::getMonthlySummaries() const {
    std::map<std::string, MonthSummary> map;
    for (const auto& t : m_records) {
        std::string month = t.date.substr(0, 7);
        auto& ms = map[month];
        ms.month = month;
        if (t.type == RecordType::INCOME)
            ms.totalIncome += t.amount;
        else
            ms.totalExpense += t.amount;
    }
    std::vector<MonthSummary> result;
    for (auto& [m, ms] : map) result.push_back(ms);
    std::sort(result.begin(), result.end(),
        [](const MonthSummary& a, const MonthSummary& b) { return a.month > b.month; });
    return result;
}

std::vector<CategorySummary> Ledger::getCategorySummaries(RecordType type) const {
    std::map<std::string, CategorySummary> map;
    double grandTotal = 0.0;
    for (const auto& t : m_records) {
        if (t.type != type) continue;
        auto& cs = map[t.category];
        cs.category = t.category;
        cs.type = type;
        cs.totalAmount += t.amount;
        cs.count++;
        grandTotal += t.amount;
    }
    std::vector<CategorySummary> result;
    for (auto& [cat, cs] : map) {
        cs.percentage = (grandTotal > 0) ? (cs.totalAmount / grandTotal * 100.0) : 0.0;
        result.push_back(cs);
    }
    std::sort(result.begin(), result.end(),
        [](const CategorySummary& a, const CategorySummary& b) {
            return a.totalAmount > b.totalAmount;
        });
    return result;
}

void Ledger::recalculateTotals() {
    m_totalIncome = 0.0;
    m_totalExpense = 0.0;
    for (const auto& t : m_records) {
        if (t.type == RecordType::INCOME)
            m_totalIncome += t.amount;
        else
            m_totalExpense += t.amount;
    }
}

void Ledger::refreshInUseCategories() {
    std::set<std::string> expenseCats, incomeCats;
    for (const auto& t : m_records) {
        if (t.type == RecordType::EXPENSE)
            expenseCats.insert(t.category);
        else
            incomeCats.insert(t.category);
    }
    m_catMan.setInUseCategories(expenseCats, incomeCats);
}
