#ifndef KAMAKURA_H
#define KAMAKURA_H

#include <QMainWindow>
#include <QGridLayout>
#include <QWidget>
#include <QTabWidget>
//#include <QSyntaxHighlighter>
#include <QString>
#include <QMenu>
#include <QMessageBox>
#include <QErrorMessage>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QFileInfo>
//#include <QPrinter>
#include <QDockWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeView>
#include <QDirModel>
#include <QModelIndex>
#include <QToolBar>
#include <QTabBar>
#include <QAction>
#include <QToolButton>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QList>
#include <QUrl>
#include <QFont>
#include <QTextFormat>
#include <QTextCursor>
#include <QIODevice>
#include <QCloseEvent>                  // closeEvent
#include <QLabel>                       // GUI labels
#include <QActionGroup>
#include <QStandardPaths>               // see default directory
#include <QTabWidget>
#include "linenumberarea.h"
#include "codeeditor.h"
#include "highlighter.h"
#include "finddialog.h"
#include "metrics.h"
#include "documentmetrics.h"

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

QT_BEGIN_NAMESPACE
namespace Ui { class kamakura; }
QT_END_NAMESPACE

class kamakura : public QMainWindow
{
    Q_OBJECT

public:
   explicit kamakura(QWidget *parent = nullptr);
    ~kamakura();

private:
    Ui::kamakura *ui;
    QTabWidget* tabs;
    Highlighter* highlighter;
    CodeEditor *editor = nullptr;
    QDirModel* file_system_model;
    QTreeView* file_system_view;
    // QListWidget* opened_docs_widget;
    QDockWidget* opened_docs_dock;
    FindDialog *findDialog;
    QListWidget* opened_docs_widget;
    MetricReporter *metricReporter;
    const QString WINDOW_STATUS_BAR = "window_status_bar";

    void SetupTabWidget();
     void SetupOpenedDocsDock();

    void reconnectsignals();
     void disconnectsignals();

private slots:
    void UpdateParameter();



    void on_actionQuit_triggered();

    void on_actionKamakura_triggered();


    void OpenFile(const QString&);

    void on_actionOpen_triggered();

    void on_actionSave_triggered();


    void on_actionZoom_triggered();

    void on_actionZoom_2_triggered();

    void on_actionSave_2_triggered();

    void on_actionNew_triggered();

    void on_actionCut_triggered();

    void on_actionPaste_triggered();

    void on_actionCopy_triggered();

    void dragEnterEvent(QDragEnterEvent* drag_event);

    void dropEvent(QDropEvent* drop_event);

    void OpenFile(QModelIndex);

    void on_actionSearch_and_Replace_triggered();


    void CloseFile();
    void CloseFile(int);

  void ChangeTabIndexInList(int, int);
    void UpdateCurrentIndex(QListWidgetItem*);
    void UpdateCurrentIndexOnDelete(int);
    void DeleteTabFromList(int);
    void UpdateCurrentIndex(int);














    void on_actionHowTo_triggered();
};
#endif // KAMAKURA_H
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
