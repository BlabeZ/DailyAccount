/*
 * ===========================================================================
 * 文件名称: category.cpp
 * 所属模块: backend
 * 功能描述: CategoryManager 实现 —— 预设分类定义和自定义分类管理。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#include "category.h"
#include <algorithm>

CategoryManager::CategoryManager() {
    // =========================================================================
    // 预设支出分类（13个）—— 涵盖个人日常记账的主要支出场景
    // =========================================================================
    m_presetExpense = {
        "饮食",       // 三餐、外卖、零食、水果、饮品等
        "交通",       // 公交、地铁、打车、加油、停车、共享单车
        "房租",       // 每月房租
        "水电气",     // 水费、电费、燃气费、暖气费
        "通讯",       // 手机话费、宽带网费、快递费
        "购物",       // 日用品、衣物鞋帽、数码产品、家居家电
        "娱乐",       // 电影、游戏、KTV、旅游、运动健身
        "医疗",       // 看病挂号、买药、体检、牙科
        "教育",       // 学费、培训、考试费、书本资料
        "居住",       // 物业费、维修保养、家居用品、清洁
        "服饰美容",   // 衣物、鞋包、化妆品、理发
        "人情往来",   // 红包、请客聚会、送礼、婚礼
        "其他"        // 不属于以上分类的支出
    };

    // =========================================================================
    // 预设收入分类（10个）—— 涵盖个人日常记账的主要收入来源
    // =========================================================================
    m_presetIncome = {
        "工资",       // 全职工作的月薪、底薪
        "奖金",       // 年终奖、季度奖、项目奖金、绩效
        "稿费",       // 写作、翻译、设计等稿酬收入
        "视频收益",   // 短视频平台、自媒体平台收益
        "投资收益",   // 股票、基金、理财、房租收入
        "兼职",       // 周末或业余兼职收入
        "个人转账",   // 亲友转账、AA制收款
        "退款",       // 购物退款、押金退还
        "礼金",       // 红包、压岁钱、份子钱
        "其他"        // 不属于以上分类的收入
    };
}

std::vector<std::string> CategoryManager::getCategories(RecordType type) const {
    const auto& preset = (type == RecordType::EXPENSE) ? m_presetExpense : m_presetIncome;
    const auto& custom = (type == RecordType::EXPENSE) ? m_customExpense : m_customIncome;

    std::vector<std::string> result;
    result.reserve(preset.size() + custom.size());
    for (const auto& c : preset) result.push_back(c);
    for (const auto& c : custom) result.push_back(c);
    return result;
}

bool CategoryManager::addCustomCategory(RecordType type, const std::string& name) {
    if (name.empty()) return false;

    auto all = getCategories(type);
    if (std::find(all.begin(), all.end(), name) != all.end()) {
        return false;
    }

    auto& custom = (type == RecordType::EXPENSE) ? m_customExpense : m_customIncome;
    custom.push_back(name);
    return true;
}

bool CategoryManager::removeCustomCategory(RecordType type, const std::string& name) {
    if (isPreset(type, name)) return false;

    const auto& inUse = (type == RecordType::EXPENSE) ? m_inUseExpense : m_inUseIncome;
    if (inUse.find(name) != inUse.end()) return false;

    auto& custom = (type == RecordType::EXPENSE) ? m_customExpense : m_customIncome;
    auto it = std::find(custom.begin(), custom.end(), name);
    if (it == custom.end()) return false;
    custom.erase(it);
    return true;
}

bool CategoryManager::isPreset(RecordType type, const std::string& name) const {
    const auto& preset = (type == RecordType::EXPENSE) ? m_presetExpense : m_presetIncome;
    return std::find(preset.begin(), preset.end(), name) != preset.end();
}

bool CategoryManager::categoryExists(RecordType type, const std::string& name) const {
    auto all = getCategories(type);
    return std::find(all.begin(), all.end(), name) != all.end();
}

void CategoryManager::setInUseCategories(const std::set<std::string>& expenseCategories,
                                         const std::set<std::string>& incomeCategories) {
    m_inUseExpense = expenseCategories;
    m_inUseIncome = incomeCategories;
}

std::set<std::string> CategoryManager::getInUseCategories(RecordType type) const {
    return (type == RecordType::EXPENSE) ? m_inUseExpense : m_inUseIncome;
}

std::vector<std::pair<RecordType, std::string>> CategoryManager::getCustomCategories() const {
    std::vector<std::pair<RecordType, std::string>> result;
    for (const auto& c : m_customExpense)
        result.emplace_back(RecordType::EXPENSE, c);
    for (const auto& c : m_customIncome)
        result.emplace_back(RecordType::INCOME, c);
    return result;
}

void CategoryManager::addCustomCategoryFromStorage(RecordType type, const std::string& name) {
    auto& custom = (type == RecordType::EXPENSE) ? m_customExpense : m_customIncome;
    if (std::find(custom.begin(), custom.end(), name) == custom.end()) {
        custom.push_back(name);
    }
}
