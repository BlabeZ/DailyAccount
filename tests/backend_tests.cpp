#include "category.h"
#include "ledger.h"
#include "record.h"
#include "storage.h"

#include <cmath>
#include <climits>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

namespace {

class TestFailure : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            throw TestFailure(std::string("CHECK failed: ") + #condition + \
                              " at " + __FILE__ + ":" + std::to_string(__LINE__)); \
        } \
    } while (false)

class TempDirectory {
public:
    TempDirectory() {
        static unsigned long counter = 0;
        m_path = fs::temp_directory_path() /
                 ("dailyaccount-tests-" + std::to_string(++counter));
        std::error_code ec;
        fs::remove_all(m_path, ec);
        if (!fs::create_directories(m_path, ec) || ec) {
            throw TestFailure("cannot create temporary test directory");
        }
    }

    ~TempDirectory() {
        std::error_code ec;
        fs::remove_all(m_path, ec);
    }

    const fs::path& path() const { return m_path; }

private:
    fs::path m_path;
};

void writeText(const fs::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output || !(output << content)) {
        throw TestFailure("cannot write test fixture: " + path.string());
    }
}

std::string readText(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input),
                       std::istreambuf_iterator<char>());
}

Record expense(std::string date, Money amountCents, std::string category,
               std::string note = {}) {
    Record record;
    record.date = std::move(date);
    record.type = RecordType::EXPENSE;
    record.amountCents = amountCents;
    record.category = std::move(category);
    record.note = std::move(note);
    return record;
}

class FakeStorage : public LedgerStorage {
public:
    bool load(StoredData& data) override {
        if (failLoad) {
            m_error = "injected load failure";
            return false;
        }
        data = persisted;
        m_error.clear();
        return true;
    }

    bool save(const StoredData& data) override {
        ++saveCalls;
        if (failSave) {
            m_error = "injected save failure";
            return false;
        }
        persisted = data;
        m_error.clear();
        return true;
    }

    const std::string& lastError() const override { return m_error; }

    StoredData persisted;
    bool failLoad = false;
    bool failSave = false;
    int saveCalls = 0;

private:
    std::string m_error;
};

void testMoneyHelpersAreExactAndChecked() {
    Money cents = 0;
    CHECK(moneyFromDouble(0.10, cents));
    CHECK(cents == 10);
    CHECK(moneyFromDouble(99999999.99, cents));
    CHECK(cents == kMaxRecordAmountCents);
    CHECK(!moneyFromDouble(std::numeric_limits<double>::infinity(), cents));
    CHECK(!moneyFromDouble(100000000.00, cents));
    CHECK(formatMoney(0) == "0.00");
    CHECK(formatMoney(123) == "1.23");
    CHECK(formatMoney(-123) == "-1.23");

    Money result = 0;
    CHECK(checkedAddMoney(10, 20, result));
    CHECK(result == 30);
    CHECK(!checkedAddMoney(std::numeric_limits<Money>::max(), 1, result));
    CHECK(!isValidIsoDate("0099-12-31"));
    CHECK(isValidIsoDate("0100-01-01"));
    CHECK(isValidIsoDate("9999-12-31"));
}

