#include "storage.h"

#include "category.h"

#include <algorithm>
#include <charconv>
#include <climits>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <limits>
#include <set>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace {

constexpr const char* kHeader = "#DAILYACCOUNT_V3";
constexpr std::int64_t kMaxNextId = static_cast<std::int64_t>(INT_MAX) + 1;
constexpr std::uint64_t kChecksumOffset = 14695981039346656037ULL;
constexpr std::uint64_t kChecksumPrime = 1099511628211ULL;

std::string pathText(const fs::path& path) {
    return path.u8string();
}

void stripCarriageReturn(std::string& line) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
}

std::string trimAsciiWhitespace(const std::string& value) {
    std::size_t begin = 0;
    while (begin < value.size() &&
           std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }
    std::size_t end = value.size();
    while (end > begin &&
           std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(begin, end - begin);
}

template <typename Integer>
bool parseInteger(const std::string& text, Integer& value) {
    if (text.empty()) return false;
    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto parsed = std::from_chars(begin, end, value);
    return parsed.ec == std::errc() && parsed.ptr == end;
}

void addChecksumLine(std::uint64_t& checksum, const std::string& line) {
    for (const unsigned char character : line) {
        checksum ^= character;
        checksum *= kChecksumPrime;
    }
    checksum ^= static_cast<unsigned char>('\n');
    checksum *= kChecksumPrime;
}

bool parseChecksum(const std::string& text, std::uint64_t& checksum) {
    if (text.size() != 16) return false;
    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto parsed = std::from_chars(begin, end, checksum, 16);
    return parsed.ec == std::errc() && parsed.ptr == end;
}

std::string formatChecksum(std::uint64_t checksum) {
    std::ostringstream output;
    output << std::hex << std::uppercase << std::setw(16) << std::setfill('0')
           << checksum;
    return output.str();
}

bool parseLegacyMoney(const std::string& source, Money& cents) {
    const std::string text = trimAsciiWhitespace(source);
    if (text.empty()) return false;

    std::size_t position = text[0] == '+' ? 1 : 0;
    if (position == text.size() || text[position] == '-') return false;

    Money whole = 0;
    bool hasWholeDigit = false;
    while (position < text.size() && text[position] >= '0' && text[position] <= '9') {
        hasWholeDigit = true;
        const int digit = text[position] - '0';
        if (whole > (kMaxRecordAmountCents / 100 - digit) / 10) return false;
        whole = whole * 10 + digit;
        ++position;
    }

    Money fraction = 0;
    int fractionDigits = 0;
    if (position < text.size() && text[position] == '.') {
        ++position;
        while (position < text.size() && text[position] >= '0' && text[position] <= '9') {
            if (fractionDigits == 2) return false;
            fraction = fraction * 10 + (text[position] - '0');
            ++fractionDigits;
            ++position;
        }
    }

    if ((!hasWholeDigit && fractionDigits == 0) || position != text.size()) return false;
    if (fractionDigits == 1) fraction *= 10;

    const Money result = whole * 100 + fraction;
    if (result <= 0 || result > kMaxRecordAmountCents) return false;
    cents = result;
    return true;
}

std::vector<std::string> splitFields(const std::string& line) {
    std::vector<std::string> fields;
    std::size_t start = 0;
    while (true) {
        const std::size_t separator = line.find('|', start);
        if (separator == std::string::npos) {
            fields.push_back(line.substr(start));
            return fields;
        }
        fields.push_back(line.substr(start, separator - start));
        start = separator + 1;
    }
}

int hexValue(char character) {
    if (character >= '0' && character <= '9') return character - '0';
    if (character >= 'A' && character <= 'F') return character - 'A' + 10;
    if (character >= 'a' && character <= 'f') return character - 'a' + 10;
    return -1;
}

std::string encodeField(const std::string& value) {
    static constexpr char hex[] = "0123456789ABCDEF";
    std::string encoded;
    encoded.reserve(value.size());
    for (const unsigned char character : value) {
        const bool safe = (character >= 'a' && character <= 'z') ||
                          (character >= 'A' && character <= 'Z') ||
                          (character >= '0' && character <= '9') ||
                          character == '-' || character == '_' ||
                          character == '.' || character == '~' || character == ' ';
        if (safe) {
            encoded.push_back(static_cast<char>(character));
        } else {
            encoded.push_back('%');
            encoded.push_back(hex[(character >> 4) & 0x0F]);
            encoded.push_back(hex[character & 0x0F]);
        }
    }
    return encoded;
}

bool decodeField(const std::string& encoded, std::string& value) {
    std::string decoded;
    decoded.reserve(encoded.size());
    for (std::size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] != '%') {
            decoded.push_back(encoded[i]);
            continue;
        }
        if (i + 2 >= encoded.size()) return false;
        const int high = hexValue(encoded[i + 1]);
        const int low = hexValue(encoded[i + 2]);
        if (high < 0 || low < 0) return false;
        decoded.push_back(static_cast<char>((high << 4) | low));
        i += 2;
    }
    value = std::move(decoded);
    return true;
}

