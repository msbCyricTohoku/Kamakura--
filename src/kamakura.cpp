#include "kamakura.h"
#include "ui_kamakura.h"
#include "codeeditor.h"
#include "highlighter.h"
#include "finddialog.h"
#include "metrics.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidget>
#include <QDockWidget>
#include <QMenu>
#include <QActionGroup>
#include <QMimeData>
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QDebug>
#include <QStringListModel>
#include <QCoreApplication>

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

kamakura::kamakura(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::kamakura)
{
    // This function automatically connects slots like on_actionNew_triggered()
    ui->setupUi(this); 
    setAcceptDrops(true);

    // Load language files from the embedded Qt resources, using the correct prefix.
    highlighter = new Highlighter({":/new/prefix1/resources/phits_commands.xml",
                                   ":/new/prefix1/resources/python_lang.xml",
                                   ":/new/prefix1/resources/cpp_lang.xml",
                                   ":/new/prefix1/resources/rust_lang.xml",
                                   ":/new/prefix1/resources/haskell_lang.xml",
                                   ":/new/prefix1/resources/fortran_lang.xml",
                                   ":/new/prefix1/resources/web_lang.xml"}, this);

    tabs = new QTabWidget(this);
    tabs->setMovable(true);
    tabs->setTabsClosable(true);
    tabs->setFont(QFont("Sans Serif", 10));
    setCentralWidget(tabs);
    
    findDialog = new FindDialog(this);
    metricReporter = new MetricReporter(this);
    ui->statusbar->addPermanentWidget(metricReporter, 1);

    setupDocks();
    setupConnections();

    //setDarkTheme();

    setLightTheme();

    on_actionNew_triggered(); // Start with a new, empty tab
    updateWindowTitle("");
}

kamakura::~kamakura()
{
    delete ui;
}

void kamakura::setupDocks()
{
    opened_docs_dock  = new QDockWidget("Opened Files", this);
    opened_docs_widget = new QListWidget(opened_docs_dock);
    opened_docs_widget->setFont(QFont("Sans Serif", 10));
    opened_docs_dock->setWidget(opened_docs_widget);
    addDockWidget(Qt::RightDockWidgetArea, opened_docs_dock);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(opened_docs_dock->toggleViewAction());

    QActionGroup* themeGroup = new QActionGroup(this);
    QAction* lightTheme = viewMenu->addAction(tr("Light Theme"));
    QAction* darkTheme = viewMenu->addAction(tr("Dark Theme"));
    lightTheme->setCheckable(true);
    darkTheme->setCheckable(true);
    themeGroup->addAction(lightTheme);
    themeGroup->addAction(darkTheme);
    //darkTheme->setChecked(true);
    lightTheme->setChecked(true);

    connect(lightTheme, &QAction::triggered, this, &kamakura::setLightTheme);
    connect(darkTheme, &QAction::triggered, this, &kamakura::setDarkTheme);
}

void kamakura::setupConnections()
{
    connect(tabs, &QTabWidget::currentChanged, this, &kamakura::onCurrentTabChanged);
    connect(tabs, &QTabWidget::tabCloseRequested, this, &kamakura::closeTab);
    connect(tabs->tabBar(), &QTabBar::tabMoved, this, &kamakura::handleTabMoved);
    connect(opened_docs_widget, &QListWidget::itemClicked, this, &kamakura::syncTabSelectionWithList);
}

void kamakura::setupEditor(CodeEditor* editor)
{
    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(12);
    editor->setFont(font);

    if (darkThemeEnabled)
        editor->applyDarkTheme();
    else
        editor->applyLightTheme();

    connect(editor, &QPlainTextEdit::modificationChanged, this, &kamakura::updateTabDirtyStatus);
}

CodeEditor* kamakura::currentEditor()
{
    return qobject_cast<CodeEditor*>(tabs->currentWidget());
}

// --- Slot Implementations ---

void kamakura::on_actionNew_triggered()
{
    CodeEditor* editor = new CodeEditor;
    setupEditor(editor);
    
    int index = tabs->addTab(editor, "Untitled");
    tabs->setTabToolTip(index, ""); 
    tabs->setCurrentIndex(index);

    opened_docs_widget->addItem("Untitled");
    syncListSelectionWithTab(index);

    // Ensure highlighter is set even if the currentChanged signal isn't emitted
    onCurrentTabChanged(index);
}

void kamakura::on_actionOpen_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath());
    if (!filePath.isEmpty()) {
        openFileByPath(filePath);
    }
}