void testLegacyFilesLoadAsOneValidatedState() {
    TempDirectory temp;
    writeText(temp.path() / "categories.dat",
              "EXPENSE|项目(A)\n"
              "EXPENSE|饮食(午饭)\n");
    writeText(temp.path() / "records.dat",
              "1|2026-01-02|EXPENSE|12.30|项目(A)|字面分类\n"
              "2|2026-01-03|EXPENSE|5.00|饮食(午饭)|字面饮食分类\n"
              "3|2026-01-04|EXPENSE|2.50|交通(地铁)|旧版子分类\n");

    StorageManager storage(temp.path());
    Ledger ledger(storage);

    CHECK(ledger.load());
    CHECK(ledger.loadedLegacyFormat());
    CHECK(ledger.getAllRecords().size() == 3);
    CHECK(ledger.getAllRecords()[0].category == "项目(A)");
    CHECK(ledger.getAllRecords()[0].subcategory.empty());
    CHECK(ledger.getAllRecords()[1].category == "饮食(午饭)");
    CHECK(ledger.getAllRecords()[1].subcategory.empty());
    CHECK(ledger.getAllRecords()[2].category == "交通");
    CHECK(ledger.getAllRecords()[2].subcategory == "地铁");
    CHECK(ledger.getTotalExpense() == 1980);

    CHECK(ledger.save());
    CHECK(fs::exists(temp.path() / "records.dat"));
    CHECK(fs::exists(temp.path() / "categories.dat"));
    CHECK(readText(storage.dataFilePath()).find("#DAILYACCOUNT_V3\n") == 0);

    StorageManager reloadedStorage(temp.path());
    Ledger reloaded(reloadedStorage);
    CHECK(reloaded.load());
    CHECK(!reloaded.loadedLegacyFormat());
    CHECK(reloaded.getAllRecords()[2].displayCategory() == "交通(地铁)");
}

void testLegacyLongCategoryIsPreserved() {
    TempDirectory temp;
    const std::string longCategory(300, 'x');
    CategoryManager freshCategories;
    CHECK(!freshCategories.addCustomCategory(RecordType::EXPENSE, longCategory));
    CHECK(freshCategories.addStoredCategory(RecordType::EXPENSE, longCategory));
    writeText(temp.path() / "categories.dat",
              "EXPENSE|" + longCategory + "\n");
    writeText(temp.path() / "records.dat",
              "1|2026-01-02|EXPENSE|12.30|" + longCategory + "|legacy\n");

    StorageManager storage(temp.path());
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.categories().categoryExists(RecordType::EXPENSE, longCategory));
    CHECK(ledger.getAllRecords().front().category == longCategory);
    CHECK(ledger.save());

    StorageManager reloadedStorage(temp.path());
    Ledger reloaded(reloadedStorage);
    CHECK(reloaded.load());
    CHECK(reloaded.getAllRecords().front().category == longCategory);
}

void testIncompleteLegacyPairIsRejected() {
    TempDirectory recordsOnly;
    writeText(recordsOnly.path() / "records.dat",
              "1|2026-01-02|EXPENSE|12.30|饮食|legacy\n");
    StorageManager recordsStorage(recordsOnly.path());
    StoredData data;
    CHECK(!recordsStorage.load(data));
    CHECK(recordsStorage.lastError().find("pair") != std::string::npos);

    TempDirectory categoriesOnly;
    writeText(categoriesOnly.path() / "categories.dat", "EXPENSE|项目\n");
    StorageManager categoriesStorage(categoriesOnly.path());
    CHECK(!categoriesStorage.load(data));
    CHECK(categoriesStorage.lastError().find("pair") != std::string::npos);
}

void testLegacyRecordCategoryIsRecoveredWhenCatalogEntryIsMissing() {
    TempDirectory temp;
    writeText(temp.path() / "categories.dat", "");
    writeText(temp.path() / "records.dat",
              "1|2026-01-02|EXPENSE|12.30|已恢复分类|legacy\n");

    StorageManager storage(temp.path());
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.categories().categoryExists(
        RecordType::EXPENSE, "已恢复分类"));
    CHECK(ledger.getAllRecords().front().category == "已恢复分类");
}

void testVersionedRoundTripEscapesEveryTextField() {
    TempDirectory temp;
    StorageManager storage(temp.path());
    Ledger ledger(storage);
    CHECK(ledger.load());

    const std::string category = "项目|特殊%类";
    CHECK(ledger.addCustomCategory(RecordType::EXPENSE, category));

    Record record = expense("2026-09-03", 12345, category, "A|B%\n下一行");
    CHECK(ledger.addRecord(record));
    CHECK(fs::exists(storage.backupFilePath()));

    StorageManager reloadedStorage(temp.path());
    Ledger reloaded(reloadedStorage);
    CHECK(reloaded.load());
    const auto records = reloaded.getAllRecords();
    CHECK(records.size() == 1);
    const Record& actual = records.front();
    CHECK(actual.amountCents == 12345);
    CHECK(actual.category == category);
    CHECK(actual.note == "A|B%\n下一行");
    CHECK(reloaded.categories().categoryExists(RecordType::EXPENSE, category));
}