bool validRecordType(RecordType type) {
    return type == RecordType::INCOME || type == RecordType::EXPENSE;
}

bool validateStoredData(const StoredData& data, bool requireKnownCategories,
                        std::string& error) {
    if (data.nextId < 1 || data.nextId > kMaxNextId) {
        error = "next ID is outside the supported range";
        return false;
    }

    CategoryManager categories;
    std::set<std::pair<RecordType, std::string>> catalogRows;
    for (const auto& [type, name] : data.customCategories) {
        if (!validRecordType(type) || !catalogRows.emplace(type, name).second ||
            (!categories.isPreset(type, name) &&
             !categories.addStoredCategory(type, name))) {
            error = "invalid or duplicate custom category: " + name;
            return false;
        }
    }

    std::set<int> ids;
    int maxId = 0;
    Money income = 0;
    Money expense = 0;
    for (const auto& record : data.records) {
        if (record.id <= 0 || !ids.insert(record.id).second) {
            error = "record IDs must be unique positive integers";
            return false;
        }
        if (!isValidIsoDate(record.date)) {
            error = "invalid record date for ID " + std::to_string(record.id);
            return false;
        }
        if (!validRecordType(record.type)) {
            error = "invalid record type for ID " + std::to_string(record.id);
            return false;
        }
        if (record.amountCents <= 0 || record.amountCents > kMaxRecordAmountCents) {
            error = "invalid record amount for ID " + std::to_string(record.id);
            return false;
        }
        if (record.category.empty()) {
            error = "empty record category for ID " + std::to_string(record.id);
            return false;
        }
        if (requireKnownCategories &&
            !categories.categoryExists(record.type, record.category)) {
            error = "unknown category for record ID " + std::to_string(record.id) +
                    ": " + record.category;
            return false;
        }

        Money updated = 0;
        Money& total = record.type == RecordType::INCOME ? income : expense;
        if (!checkedAddMoney(total, record.amountCents, updated)) {
            error = "money total overflow";
            return false;
        }
        total = updated;
        maxId = std::max(maxId, record.id);
    }

    if (data.nextId <= maxId) {
        error = "next ID must be greater than every existing record ID";
        return false;
    }
    return true;
}

std::string lineError(const fs::path& path, std::size_t line,
                      const std::string& message) {
    return pathText(path) + " line " + std::to_string(line) + ": " + message;
}