void kamakura::openFileByPath(const QString& path)
{
    for (int i = 0; i < tabs->count(); ++i) {
        if (tabs->tabToolTip(i) == path) {
            tabs->setCurrentIndex(i);
            return;
        }
    }

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not open file: " + file.errorString());
        return;
    }
    
    QString contents = file.readAll();
    file.close();
    
    CodeEditor* editor = currentEditor();
    if (tabs->count() == 1 && tabs->tabText(0) == "Untitled" && !editor->document()->isModified())
    {
       tabs->removeTab(0);
       opened_docs_widget->takeItem(0);
    }

    editor = new CodeEditor;
    setupEditor(editor);
    editor->setPlainText(contents);

    QFileInfo fileInfo(path);
    int index = tabs->addTab(editor, fileInfo.fileName());
    tabs->setTabToolTip(index, path); 
    tabs->setCurrentIndex(index);

    opened_docs_widget->addItem(fileInfo.fileName());
    syncListSelectionWithTab(index);

    // Explicitly update highlighter in case currentChanged is not emitted
    onCurrentTabChanged(index);
}

void kamakura::on_actionSave_triggered()
{
    CodeEditor* editor = currentEditor();
    if (!editor) return;

    QString filePath = QFileDialog::getSaveFileName(this, "Save File As", QDir::homePath());
    if (filePath.isEmpty()) return;

    tabs->setTabToolTip(tabs->currentIndex(), filePath);
    on_actionSave_2_triggered();

    QFileInfo fileInfo(filePath);
    tabs->setTabText(tabs->currentIndex(), fileInfo.fileName());
    opened_docs_widget->item(tabs->currentIndex())->setText(fileInfo.fileName());
    onCurrentTabChanged(tabs->currentIndex());
}

void kamakura::on_actionSave_2_triggered()
{
    CodeEditor* editor = currentEditor();
    if (!editor) return;

    QString filePath = tabs->tabToolTip(tabs->currentIndex());
    if (filePath.isEmpty()) {
        on_actionSave_triggered();
        return;
    }

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error", "Could not save file: " + file.errorString());
        return;
    }

    file.write(editor->toPlainText().toUtf8());
    file.close();
    editor->document()->setModified(false);
    ui->statusbar->showMessage("File saved", 2000);
}


