/*
 * ===========================================================================
 * : storage.h
 * : backend
 * :  StorageManager   /д
 *
 * :
 *   洢:
 *     1. records.dat  洢н
 *     2. categories.dat   洢
 *
 * :
 *   - 谲SQLite/MySQL 
 *   - ü±/
 *   - 
 *   - С
 *     
 *
 * :
 *   records.dat :
 *     id|date|time|type|amount|category|note
 *     : 1|2026-06-19|12:30|EXPENSE|35.50||
 *
 *   categories.dat :
 *     INCOME|    EXPENSE|
 *     : EXPENSE|
 *            INCOME|
 *
 * :
 *    data/ С
 *    data/ в
 *
 * : GB2312
 * ===========================================================================
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <string>    //  std::string  ·
#include <vector>    //  std::vector  б
#include "record.h"      //  Record  RecordType
#include "category.h"     //  CategoryManager /

// ============================================================================
// : StorageManager
// ----------------------------------------------------------------------------
// : 洢 / 
// :
//   е浽
//   档
//   "浵/"顣
//
// :
//   1.  data/ 
//   2. 棨д
//   3. 棨д
//   4.  IDΨ
//
// е:
//   : loadRecords()   records.dat 
//           loadCategories()     categories.dat 
//   : //  浽
//   : 棩
// ============================================================================
class StorageManager {
public:
    // =========================================================================
    // : StorageManager
    // : 洢·
    // :
    //   dataDir   "data"
    //             
    // :
    //   1. ·: dataDir/records.dat  dataDir/categories.dat
    //   2.  ensureDataDir() 
    // :
    //   StorageManager storage("data");  //  ./data/ 
    // =========================================================================
    StorageManager(const std::string& dataDir = "data");

    // =========================================================================
    // : loadRecords
    // -------------------------------------------------------------------------
    // :  records.dat мн档
    //
    // :
    //   id|date|time|type|amount|category|note
    //   ù | 7Ρ
    //    | Σ
    //
    // :
    //   - 
    //   - 6  
    //   -   
    //   -  ID  ID
    //
    // :
    //   std::vector<Record>  гб
    //   б
    // =========================================================================
    std::vector<Record> loadRecords();

    // =========================================================================
    // : saveRecords
    // -------------------------------------------------------------------------
    // : енд records.dat 
    //       д棩
    //
    //  vs :
    //   : емд顣
    //   
    //   á
    //
    // : λС 35.50  35.5 std::fixed + std::setprecision(2)
    //
    // :
    //   records  б
    // :
    //   bool  true false д
    // =========================================================================
    bool saveRecords(const std::vector<Record>& records);

    // =========================================================================
    // : loadCategories
    // -------------------------------------------------------------------------
    // :  categories.dat м CategoryManager
    //
    // :
    //   EXPENSE|    INCOME|
    //
    // :
    //   - 
    //   -  | :   
    //   -  CategoryManager::addCustomCategoryFromStorage 
    //
    // :  CategoryManager 
    //
    // :
    //   catMan  CategoryManager 
    // =========================================================================
    void loadCategories(CategoryManager& catMan);

    // =========================================================================
    // : saveCategories
    // -------------------------------------------------------------------------
    // :  CategoryManager е浽 categories.dat 
    //       棨δ
    //
    // :
    //   catMan  CategoryManager б
    // :
    //   bool  true false 
    // =========================================================================
    bool saveCategories(const CategoryManager& catMan);

    // =========================================================================
    // ID 
    // -------------------------------------------------------------------------
    // getNextId: 
    // setNextId:  ID 
    //
    // ID :
    //   1.  records.dat  ID
    //   2.  m_nextId  maxId + 1
    //   3.  m_nextId  m_nextId 
    //   4.  ID Ψ
    // =========================================================================
    int getNextId() const { return m_nextId; }
    void setNextId(int id) { m_nextId = id; }

private:
    // =========================================================================
    // 
    // =========================================================================

    // · "data"
    std::string m_dataDir;

    // · "data/records.dat"
    std::string m_recFile;

    // · "data/categories.dat"
    std::string m_catFile;

    //  ID 1
    int m_nextId = 1;

    // =========================================================================
    // и: ensureDataDir
    // : 
    //       дá
    //       ò API:
    //         Windows  _mkdir()
    //         Linux/macOS  mkdir()
    // =========================================================================
    void ensureDataDir();
};

#endif // STORAGE_H
// ============================================================================
// : storage.h
// ""
// ============================================================================
