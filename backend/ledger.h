#ifndef LEDGER_H
#define LEDGER_H

#include "category.h"
#include "record.h"
#include "storage.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct DaySummary {
    std::string date;
    Money totalIncome = 0;
    Money totalExpense = 0;
    int recordCount = 0;

    Money net() const { return totalIncome - totalExpense; }
};

struct CategorySummary {
    std::string category;
    RecordType type = RecordType::EXPENSE;
    Money totalAmount = 0;
    int count = 0;
    double percentage = 0.0;
};

struct MonthSummary {
    std::string month;
    Money totalIncome = 0;
    Money totalExpense = 0;

    Money net() const { return totalIncome - totalExpense; }
};

class Ledger {
public:
    explicit Ledger(LedgerStorage& storage);

    bool load();
    bool save();
    bool saveTo(LedgerStorage& storage);
    bool loadedLegacyFormat() const { return m_loadedLegacyFormat; }
    const std::string& lastError() const { return m_lastError; }

    bool addRecord(const Record& record);
    bool updateRecord(int id, const Record& updated);
    bool deleteRecord(int id);
    std::optional<Record> findRecord(int id) const;
    std::vector<Record> getAllRecords() const { return m_records; }
    const CategoryManager& categories() const { return m_categoryManager; }

    bool addCustomCategory(RecordType type, const std::string& name);
    bool removeCustomCategory(RecordType type, const std::string& name);
    bool clearAllData();

    std::vector<Record> getRecordsByDate(const std::string& date) const;
    std::vector<Record> getRecordsByDateRange(
        const std::string& startDate, const std::string& endDate) const;
    std::vector<Record> getRecordsByCategory(
        RecordType type, const std::string& category) const;

    std::vector<std::string> getUniqueDates() const;
    DaySummary getDaySummary(const std::string& date) const;
    std::vector<DaySummary> getDailySummaries() const;
    std::vector<MonthSummary> getMonthlySummaries() const;
    std::vector<CategorySummary> getCategorySummaries(RecordType type) const;
    std::vector<CategorySummary> getCategorySummaries(
        RecordType type, const std::string& startDate,
        const std::string& endDate) const;

    Money getTotalIncome() const { return m_totalIncome; }
    Money getTotalExpense() const { return m_totalExpense; }
    Money getBalance() const { return m_totalIncome - m_totalExpense; }

private:
    bool persistCandidate(std::vector<Record> records,
                          CategoryManager categories,
                          std::int64_t nextId);
    std::vector<CategorySummary> categorySummaries(
        RecordType type, const std::string* startDate,
        const std::string* endDate) const;

    LedgerStorage& m_storage;
    CategoryManager m_categoryManager;
    std::vector<Record> m_records;
    std::int64_t m_nextId = 1;
    Money m_totalIncome = 0;
    Money m_totalExpense = 0;
    std::string m_lastError;
    bool m_loadedLegacyFormat = false;
};

#endif // LEDGER_H
