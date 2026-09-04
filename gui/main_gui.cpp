// DailyAccount GUI entry point and startup data recovery flow.
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QLockFile>
#include <QMessageBox>
#include <QStandardPaths>
#include <QStringList>

#include <algorithm>
#include <filesystem>
#include <string>
#include <system_error>

#include "mainwindow.h"

#include "ledger.h"
#include "storage.h"

namespace {

bool directoryHasLedgerSnapshot(const QString& directory)
{
    return QFileInfo::exists(QDir(directory).filePath("ledger.dat"));
}

bool directoryHasLegacyData(const QString& directory)
{
    const QDir dir(directory);
    return QFileInfo::exists(dir.filePath("records.dat")) ||
           QFileInfo::exists(dir.filePath("categories.dat"));
}

bool directoryHasPrimaryData(const QString& directory)
{
    return directoryHasLedgerSnapshot(directory) ||
           directoryHasLegacyData(directory);
}

bool directoryHasAnyData(const QString& directory)
{
    return directoryHasPrimaryData(directory) ||
           QFileInfo::exists(QDir(directory).filePath("ledger.dat.bak"));
}

std::filesystem::path nativePath(const QString& path)
{
#ifdef Q_OS_WIN
    return std::filesystem::path(path.toStdWString());
#else
    return std::filesystem::u8path(path.toUtf8().constData());
#endif
}

bool samePhysicalDirectory(const QString& left, const QString& right)
{
    std::error_code error;
    const bool equivalent = std::filesystem::equivalent(
        nativePath(left), nativePath(right), error);
    if (!error) return equivalent;
    return QDir::cleanPath(QDir(left).absolutePath()) ==
           QDir::cleanPath(QDir(right).absolutePath());
}

bool migrateDiscoveredData(StorageManager& targetStorage,
                           const QString& targetDirectory,
                           QString& migratedFrom, QString& error)
{
    if (directoryHasAnyData(targetDirectory)) return true;

    const QStringList rawCandidates = {
        QDir(QDir::currentPath()).filePath("data"),
        QDir(QCoreApplication::applicationDirPath()).filePath("data")
    };

    QStringList candidates;
    for (const QString& rawCandidate : rawCandidates) {
        const QString candidate = QDir::cleanPath(QDir(rawCandidate).absolutePath());
        const bool duplicate = std::any_of(
            candidates.cbegin(), candidates.cend(),
            [&candidate](const QString& existing) {
                return samePhysicalDirectory(candidate, existing);
            });
        if (samePhysicalDirectory(candidate, targetDirectory) || duplicate ||
            !directoryHasAnyData(candidate)) {
            continue;
        }
        candidates.append(candidate);
    }

    if (candidates.size() > 1) {
        error = QStringLiteral("检测到多个旧数据目录，已停止自动迁移以避免合并错误：\n") +
                candidates.join("\n");
        return false;
    }
    if (candidates.isEmpty()) return true;

    const QString sourceDirectory = candidates.front();
    StorageManager sourceStorage(nativePath(sourceDirectory));
    if (!sourceStorage.isReady()) {
        error = QString::fromStdString(sourceStorage.lastError());
        return false;
    }

    StoredData backupData;
    const bool hasBackup = sourceStorage.hasBackup();
    const bool backupValid = hasBackup && sourceStorage.loadBackup(backupData);
    const QString backupError = hasBackup && !backupValid
        ? QString::fromStdString(sourceStorage.lastError())
        : QString();

    Ledger sourceLedger(sourceStorage);
    if (directoryHasLedgerSnapshot(sourceDirectory)) {
        if (sourceLedger.load()) {
            if (!sourceLedger.saveTo(targetStorage)) {
                error = QString::fromStdString(sourceLedger.lastError());
                return false;
            }
        } else {
            const QString currentError = QString::fromStdString(sourceLedger.lastError());
            if (!backupValid) {
                error = currentError;
                if (!backupError.isEmpty()) {
                    error += QStringLiteral("\n备份也无效：") + backupError;
                }
                return false;
            }
            const auto choice = QMessageBox::question(
                nullptr, "旧数据快照损坏",
                currentError + "\n\n是否改用该目录中的完整备份进行迁移？\n" +
                    sourceDirectory,
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (choice != QMessageBox::Yes) {
                error = "用户取消了从旧目录备份迁移，原数据未更改。";
                return false;
            }
            if (!targetStorage.save(backupData)) {
                error = QString::fromStdString(targetStorage.lastError());
                return false;
            }
        }
    } else {
        const bool hasLegacyData = directoryHasLegacyData(sourceDirectory);
        const bool legacyValid = hasLegacyData && sourceLedger.load();
        const QString legacyError = hasLegacyData && !legacyValid
            ? QString::fromStdString(sourceLedger.lastError())
            : QString();

        bool useBackup = false;
        if (backupValid && legacyValid) {
            const auto choice = QMessageBox::question(
                nullptr, "选择迁移数据",
                QStringLiteral(
                    "旧目录同时包含完整安全备份和旧版数据。\n"
                    "选择“是”使用安全备份，选择“否”使用旧版 records.dat / "
                    "categories.dat，选择“取消”停止启动。\n\n") +
                    sourceDirectory,
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::Yes);
            if (choice == QMessageBox::Cancel) {
                error = "用户取消了旧数据迁移，原数据未更改。";
                return false;
            }
            useBackup = choice == QMessageBox::Yes;
        } else if (backupValid) {
            const auto choice = QMessageBox::question(
                nullptr, "发现旧数据备份",
                QStringLiteral("旧目录中仅检测到可用的安全备份。是否从该备份迁移？\n\n") +
                    sourceDirectory,
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (choice != QMessageBox::Yes) {
                error = "用户取消了从旧目录备份迁移，原数据未更改。";
                return false;
            }
            useBackup = true;
        } else if (!legacyValid) {
            error = legacyError.isEmpty() ? backupError : legacyError;
            if (!legacyError.isEmpty() && !backupError.isEmpty()) {
                error += QStringLiteral("\n备份也无效：") + backupError;
            }
            return false;
        }

        if (useBackup) {
            if (!targetStorage.save(backupData)) {
                error = QString::fromStdString(targetStorage.lastError());
                return false;
            }
        } else if (!sourceLedger.saveTo(targetStorage)) {
            error = QString::fromStdString(sourceLedger.lastError());
            return false;
        }
    }

    migratedFrom = sourceDirectory;
    return true;
}

QString lockFailureMessage(QLockFile::LockError error)
{
    switch (error) {
    case QLockFile::LockFailedError:
        return "另一个日常记账实例正在运行。";
    case QLockFile::PermissionError:
        return "没有权限创建数据锁文件。";
    case QLockFile::UnknownError:
    case QLockFile::NoError:
        return "无法锁定数据目录。";
    }
    return "无法锁定数据目录。";
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("DailyAccount");
    QCoreApplication::setOrganizationDomain("dailyaccount.local");
    QCoreApplication::setApplicationName("DailyAccount");

    QFont font("Microsoft YaHei", 10);
    app.setFont(font);

    // Keep the visual language centralized so dynamically created widgets match.
    app.setStyleSheet(R"(
        QWidget {
            background-color: #F5F7FA;
            color: #2C3E50;
            font-family: "Microsoft YaHei", "SimHei", sans-serif;
        }

        QPushButton {
            background-color: #3498DB;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2980B9;
        }
        QPushButton:pressed {
            background-color: #2471A3;
        }
        QPushButton:disabled {
            background-color: #BDC3C7;
            color: #ECF0F1;
        }

        QPushButton[class="nav"] {
            background-color: transparent;
            color: #2C3E50;
            border: none;
            border-radius: 8px;
            padding: 14px 16px;
            text-align: left;
            font-size: 14px;
            font-weight: normal;
        }
        QPushButton[class="nav"]:hover {
            background-color: #E8EDF2;
        }
        QPushButton[class="nav"][active="true"] {
            background-color: #3498DB;
            color: white;
            font-weight: bold;
        }

        QFrame[class="card"] {
            background-color: #FFFFFF;
            border: 1px solid #E8ECF1;
            border-radius: 10px;
            padding: 16px;
        }

        QLineEdit, QDateEdit, QComboBox, QSpinBox, QDoubleSpinBox {
            background-color: #FFFFFF;
            border: 1px solid #D5DCE6;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 13px;
            color: #2C3E50;
        }
        QLineEdit:focus, QDateEdit:focus, QComboBox:focus {
            border-color: #3498DB;
            outline: none;
        }

        /* 日期选择器右侧日历按钮 */
        QDateEdit::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: center right;
            width: 28px;
            border-left: 1px solid #D5DCE6;
            border-top-right-radius: 6px;
            border-bottom-right-radius: 6px;
            background: #EBF5FB;
        }
        QDateEdit::drop-down:hover {
            background: #3498DB;
        }
        QDateEdit::down-arrow {
            image: none;
            width: 0; height: 0;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #3498DB;
        }
        QDateEdit::drop-down:hover .down-arrow {
            border-top-color: white;
        }

        /* 日期选择器上下增减箭头按钮 */
        QDateEdit::up-button, QDateEdit::down-button {
            background: #EBF5FB;
            border: 1px solid #D5DCE6;
            border-radius: 2px;
            width: 18px;
        }
        QDateEdit::up-button:hover, QDateEdit::down-button:hover {
            background: #3498DB;
            border-color: #3498DB;
        }
        QDateEdit::up-arrow {
            image: none;
            width: 0; height: 0;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-bottom: 5px solid #3498DB;
        }
        QDateEdit::down-arrow {
            image: none;
            width: 0; height: 0;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid #3498DB;
        }
        QDateEdit::up-button:hover .up-arrow {
            border-bottom-color: white;
        }
        QDateEdit::down-button:hover .down-arrow {
            border-top-color: white;
        }

        QTableWidget, QTreeWidget {
            background-color: #FFFFFF;
            border: 1px solid #E8ECF1;
            border-radius: 8px;
            gridline-color: #F0F3F7;
            selection-background-color: #EBF5FB;
            selection-color: #2C3E50;
        }
        QTableWidget::item, QTreeWidget::item {
            padding: 8px;
        }
        QHeaderView::section {
            background-color: #F5F7FA;
            color: #7F8C8D;
            border: none;
            border-bottom: 2px solid #E8ECF1;
            padding: 10px 8px;
            font-weight: bold;
            font-size: 12px;
        }

        QListWidget {
            background-color: #FFFFFF;
            border: 1px solid #E8ECF1;
            border-radius: 8px;
        }
        QListWidget::item {
            padding: 10px 14px;
            border-bottom: 1px solid #F0F3F7;
        }
        QListWidget::item:hover {
            background-color: #F5F7FA;
        }

        QScrollBar:vertical {
            background: #F5F7FA;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #CBD5E0;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #A0AEC0;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }

        QTabWidget::pane {
            border: none;
            background: transparent;
        }
        QTabBar::tab {
            background: transparent;
            color: #7F8C8D;
            border: none;
            padding: 10px 20px;
            font-size: 13px;
        }
        QTabBar::tab:selected {
            color: #3498DB;
            border-bottom: 3px solid #3498DB;
            font-weight: bold;
        }

        QGroupBox {
            font-weight: bold;
            border: 1px solid #E8ECF1;
            border-radius: 8px;
            margin-top: 16px;
            padding-top: 16px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 8px;
            color: #2C3E50;
        }

        QStatusBar {
            background-color: #FFFFFF;
            border-top: 1px solid #E8ECF1;
            color: #2C3E50;
            font-size: 13px;
        }

        QToolTip {
            background-color: #2C3E50;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 10px;
        }
    )");

    const QString dataDirectory =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDirectory.isEmpty() || !QDir().mkpath(dataDirectory) ||
        !QFileInfo(dataDirectory).isDir()) {
        QMessageBox::critical(nullptr, "启动失败",
                              "无法创建应用数据目录。请检查当前用户权限。");
        return 1;
    }

    QLockFile dataLock(QDir(dataDirectory).filePath("dailyaccount.lock"));
    dataLock.setStaleLockTime(0);
    if (!dataLock.tryLock(100)) {
        QMessageBox::critical(nullptr, "启动失败",
                              lockFailureMessage(dataLock.error()));
        return 1;
    }

    StorageManager storage(nativePath(dataDirectory));
    if (!storage.isReady()) {
        QMessageBox::critical(nullptr, "启动失败",
            QString::fromStdString(storage.lastError()));
        return 1;
    }

    QString migratedFrom;
    QString migrationError;
    if (!migrateDiscoveredData(storage, dataDirectory,
                               migratedFrom, migrationError)) {
        QMessageBox::critical(nullptr, "数据迁移失败", migrationError);
        return 1;
    }

    if (!directoryHasLedgerSnapshot(dataDirectory) && storage.hasBackup()) {
        const bool hasLegacyData = directoryHasLegacyData(dataDirectory);
        StoredData backupData;
        if (!storage.loadBackup(backupData)) {
            if (!hasLegacyData) {
                QMessageBox::critical(nullptr, "数据加载失败",
                    QStringLiteral("数据目录中仅有备份，但备份无效：\n") +
                    QString::fromStdString(storage.lastError()));
                return 1;
            }
            QMessageBox::warning(nullptr, "备份无效",
                QStringLiteral("新版备份文件已损坏，将尝试读取保留的旧版数据：\n") +
                QString::fromStdString(storage.lastError()));
        } else {
            const auto choice = QMessageBox::question(
                nullptr, "发现数据备份",
                QString("当前数据快照缺失，但检测到完整备份。是否恢复该备份？%1")
                    .arg(hasLegacyData
                        ? "\n选择“否”将改为读取较旧的旧版数据。"
                        : ""),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            if (choice == QMessageBox::Yes && !storage.restoreBackup()) {
                QMessageBox::critical(nullptr, "备份恢复失败",
                    QString::fromStdString(storage.lastError()));
                return 1;
            }
            if (choice != QMessageBox::Yes && !hasLegacyData) return 1;
        }
    }

    Ledger ledger(storage);
    if (!ledger.load()) {
        const QString loadError = QString::fromStdString(ledger.lastError());
        if (!storage.hasBackup()) {
            QMessageBox::critical(nullptr, "数据加载失败", loadError);
            return 1;
        }

        StoredData backupData;
        if (!storage.loadBackup(backupData)) {
            QMessageBox::critical(nullptr, "数据加载失败",
                loadError + "\n\n检测到备份文件，但备份也已损坏：\n" +
                QString::fromStdString(storage.lastError()));
            return 1;
        }

        const auto choice = QMessageBox::question(
            nullptr, "数据加载失败",
            loadError + "\n\n检测到上一版完整备份，是否恢复备份？\n"
                        "当前损坏文件会保留为 ledger.dat.corrupt。",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (choice != QMessageBox::Yes) return 1;
        if (!storage.restoreBackup() || !ledger.load()) {
            const std::string error = storage.lastError().empty()
                ? ledger.lastError()
                : storage.lastError();
            QMessageBox::critical(nullptr, "备份恢复失败",
                                  QString::fromStdString(error));
            return 1;
        }
    }

    if (ledger.loadedLegacyFormat() && !ledger.save()) {
        QMessageBox::critical(nullptr, "数据升级失败",
            QStringLiteral("旧数据已成功读取，但无法写入新版安全快照：\n") +
            QString::fromStdString(ledger.lastError()));
        return 1;
    }
    if (!migratedFrom.isEmpty()) {
        QMessageBox::information(nullptr, "数据迁移完成",
            QStringLiteral("旧数据已验证并转换为安全快照。原文件仍保留在：\n") +
            migratedFrom);
    }

    MainWindow window(ledger);
    window.show();

    return app.exec();
}