bool parseV3File(const fs::path& path, StoredData& data, std::string& error) {
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        error = "cannot open data file: " + pathText(path);
        return false;
    }

    std::string line;
    if (!std::getline(input, line)) {
        error = "data file is empty: " + pathText(path);
        return false;
    }
    stripCarriageReturn(line);
    if (line != kHeader) {
        error = "unsupported data format in " + pathText(path) + ": " + line;
        return false;
    }

    StoredData candidate;
    candidate.format = StorageFormat::V3;
    bool hasNextId = false;
    bool hasEnd = false;
    std::size_t expectedRecords = 0;
    std::size_t expectedCategories = 0;
    std::uint64_t checksum = kChecksumOffset;
    std::uint64_t expectedChecksum = 0;
    addChecksumLine(checksum, line);
    std::set<int> ids;
    std::size_t lineNumber = 1;

    while (std::getline(input, line)) {
        ++lineNumber;
        stripCarriageReturn(line);
        const auto fields = splitFields(line);

        if (hasEnd) {
            error = lineError(path, lineNumber, "data found after END row");
            return false;
        }


        if (fields[0] == "END") {
            if (fields.size() != 4 ||
                !parseInteger(fields[1], expectedRecords) ||
                !parseInteger(fields[2], expectedCategories) ||
                !parseChecksum(fields[3], expectedChecksum)) {
                error = lineError(path, lineNumber, "invalid END row");
                return false;
            }
            hasEnd = true;
            continue;
        }

        addChecksumLine(checksum, line);

        if (fields[0] == "NEXT_ID") {
            if (fields.size() != 2 || hasNextId ||
                !parseInteger(fields[1], candidate.nextId)) {
                error = lineError(path, lineNumber, "invalid NEXT_ID row");
                return false;
            }
            hasNextId = true;
            continue;
        }

        if (fields[0] == "CATEGORY") {
            if (fields.size() != 3) {
                error = lineError(path, lineNumber, "invalid CATEGORY row");
                return false;
            }
            const auto type = parseRecordType(fields[1]);
            std::string name;
            if (!type || !decodeField(fields[2], name)) {
                error = lineError(path, lineNumber, "invalid CATEGORY value");
                return false;
            }
            candidate.customCategories.emplace_back(*type, std::move(name));
            continue;
        }

        if (fields[0] == "RECORD") {
            if (fields.size() != 8) {
                error = lineError(path, lineNumber, "invalid RECORD row");
                return false;
            }

            long long parsedId = 0;
            Money amount = 0;
            const auto type = parseRecordType(fields[3]);
            Record record;
            if (!parseInteger(fields[1], parsedId) || parsedId <= 0 ||
                parsedId > INT_MAX || !type ||
                !parseInteger(fields[4], amount) || amount <= 0 ||
                amount > kMaxRecordAmountCents ||
                !decodeField(fields[5], record.category) ||
                !decodeField(fields[6], record.subcategory) ||
                !decodeField(fields[7], record.note)) {
                error = lineError(path, lineNumber, "invalid RECORD value");
                return false;
            }
            record.id = static_cast<int>(parsedId);
            record.date = fields[2];
            record.type = *type;
            record.amountCents = amount;
            if (!ids.insert(record.id).second) {
                error = lineError(path, lineNumber, "duplicate record ID");
                return false;
            }
            candidate.records.push_back(std::move(record));
            continue;
        }

        error = lineError(path, lineNumber, "unknown row type");
        return false;
    }

    if (input.bad()) {
        error = "failed while reading data file: " + pathText(path);
        return false;
    }
    if (!hasNextId) {
        error = "missing NEXT_ID row in " + pathText(path);
        return false;
    }
    if (!hasEnd) {
        error = "missing END row in " + pathText(path);
        return false;
    }
    if (candidate.records.size() != expectedRecords ||
        candidate.customCategories.size() != expectedCategories) {
        error = "row count does not match END row in " + pathText(path);
        return false;
    }
    if (checksum != expectedChecksum) {
        error = "checksum mismatch in " + pathText(path);
        return false;
    }

    std::string validationError;
    if (!validateStoredData(candidate, true, validationError)) {
        error = "invalid data file " + pathText(path) + ": " + validationError;
        return false;
    }

    data = std::move(candidate);
    return true;
}

bool parseLegacyCategories(const fs::path& path, StoredData& data,
                           std::string& error) {
    std::error_code existsError;
    if (!fs::exists(path, existsError)) {
        if (existsError) {
            error = "cannot inspect legacy categories file: " + existsError.message();
            return false;
        }
        return true;
    }

    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        error = "cannot open legacy categories file: " + pathText(path);
        return false;
    }

    CategoryManager categories;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        stripCarriageReturn(line);
        if (line.empty()) continue;
        const std::size_t separator = line.find('|');
        if (separator == std::string::npos) {
            error = lineError(path, lineNumber, "missing category separator");
            return false;
        }
        const auto type = parseRecordType(line.substr(0, separator));
        const std::string name = line.substr(separator + 1);
        if (!type || !categories.addStoredCategory(*type, name)) {
            error = lineError(path, lineNumber, "invalid or duplicate category");
            return false;
        }
        data.customCategories.emplace_back(*type, name);
    }
    if (input.bad()) {
        error = "failed while reading legacy categories file: " + pathText(path);
        return false;
    }
    return true;
}