void testBackupCanRestoreLastCompleteSnapshot() {
    TempDirectory temp;
    StorageManager storage(temp.path());
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.addCustomCategory(RecordType::EXPENSE, "备份分类"));
    CHECK(ledger.addRecord(expense("2026-09-03", 100, "备份分类")));

    writeText(storage.dataFilePath(), "corrupt\n");
    StoredData ignored;
    CHECK(!storage.load(ignored));
    CHECK(storage.restoreBackup());
    CHECK(readText(temp.path() / "ledger.dat.corrupt") == "corrupt\n");

    Ledger restored(storage);
    CHECK(restored.load());
    CHECK(restored.getAllRecords().empty());
    CHECK(restored.categories().categoryExists(RecordType::EXPENSE, "备份分类"));
}

void testFailedMutationsDoNotChangeLiveState() {
    FakeStorage storage;
    Ledger ledger(storage);
    CHECK(ledger.load());

    storage.failSave = true;
    CHECK(!ledger.addRecord(expense("2026-09-03", 100, "饮食")));
    CHECK(ledger.getAllRecords().empty());
    CHECK(!ledger.lastError().empty());

    storage.failSave = false;
    CHECK(ledger.addRecord(expense("2026-09-03", 100, "饮食")));
    CHECK(ledger.getAllRecords().front().id == 1);
    const auto retainedRecord = ledger.findRecord(1);
    CHECK(retainedRecord.has_value());
    CHECK(ledger.addCustomCategory(RecordType::EXPENSE, "宠物"));
    CHECK(retainedRecord->amountCents == 100);

    storage.failSave = true;
    Record changed = expense("2026-09-04", 999, "饮食");
    CHECK(!ledger.updateRecord(1, changed));
    CHECK(ledger.findRecord(1)->amountCents == 100);
    CHECK(!ledger.deleteRecord(1));
    CHECK(ledger.findRecord(1).has_value());
    CHECK(!ledger.addCustomCategory(RecordType::EXPENSE, "失败分类"));
    CHECK(!ledger.categories().categoryExists(RecordType::EXPENSE, "失败分类"));
    CHECK(!ledger.removeCustomCategory(RecordType::EXPENSE, "宠物"));
    CHECK(ledger.categories().categoryExists(RecordType::EXPENSE, "宠物"));
    CHECK(!ledger.clearAllData());
    CHECK(ledger.getAllRecords().size() == 1);
    CHECK(ledger.categories().categoryExists(RecordType::EXPENSE, "宠物"));
}

void testClearIsOneSnapshotAndResetsNextId() {
    FakeStorage storage;
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.addCustomCategory(RecordType::EXPENSE, "宠物"));
    CHECK(ledger.addRecord(expense("2026-09-03", 100, "宠物")));

    const int savesBeforeClear = storage.saveCalls;
    CHECK(ledger.clearAllData());
    CHECK(storage.saveCalls == savesBeforeClear + 1);
    CHECK(ledger.getAllRecords().empty());
    CHECK(!ledger.categories().categoryExists(RecordType::EXPENSE, "宠物"));
    CHECK(ledger.getTotalIncome() == 0);
    CHECK(ledger.getTotalExpense() == 0);

    CHECK(ledger.addRecord(expense("2026-09-04", 200, "交通")));
    CHECK(ledger.getAllRecords().front().id == 1);
}

void testFailedReloadKeepsPreviouslyLoadedState() {
    FakeStorage storage;
    storage.persisted.records.push_back(expense("2026-09-03", 100, "饮食"));
    storage.persisted.records.front().id = 1;
    storage.persisted.nextId = 2;
    storage.persisted.format = StorageFormat::V3;

    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.getAllRecords().size() == 1);

    storage.persisted.records.push_back(storage.persisted.records.front());
    CHECK(!ledger.load());
    CHECK(ledger.getAllRecords().size() == 1);
    CHECK(ledger.getTotalExpense() == 100);
}

