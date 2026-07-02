/*
 * ===========================================================================
 * 文件名称: goal.h
 * 所属模块: backend（后端业务逻辑层）
 * 功能描述: 定义储蓄目标模块的数据结构和 GoalManager 类。
 *           支持设定储蓄目标、追踪进度、计算每月需存金额。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#ifndef GOAL_H
#define GOAL_H

#include <string>
#include <vector>
#include <ctime>

// ============================================================================
// 结构体: SavingsGoal
// ----------------------------------------------------------------------------
// 描述: 单条储蓄目标记录
// 字段:
//   id           - 唯一编号
//   name         - 目标名称（如 "年底旅行基金"）
//   targetAmount - 目标金额
//   currentSaved - 目前已存金额（运行时从账本结余计算）
//   deadline     - 截止日期，格式 "YYYY-MM-DD"
//   createdDate  - 创建日期，格式 "YYYY-MM-DD"
// ============================================================================
struct SavingsGoal {
    int id = 0;
    std::string name;            // 目标名称
    double targetAmount = 0.0;   // 目标金额
    double currentSaved = 0.0;   // 目前已存
    std::string deadline;        // 截止日期
    std::string createdDate;     // 创建日期

    // 计算剩余月份数
    int remainingMonths() const;

    // 每月需存金额
    double monthlyNeeded() const;

    // 每天需存金额
    double dailyNeeded() const;

    // 进度百分比 0~100
    double progressPercent() const {
        if (targetAmount <= 0.0) return 0.0;
        double pct = (currentSaved / targetAmount) * 100.0;
        return (pct > 100.0) ? 100.0 : pct;
    }

    // 是否已完成
    bool isCompleted() const { return currentSaved >= targetAmount; }
};

// ============================================================================
// 类: GoalManager
// ----------------------------------------------------------------------------
// 描述: 管理所有储蓄目标的增删改查和持久化
//
// 存储格式 (data/goals.dat):
//   id|name|targetAmount|currentSaved|deadline|createdDate
//   1|年底旅行基金|30000.00|5000.00|2026-12-31|2026-07-01
//
// 使用示例:
//   GoalManager gm("data");
//   gm.load();
//   SavingsGoal g;
//   g.name = "旅行基金";
//   g.targetAmount = 30000.00;
//   g.deadline = "2026-12-31";
//   gm.addGoal(g);
//   gm.save();
// ============================================================================
class GoalManager {
public:
    GoalManager(const std::string& dataDir = "data");

    // ---- 持久化 ----
    void load();
    void save();

    // ---- CRUD ----
    int addGoal(const SavingsGoal& goal);
    bool updateGoal(int id, const SavingsGoal& goal);
    bool deleteGoal(int id);
    SavingsGoal* findGoal(int id);
    const std::vector<SavingsGoal>& getAllGoals() const { return m_goals; }

    // ---- 进度更新 ----
    // 用当前总结余（总收入-总支出）更新所有目标的 currentSaved
    void updateProgress(double currentBalance);

private:
    int getNextId() const;

    std::string m_dataDir;
    std::string m_filePath;
    std::vector<SavingsGoal> m_goals;
};

#endif // GOAL_H