bool parseLegacyRecord(const std::string& line, const CategoryManager& categories,
                       Record& record, std::string& reason) {
    std::size_t start = 0;
    std::string fixedFields[4];
    for (std::string& field : fixedFields) {
        const std::size_t separator = line.find('|', start);
        if (separator == std::string::npos) {
            reason = "record has fewer than five fields";
            return false;
        }
        field = line.substr(start, separator - start);
        start = separator + 1;
    }
    const std::string categoryAndNote = line.substr(start);

    long long parsedId = 0;
    Money amount = 0;
    const auto type = parseRecordType(fixedFields[2]);
    if (!parseInteger(fixedFields[0], parsedId) || parsedId <= 0 ||
        parsedId > INT_MAX || !type || !parseLegacyMoney(fixedFields[3], amount)) {
        reason = "invalid ID, type, or amount";
        return false;
    }

    std::string matchedCategory;
    std::string matchedNote;
    for (const auto& known : categories.getCategories(*type)) {
        if (categoryAndNote == known && known.size() > matchedCategory.size()) {
            matchedCategory = known;
            matchedNote.clear();
        } else if (categoryAndNote.size() > known.size() &&
                   categoryAndNote.compare(0, known.size(), known) == 0 &&
                   categoryAndNote[known.size()] == '|' &&
                   known.size() > matchedCategory.size()) {
            matchedCategory = known;
            matchedNote = categoryAndNote.substr(known.size() + 1);
        }
    }

    if (matchedCategory.empty()) {
        const std::size_t separator = categoryAndNote.find('|');
        if (separator == std::string::npos) {
            matchedCategory = categoryAndNote;
        } else {
            matchedCategory = categoryAndNote.substr(0, separator);
            matchedNote = categoryAndNote.substr(separator + 1);
        }
    }
    if (matchedCategory.empty()) {
        reason = "record category is empty";
        return false;
    }

    record.id = static_cast<int>(parsedId);
    record.date = fixedFields[1];
    record.type = *type;
    record.amountCents = amount;
    record.category = std::move(matchedCategory);
    record.note = std::move(matchedNote);
    return true;
}

bool parseLegacyRecords(const fs::path& path, StoredData& data,
                        std::string& error) {
    std::error_code existsError;
    if (!fs::exists(path, existsError)) {
        if (existsError) {
            error = "cannot inspect legacy records file: " + existsError.message();
            return false;
        }
        return true;
    }

    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        error = "cannot open legacy records file: " + pathText(path);
        return false;
    }

    CategoryManager categories;
    for (const auto& [type, name] : data.customCategories) {
        if (!categories.addStoredCategory(type, name)) {
            error = "legacy categories are inconsistent";
            return false;
        }
    }

    std::set<int> ids;
    int maxId = 0;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        stripCarriageReturn(line);
        if (line.empty()) continue;

        Record record;
        std::string reason;
        if (!parseLegacyRecord(line, categories, record, reason)) {
            error = lineError(path, lineNumber, reason);
            return false;
        }
        if (!ids.insert(record.id).second) {
            error = lineError(path, lineNumber, "duplicate record ID");
            return false;
        }
        maxId = std::max(maxId, record.id);
        data.records.push_back(std::move(record));
    }
    if (input.bad()) {
        error = "failed while reading legacy records file: " + pathText(path);
        return false;
    }

    data.nextId = static_cast<std::int64_t>(maxId) + 1;
    return true;
}

bool syncFile(const fs::path& path, std::string& error) {
#ifdef _WIN32
    const HANDLE handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        error = "cannot open temporary file for flush (Windows error " +
                std::to_string(GetLastError()) + ")";
        return false;
    }
    const bool success = FlushFileBuffers(handle) != 0;
    const DWORD flushError = success ? ERROR_SUCCESS : GetLastError();
    CloseHandle(handle);
    if (!success) {
        error = "cannot flush temporary file (Windows error " +
                std::to_string(flushError) + ")";
    }
    return success;
#else
    const int descriptor = ::open(path.c_str(), O_RDWR);
    if (descriptor < 0) {
        error = "cannot open temporary file for flush";
        return false;
    }
    const bool success = ::fsync(descriptor) == 0;
    ::close(descriptor);
    if (!success) error = "cannot flush temporary file";
    return success;
#endif
}