void testInvalidVersionedDataFailsWithLineInformation() {
    TempDirectory temp;
    writeText(temp.path() / "ledger.dat",
              "#DAILYACCOUNT_V3\n"
              "NEXT_ID|2\n"
              "RECORD|1|2026-09-03|EXPENSE|100|%ZZ||note\n");

    StorageManager storage(temp.path());
    StoredData data;
    CHECK(!storage.load(data));
    CHECK(storage.lastError().find("line 3") != std::string::npos);

    writeText(temp.path() / "ledger.dat", "#DAILYACCOUNT_V99\n");
    CHECK(!storage.load(data));
    CHECK(storage.lastError().find("unsupported") != std::string::npos);
}

void testVersionedDataRequiresCompleteEndMarker() {
    TempDirectory temp;
    writeText(temp.path() / "ledger.dat",
              "#DAILYACCOUNT_V3\n"
              "NEXT_ID|2\n"
              "RECORD|1|2026-09-03|EXPENSE|100|%E9%A5%AE%E9%A3%9F||note\n");

    StorageManager storage(temp.path());
    StoredData data;
    CHECK(!storage.load(data));
    CHECK(storage.lastError().find("END") != std::string::npos);

    TempDirectory generated;
    StorageManager generatedStorage(generated.path());
    Ledger generatedLedger(generatedStorage);
    CHECK(generatedLedger.load());
    CHECK(generatedLedger.addRecord(expense("2026-09-03", 100, "饮食")));
    const std::string complete = readText(generatedStorage.dataFilePath());
    const std::size_t endStart = complete.rfind("END|");
    CHECK(endStart != std::string::npos);
    for (std::size_t newline = complete.find('\n');
         newline != std::string::npos && newline < endStart;
         newline = complete.find('\n', newline + 1)) {
        writeText(generatedStorage.dataFilePath(), complete.substr(0, newline + 1));
        StoredData truncated;
        CHECK(!generatedStorage.load(truncated));
    }
}

void testChecksumRejectsSyntacticallyValidCorruption() {
    TempDirectory temp;
    StorageManager storage(temp.path());
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.addRecord(expense("2026-09-03", 100, "饮食")));

    std::string corrupted = readText(storage.dataFilePath());
    const std::string original = "|EXPENSE|100|";
    const std::size_t amount = corrupted.find(original);
    CHECK(amount != std::string::npos);
    corrupted.replace(amount, original.size(), "|EXPENSE|200|");
    writeText(storage.dataFilePath(), corrupted);

    StoredData ignored;
    CHECK(!storage.load(ignored));
    CHECK(storage.lastError().find("checksum") != std::string::npos);
}

void testCorruptCurrentDoesNotReplaceGoodBackup() {
    TempDirectory temp;
    StorageManager storage(temp.path());
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.addCustomCategory(RecordType::EXPENSE, "备份分类"));
    CHECK(ledger.addRecord(expense("2026-09-03", 100, "备份分类")));

    const std::string goodBackup = readText(storage.backupFilePath());
    writeText(storage.dataFilePath(), "corrupt\n");

    CHECK(!ledger.addRecord(expense("2026-09-04", 200, "备份分类")));
    CHECK(readText(storage.backupFilePath()) == goodBackup);
}

void testInterruptedBackupRotationIsRecovered() {
    TempDirectory temp;
    StorageManager storage(temp.path());
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.addCustomCategory(RecordType::EXPENSE, "备份分类"));
    CHECK(ledger.addRecord(expense("2026-09-03", 100, "备份分类")));

    const std::string current = readText(storage.dataFilePath());
    const std::string previousBackup = readText(storage.backupFilePath());
    writeText(temp.path() / "ledger.dat.bak.previous", previousBackup);
    writeText(storage.backupFilePath(), current);
    writeText(temp.path() / "ledger.dat.tmp", current);

    StorageManager recovered(temp.path());
    CHECK(recovered.isReady());
    CHECK(readText(recovered.dataFilePath()) == current);
    CHECK(readText(recovered.backupFilePath()) == previousBackup);
    CHECK(!fs::exists(temp.path() / "ledger.dat.tmp"));
    CHECK(!fs::exists(temp.path() / "ledger.dat.bak.previous"));

    writeText(temp.path() / "ledger.dat.bak.previous", previousBackup);
    writeText(recovered.backupFilePath(), current);
    StorageManager completed(temp.path());
    CHECK(completed.isReady());
    CHECK(readText(completed.backupFilePath()) == current);
    CHECK(!fs::exists(temp.path() / "ledger.dat.bak.previous"));
}

