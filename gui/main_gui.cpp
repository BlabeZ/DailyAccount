/*
 * ===========================================================================
 * : main_gui.cpp
 * : gui
 * : Qt GUI main 
 *           : (1)  Qt ó
 *                 (2) QSS
 *                 (3) 飨洢
 *                 (4)  Qt 
 *
 *  GUI ""  main() 
 *  GUI 
 *
 *  console/main.cpp :
 *     ν棨棨console/main.cpp
 *   backend/ г
 *
 * : GB2312
 * ===========================================================================
 */

// ---- Qt  ----
#include <QApplication>   // Qt ó   GUI 
#include <QFont>          // Qt   ó

// ---- GUI  ----
#include "mainwindow.h"   //    +  + 

// ----  ----
#include "storage.h"      // 洢  
#include "category.h"     //   +壩
#include "ledger.h"       //   

// ============================================================================
// : main   C++ 
// ----------------------------------------------------------------------------
// :
//   argc  вargument count
//   argv  в飨argument vector
//
// :
//   int  0 0
//
// °У:
//   1.  QApplication    Qt 
//   2.   
//   3.   尴п
//   4.   洢    
//   5.   α
//   6.   
//   7.  Qt   
// ============================================================================
int main(int argc, char *argv[])
{
    // =========================================================================
    // 1:  QApplication 
    // QApplication  GUI "" У
    // 
    //  Qt GUI  QApplication 
    // =========================================================================
    QApplication app(argc, argv);

    // =========================================================================
    // 2: 
    // : "Microsoft YaHei" Windows 
    // С: 10pt  е
    // ÷Χ: н
    // =========================================================================
    QFont font("Microsoft YaHei", 10);
    app.setFont(font);

    // =========================================================================
    // 3: QSS = Qt Style Sheets
    // -------------------------------------------------------------------------
    //  QSS:
    //   QSS е CSS Qt 
    //   : " { : ; }"
    //   : "QPushButton { background-color: #3498DB; }" а
    //
    // :
    //   1. 壬Ч  
    //   2.   /
    //   3. α  hoverpresseddisabled
    //
    // : 
    //   : #F5F7FA 
    //   :   #FFFFFF 
    //   :   #3498DB 
    //   : #27AE60 /
    //   : #E74C3C /
    //   : #2C3E50 
    // =========================================================================
    app.setStyleSheet(R"(
        /* ====================================================================
         *    QWidget 
         *  #F5F7FA 
         *  #2C3E50 
         * ==================================================================== */
        QWidget {
            background-color: #F5F7FA;
            color: #2C3E50;
            font-family: "Microsoft YaHei", "SimHei", sans-serif;
        }

        /* ====================================================================
         *   е""""""
         *  +  6pxhover 
         * ==================================================================== */
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

        /* ====================================================================
         *    class="nav" 
         * : 
         *  [active="true"]:  + 
         * ==================================================================== */
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

        /* ====================================================================
         *   
         *  +  + 10px ""Ч
         * ==================================================================== */
        QFrame[class="card"] {
            background-color: #FFFFFF;
            border: 1px solid #E8ECF1;
            border-radius: 10px;
            padding: 16px;
        }

        /* ====================================================================
         *   
         *  + 
         * ==================================================================== */
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

        /* ====================================================================
         *   б
         * и
         * ==================================================================== */
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
        /*    +  +  */
        QHeaderView::section {
            background-color: #F5F7FA;
            color: #7F8C8D;
            border: none;
            border-bottom: 2px solid #E8ECF1;
            padding: 10px 8px;
            font-weight: bold;
            font-size: 12px;
        }

        /* ====================================================================
         * б  /б
         * ==================================================================== */
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

        /* ====================================================================
         *   
         * 8px + л
         * ==================================================================== */
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

        /* ====================================================================
         *   ò
         * ==================================================================== */
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

        /* ====================================================================
         *   
         * ==================================================================== */
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

        /* ====================================================================
         *   //
         * ==================================================================== */
        QStatusBar {
            background-color: #FFFFFF;
            border-top: 1px solid #E8ECF1;
            color: #2C3E50;
            font-size: 13px;
        }

        /* ====================================================================
         *   
         * ==================================================================== */
        QToolTip {
            background-color: #2C3E50;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 6px 10px;
        }
    )");

    // =========================================================================
    // 4: 
    // :  StorageManager  CategoryManager
    //  Ledger Ledger 
    // =========================================================================

    // 4a. 洢   "data"
    //     / data/ 
    StorageManager storage("data");

    // 4b.   
    //     ... н...
    CategoryManager catMan;

    // 4c. 
    //     Уcategories.dat 
    storage.loadCategories(catMan);

    // 4d.   洢
    //     : &ζ Ledger 
    //      storage  catMan 
    Ledger ledger(storage, catMan);

    // 4e.    records.dat
    //     ·
    ledger.load();

    // =========================================================================
    // 5: 
    // MainWindow :
    //   - ///
    //   - 4沢 QStackedWidget
    //   - 
    //   - 棨
    // =========================================================================
    MainWindow window(ledger, catMan);
    window.show();  // show() Qt 

    // =========================================================================
    // 6:  Qt 
    // app.exec()  Qt 
    // ""
    //  exec() main 
    // =========================================================================
    return app.exec();
}

// ============================================================================
// : main_gui.cpp
//  GUI νá
// ============================================================================