bool replacePath(const fs::path& replacement, const fs::path& target,
                 std::string& error) {
#ifdef _WIN32
    if (!MoveFileExW(replacement.c_str(), target.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        error = "cannot replace " + pathText(target) + " (Windows error " +
                std::to_string(GetLastError()) + ")";
        return false;
    }
    return true;
#else
    std::error_code renameError;
    fs::rename(replacement, target, renameError);
    if (renameError) {
        error = "cannot replace " + pathText(target) + ": " + renameError.message();
        return false;
    }
    return true;
#endif
}

bool removeIfPresent(const fs::path& path, std::string& error) {
    std::error_code removeError;
    fs::remove(path, removeError);
    if (removeError) {
        error = "cannot remove stale temporary file " + pathText(path) + ": " +
                removeError.message();
        return false;
    }
    return true;
}

bool copyAndSync(const fs::path& source, const fs::path& target,
                 std::string& error) {
    std::error_code copyError;
    fs::copy_file(source, target, fs::copy_options::overwrite_existing, copyError);
    if (copyError) {
        error = "cannot copy " + pathText(source) + " to " + pathText(target) +
                ": " + copyError.message();
        return false;
    }
    return syncFile(target, error);
}

} // namespace

StorageManager::StorageManager(fs::path dataDirectory)
    : m_dataDirectory(std::move(dataDirectory))
    , m_dataFile(m_dataDirectory / "ledger.dat")
    , m_backupFile(m_dataDirectory / "ledger.dat.bak")
    , m_legacyRecordsFile(m_dataDirectory / "records.dat")
    , m_legacyCategoriesFile(m_dataDirectory / "categories.dat") {
    m_ready = ensureDataDirectory();
    if (m_ready) m_ready = recoverInterruptedSave();
}

bool StorageManager::ensureDataDirectory() {
    std::error_code error;
    if (fs::exists(m_dataDirectory, error)) {
        if (error) {
            m_lastError = "cannot inspect data directory: " + error.message();
            return false;
        }
        if (!fs::is_directory(m_dataDirectory, error) || error) {
            m_lastError = "data path is not a directory: " + pathText(m_dataDirectory);
            return false;
        }
        return true;
    }

    if (!fs::create_directories(m_dataDirectory, error) || error) {
        m_lastError = "cannot create data directory " + pathText(m_dataDirectory) +
                      ": " + error.message();
        return false;
    }
    return true;
}

bool StorageManager::recoverInterruptedSave() {
    fs::path dataTemporary = m_dataFile;
    dataTemporary += ".tmp";
    fs::path backupTemporary = m_backupFile;
    backupTemporary += ".tmp";
    fs::path previousBackup = m_backupFile;
    previousBackup += ".previous";

    std::error_code dataTempError;
    const bool hasDataTemporary = fs::exists(dataTemporary, dataTempError);
    if (dataTempError) {
        m_lastError = "cannot inspect interrupted data file";
        return false;
    }

    // Once the data temporary file is gone, the commit is complete. Obsolete
    // cleanup artifacts must not make an otherwise valid snapshot unreadable;
    // a later save will still fail safely if they cannot be removed.
    if (!hasDataTemporary) {
        std::string ignoredError;
        removeIfPresent(backupTemporary, ignoredError);
        ignoredError.clear();
        removeIfPresent(previousBackup, ignoredError);
        return true;
    }

    std::error_code previousError;
    const bool hasPreviousBackup = fs::exists(previousBackup, previousError);
    if (previousError) {
        m_lastError = "cannot inspect interrupted backup file";
        return false;
    }

    // A remaining data temporary file means the new snapshot was not committed.
    // Restore the old backup if rotation had already started.
    if (hasPreviousBackup && hasDataTemporary &&
        !replacePath(previousBackup, m_backupFile, m_lastError)) {
        return false;
    }

    std::string cleanupError;
    if (!removeIfPresent(dataTemporary, cleanupError) ||
        !removeIfPresent(backupTemporary, cleanupError)) {
        m_lastError = cleanupError;
        return false;
    }

    return true;
}