void testObsoleteRotationArtifactDoesNotBlockReads() {
    TempDirectory temp;
    StorageManager storage(temp.path());
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.addCustomCategory(RecordType::EXPENSE, "遗留文件测试"));
    CHECK(ledger.addRecord(expense("2026-09-03", 100, "遗留文件测试")));
    CHECK(storage.hasBackup());
    const std::string current = readText(storage.dataFilePath());

    const fs::path staleArtifact = temp.path() / "ledger.dat.bak.previous";
    CHECK(fs::create_directory(staleArtifact));
    writeText(staleArtifact / "unexpected", "not a rotation file");

    StorageManager reopened(temp.path());
    CHECK(reopened.isReady());
    StoredData data;
    CHECK(reopened.load(data));
    CHECK(data.records.size() == 1);
    CHECK(!reopened.save(data));
    CHECK(readText(reopened.dataFilePath()) == current);
}

void testLegacyStateCanBeSavedToAnotherStorageAtomically() {
    TempDirectory source;
    writeText(source.path() / "categories.dat", "EXPENSE|项目(A)\n");
    writeText(source.path() / "records.dat",
              "7|2026-09-03|EXPENSE|12.30|项目(A)|迁移记录\n");
    const std::string sourceCategories = readText(source.path() / "categories.dat");
    const std::string sourceRecords = readText(source.path() / "records.dat");

    StorageManager sourceStorage(source.path());
    Ledger sourceLedger(sourceStorage);
    CHECK(sourceLedger.load());

    TempDirectory target;
    StorageManager targetStorage(target.path());
    CHECK(sourceLedger.saveTo(targetStorage));
    CHECK(readText(source.path() / "categories.dat") == sourceCategories);
    CHECK(readText(source.path() / "records.dat") == sourceRecords);
    CHECK(!fs::exists(target.path() / "categories.dat"));
    CHECK(!fs::exists(target.path() / "records.dat"));

    Ledger migrated(targetStorage);
    CHECK(migrated.load());
    CHECK(migrated.getAllRecords().size() == 1);
    CHECK(migrated.getAllRecords().front().id == 7);
    CHECK(migrated.getTotalExpense() == 1230);
}

void testRecordQueriesReturnValueSnapshots() {
    static_assert(std::is_same_v<
        decltype(std::declval<const Ledger&>().findRecord(1)),
        std::optional<Record>>);
    static_assert(std::is_same_v<
        decltype(std::declval<const Ledger&>().getAllRecords()),
        std::vector<Record>>);

    FakeStorage storage;
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.addRecord(expense("2026-09-03", 100, "饮食")));
    const auto record = ledger.findRecord(1);
    const auto records = ledger.getAllRecords();
    CHECK(ledger.addCustomCategory(RecordType::EXPENSE, "宠物"));
    CHECK(record && record->amountCents == 100);
    CHECK(records.size() == 1 && records.front().amountCents == 100);
}

void testImpossibleNextIdIsRejected() {
    FakeStorage storage;
    storage.persisted.format = StorageFormat::V3;
    storage.persisted.nextId = static_cast<std::int64_t>(INT_MAX) + 2;

    Ledger ledger(storage);
    CHECK(!ledger.load());
}

void testPresetCatalogRowsRemainCompatible() {
    TempDirectory temp;
    StorageManager storage(temp.path());
    Ledger written(storage);
    CHECK(written.load());
    CHECK(written.addRecord(expense("2026-09-03", 100, "饮食")));
    CHECK(readText(storage.dataFilePath()).find(
              "CATEGORY|EXPENSE|%E9%A5%AE%E9%A3%9F\n") != std::string::npos);

    StorageManager reloadedStorage(temp.path());
    Ledger reloaded(reloadedStorage);
    CHECK(reloaded.load());
    CHECK(reloaded.getAllRecords().size() == 1);
}

