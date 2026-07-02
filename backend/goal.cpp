/*
 * ===========================================================================
 * 文件名称: goal.cpp
 * 所属模块: backend
 * 功能描述: GoalManager 类实现 —— 储蓄目标的增删改查、进度计算和持久化。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#include "goal.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <sys/stat.h>
#include <ctime>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

// ---- 辅助函数：解析日期字符串为 struct tm ----
static bool parseDate(const std::string& dateStr, std::tm& tmOut)
{
    // 格式: YYYY-MM-DD
    if (dateStr.length() < 10) return false;
    try {
        tmOut.tm_year = std::stoi(dateStr.substr(0, 4)) - 1900;
        tmOut.tm_mon  = std::stoi(dateStr.substr(5, 2)) - 1;
        tmOut.tm_mday = std::stoi(dateStr.substr(8, 2));
        tmOut.tm_hour = 0;
        tmOut.tm_min = 0;
        tmOut.tm_sec = 0;
        return true;
    } catch (...) {
        return false;
    }
}

// ---- 辅助函数：获取今天日期字符串 ----
static std::string todayStr()
{
    std::time_t now = std::time(nullptr);
    std::tm* tmNow = std::localtime(&now);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                  tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday);
    return std::string(buf);
}

// ---- 辅助函数：两个日期之间的月份差 ----
static int monthsBetween(const std::string& from, const std::string& to)
{
    std::tm tmFrom{}, tmTo{};
    if (!parseDate(from, tmFrom) || !parseDate(to, tmTo)) return 1;
    int years = (tmTo.tm_year + 1900) - (tmFrom.tm_year + 1900);
    int months = tmTo.tm_mon - tmFrom.tm_mon;
    int totalMonths = years * 12 + months;
    return (totalMonths > 0) ? totalMonths : 1;  // 至少1个月
}

// ---- SavingsGoal 方法实现 ----

int SavingsGoal::remainingMonths() const
{
    return monthsBetween(todayStr(), deadline);
}

double SavingsGoal::monthlyNeeded() const
{
    double remaining = targetAmount - currentSaved;
    if (remaining <= 0.0) return 0.0;
    int months = remainingMonths();
    return remaining / months;
}

double SavingsGoal::dailyNeeded() const
{
    return monthlyNeeded() / 30.0;
}

// ---- GoalManager 实现 ----

GoalManager::GoalManager(const std::string& dataDir)
    : m_dataDir(dataDir)
{
    m_filePath = m_dataDir + "/goals.dat";
    MKDIR(m_dataDir.c_str());
}

void GoalManager::load()
{
    m_goals.clear();
    std::ifstream file(m_filePath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // 格式: id|name|targetAmount|currentSaved|deadline|createdDate
        std::stringstream ss(line);
        std::string idStr, name, targetStr, savedStr, deadline, createdDate;
        if (std::getline(ss, idStr, '|') &&
            std::getline(ss, name, '|') &&
            std::getline(ss, targetStr, '|') &&
            std::getline(ss, savedStr, '|') &&
            std::getline(ss, deadline, '|') &&
            std::getline(ss, createdDate)) {
            SavingsGoal g;
            try {
                g.id = std::stoi(idStr);
                g.targetAmount = std::stod(targetStr);
                g.currentSaved = std::stod(savedStr);
            } catch (...) {
                continue;
            }
            g.name = name;
            g.deadline = deadline;
            g.createdDate = createdDate;
            m_goals.push_back(g);
        }
    }
    file.close();
}

void GoalManager::save()
{
    std::ofstream file(m_filePath, std::ios::trunc);
    if (!file.is_open()) return;

    for (const auto& g : m_goals) {
        file << g.id << "|"
             << g.name << "|"
             << std::fixed << std::setprecision(2) << g.targetAmount << "|"
             << std::fixed << std::setprecision(2) << g.currentSaved << "|"
             << g.deadline << "|"
             << g.createdDate << "\n";
    }
    file.close();
}

int GoalManager::getNextId() const
{
    int maxId = 0;
    for (const auto& g : m_goals) {
        if (g.id > maxId) maxId = g.id;
    }
    return maxId + 1;
}

int GoalManager::addGoal(const SavingsGoal& goal)
{
    SavingsGoal g = goal;
    g.id = getNextId();
    if (g.createdDate.empty()) {
        g.createdDate = todayStr();
    }
    m_goals.push_back(g);
    save();
    return g.id;
}

bool GoalManager::updateGoal(int id, const SavingsGoal& goal)
{
    for (auto& g : m_goals) {
        if (g.id == id) {
            g.name = goal.name;
            g.targetAmount = goal.targetAmount;
            g.deadline = goal.deadline;
            save();
            return true;
        }
    }
    return false;
}

bool GoalManager::deleteGoal(int id)
{
    auto it = std::find_if(m_goals.begin(), m_goals.end(),
        [id](const SavingsGoal& g) { return g.id == id; });
    if (it != m_goals.end()) {
        m_goals.erase(it);
        save();
        return true;
    }
    return false;
}

SavingsGoal* GoalManager::findGoal(int id)
{
    for (auto& g : m_goals) {
        if (g.id == id) return &g;
    }
    return nullptr;
}

void GoalManager::updateProgress(double currentBalance)
{
    // 将当前总结余分配为所有目标的 currentSaved
    // 简单策略：每个目标平均分配结余
    // 实际应用中，可以按目标创建时间或优先级分配
    if (m_goals.empty() || currentBalance <= 0.0) return;

    double perGoal = currentBalance / m_goals.size();
    for (auto& g : m_goals) {
        g.currentSaved = std::min(perGoal, g.targetAmount);
    }
}
