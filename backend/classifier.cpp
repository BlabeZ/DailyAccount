/*
 * ===========================================================================
 * 文件名称: classifier.cpp
 * 所属模块: backend
 * 功能描述: SmartClassifier 类实现 —— 关键词学习和分类建议。
 * 编码格式: UTF-8
 * ===========================================================================
 */

#include "classifier.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <map>
#include <set>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

SmartClassifier::SmartClassifier(const std::string& dataDir)
    : m_dataDir(dataDir)
{
    m_filePath = m_dataDir + "/keywords.dat";
    MKDIR(m_dataDir.c_str());
}

void SmartClassifier::load()
{
    m_keywordMap.clear();
    std::ifstream file(m_filePath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // 格式: keyword|category
        size_t sep = line.find('|');
        if (sep != std::string::npos) {
            std::string keyword = line.substr(0, sep);
            std::string category = line.substr(sep + 1);
            if (!keyword.empty() && !category.empty()) {
                m_keywordMap[keyword] = category;
            }
        }
    }
    file.close();
}

void SmartClassifier::save()
{
    std::ofstream file(m_filePath, std::ios::trunc);
    if (!file.is_open()) return;

    for (const auto& [keyword, category] : m_keywordMap) {
        file << keyword << "|" << category << "\n";
    }
    file.close();
}

std::vector<std::string> SmartClassifier::extractBigrams(const std::string& text)
{
    std::vector<std::string> result;
    if (text.length() < 2) return result;

    // 提取所有连续2字词组
    for (size_t i = 0; i < text.length() - 1; i++) {
        std::string bigram = text.substr(i, 2);
        result.push_back(bigram);
    }
    // 也提取3字词组（提高匹配精度）
    for (size_t i = 0; i + 2 < text.length(); i++) {
        std::string trigram = text.substr(i, 3);
        result.push_back(trigram);
    }
    return result;
}

void SmartClassifier::learn(const std::vector<Record>& records)
{
    // 第一步：统计每个(keyword, category)出现的次数
    // 结构: keyword → (category → count)
    std::map<std::string, std::map<std::string, int>> stats;

    for (const auto& rec : records) {
        if (rec.note.empty()) continue;

        auto bigrams = extractBigrams(rec.note);
        for (const auto& bg : bigrams) {
            stats[bg][rec.category]++;
        }
    }

    // 第二步：对于每个关键词，找出最频繁的分类
    // 如果某个分类占比 >= 60%，则建立映射
    for (const auto& [keyword, catCounts] : stats) {
        int totalCount = 0;
        std::string bestCat;
        int bestCount = 0;

        for (const auto& [cat, count] : catCounts) {
            totalCount += count;
            if (count > bestCount) {
                bestCount = count;
                bestCat = cat;
            }
        }

        // 出现次数至少2次，且占比 >= 60%，且该关键词尚未映射
        if (totalCount >= 2 && bestCount > 0 &&
            (double)bestCount / totalCount >= 0.6 &&
            m_keywordMap.find(keyword) == m_keywordMap.end()) {
            m_keywordMap[keyword] = bestCat;
        }
    }

    save();
}

std::string SmartClassifier::suggest(const std::string& note, RecordType type) const
{
    if (note.empty()) return "";

    // 直接匹配：检查note中是否包含已知关键词
    // 优先匹配更长的关键词（更精确）
    std::string bestKeyword;
    std::string bestCategory;
    size_t bestLen = 0;

    for (const auto& [keyword, category] : m_keywordMap) {
        if (note.find(keyword) != std::string::npos) {
            // 检查分类是否匹配记录类型
            // （简单处理：支出关键词通常对应支出分类，收入对应收入分类）
            if (keyword.length() > bestLen) {
                bestLen = keyword.length();
                bestKeyword = keyword;
                bestCategory = category;
            }
        }
    }

    return bestCategory;
}

void SmartClassifier::addMapping(const std::string& keyword, const std::string& category)
{
    m_keywordMap[keyword] = category;
    save();
}

void SmartClassifier::removeMapping(const std::string& keyword)
{
    m_keywordMap.erase(keyword);
    save();
}