void testInvalidRecordTypesAreRejectedByCategories() {
    CategoryManager categories;
    const auto invalid = static_cast<RecordType>(99);
    CHECK(categories.getCategories(invalid).empty());
    CHECK(!categories.addCustomCategory(invalid, "非法类型"));
    CHECK(!categories.removeCustomCategory(invalid, "非法类型"));
    CHECK(!categories.isPreset(invalid, "工资"));
    CHECK(!categories.categoryExists(invalid, "工资"));
    CHECK(categories.getInUseCategories(invalid).empty());
}

void testSummariesUseExactCentsAndDateRanges() {
    FakeStorage storage;
    Ledger ledger(storage);
    CHECK(ledger.load());
    CHECK(ledger.addRecord(expense("2026-08-31", 10, "饮食")));
    CHECK(ledger.addRecord(expense("2026-09-01", 20, "饮食")));
    CHECK(ledger.addRecord(expense("2026-09-02", 30, "交通")));

    CHECK(ledger.getTotalExpense() == 60);
    auto september = ledger.getCategorySummaries(
        RecordType::EXPENSE, "2026-09-01", "2026-09-30");
    CHECK(september.size() == 2);
    CHECK(september[0].totalAmount == 30);
    CHECK(september[1].totalAmount == 20);
    CHECK(ledger.getRecordsByDateRange("2026-09-30", "2026-09-01").empty());
}

} // namespace

int main() {
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"money helpers are exact and checked", testMoneyHelpersAreExactAndChecked},
        {"legacy files load as one validated state", testLegacyFilesLoadAsOneValidatedState},
        {"legacy long categories are preserved", testLegacyLongCategoryIsPreserved},
        {"incomplete legacy pairs are rejected", testIncompleteLegacyPairIsRejected},
        {"missing legacy catalog entries recover", testLegacyRecordCategoryIsRecoveredWhenCatalogEntryIsMissing},
        {"versioned round trip escapes text", testVersionedRoundTripEscapesEveryTextField},
        {"backup restores a complete snapshot", testBackupCanRestoreLastCompleteSnapshot},
        {"failed mutations keep live state", testFailedMutationsDoNotChangeLiveState},
        {"clear uses one snapshot and resets id", testClearIsOneSnapshotAndResetsNextId},
        {"failed reload keeps live state", testFailedReloadKeepsPreviouslyLoadedState},
        {"invalid V3 data reports its line", testInvalidVersionedDataFailsWithLineInformation},
        {"V3 data requires an end marker", testVersionedDataRequiresCompleteEndMarker},
        {"V3 checksum rejects valid-looking corruption", testChecksumRejectsSyntacticallyValidCorruption},
        {"corrupt current preserves backup", testCorruptCurrentDoesNotReplaceGoodBackup},
        {"interrupted backup rotation recovers", testInterruptedBackupRotationIsRecovered},
        {"obsolete rotation artifacts do not block reads", testObsoleteRotationArtifactDoesNotBlockReads},
        {"legacy state migrates in one snapshot", testLegacyStateCanBeSavedToAnotherStorageAtomically},
        {"record queries return snapshots", testRecordQueriesReturnValueSnapshots},
        {"impossible next ID is rejected", testImpossibleNextIdIsRejected},
        {"preset catalog rows stay compatible", testPresetCatalogRowsRemainCompatible},
        {"invalid record types are rejected", testInvalidRecordTypesAreRejectedByCategories},
        {"summaries use exact cents and ranges", testSummariesUseExactCentsAndDateRanges},
    };

    int failures = 0;
    for (const auto& [name, test] : tests) {
        try {
            test();
            std::cout << "PASS: " << name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "FAIL: " << name << " - " << error.what() << '\n';
        }
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    std::cout << tests.size() << " test(s) passed\n";
    return 0;
}