bool StorageManager::load(StoredData& data) {
    m_lastError.clear();
    if (!m_ready) return false;

    std::error_code existsError;
    const bool hasCurrentData = fs::exists(m_dataFile, existsError);
    if (existsError) {
        m_lastError = "cannot inspect data file: " + existsError.message();
        return false;
    }

    StoredData candidate;
    if (hasCurrentData) {
        if (!parseV3File(m_dataFile, candidate, m_lastError)) return false;
    } else {
        std::error_code recordsError;
        std::error_code categoriesError;
        const bool hasLegacyRecords = fs::exists(m_legacyRecordsFile, recordsError);
        const bool hasLegacyCategories = fs::exists(m_legacyCategoriesFile, categoriesError);
        if (recordsError || categoriesError) {
            m_lastError = "cannot inspect legacy data files";
            return false;
        }
        if (!hasLegacyRecords && !hasLegacyCategories) {
            data = StoredData{};
            return true;
        }
        if (hasLegacyRecords != hasLegacyCategories) {
            m_lastError = "legacy records.dat and categories.dat must exist as a pair";
            return false;
        }

        candidate.format = StorageFormat::Legacy;
        if (!parseLegacyCategories(m_legacyCategoriesFile, candidate, m_lastError) ||
            !parseLegacyRecords(m_legacyRecordsFile, candidate, m_lastError)) {
            return false;
        }
        std::string validationError;
        if (!validateStoredData(candidate, false, validationError)) {
            m_lastError = "invalid legacy data: " + validationError;
            return false;
        }
    }

    data = std::move(candidate);
    return true;
}

bool StorageManager::save(const StoredData& data) {
    m_lastError.clear();
    if (!m_ready && !ensureDataDirectory()) return false;
    if (!recoverInterruptedSave()) return false;
    m_ready = true;

    std::string validationError;
    if (!validateStoredData(data, true, validationError)) {
        m_lastError = "refusing to save invalid state: " + validationError;
        return false;
    }

    fs::path temporary = m_dataFile;
    temporary += ".tmp";
    fs::path backupTemporary = m_backupFile;
    backupTemporary += ".tmp";
    fs::path previousBackup = m_backupFile;
    previousBackup += ".previous";
    if (!removeIfPresent(temporary, m_lastError) ||
        !removeIfPresent(backupTemporary, m_lastError) ||
        !removeIfPresent(previousBackup, m_lastError)) {
        return false;
    }

    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output.is_open()) {
        m_lastError = "cannot create temporary data file: " + pathText(temporary);
        return false;
    }

    std::uint64_t checksum = kChecksumOffset;
    const auto writeLine = [&output, &checksum](const std::string& line) {
        output << line << '\n';
        addChecksumLine(checksum, line);
    };

    writeLine(kHeader);
    writeLine("NEXT_ID|" + std::to_string(data.nextId));
    for (const auto& [type, name] : data.customCategories) {
        writeLine("CATEGORY|" + typeToString(type) + '|' + encodeField(name));
    }
    for (const auto& record : data.records) {
        writeLine("RECORD|" + std::to_string(record.id) + '|' + record.date + '|' +
                  typeToString(record.type) + '|' +
                  std::to_string(record.amountCents) + '|' +
                  encodeField(record.category) + '|' +
                  encodeField(record.subcategory) + '|' + encodeField(record.note));
    }
    output << "END|" << data.records.size() << '|'
           << data.customCategories.size() << '|' << formatChecksum(checksum) << '\n';
    output.flush();
    if (!output.good()) {
        output.close();
        removeIfPresent(temporary, validationError);
        m_lastError = "failed while writing temporary data file";
        return false;
    }
    output.close();
    if (output.fail()) {
        removeIfPresent(temporary, validationError);
        m_lastError = "failed while closing temporary data file";
        return false;
    }
    if (!syncFile(temporary, m_lastError)) {
        removeIfPresent(temporary, validationError);
        return false;
    }

    {
        StoredData writtenData;
        if (!parseV3File(temporary, writtenData, m_lastError)) {
            const std::string verificationError = m_lastError;
            removeIfPresent(temporary, validationError);
            m_lastError = "temporary data verification failed: " + verificationError;
            return false;
        }
    }

    std::error_code existsError;
    const bool targetExists = fs::exists(m_dataFile, existsError);
    if (existsError) {
        removeIfPresent(temporary, validationError);
        m_lastError = "cannot inspect current data file: " + existsError.message();
        return false;
    }
    if (targetExists) {
        StoredData currentData;
        if (!parseV3File(m_dataFile, currentData, m_lastError) ||
            !copyAndSync(m_dataFile, backupTemporary, m_lastError)) {
            removeIfPresent(temporary, validationError);
            removeIfPresent(backupTemporary, validationError);
            return false;
        }

        StoredData copiedData;
        if (!parseV3File(backupTemporary, copiedData, m_lastError)) {
            removeIfPresent(temporary, validationError);
            removeIfPresent(backupTemporary, validationError);
            return false;
        }

        std::error_code backupError;
        const bool backupExists = fs::exists(m_backupFile, backupError);
        if (backupError) {
            removeIfPresent(temporary, validationError);
            removeIfPresent(backupTemporary, validationError);
            m_lastError = "cannot inspect backup file: " + backupError.message();
            return false;
        }

        bool movedPreviousBackup = false;
        if (backupExists) {
            if (!replacePath(m_backupFile, previousBackup, m_lastError)) {
                removeIfPresent(temporary, validationError);
                removeIfPresent(backupTemporary, validationError);
                return false;
            }
            movedPreviousBackup = true;
        }

        if (!replacePath(backupTemporary, m_backupFile, m_lastError)) {
            const std::string rotationError = m_lastError;
            bool rolledBack = true;
            if (movedPreviousBackup) {
                rolledBack = replacePath(previousBackup, m_backupFile, m_lastError);
            }
            if (rolledBack) {
                removeIfPresent(temporary, validationError);
                removeIfPresent(backupTemporary, validationError);
                m_lastError = rotationError;
            } else {
                m_lastError = rotationError + "; backup rollback also failed: " +
                              m_lastError;
            }
            return false;
        }

        if (!replacePath(temporary, m_dataFile, m_lastError)) {
            const std::string commitError = m_lastError;
            bool rolledBack = true;
            if (movedPreviousBackup) {
                rolledBack = replacePath(previousBackup, m_backupFile, m_lastError);
            } else {
                rolledBack = removeIfPresent(m_backupFile, m_lastError);
            }
            if (rolledBack) {
                removeIfPresent(temporary, validationError);
                m_lastError = commitError;
            } else {
                m_lastError = commitError + "; backup rollback also failed: " +
                              m_lastError;
            }
            return false;
        }

        if (movedPreviousBackup) {
            removeIfPresent(previousBackup, validationError);
        }
        return true;
    }

    if (!replacePath(temporary, m_dataFile, m_lastError)) {
        removeIfPresent(temporary, validationError);
        return false;
    }
    return true;
}

