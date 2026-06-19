/*
 * ===========================================================================
 * 文件名称: record.h
 * 所属模块: backend（后端业务逻辑层）
 * 功能描述: 定义整个记账工具的核心数据结构 —— 流水记录。
 *           每条记录精确到日期，不再包含时间字段。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#ifndef RECORD_H
#define RECORD_H

#include <string>
#include <vector>

// 记录类型枚举 —— 收入（INCOME）或支出（EXPENSE）
enum class RecordType {
    INCOME,
    EXPENSE
};

// 流水记录结构体 —— 包含一条记账记录的全部信息（共6个字段）
// 字段: id(编号) | date(日期) | type(类型) | amount(金额) | category(分类) | note(备注)
struct Record {
    int id = 0;                                     // 唯一编号，系统自动分配
    std::string date;                               // 日期，格式 YYYY-MM-DD
    RecordType type = RecordType::EXPENSE;           // 记录类型
    double amount = 0.0;                            // 金额（正数），保留两位小数
    std::string category;                           // 分类名称
    std::string note;                               // 备注说明（可选）

    // 获取带符号金额：收入返回正数，支出返回负数
    double signedAmount() const {
        return (type == RecordType::INCOME) ? amount : -amount;
    }
};

// 工具函数 —— 字符串 → 记录类型枚举
inline RecordType stringToType(const std::string& s) {
    return (s == "INCOME") ? RecordType::INCOME : RecordType::EXPENSE;
}

// 工具函数 —— 记录类型枚举 → 英文字符串（存储用）
inline std::string typeToString(RecordType t) {
    return (t == RecordType::INCOME) ? "INCOME" : "EXPENSE";
}

// 工具函数 —— 记录类型枚举 → 中文字符串（显示用）
inline std::string typeToChinese(RecordType t) {
    return (t == RecordType::INCOME) ? "收入" : "支出";
}

#endif // RECORD_H
