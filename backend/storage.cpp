/*
 * ===========================================================================
 * 文件名称: storage.cpp
 * 所属模块: backend
 * 功能描述: StorageManager 实现 —— 流水记录和分类的磁盘读写。
 * 存储格式（管道符分隔，共6个字段）:
 *   id|date|type|amount|category|note
 * 编码格式: UTF-8
 * ===========================================================================
 */

#include "storage.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir_impl(path) _mkdir(path)
#else
#include <sys/stat.h>
#define mkdir_impl(path) mkdir(path, 0755)
#endif

StorageManager::StorageManager(const std::string& dataDir)
    : m_dataDir(dataDir)
    , m_recFile(dataDir + "/records.dat")
    , m_catFile(dataDir + "/categories.dat")
{
    ensureDataDir();
}

void StorageManager::ensureDataDir() {
    mkdir_impl(m_dataDir.c_str());
}

// 加载流水记录: 逐行解析 id|date|type|amount|category|note（6个字段）
std::vector<Record> StorageManager::loadRecords() {
    std::vector<Record> result;
    std::ifstream fin(m_recFile.c_str());
    if (!fin.is_open()) return result;

    std::string line;
    int maxId = 0;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;

        // 按 | 分割，但只分6段（备注可能含 |）
        std::vector<std::string> parts;
        std::stringstream ss(line);
        std::string part;
        int fieldCount = 0;
        while (std::getline(ss, part, '|')) {
            parts.push_back(part);
            fieldCount++;
            // 第5次分割后剩余内容全部作为备注
            if (fieldCount == 5 && !ss.eof()) {
                std::string rest;
                std::getline(ss, rest);
                parts.push_back(rest);
                break;
            }
        }

        if (parts.size() < 5) continue; // 至少需要5个字段

        try {
            Record t;
            t.id       = std::stoi(parts[0]);  // 字段0: ID
            t.date     = parts[1];              // 字段1: 日期
            t.type     = stringToType(parts[2]);// 字段2: 类型
            t.amount   = std::stod(parts[3]);   // 字段3: 金额
            t.category = parts[4];              // 字段4: 分类
            t.note     = (parts.size() >= 6) ? parts[5] : ""; // 字段5: 备注

            if (t.id > maxId) maxId = t.id;
            result.push_back(t);
        } catch (...) {
            continue;
        }
    }

    m_nextId = maxId + 1;
    return result;
}

// 保存流水记录: 输出格式 id|date|type|amount|category|note
bool StorageManager::saveRecords(const std::vector<Record>& records) {
    std::ofstream fout(m_recFile.c_str());
    if (!fout.is_open()) return false;

    for (const auto& t : records) {
        fout << t.id << '|'
             << t.date << '|'
             << typeToString(t.type) << '|'
             << std::fixed << std::setprecision(2) << t.amount << '|'
             << t.category << '|'
             << t.note << '\n';
    }
    return true;
}

// 加载自定义分类
void StorageManager::loadCategories(CategoryManager& catMan) {
    std::ifstream fin(m_catFile.c_str());
    if (!fin.is_open()) return;

    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        auto pos = line.find('|');
        if (pos == std::string::npos) continue;
        std::string typeStr = line.substr(0, pos);
        std::string name = line.substr(pos + 1);
        RecordType type = stringToType(typeStr);
        catMan.addCustomCategoryFromStorage(type, name);
    }
}

// 保存自定义分类
bool StorageManager::saveCategories(const CategoryManager& catMan) {
    std::ofstream fout(m_catFile.c_str());
    if (!fout.is_open()) return false;

    auto customs = catMan.getCustomCategories();
    for (const auto& [type, name] : customs) {
        fout << typeToString(type) << '|' << name << '\n';
    }
    return true;
}
