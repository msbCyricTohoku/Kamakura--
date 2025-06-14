#ifndef KAMAKURA_H
#define KAMAKURA_H

#include <QMainWindow>
#include <QList>

// Forward Declarations
QT_BEGIN_NAMESPACE
namespace Ui { class kamakura; }
QT_END_NAMESPACE
class QTabWidget;
class Highlighter;
class CodeEditor;
class QDockWidget;
class QListWidget;
class FindDialog;
class MetricReporter;
class QListWidgetItem;
class QModelIndex;
class QDragEnterEvent;
class QDropEvent;


//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

class kamakura : public QMainWindow
{
    Q_OBJECT

public:
   explicit kamakura(QWidget *parent = nullptr);
    ~kamakura();

protected:
    void dragEnterEvent(QDragEnterEvent* drag_event) override;
    void dropEvent(QDropEvent* drop_event) override;

private slots:
    // Slots named to match the .ui file for auto-connection
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_2_triggered();
    void on_actionQuit_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionZoom_triggered();
    void on_actionZoom_2_triggered();
    void on_actionSearch_and_Replace_triggered();
    void on_actionKamakura_triggered();
    void on_actionHowTo_triggered();

    void setLightTheme();
    void setDarkTheme();

    // Internal Logic Slots
    void openFileByPath(const QString& path);
    void closeTab(int index);
    void updateTabDirtyStatus();
    void onCurrentTabChanged(int index);
    void syncListSelectionWithTab(int index);
    void syncTabSelectionWithList(QListWidgetItem* item);
    void handleTabMoved(int from, int to);


private:
    Ui::kamakura *ui;
    QTabWidget* tabs;
    Highlighter* highlighter;
    FindDialog *findDialog;
    MetricReporter *metricReporter;

    //bool darkThemeEnabled{true};
    bool darkThemeEnabled{false};
    
    QDockWidget* opened_docs_dock;
    QListWidget* opened_docs_widget;

    void setupEditor(CodeEditor* editor);
    void setupDocks();
    void setupConnections();
    void updateWindowTitle(const QString& currentFile);
    CodeEditor* currentEditor();
};
#endif // KAMAKURA_H
