/*
 * ===========================================================================
 * 文件名称: budget.cpp
 * 所属模块: backend
 * 功能描述: BudgetManager 类实现 —— 预算记录的增删改查和持久化。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#include "budget.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

BudgetManager::BudgetManager(const std::string& dataDir)
    : m_dataDir(dataDir)
{
    m_filePath = m_dataDir + "/budgets.dat";
    // 确保数据目录存在
    MKDIR(m_dataDir.c_str());
}

void BudgetManager::load()
{
    m_budgets.clear();
    std::ifstream file(m_filePath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // 格式: month|category|amount
        std::stringstream ss(line);
        std::string month, category, amountStr;
        if (std::getline(ss, month, '|') &&
            std::getline(ss, category, '|') &&
            std::getline(ss, amountStr)) {
            Budget b;
            b.month = month;
            b.category = category;
            try {
                b.amount = std::stod(amountStr);
            } catch (...) {
                b.amount = 0.0;
            }
            m_budgets.push_back(b);
        }
    }
    file.close();
}

void BudgetManager::save()
{
    std::ofstream file(m_filePath, std::ios::trunc);
    if (!file.is_open()) return;

    for (const auto& b : m_budgets) {
        file << b.month << "|"
             << b.category << "|"
             << std::fixed << std::setprecision(2) << b.amount << "\n";
    }
    file.close();
}

void BudgetManager::setBudget(const std::string& month, const std::string& category, double amount)
{
    // 查找是否已存在
    for (auto& b : m_budgets) {
        if (b.month == month && b.category == category) {
            b.amount = amount;
            save();
            return;
        }
    }
    // 不存在则新增
    Budget b;
    b.month = month;
    b.category = category;
    b.amount = amount;
    m_budgets.push_back(b);
    save();
}

Budget BudgetManager::getBudget(const std::string& month, const std::string& category) const
{
    for (const auto& b : m_budgets) {
        if (b.month == month && b.category == category) {
            return b;
        }
    }
    Budget empty;
    empty.month = month;
    empty.category = category;
    return empty;
}

std::vector<Budget> BudgetManager::getMonthBudgets(const std::string& month) const
{
    std::vector<Budget> result;
    for (const auto& b : m_budgets) {
        if (b.month == month) {
            result.push_back(b);
        }
    }
    return result;
}

bool BudgetManager::deleteBudget(const std::string& month, const std::string& category)
{
    auto it = std::find_if(m_budgets.begin(), m_budgets.end(),
        [&](const Budget& b) {
            return b.month == month && b.category == category;
        });
    if (it != m_budgets.end()) {
        m_budgets.erase(it);
        save();
        return true;
    }
    return false;
}

void BudgetManager::updateSpent(const std::string& month,
                                 const std::map<std::string, double>& expenseByCategory,
                                 double totalExpense)
{
    for (auto& b : m_budgets) {
        if (b.month != month) continue;
        if (b.category == "OVERALL") {
            b.spent = totalExpense;
        } else {
            auto it = expenseByCategory.find(b.category);
            b.spent = (it != expenseByCategory.end()) ? it->second : 0.0;
        }
    }
}
