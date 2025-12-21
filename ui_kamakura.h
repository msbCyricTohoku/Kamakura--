/********************************************************************************
** Form generated from reading UI file 'kamakura.ui'
**
** Created by: Qt User Interface Compiler version 5.15.18
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_KAMAKURA_H
#define UI_KAMAKURA_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "codeeditor.h"

QT_BEGIN_NAMESPACE

class Ui_kamakura
{
public:
    QAction *actionOpen;
    QAction *actionNew;
    QAction *actionQuit;
    QAction *actionSave;
    QAction *actionKamakura;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionCut;
    QAction *actionZoom;
    QAction *actionZoom_2;
    QAction *actionSave_2;
    QAction *actionSearch_and_Replace;
    QAction *actionGoToLine;
    QAction *actionDuplicate_Line;
    QAction *actionToggle_Comment;
    QAction *actionInsert_DateTime;
    QAction *actionTrim_Trailing_Spaces;
    QAction *actionHowTo;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_3;
    QHBoxLayout *horizontalLayout;
    CodeEditor *plainTextEdit;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuAbout;
    QStatusBar *statusbar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *kamakura)
    {
        if (kamakura->objectName().isEmpty())
            kamakura->setObjectName(QString::fromUtf8("kamakura"));
        kamakura->resize(748, 677);
        QFont font;
        font.setPointSize(10);
        kamakura->setFont(font);
        actionOpen = new QAction(kamakura);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/new/prefix1/resources/open.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen->setIcon(icon);
        QFont font1;
        font1.setFamily(QString::fromUtf8("TeX Gyre Adventor"));
        font1.setPointSize(10);
        actionOpen->setFont(font1);
        actionNew = new QAction(kamakura);
        actionNew->setObjectName(QString::fromUtf8("actionNew"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/new/prefix1/resources/new.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionNew->setIcon(icon1);
        actionNew->setFont(font1);
        actionQuit = new QAction(kamakura);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/new/prefix1/resources/exit.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionQuit->setIcon(icon2);
        actionQuit->setFont(font1);
        actionSave = new QAction(kamakura);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/new/prefix1/resources/saveas.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave->setIcon(icon3);
        actionSave->setFont(font1);
        actionKamakura = new QAction(kamakura);
        actionKamakura->setObjectName(QString::fromUtf8("actionKamakura"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/new/prefix1/resources/about.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionKamakura->setIcon(icon4);
        actionKamakura->setFont(font1);
        actionCopy = new QAction(kamakura);
        actionCopy->setObjectName(QString::fromUtf8("actionCopy"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/new/prefix1/resources/copy.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCopy->setIcon(icon5);
        actionCopy->setFont(font1);
        actionPaste = new QAction(kamakura);
        actionPaste->setObjectName(QString::fromUtf8("actionPaste"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/new/prefix1/resources/paste.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionPaste->setIcon(icon6);
        actionPaste->setFont(font1);
        actionCut = new QAction(kamakura);
        actionCut->setObjectName(QString::fromUtf8("actionCut"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/new/prefix1/resources/cut.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionCut->setIcon(icon7);
        actionCut->setFont(font1);
        actionZoom = new QAction(kamakura);
        actionZoom->setObjectName(QString::fromUtf8("actionZoom"));
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/new/prefix1/resources/zoomin.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom->setIcon(icon8);
        actionZoom->setFont(font1);
        actionZoom_2 = new QAction(kamakura);
        actionZoom_2->setObjectName(QString::fromUtf8("actionZoom_2"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/new/prefix1/resources/zoomout.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_2->setIcon(icon9);
        actionZoom_2->setFont(font1);
        actionSave_2 = new QAction(kamakura);
        actionSave_2->setObjectName(QString::fromUtf8("actionSave_2"));
        QIcon icon10;
        icon10.addFile(QString::fromUtf8(":/new/prefix1/resources/save.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_2->setIcon(icon10);
        actionSave_2->setFont(font1);
        actionSearch_and_Replace = new QAction(kamakura);
        actionSearch_and_Replace->setObjectName(QString::fromUtf8("actionSearch_and_Replace"));
        QIcon icon11;
        icon11.addFile(QString::fromUtf8(":/new/prefix1/resources/findreplace.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSearch_and_Replace->setIcon(icon11);
        actionSearch_and_Replace->setFont(font1);
        actionGoToLine = new QAction(kamakura);
        actionGoToLine->setObjectName(QString::fromUtf8("actionGoToLine"));
        QIcon icon12;
        icon12.addFile(QString::fromUtf8(":/new/prefix1/resources/goto.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionGoToLine->setIcon(icon12);
        actionGoToLine->setFont(font1);
        actionDuplicate_Line = new QAction(kamakura);
        actionDuplicate_Line->setObjectName(QString::fromUtf8("actionDuplicate_Line"));
        QIcon icon13;
        icon13.addFile(QString::fromUtf8(":/new/prefix1/resources/dup.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDuplicate_Line->setIcon(icon13);
        actionDuplicate_Line->setFont(font1);
        actionToggle_Comment = new QAction(kamakura);
        actionToggle_Comment->setObjectName(QString::fromUtf8("actionToggle_Comment"));
        QIcon icon14;
        icon14.addFile(QString::fromUtf8(":/new/prefix1/resources/comment.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionToggle_Comment->setIcon(icon14);
        actionToggle_Comment->setFont(font1);
        actionInsert_DateTime = new QAction(kamakura);
        actionInsert_DateTime->setObjectName(QString::fromUtf8("actionInsert_DateTime"));
        QIcon icon15;
        icon15.addFile(QString::fromUtf8(":/new/prefix1/resources/date.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionInsert_DateTime->setIcon(icon15);
        actionTrim_Trailing_Spaces = new QAction(kamakura);
        actionTrim_Trailing_Spaces->setObjectName(QString::fromUtf8("actionTrim_Trailing_Spaces"));
        QIcon icon16;
        icon16.addFile(QString::fromUtf8(":/new/prefix1/resources/white.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionTrim_Trailing_Spaces->setIcon(icon16);
        actionHowTo = new QAction(kamakura);
        actionHowTo->setObjectName(QString::fromUtf8("actionHowTo"));
        QIcon icon17;
        icon17.addFile(QString::fromUtf8(":/new/prefix1/resources/example.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHowTo->setIcon(icon17);
        centralwidget = new QWidget(kamakura);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        centralwidget->setFont(font1);
        horizontalLayout_3 = new QHBoxLayout(centralwidget);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        plainTextEdit = new CodeEditor(centralwidget);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        plainTextEdit->setFont(font1);

        horizontalLayout->addWidget(plainTextEdit);


        horizontalLayout_3->addLayout(horizontalLayout);

        kamakura->setCentralWidget(centralwidget);
        menubar = new QMenuBar(kamakura);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 748, 22));
        menubar->setFont(font1);
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuFile->setFont(font1);
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuEdit->setFont(font1);
        menuAbout = new QMenu(menubar);
        menuAbout->setObjectName(QString::fromUtf8("menuAbout"));
        menuAbout->setFont(font1);
        kamakura->setMenuBar(menubar);
        statusbar = new QStatusBar(kamakura);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        statusbar->setFont(font);
        kamakura->setStatusBar(statusbar);
        toolBar = new QToolBar(kamakura);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        toolBar->setFont(font);
        kamakura->addToolBar(Qt::TopToolBarArea, toolBar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuAbout->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionQuit);
        menuEdit->addAction(actionSave_2);
        menuEdit->addAction(actionSave);
        menuEdit->addAction(actionCopy);
        menuEdit->addAction(actionPaste);
        menuEdit->addAction(actionCut);
        menuEdit->addAction(actionDuplicate_Line);
        menuEdit->addAction(actionToggle_Comment);
        menuEdit->addAction(actionZoom);
        menuEdit->addAction(actionZoom_2);
        menuEdit->addAction(actionSearch_and_Replace);
        menuEdit->addAction(actionGoToLine);
        menuEdit->addAction(actionInsert_DateTime);
        menuEdit->addAction(actionTrim_Trailing_Spaces);
        menuAbout->addAction(actionHowTo);
        menuAbout->addAction(actionKamakura);
        toolBar->addAction(actionNew);
        toolBar->addAction(actionOpen);
        toolBar->addAction(actionSave_2);
        toolBar->addAction(actionSave);
        toolBar->addSeparator();
        toolBar->addAction(actionCopy);
        toolBar->addAction(actionCut);
        toolBar->addAction(actionPaste);
        toolBar->addSeparator();
        toolBar->addAction(actionZoom);
        toolBar->addAction(actionZoom_2);
        toolBar->addSeparator();
        toolBar->addAction(actionSearch_and_Replace);
        toolBar->addAction(actionGoToLine);
        toolBar->addAction(actionInsert_DateTime);
        toolBar->addSeparator();
        toolBar->addAction(actionHowTo);
        toolBar->addAction(actionKamakura);
        toolBar->addSeparator();
        toolBar->addAction(actionQuit);

        retranslateUi(kamakura);

        QMetaObject::connectSlotsByName(kamakura);
    } // setupUi

    void retranslateUi(QMainWindow *kamakura)
    {
        kamakura->setWindowTitle(QCoreApplication::translate("kamakura", "Kamakura - -", nullptr));
        actionOpen->setText(QCoreApplication::translate("kamakura", "Open", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionNew->setText(QCoreApplication::translate("kamakura", "New", nullptr));
#if QT_CONFIG(shortcut)
        actionNew->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        actionQuit->setText(QCoreApplication::translate("kamakura", "Quit", nullptr));
#if QT_CONFIG(shortcut)
        actionQuit->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave->setText(QCoreApplication::translate("kamakura", "Save As", nullptr));
#if QT_CONFIG(shortcut)
        actionSave->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+Shift+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionKamakura->setText(QCoreApplication::translate("kamakura", "Kamakura", nullptr));
#if QT_CONFIG(shortcut)
        actionKamakura->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+K", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCopy->setText(QCoreApplication::translate("kamakura", "Copy", nullptr));
#if QT_CONFIG(shortcut)
        actionCopy->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionPaste->setText(QCoreApplication::translate("kamakura", "Paste", nullptr));
#if QT_CONFIG(shortcut)
        actionPaste->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCut->setText(QCoreApplication::translate("kamakura", "Cut", nullptr));
#if QT_CONFIG(shortcut)
        actionCut->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+X", nullptr));
#endif // QT_CONFIG(shortcut)
        actionZoom->setText(QCoreApplication::translate("kamakura", "Zoom+", nullptr));
#if QT_CONFIG(shortcut)
        actionZoom->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+=", nullptr));
#endif // QT_CONFIG(shortcut)
        actionZoom_2->setText(QCoreApplication::translate("kamakura", "Zoom-", nullptr));
#if QT_CONFIG(shortcut)
        actionZoom_2->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+-", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSave_2->setText(QCoreApplication::translate("kamakura", "Save", nullptr));
#if QT_CONFIG(shortcut)
        actionSave_2->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSearch_and_Replace->setText(QCoreApplication::translate("kamakura", "Search and Replace", nullptr));
#if QT_CONFIG(shortcut)
        actionSearch_and_Replace->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+F", nullptr));
#endif // QT_CONFIG(shortcut)
        actionGoToLine->setText(QCoreApplication::translate("kamakura", "Go to Line", nullptr));
#if QT_CONFIG(shortcut)
        actionGoToLine->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+G", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDuplicate_Line->setText(QCoreApplication::translate("kamakura", "Duplicate Line", nullptr));
#if QT_CONFIG(shortcut)
        actionDuplicate_Line->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+D", nullptr));
#endif // QT_CONFIG(shortcut)
        actionToggle_Comment->setText(QCoreApplication::translate("kamakura", "Toggle Comment", nullptr));
#if QT_CONFIG(shortcut)
        actionToggle_Comment->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+/", nullptr));
#endif // QT_CONFIG(shortcut)
        actionInsert_DateTime->setText(QCoreApplication::translate("kamakura", "Insert Date/Time", nullptr));
#if QT_CONFIG(shortcut)
        actionInsert_DateTime->setShortcut(QCoreApplication::translate("kamakura", "F5", nullptr));
#endif // QT_CONFIG(shortcut)
        actionTrim_Trailing_Spaces->setText(QCoreApplication::translate("kamakura", "Trim Trailing Spaces", nullptr));
#if QT_CONFIG(shortcut)
        actionTrim_Trailing_Spaces->setShortcut(QCoreApplication::translate("kamakura", "Ctrl+Shift+T", nullptr));
#endif // QT_CONFIG(shortcut)
        actionHowTo->setText(QCoreApplication::translate("kamakura", "HowTo", nullptr));
        menuFile->setTitle(QCoreApplication::translate("kamakura", "File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("kamakura", "Edit", nullptr));
        menuAbout->setTitle(QCoreApplication::translate("kamakura", "About", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("kamakura", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class kamakura: public Ui_kamakura {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_KAMAKURA_H
