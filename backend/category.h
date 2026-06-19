/*
 * ===========================================================================
 * : category.h
 * : backend
 * :  CategoryManager   н
 *
 * :
 *   1. б"""н"
 *   2. б
 *   3. Щ飬÷
 *   4. 
 *
 * :
 *   У:
 *   - GUI 
 *   - 
 *   - а費в
 *
 * : GB2312
 * ===========================================================================
 */

#ifndef CATEGORY_H
#define CATEGORY_H

#include <string>    //  std::string  
#include <vector>    //  std::vector  б
#include <set>       //  std::set  ""+
#include "record.h"  //  RecordType INCOME / EXPENSE

// ============================================================================
// : CategoryManager
// ----------------------------------------------------------------------------
// : 
// :
//   е
//   з:
//     - : ""
//     - : н""
//
// :
//   1.   StorageManager 
//   2.   棨GUI//
//   3.   StorageManager 浽 categories.dat 
//
// :
//   - 1: 
//   - 2: 
//     ÷
//       
//
// :
//   -  Ledger  Щ
//   -  StorageManager洢  /
//   -  GUI/  б
// ============================================================================
class CategoryManager {
public:
    // =========================================================================
    // : CategoryManager()
    // : 
    //      ÷
    // 10: á
    // 8:  н
    // :  loadCategories 
    // =========================================================================
    CategoryManager();

    // =========================================================================
    // : getCategories
    // : б
    //      б =  + 
    // :
    //   type  RecordType::INCOME б
    //           RecordType::EXPENSE б
    // : std::vector<std::string>  б
    // ó: GUI//
    // =========================================================================
    std::vector<std::string> getCategories(RecordType type) const;

    // =========================================================================
    // : addCustomCategory
    // : 
    //      
    // :
    //   type  ÷ or 
    //   name   """""" 
    // : bool  true 
    //                  false : з
    // ó: ""·
    // =========================================================================
    bool addCustomCategory(RecordType type, const std::string& name);

    // =========================================================================
    // : removeCustomCategory
    // : 
    //      : (1)  (2) 
    // :
    //   type  ÷
    //   name  
    // : bool  true 
    //                  false     
    // =========================================================================
    bool removeCustomCategory(RecordType type, const std::string& name);

    // =========================================================================
    // : isPreset
    // : ж
    //      
    // :
    //   type  
    //   name  
    // : bool  true false 
    // =========================================================================
    bool isPreset(RecordType type, const std::string& name) const;

    // =========================================================================
    // : categoryExists
    // : 壩
    //      ·顣
    // :
    //   type  
    //   name  
    // : bool  true false 
    // =========================================================================
    bool categoryExists(RecordType type, const std::string& name) const;

    // =========================================================================
    // : setInUseCategories
    // : ""
    //       Ledger
    //      н CategoryManager
    // :
    //   expenseCategories  г
    //   incomeCategories   г
    // : ÷á
    //            ""
    // =========================================================================
    void setInUseCategories(const std::set<std::string>& expenseCategories,
                            const std::set<std::string>& incomeCategories);

    // =========================================================================
    // : getInUseCategories
    // : ""
    // : type  
    // : std::set<std::string>  
    // ó: Щн
    // =========================================================================
    std::set<std::string> getInUseCategories(RecordType type) const;

    // =========================================================================
    // : getCustomCategories
    // : б
    //        浽 categories.dat 
    // : std::vector<std::pair<RecordType, std::string>>
    //           (, ) 
    // : 棨
    // =========================================================================
    std::vector<std::pair<RecordType, std::string>> getCustomCategories() const;

    // =========================================================================
    // : addCustomCategoryFromStorage
    // : 飩
    //       addCustomCategory :
    //      - addCustomCategory 
    //      - addCustomCategoryFromStorage 
    // :
    //   type  
    //   name  
    // ó:  StorageManager::loadCategories 
    // =========================================================================
    void addCustomCategoryFromStorage(RecordType type, const std::string& name);

private:
    // =========================================================================
    // 洢
    // : m_  member
    // =========================================================================

    // ---- б----
    // 洢: "", "", "", "", "" 
    std::vector<std::string> m_presetExpense;

    // 洢: "н", "", "", "", "" 
    std::vector<std::string> m_presetIncome;

    // ---- б----
    // 洢: "", "", "" 
    std::vector<std::string> m_customExpense;

    // 洢: "", "", "" 
    std::vector<std::string> m_customIncome;

    // ---- ""----
    //  std::set  std::vector:
    //   set Ψ + O(log n)
    //   ""
    // 洢г
    std::set<std::string> m_inUseExpense;

    // 洢г
    std::set<std::string> m_inUseIncome;
};

#endif // CATEGORY_H
// ============================================================================
// : category.h
// """"
// ============================================================================
