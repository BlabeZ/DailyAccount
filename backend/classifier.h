/*
 * ===========================================================================
 * 文件名称: classifier.h
 * 所属模块: backend（后端业务逻辑层）
 * 功能描述: 定义 SmartClassifier 类 —— 基于关键词的智能分类引擎。
 *           从历史记录中学习备注关键词与分类的对应关系，自动建议分类。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include <string>
#include <vector>
#include <map>
#include "record.h"

// ============================================================================
// 类: SmartClassifier
// ----------------------------------------------------------------------------
// 描述: 基于关键词匹配的智能分类建议引擎
//
// 核心思路:
//   1. 从历史记录中提取备注关键词 → 分类的映射关系
//   2. 用户输入备注时，匹配已知关键词，自动建议分类
//   3. 支持手动添加/编辑/删除映射规则
//
// 学习算法:
//   对每条有备注的记录，提取备注中的所有2字词组（中文双字词），
//   统计每个词组出现时对应的分类频率。若某词组在某个分类下的出现
//   频率 >= 阈值（默认80%），则建立映射。
//
// 存储格式 (data/keywords.dat):
//   瑞幸|饮食(咖啡)
//   地铁|交通(地铁)
//   淘宝|购物
//
// 使用示例:
//   SmartClassifier sc("data");
//   sc.load();
//   sc.learn(ledger.getAllRecords());  // 从历史记录学习
//   std::string cat = sc.suggest("瑞幸咖啡", RecordType::EXPENSE);
//   // cat = "饮食(咖啡)"
//   sc.save();
// ============================================================================
class SmartClassifier {
public:
    // 构造函数：传入数据目录路径
    SmartClassifier(const std::string& dataDir = "data");

    // ---- 持久化 ----
    void load();   // 从 data/keywords.dat 加载映射
    void save();   // 保存映射到 data/keywords.dat

    // ---- 学习 ----
    // 从一组记录中学习关键词→分类映射
    // 提取所有记录备注中的2字词组，统计与分类的关联度
    void learn(const std::vector<Record>& records);

    // ---- 分类建议 ----
    // 根据备注文本和记录类型，返回建议的分类名称
    // 返回空字符串表示无建议
    std::string suggest(const std::string& note, RecordType type) const;

    // ---- 手动管理映射 ----
    // 手动添加/更新关键词映射
    void addMapping(const std::string& keyword, const std::string& category);

    // 删除关键词映射
    void removeMapping(const std::string& keyword);

    // 获取所有映射（用于UI展示）
    const std::map<std::string, std::string>& getAllMappings() const { return m_keywordMap; }

private:
    // 从文本中提取所有2字词组（中文双字词）
    // 例如 "瑞幸咖啡" → {"瑞幸", "幸咖", "咖啡"}
    static std::vector<std::string> extractBigrams(const std::string& text);

    std::string m_dataDir;
    std::string m_filePath;
    std::map<std::string, std::string> m_keywordMap;  // 关键词 → 分类
};

#endif // CLASSIFIER_H
