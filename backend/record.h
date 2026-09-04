#ifndef RECORD_H
#define RECORD_H

#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <string>

using Money = std::int64_t;

// Matches the amount accepted by the Qt input control: 99,999,999.99.
constexpr Money kMaxRecordAmountCents = 9'999'999'999LL;
constexpr int kMinimumRecordYear = 100;
constexpr int kMaximumRecordYear = 9999;

enum class RecordType {
    INCOME,
    EXPENSE
};

struct Record {
    int id = 0;
    std::string date;
    RecordType type = RecordType::EXPENSE;
    Money amountCents = 0;
    std::string category;
    std::string subcategory;
    std::string note;

    Money signedAmountCents() const {
        return type == RecordType::INCOME ? amountCents : -amountCents;
    }

    std::string displayCategory() const {
        return subcategory.empty()
            ? category
            : category + "(" + subcategory + ")";
    }
};

inline std::optional<RecordType> parseRecordType(const std::string& value) {
    if (value == "INCOME") return RecordType::INCOME;
    if (value == "EXPENSE") return RecordType::EXPENSE;
    return std::nullopt;
}

inline std::string typeToString(RecordType type) {
    return type == RecordType::INCOME ? "INCOME" : "EXPENSE";
}

inline std::string typeToChinese(RecordType type) {
    return type == RecordType::INCOME ? "收入" : "支出";
}

inline bool moneyFromDouble(double value, Money& result) {
    if (!std::isfinite(value) || value < 0.0) return false;

    const double scaled = value * 100.0;
    if (!std::isfinite(scaled) ||
        scaled > static_cast<double>(kMaxRecordAmountCents) + 0.25) {
        return false;
    }

    const auto rounded = static_cast<Money>(std::llround(scaled));
    if (rounded < 0 || rounded > kMaxRecordAmountCents ||
        std::fabs(scaled - static_cast<double>(rounded)) > 0.0001) {
        return false;
    }

    result = rounded;
    return true;
}

inline double moneyToDouble(Money cents) {
    return static_cast<double>(cents) / 100.0;
}

inline bool checkedAddMoney(Money left, Money right, Money& result) {
    if ((right > 0 && left > std::numeric_limits<Money>::max() - right) ||
        (right < 0 && left < std::numeric_limits<Money>::min() - right)) {
        return false;
    }
    result = left + right;
    return true;
}

inline std::string formatMoney(Money cents) {
    const bool negative = cents < 0;
    const std::uint64_t magnitude = negative
        ? static_cast<std::uint64_t>(-(cents + 1)) + 1
        : static_cast<std::uint64_t>(cents);

    std::ostringstream output;
    if (negative) output << '-';
    output << magnitude / 100 << '.'
           << std::setw(2) << std::setfill('0') << magnitude % 100;
    return output.str();
}

inline bool isValidIsoDate(const std::string& date) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (date[i] < '0' || date[i] > '9') return false;
    }

    const int year = std::stoi(date.substr(0, 4));
    const int month = std::stoi(date.substr(5, 2));
    const int day = std::stoi(date.substr(8, 2));
    // QDateTimeEdit supports editable dates from the start of 100 CE.
    if (year < kMinimumRecordYear || year > kMaximumRecordYear ||
        month < 1 || month > 12 || day < 1) {
        return false;
    }

    static constexpr int daysPerMonth[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    int maxDay = daysPerMonth[month - 1];
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    if (month == 2 && leap) maxDay = 29;
    return day <= maxDay;
}

#endif // RECORD_H