void kamakura::closeTab(int index)
{
    CodeEditor* editor = qobject_cast<CodeEditor*>(tabs->widget(index));
    if (editor && editor->document()->isModified()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Unsaved Changes",
            "'" + tabs->tabText(index) + "' has been modified. Save changes?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            on_actionSave_triggered();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }
    tabs->removeTab(index);
    delete opened_docs_widget->takeItem(index);

    if (tabs->count() == 0) {
        on_actionNew_triggered();
    }
}


void kamakura::dragEnterEvent(QDragEnterEvent* drag_event)
{
    if (drag_event->mimeData()->hasUrls())
        drag_event->acceptProposedAction();
}

void kamakura::dropEvent(QDropEvent* drop_event)
{
    foreach (const QUrl &url, drop_event->mimeData()->urls()) {
        QString filePath = url.toLocalFile();
        if (!filePath.isEmpty()) {
            openFileByPath(filePath);
        }
    }
}


void kamakura::onCurrentTabChanged(int index)
{
    if (index < 0) {
        metricReporter->clearMetrics();
        updateWindowTitle("");
        return;
    }

    CodeEditor* editor = currentEditor();
    if (!editor) return;

    QFileInfo fileInfo(tabs->tabToolTip(index));
    highlighter->setDocument(editor->document());
    highlighter->setExtension(fileInfo.suffix());
    highlighter->rehighlight();

    
    disconnect(findDialog, &FindDialog::startFinding, nullptr, nullptr);
    disconnect(findDialog, &FindDialog::startFindingBackward, nullptr, nullptr);
    disconnect(findDialog, &FindDialog::startReplacing, nullptr, nullptr);
    disconnect(findDialog, &FindDialog::startReplacingAll, nullptr, nullptr);
    disconnect(editor, &CodeEditor::findResultReady, nullptr, nullptr);

    connect(findDialog, &FindDialog::startFinding, editor, &CodeEditor::find);
    connect(findDialog, &FindDialog::startFindingBackward, editor, &CodeEditor::findBackward);
    connect(findDialog, &FindDialog::startReplacing, editor, &CodeEditor::replace);
    connect(findDialog, &FindDialog::startReplacingAll, editor, &CodeEditor::replaceAll);
    connect(editor, &CodeEditor::findResultReady, findDialog, &FindDialog::onFindResultReady);

    disconnect(editor, &CodeEditor::wordCountChanged, nullptr, nullptr);
    disconnect(editor, &CodeEditor::charCountChanged, nullptr, nullptr);
    disconnect(editor, &CodeEditor::lineChanged, nullptr, nullptr);
    disconnect(editor, &CodeEditor::columnChanged, nullptr, nullptr);

    connect(editor, &CodeEditor::wordCountChanged, metricReporter, &MetricReporter::updateWordCount);
    connect(editor, &CodeEditor::charCountChanged, metricReporter, &MetricReporter::updateCharCount);
    connect(editor, &CodeEditor::lineChanged, metricReporter, &MetricReporter::updateLineCount);
    connect(editor, &CodeEditor::columnChanged, metricReporter, &MetricReporter::updateColumnCount);
    
    editor->updateMetrics(); 
    updateWindowTitle(tabs->tabToolTip(index));
    syncListSelectionWithTab(index);
    editor->setFocus();
}

void kamakura::updateTabDirtyStatus()
{
    CodeEditor* senderEditor = qobject_cast<CodeEditor*>(sender());
    if (!senderEditor) return;
    
    for (int i = 0; i < tabs->count(); ++i) {
        if (tabs->widget(i) == senderEditor) {
            QString tabText = tabs->tabText(i);
            if (tabText.endsWith('*')) {
                tabText.chop(1);
            }
            if(senderEditor->document()->isModified()) {
                tabText += "*";
            }
            tabs->setTabText(i, tabText);
            opened_docs_widget->item(i)->setText(tabText);
            break;
        }
    }
}


void kamakura::updateWindowTitle(const QString& currentFile)
{
    QString title = "Kamakura";
    if (!currentFile.isEmpty()) {
        title += " - " + QFileInfo(currentFile).fileName();
    }
    setWindowTitle(title);
}

void kamakura::syncListSelectionWithTab(int index)
{
    if (index < 0 || index >= opened_docs_widget->count()) return;
    opened_docs_widget->blockSignals(true);
    opened_docs_widget->setCurrentRow(index);
    opened_docs_widget->blockSignals(false);
}

void kamakura::syncTabSelectionWithList(QListWidgetItem* item)
{
    int row = opened_docs_widget->row(item);
    tabs->setCurrentIndex(row);
}

void kamakura::handleTabMoved(int from, int to)
{
    QListWidgetItem* item = opened_docs_widget->takeItem(from);
    opened_docs_widget->insertItem(to, item);
    syncListSelectionWithTab(to);
}

void kamakura::on_actionCut_triggered() { if (currentEditor()) currentEditor()->cut(); }
void kamakura::on_actionCopy_triggered() { if (currentEditor()) currentEditor()->copy(); }
void kamakura::on_actionPaste_triggered() { if (currentEditor()) currentEditor()->paste(); }
void kamakura::on_actionZoom_triggered() { if (currentEditor()) currentEditor()->zoomIn(2); }
void kamakura::on_actionZoom_2_triggered() { if (currentEditor()) currentEditor()->zoomOut(2); }

void kamakura::on_actionQuit_triggered() { QApplication::quit(); }

void kamakura::on_actionSearch_and_Replace_triggered()
{
    findDialog->show();
    findDialog->activateWindow();
}

void kamakura::on_actionKamakura_triggered()
{
    QMessageBox::about(this, "About Kamakura",
        "<p><b>Kamakura Code Editor</b></p>"
        "<p>Version 2.0</p>"
        "<p>A lightweight, extensible code editor.</p>"
        "<p>By Mehrdad S. Beni & Hiroshi Watabe, 2025.</p>");
}

void kamakura::on_actionHowTo_triggered()
{
    QMessageBox::information(this, "How To",
        "Open a file (.inp, .i, .py) to see syntax highlighting and get code completion suggestions. "
        "More language definitions can be added by creating new XML files.");
}

void kamakura::setLightTheme()
{
    darkThemeEnabled = false;
    qApp->setPalette(qApp->style()->standardPalette());
    for (int i = 0; i < tabs->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {
            editor->applyLightTheme();
        }
    }
}

void kamakura::setDarkTheme()
{
    qApp->setStyle("Fusion");
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(42,42,42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66,66,66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    darkThemeEnabled = true;
    qApp->setPalette(darkPalette);
    for (int i = 0; i < tabs->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {
            editor->applyDarkTheme();
        }
    }
}
