#ifndef STORAGE_H
#define STORAGE_H

#include "record.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

enum class StorageFormat {
    Empty,
    Legacy,
    V3
};

struct StoredData {
    std::vector<Record> records;
    std::vector<std::pair<RecordType, std::string>> customCategories;
    std::int64_t nextId = 1;
    StorageFormat format = StorageFormat::Empty;
};

class LedgerStorage {
public:
    virtual ~LedgerStorage() = default;
    virtual bool load(StoredData& data) = 0;
    virtual bool save(const StoredData& data) = 0;
    virtual const std::string& lastError() const = 0;
};

class StorageManager final : public LedgerStorage {
public:
    explicit StorageManager(std::filesystem::path dataDirectory = "data");

    bool load(StoredData& data) override;
    bool save(const StoredData& data) override;
    const std::string& lastError() const override { return m_lastError; }

    bool isReady() const { return m_ready; }
    bool hasBackup() const;
    bool loadBackup(StoredData& data);
    bool restoreBackup();

    const std::filesystem::path& dataDirectory() const { return m_dataDirectory; }
    const std::filesystem::path& dataFilePath() const { return m_dataFile; }
    const std::filesystem::path& backupFilePath() const { return m_backupFile; }

private:
    bool ensureDataDirectory();
    bool recoverInterruptedSave();

    std::filesystem::path m_dataDirectory;
    std::filesystem::path m_dataFile;
    std::filesystem::path m_backupFile;
    std::filesystem::path m_legacyRecordsFile;
    std::filesystem::path m_legacyCategoriesFile;
    std::string m_lastError;
    bool m_ready = false;
};

#endif // STORAGE_H