bool StorageManager::hasBackup() const {
    std::error_code error;
    return fs::is_regular_file(m_backupFile, error) && !error;
}

bool StorageManager::loadBackup(StoredData& data) {
    m_lastError.clear();
    if (!m_ready) return false;
    if (!hasBackup()) {
        m_lastError = "no backup file is available";
        return false;
    }
    return parseV3File(m_backupFile, data, m_lastError);
}

bool StorageManager::restoreBackup() {
    m_lastError.clear();
    StoredData backupData;
    if (!loadBackup(backupData)) return false;

    fs::path restoreTemporary = m_dataFile;
    restoreTemporary += ".restore.tmp";
    fs::path corruptFile = m_dataFile;
    corruptFile += ".corrupt";
    fs::path corruptTemporary = corruptFile;
    corruptTemporary += ".tmp";
    std::string cleanupError;

    if (!removeIfPresent(restoreTemporary, m_lastError) ||
        !removeIfPresent(corruptTemporary, m_lastError)) {
        return false;
    }
    if (!copyAndSync(m_backupFile, restoreTemporary, m_lastError)) {
        removeIfPresent(restoreTemporary, cleanupError);
        return false;
    }

    {
        StoredData restoredData;
        if (!parseV3File(restoreTemporary, restoredData, m_lastError)) {
            const std::string verificationError = m_lastError;
            removeIfPresent(restoreTemporary, cleanupError);
            m_lastError = "backup copy verification failed: " + verificationError;
            return false;
        }
    }

    std::error_code existsError;
    if (fs::exists(m_dataFile, existsError)) {
        if (existsError || !copyAndSync(m_dataFile, corruptTemporary, m_lastError) ||
            !replacePath(corruptTemporary, corruptFile, m_lastError)) {
            removeIfPresent(restoreTemporary, cleanupError);
            removeIfPresent(corruptTemporary, cleanupError);
            if (existsError) m_lastError = "cannot inspect corrupt data file";
            return false;
        }
    }

    if (!replacePath(restoreTemporary, m_dataFile, m_lastError)) {
        removeIfPresent(restoreTemporary, cleanupError);
        return false;
    }
    return true;
}
