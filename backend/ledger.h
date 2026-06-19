/*
 * ===========================================================================
 * : ledger.h
 * : backend
 * :  Ledger 塣
 *           Ledger "" ж
 *           顢 Ledger ɡ
 *
 * λ:
 *   Ledger λGUI/Console洢StorageManager:
 *        Ledger   Ledger   StorageManager 
 *         Ledger 
 *
 * : GB2312
 * ===========================================================================
 */

#ifndef LEDGER_H
#define LEDGER_H

#include <string>     //  std::string
#include <vector>     //  std::vector  б
#include <map>        //  std::map  
#include <set>        //  std::set  б
#include "record.h"      //  Record
#include "storage.h"      //  StorageManager
#include "category.h"     //  CategoryManager

// ============================================================================
// : DaySummary
// ----------------------------------------------------------------------------
// :  2026-06-19
//       ""Ул
// :
//   date               "2026-06-19"
//   totalIncome       
//   totalExpense      
//   recordCount
//   net()              =  - 
// ============================================================================
struct DaySummary {
    std::string date;                    // YYYY-MM-DD 
    double totalIncome = 0.0;           // 0
    double totalExpense = 0.0;          // 0
    int recordCount = 0;           // 0

    // ==
    double net() const { return totalIncome - totalExpense; }
};

// ============================================================================
// : CategorySummary
// ----------------------------------------------------------------------------
// : """н"
//       
// :
//   category      
//   type          ÷
//   totalAmount   ÷
//   count         ÷
//   percentage    ÷ 35.5  35.5%
// ============================================================================
struct CategorySummary {
    std::string category;               //  """н"
    RecordType type;               //  or 
    double totalAmount = 0.0;           // ÷
    int count = 0;                      // ÷
    double percentage = 0.0;            // 0~100
};

// ============================================================================
// : MonthSummary
// ----------------------------------------------------------------------------
// :  2026-06
//       
// :
//   month         · "2026-06"YYYY-MM
//   totalIncome   
//   totalExpense  
//   net()          =  - 
// ============================================================================
struct MonthSummary {
    std::string month;                  // ·YYYY-MM 7λ
    double totalIncome = 0.0;           // 
    double totalExpense = 0.0;          // 

    // ==
    double net() const { return totalIncome - totalExpense; }
};

// ============================================================================
// : Ledger
// ----------------------------------------------------------------------------
// :  / 
// :
//   нб
//   е"" 
//   
//
// :
//    CRUD
//     - addRecord()    
//     - findRecord()   ID
//     - updateRecord() м
//     - deleteRecord() 
//
//   
//     - getRecordsByDate()      
//     - getRecordsByDateRange() Χ
//     - getRecordsByCategory()  
//     - getUniqueDates()             мб
//
//   
//     - getDaySummary()        
//     - getDailySummaries()    б
//     - getMonthlySummaries()  б
//     - getCategorySummaries() б
//     - getTotalIncome()       
//     - getTotalExpense()      
//     - getBalance()           -
//
// :
//    Ledger  StorageManager 浽
//   "е"
// ============================================================================
class Ledger {
public:
    // =========================================================================
    // : Ledger
    // : 洢
    // :
    //   storage  洢
    //   catMan   /·
    // :  load() 
    // =========================================================================
    Ledger(StorageManager& storage, CategoryManager& catMan);

    // =========================================================================
    // 
    // load(): 洢棬
    // save(): е浽
    // : CRUD  save()á
    // =========================================================================
    void load();
    void save();

    // =========================================================================
    // CRUD   Create / Read / Update / Delete
    // =========================================================================

    // addRecord:  ID档
    // : t  id λ
    // : бЧ
    Record* addRecord(const Record& t);

    // updateRecord:  ID мΡ档
    // : id  ID; updated  
    // : true=, false=δID
    bool updateRecord(int id, const Record& updated);

    // deleteRecord:  ID 档
    // : id  ID
    // : true=, false=δID
    bool deleteRecord(int id);

    // findRecord:  ID 
    // : id  ID
    // : δ nullptr
    Record* findRecord(int id);

    // getAllRecords: Ч
    // : б
    const std::vector<Record>& getAllRecords() const { return m_records; }

    // =========================================================================
    //   
    // =========================================================================

    // : нС
    std::vector<Record> getRecordsByDate(const std::string& date) const;

    // Χ: н
    // : startDate/endDate   YYYY-MM-DD磩
    std::vector<Record> getRecordsByDateRange(
        const std::string& startDate, const std::string& endDate) const;

    // : н
    std::vector<Record> getRecordsByCategory(
        RecordType type, const std::string& category) const;

    // =========================================================================
    //   
    // =========================================================================

    // getUniqueDates: нб=
    std::vector<std::string> getUniqueDates() const;

    // getDaySummary: 
    DaySummary getDaySummary(const std::string& date) const;

    // =========================================================================
    //   
    // =========================================================================

    // бмУ
    std::vector<DaySummary> getDailySummaries() const;

    // бУ
    std::vector<MonthSummary> getMonthlySummaries() const;

    // б: 
    // У/棩
    std::vector<CategorySummary> getCategorySummaries(RecordType type) const;

    // =========================================================================
    //   
    // =========================================================================

    // м
    double getTotalIncome() const { return m_totalIncome; }

    // м
    double getTotalExpense() const { return m_totalExpense; }

    //  =  - 
    double getBalance() const { return m_totalIncome - m_totalExpense; }

    // =========================================================================
    // 
    // : нЩ
    //        CategoryManager 
    // : ν仯///
    // =========================================================================
    void refreshInUseCategories();

private:
    // ----  ----
    StorageManager& m_storage;    // 洢  д
    CategoryManager& m_catMan;    //   

    // ----  ----
    std::vector<Record> m_records;  // ебΨ

    // ---- ----
    double m_totalIncome = 0.0;    // 
    double m_totalExpense = 0.0;   // 

    // =========================================================================
    // и
    // =========================================================================

    // recalculateTotals: ±н m_totalIncome  m_totalExpense
    // ///
    void recalculateTotals();

    // getNextId:  StorageManager  ID
    int getNextId() const;
};

#endif // LEDGER_H
// ============================================================================
// : ledger.h
// ""
// ============================================================================
