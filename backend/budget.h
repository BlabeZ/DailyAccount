/*
 * ===========================================================================
 * 文件名称: budget.h
 * 所属模块: backend（后端业务逻辑层）
 * 功能描述: 定义预算管理模块的数据结构和 BudgetManager 类。
 *           支持按月、按分类设置预算，追踪已花费金额和剩余额度。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#ifndef BUDGET_H
#define BUDGET_H

#include <string>
#include <vector>
#include <map>

// ============================================================================
// 结构体: Budget
// ----------------------------------------------------------------------------
// 描述: 单条预算记录，按月份+分类维度存储
// 字段:
//   month     - 预算所属月份，格式 "YYYY-MM"（如 "2026-07"）
//   category  - 预算分类名称，"OVERALL" 表示总预算
//   amount    - 预算金额（正数）
//   spent     - 已花费金额（从账本动态计算，非持久化字段）
// ============================================================================
struct Budget {
    std::string month;       // "2026-07"
    std::string category;    // "OVERALL" 或具体分类名如 "饮食"
    double amount = 0.0;     // 预算金额
    double spent = 0.0;      // 已花费（运行时计算）

    double remaining() const { return amount - spent; }
    double percentage() const {
        if (amount <= 0.0) return 0.0;
        return (spent / amount) * 100.0;
    }
};

// ============================================================================
// 类: BudgetManager
// ----------------------------------------------------------------------------
// 描述: 管理所有预算记录的增删改查和持久化
//
// 职责:
//   1. 设置和查询月度/分类预算
//   2. 从 Ledger 同步已花费金额
//   3. 持久化到 data/budgets.dat（管道符分隔文本文件）
//
// 存储格式 (data/budgets.dat):
//   2026-07|OVERALL|5000.00
//   2026-07|饮食|2000.00
//   2026-07|交通|500.00
//
// 使用示例:
//   BudgetManager bm("data");
//   bm.load();
//   bm.setBudget("2026-07", "OVERALL", 5000.00);
//   bm.syncSpent(ledger);  // 从账本同步本月已花费
//   Budget b = bm.getBudget("2026-07", "OVERALL");
//   double pct = b.percentage();  // 已花费百分比
//   bm.save();
// ============================================================================
class BudgetManager {
public:
    // 构造函数：传入数据目录路径（默认为 "data"）
    BudgetManager(const std::string& dataDir = "data");

    // ---- 持久化 ----
    void load();   // 从 data/budgets.dat 加载预算数据
    void save();   // 保存预算数据到 data/budgets.dat

    // ---- 预算 CRUD ----
    // 设置某月某分类的预算金额（已存在则覆盖）
    void setBudget(const std::string& month, const std::string& category, double amount);

    // 获取某月某分类的预算（不存在则返回 amount=0 的默认 Budget）
    Budget getBudget(const std::string& month, const std::string& category) const;

    // 获取某月的所有预算记录
    std::vector<Budget> getMonthBudgets(const std::string& month) const;

    // 删除某月某分类的预算
    bool deleteBudget(const std::string& month, const std::string& category);

    // 获取所有预算记录
    const std::vector<Budget>& getAllBudgets() const { return m_budgets; }

    // ---- 运行时计算 ----
    // 根据外部传入的（分类 → 已花费金额）映射，更新当前月份各预算的 spent 字段
    // expenseByCategory: 主分类名 → 本月支出总额
    void updateSpent(const std::string& month,
                     const std::map<std::string, double>& expenseByCategory,
                     double totalExpense);

private:
    std::string m_dataDir;              // 数据目录
    std::string m_filePath;             // 完整文件路径
    std::vector<Budget> m_budgets;      // 所有预算记录
};

#endif // BUDGET_H
