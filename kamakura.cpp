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
#include <QInputDialog>
#include <QSettings>
#include <QStringList>
#include <QLabel>

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

    syntaxLabel = new QLabel(tr("Syntax: Plain Text"), this);
    syntaxLabel->setContentsMargins(10, 0, 10, 0);
    ui->statusbar->addPermanentWidget(syntaxLabel);

    setupDocks();
    setupConnections();
    
    //for last opened files
    recentFilesMenu = new QMenu(tr("Recent Files"), this);
    ui->menuFile->addMenu(recentFilesMenu);
    loadRecentFiles();
    updateRecentFilesMenu();

    //setDarkTheme();

    setLightTheme();

     setLanguage(currentLanguage);

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

    //QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    
    QMenu *viewMenu = menuBar()->addMenu("&View");


    viewMenu->addAction(opened_docs_dock->toggleViewAction());

    QActionGroup* themeGroup = new QActionGroup(this);
    
    //QAction* lightTheme = viewMenu->addAction(tr("Light Theme"));
    //QAction* darkTheme = viewMenu->addAction(tr("Dark Theme"));

    QAction* lightTheme = viewMenu->addAction("Light Theme");
    QAction* darkTheme = viewMenu->addAction("Dark Theme");
    QAction* solarizedLight = viewMenu->addAction("Solarized Light Theme");
    QAction* solarizedDark = viewMenu->addAction("Solarized Dark Theme");

    lightTheme->setCheckable(true);
    darkTheme->setCheckable(true);
    solarizedLight->setCheckable(true);
    solarizedDark->setCheckable(true);


    themeGroup->addAction(lightTheme);
    themeGroup->addAction(darkTheme);
    //darkTheme->setChecked(true);
    //lightTheme->setChecked(true);

    themeGroup->addAction(solarizedLight);
    themeGroup->addAction(solarizedDark);

    switch (currentTheme) {
    case Theme::Dark:
        darkTheme->setChecked(true);
        break;
    case Theme::SolarizedLight:
        solarizedLight->setChecked(true);
        break;
    case Theme::SolarizedDark:
        solarizedDark->setChecked(true);
        break;
    default:
        lightTheme->setChecked(true);
        break;
    }

    //wordWrapAction = viewMenu->addAction(tr("Word Wrap"));

    wordWrapAction = viewMenu->addAction("Word Wrap");

    lineNumbersAction = viewMenu->addAction("Show Line Numbers");

    wordWrapAction->setCheckable(true);
    wordWrapAction->setChecked(wordWrapEnabled);

    lineNumbersAction->setCheckable(true);
    lineNumbersAction->setChecked(lineNumbersEnabled);

    connect(lightTheme, &QAction::triggered, this, &kamakura::setLightTheme);
    connect(darkTheme, &QAction::triggered, this, &kamakura::setDarkTheme);
    connect(solarizedLight, &QAction::triggered, this, &kamakura::setSolarizedLightTheme);
    connect(solarizedDark, &QAction::triggered, this, &kamakura::setSolarizedDarkTheme);



   languageMenu = menuBar()->addMenu("Language");
    QActionGroup* langGroup = new QActionGroup(this);
    englishAction = languageMenu->addAction("English");
    japaneseAction = languageMenu->addAction("Japanese");
    englishAction->setCheckable(true);
    japaneseAction->setCheckable(true);
    langGroup->addAction(englishAction);
    langGroup->addAction(japaneseAction);
    englishAction->setChecked(true);

    connect(englishAction, &QAction::triggered, [this](){ setLanguage(Language::English); });
    connect(japaneseAction, &QAction::triggered, [this](){ setLanguage(Language::Japanese); });

}

void kamakura::setupConnections()
{
    connect(tabs, &QTabWidget::currentChanged, this, &kamakura::onCurrentTabChanged);
    connect(tabs, &QTabWidget::tabCloseRequested, this, &kamakura::closeTab);
    connect(tabs->tabBar(), &QTabBar::tabMoved, this, &kamakura::handleTabMoved);
    connect(opened_docs_widget, &QListWidget::itemClicked, this, &kamakura::syncTabSelectionWithList);

    connect(wordWrapAction, &QAction::toggled, this, &kamakura::toggleWordWrap);
    connect(lineNumbersAction, &QAction::toggled, this, &kamakura::toggleLineNumbers);
}

void kamakura::setupEditor(CodeEditor* editor)
{
    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(12);
    editor->setFont(font);

    //if (darkThemeEnabled)
    //    editor->applyDarkTheme();
    //else
    //    editor->applyLightTheme();

    switch (currentTheme) {
    case Theme::Dark:
        editor->applyDarkTheme();
        break;
    case Theme::SolarizedLight:
        editor->applySolarizedLightTheme();
        break;
    case Theme::SolarizedDark:
        editor->applySolarizedDarkTheme();
        break;
    default:
        editor->applyLightTheme();
        break;
    }


    editor->setWordWrap(wordWrapEnabled);

    editor->setLineNumbersVisible(lineNumbersEnabled);

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
    //QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath());
    QString filePath = QFileDialog::getOpenFileName(this,
                                                   trLang("Open File", "\xE3\x83\x95\xE3\x82\xA1\xE3\x82\xA4\xE3\x83\xAB\xE3\x82\x92\xE9\x96\x8B\xE3\x81\x8F"),
                                                   QDir::homePath());

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
        //QMessageBox::warning(this, "Error", "Could not open file: " + file.errorString());
        QMessageBox::warning(this,
                             trLang("Error", "\xE3\x82\xA8\xE3\x83\xA9\xE3\x83\xBC"),
                             trLang("Could not open file: ", "\xE3\x83\x95\xE3\x82\xA1\xE3\x82\xA4\xE3\x83\xAB\xE3\x82\x92\xE9\x96\x8B\xE3\x81\x91\xE3\x81\xBE\xE3\x81\x9B\xE3\x82\x93: ") + file.errorString());
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

    addRecentFile(path);
}

void kamakura::on_actionSave_triggered()
{
    CodeEditor* editor = currentEditor();
    if (!editor) return;

    //QString filePath = QFileDialog::getSaveFileName(this, "Save File As", QDir::homePath());

    QString filePath = QFileDialog::getSaveFileName(this,
                                                   trLang("Save File As", "\xE5\x90\x8D\xE5\x89\x8D\xE3\x82\x92\xE4\xBB\x98\xE3\x81\x91\xE3\x81\xA6\xE4\xBF\x9D\xE5\xAD\x98"),
                                                   QDir::homePath());

    if (filePath.isEmpty()) return;

    tabs->setTabToolTip(tabs->currentIndex(), filePath);
    on_actionSave_2_triggered();

    QFileInfo fileInfo(filePath);
    tabs->setTabText(tabs->currentIndex(), fileInfo.fileName());
    opened_docs_widget->item(tabs->currentIndex())->setText(fileInfo.fileName());
    onCurrentTabChanged(tabs->currentIndex());

    addRecentFile(filePath);
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
        //QMessageBox::warning(this, "Error", "Could not save file: " + file.errorString());
        QMessageBox::warning(this,
                             trLang("Error", "\xE3\x82\xA8\xE3\x83\xA9\xE3\x83\xBC"),
                             trLang("Could not save file: ", "\xE3\x83\x95\xE3\x82\xA1\xE3\x82\xA4\xE3\x83\xAB\xE3\x82\x92\xE4\xBF\x9D\xE5\xAD\x98\xE3\x81\xA7\xE3\x81\x8D\xE3\x81\xBE\xE3\x81\x9B\xE3\x82\x93: ") + file.errorString());

        return;
    }

    file.write(editor->toPlainText().toUtf8());
    file.close();
    editor->document()->setModified(false);
    //ui->statusbar->showMessage("File saved", 2000);
    ui->statusbar->showMessage(trLang("File saved", "\xE4\xBF\x9D\xE5\xAD\x98\xE5\xAE\x8C\xE4\xBA\x86"), 2000);
}


void kamakura::closeTab(int index)
{
    CodeEditor* editor = qobject_cast<CodeEditor*>(tabs->widget(index));
    if (editor && editor->document()->isModified()) {
        QMessageBox::StandardButton reply;
        //reply = QMessageBox::question(this, "Unsaved Changes",
        //    "'" + tabs->tabText(index) + "' has been modified. Save changes?",

        reply = QMessageBox::question(this,
            trLang("Unsaved Changes", "\xE4\xBF\x9D\xE5\xAD\x98\xE3\x81\x95\xE3\x82\x8C\xE3\x81\xA6\xE3\x81\x84\xE3\x81\xAA\xE3\x81\x84\xE5\xA4\x89\xE6\x9B\xB4"),
            trLang("'" + tabs->tabText(index) + "' has been modified. Save changes?",
                  "'" + tabs->tabText(index) + "'\xE3\x81\xAF\xE7\xB7\xA8\xE9\x9B\x86\xE3\x81\x95\xE3\x82\x8C\xE3\x81\xBE\xE3\x81\x97\xE3\x81\x9F\xE3\x80\x82\xE4\xBF\x9D\xE5\xAD\x98\xE3\x81\x97\xE3\x81\xBE\xE3\x81\x99\xE3\x81\x8B\xEF\xBC\x9F"),
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
         updateSyntaxLabel(QString(), false);
        return;
    }

    CodeEditor* editor = currentEditor();
    if (!editor) return;

    QFileInfo fileInfo(tabs->tabToolTip(index));
    highlighter->setDocument(editor->document());
    //highlighter->setExtension(fileInfo.suffix());
    QString ext = fileInfo.suffix().toLower();
    hasCurrentHighlighting = highlighter->setExtension(ext);
    highlighter->rehighlight();


    //QString ext = fileInfo.suffix().toLower();
    updateSyntaxLabel(ext, hasCurrentHighlighting);

    QString prefix = "#";
    if (ext == "c" || ext == "cpp" || ext == "h" || ext == "hpp" || ext == "rs" || ext == "js")
        prefix = "//";
    else if (ext == "hs")
        prefix = "--";
    else if (ext == "f90" || ext == "f95" || ext == "f")
        prefix = "!";
    editor->setCommentPrefix(prefix);

    
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


void kamakura::on_actionGoToLine_triggered()
{
    CodeEditor* editor = currentEditor();
    if (!editor) return;

    bool ok = false;
    int maxLine = editor->document()->blockCount();
    int current = editor->textCursor().blockNumber() + 1;
    int line = QInputDialog::getInt(this,
                                    trLang("Go to Line", "\xE8\xA1\x8C\xE3\x81\xB8\xE7\xA7\xBB\xE5\x8B\x95"),
                                    trLang("Line number:", "\xE8\xA1\x8C\xE7\x95\xAA\xE5\x8F\xB7:"),
                                    current, 1, maxLine, 1, &ok);
    if (ok) {
        QTextBlock block = editor->document()->findBlockByNumber(line - 1);
        if (block.isValid()) {
            QTextCursor cursor(block);
            editor->setTextCursor(cursor);
            editor->centerCursor();
        }
    }
}


void kamakura::on_actionDuplicate_Line_triggered()
{
    if (auto editor = currentEditor()) {
        editor->duplicateLine();
    }
}


void kamakura::on_actionToggle_Comment_triggered()
{
    if (auto editor = currentEditor()) {
        editor->toggleComment();
    }
}


void kamakura::on_actionInsert_DateTime_triggered()
{
    if (auto editor = currentEditor()) {
        editor->insertDateTime();
    }
}

void kamakura::on_actionTrim_Trailing_Spaces_triggered()
{
    if (auto editor = currentEditor()) {
        editor->trimTrailingWhitespace();
        editor->updateMetrics();
        ui->statusbar->showMessage(trLang("Removed trailing spaces", "\xE6\x9C\xAB\xE5\xB0\xBE\xE3\x81\xAE\xE7\xA9\xBA\xE7\x99\xBD\xE3\x82\x92\xE5\x89\x8A\xE9\x99\xA4"), 2000);
    }
}


void kamakura::on_actionKamakura_triggered()
{
    //QMessageBox::about(this, "About Kamakura",
       // "<p><b>Kamakura Code Editor</b></p>"
        //"<p>Version 2.0</p>"
        //"<p>A lightweight, extensible code editor.</p>"
        //"<p>By Mehrdad S. Beni & Hiroshi Watabe, 2025.</p>");
QMessageBox::about(
    this,
    trLang("About Kamakura", "Kamakura\xE3\x81\xAB\xE3\x81\xA4\xE3\x81\x84\xE3\x81\xA6"),
    trLang(
        "<p><b>Kamakura Code Editor</b></p>"
        "<p>Version 4.0</p>"
        "<p>A lightweight, extensible code editor.</p>"
        "<p>By Mehrdad S. Beni & Hiroshi Watabe, 2025.</p>",
        "<p><b>Kamakura\xE3\x82\xB3\xE3\x83\xBC\xE3\x83\x89\xE3\x82\xA8\xE3\x83\x87\xE3\x82\xA3\xE3\x82\xBF</b></p>"
        "<p>\xE3\x83\x90\xE3\x83\xBC\xE3\x82\xB8\xE3\x83\xA7\xE3\x83\xB3 3.0</p>"
        "<p>\xE8\xBB\xBD\xE9\x87\x8F\xE3\x81\xA7\xE6\x8B\xA1\xE5\xBC\xB5\xE6\x80\xA7\xE3\x81\xAE\xE3\x81\x82\xE3\x82\x8B\xE3\x82\xB3\xE3\x83\xBC\xE3\x83\x89\xE3\x82\xA8\xE3\x83\x87\xE3\x82\xA3\xE3\x82\xBF\xE3\x81\xA7\xE3\x81\x99\xE3\x80\x82</p>"
        "<p>Mehrdad S. Beni & Hiroshi Watabe, 2025.</p>"
    )
);

}

void kamakura::on_actionHowTo_triggered()
{
QMessageBox::information(
    this,
    trLang("How To", "\xE4\xBD\xBF\xE3\x81\x84\xE6\x96\xB9"),
    trLang(
        "Open a file (.inp, .i, .py) to see syntax highlighting and get code completion suggestions. "
        "More language definitions can be added by creating new XML files.",
        "\xE3\x83\x95\xE3\x82\xA1\xE3\x82\xA4\xE3\x83\xAB(.inp, .i, .py)\xE3\x82\x92\xE9\x96\x8B\xE3\x81\x84\xE3\x81\xA6"
        "\xE3\x82\xB7\xE3\x83\xB3\xE3\x82\xBF\xE3\x83\x83\xE3\x82\xAF\xE3\x82\xB9\xE3\x83\x8F\xE3\x82\xA4\xE3\x83\xA9\xE3\x82\xA4"
        "\xE3\x83\x88\xE3\x82\x92\xE7\xA2\xBA\xE8\xAA\x8D\xE3\x81\x97\xE3\x80\x81\xE3\x82\xB3\xE3\x83\xBC\xE3\x83\x89\xE3\x82\xB3"
        "\xE3\x83\xB3\xE3\x83\x97\xE3\x83\xAA\xE3\x83\xBC\xE3\x82\xB7\xE3\x83\xA7\xE3\x83\xB3\xE3\x82\x92\xE8\x87\xAA\xE5\x8B\x95"
        "\xE5\xAE\x8C\xE6\x88\x90\xE3\x81\x95\xE3\x81\x9B\xE3\x82\x8B\xE3\x81\x93\xE3\x81\xA8\xE3\x81\x8C\xE3\x81\xA7\xE3\x81\x8D"
        "\xE3\x81\xBE\xE3\x81\x99\xE3\x80\x82\xE6\x96\xB0\xE3\x81\x97\xE3\x81\x84XML\xE3\x83\x95\xE3\x82\xA1\xE3\x82\xA4\xE3\x83\xAB"
        "\xE3\x82\x92\xE4\xBD\x9C\xE6\x88\x90\xE3\x81\x99\xE3\x82\x8B\xE3\x81\x93\xE3\x81\xA8\xE3\x81\xA7\xE8\xA8\x80\xE8\xAA\x9E"
        "\xE3\x82\x92\xE8\xBF\xBD\xE5\x8A\xA0\xE3\x81\xA7\xE3\x81\x8D\xE3\x81\xBE\xE3\x81\x99\xE3\x80\x82"
    )
);

}

void kamakura::setLightTheme()
{
    //darkThemeEnabled = false;
     currentTheme = Theme::Light;
    qApp->setPalette(qApp->style()->standardPalette());
    for (int i = 0; i < tabs->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {
            editor->applyLightTheme();
        }
    }
}

void kamakura::toggleWordWrap(bool enabled)
{
    wordWrapEnabled = enabled;
    for (int i = 0; i < tabs->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {
            editor->setWordWrap(enabled);
        }
    }
}


void kamakura::toggleLineNumbers(bool enabled)
{
    lineNumbersEnabled = enabled;
    for (int i = 0; i < tabs->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {
            editor->setLineNumbersVisible(enabled);
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
    //darkThemeEnabled = true;

    currentTheme = Theme::Dark;

    qApp->setPalette(darkPalette);
    for (int i = 0; i < tabs->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {
            editor->applyDarkTheme();
        }
    }
}


void kamakura::setSolarizedLightTheme()
{
    qApp->setStyle("Fusion");
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#fdf6e3"));
    palette.setColor(QPalette::WindowText, QColor("#657b83"));
    palette.setColor(QPalette::Base, QColor("#eee8d5"));
    palette.setColor(QPalette::AlternateBase, QColor("#fdf6e3"));
    palette.setColor(QPalette::ToolTipBase, QColor("#657b83"));
    palette.setColor(QPalette::ToolTipText, QColor("#657b83"));
    palette.setColor(QPalette::Text, QColor("#657b83"));
    palette.setColor(QPalette::Button, QColor("#eee8d5"));
    palette.setColor(QPalette::ButtonText, QColor("#657b83"));
    palette.setColor(QPalette::BrightText, QColor("#dc322f"));
    palette.setColor(QPalette::Highlight, QColor("#b58900"));
    palette.setColor(QPalette::HighlightedText, QColor("#fdf6e3"));
    currentTheme = Theme::SolarizedLight;
    qApp->setPalette(palette);
    for (int i = 0; i < tabs->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {
            editor->applySolarizedLightTheme();
        }
    }
}

void kamakura::setSolarizedDarkTheme()
{
    qApp->setStyle("Fusion");
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#002b36"));
    palette.setColor(QPalette::WindowText, QColor("#839496"));
    palette.setColor(QPalette::Base, QColor("#073642"));
    palette.setColor(QPalette::AlternateBase, QColor("#002b36"));
    palette.setColor(QPalette::ToolTipBase, QColor("#839496"));
    palette.setColor(QPalette::ToolTipText, QColor("#839496"));
    palette.setColor(QPalette::Text, QColor("#839496"));
    palette.setColor(QPalette::Button, QColor("#073642"));
    palette.setColor(QPalette::ButtonText, QColor("#839496"));
    palette.setColor(QPalette::BrightText, QColor("#dc322f"));
    palette.setColor(QPalette::Highlight, QColor("#586e75"));
    palette.setColor(QPalette::HighlightedText, QColor("#002b36"));
    currentTheme = Theme::SolarizedDark;
    qApp->setPalette(palette);
    for (int i = 0; i < tabs->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {
            editor->applySolarizedDarkTheme();
        }
    }
}


QString kamakura::trLang(const QString& en, const QString& ja) const
{
    return currentLanguage == Language::Japanese ? ja : en;
}

void kamakura::setLanguage(Language lang)
{
    currentLanguage = lang;

    // Update dock and actions
    opened_docs_dock->setWindowTitle(trLang("Opened Files", "\xE9\x96\x8B\xE3\x81\x84\xE3\x81\xA6\xE3\x81\x84\xE3\x82\x8B\xE3\x83\x95\xE3\x82\xA1\xE3\x82\xA4\xE3\x83\xAB"));

    ui->menuFile->setTitle(trLang("File", "\xE3\x83\x95\xE3\x82\xA1\xE3\x82\xA4\xE3\x83\xAB"));
    ui->menuEdit->setTitle(trLang("Edit", "\xE7\xB7\xA8\xE9\x9B\x86"));
    ui->menuAbout->setTitle(trLang("About", "\xE6\x83\x85\xE5\xA0\xB1"));

    ui->actionOpen->setText(trLang("Open", "\xE9\x96\x8B\xE3\x81\x8F"));
    ui->actionNew->setText(trLang("New", "\xE6\x96\xB0\xE8\xA6\x8F"));
    ui->actionQuit->setText(trLang("Quit", "\xE7\xB5\x82\xE4\xBA\x86"));
    ui->actionSave->setText(trLang("Save As", "\xE5\x90\x8D\xE5\x89\x8D\xE3\x82\x92\xE4\xBB\x98\xE3\x81\x91\xE3\x81\xA6\xE4\xBF\x9D\xE5\xAD\x98"));
    ui->actionSave_2->setText(trLang("Save", "\xE4\xBF\x9D\xE5\xAD\x98"));
    ui->actionCopy->setText(trLang("Copy", "\xE3\x82\xB3\xE3\x83\x94\xE3\x83\xBC"));
    ui->actionPaste->setText(trLang("Paste", "\xE8\xB2\xBC\xE3\x82\x8A\xE4\xBB\x98\xE3\x81\x91"));
    ui->actionCut->setText(trLang("Cut", "\xE5\x89\xB2\xE3\x82\x8A\xE5\x8F\x96\xE3\x82\x8A"));
    ui->actionDuplicate_Line->setText(trLang("Duplicate Line", "\xE8\xA1\x8C\xE3\x82\x92\xE8\xA4\x87\xE8\xA3\xBD"));
    ui->actionZoom->setText(trLang("Zoom+", "\xE3\x82\xBA\xE3\x83\xBC\xE3\x83\xA0+"));
    ui->actionZoom_2->setText(trLang("Zoom-", "\xE3\x82\xBA\xE3\x83\xBC\xE3\x83\xA0-"));
    ui->actionSearch_and_Replace->setText(trLang("Search and Replace", "\xE6\xA4\x9C\xE7\xB4\xA2\xE7\xAD\x89\xE3\x81\xA8\xE7\xBD\xAE\xE6\x8F\x9B"));
    ui->actionHowTo->setText(trLang("HowTo", "\xE4\xBD\xBF\xE3\x81\x84\xE6\x96\xB9"));
    ui->actionKamakura->setText(trLang("Kamakura", "Kamakura"));

    englishAction->setText(trLang("English", "\xE8\x8B\xB1\xE8\xAA\x9E"));
    japaneseAction->setText(trLang("Japanese", "\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"));
    if(languageMenu)
        languageMenu->setTitle(trLang("Language", "\xE8\xA8\x80\xE8\xAA\x9E"));

    wordWrapAction->setText(trLang("Word Wrap", "\xE8\xA1\x8C\xE6\x8A\x98\xE3\x82\x8A\xE8\xBE\xBC\xE3\x81\xBF"));
    if(lineNumbersAction)
        lineNumbersAction->setText(trLang("Show Line Numbers", "\xE8\xA1\x8C\xE7\x95\xAA\xE5\x8F\xB7\xE3\x82\x92\xE8\xA1\xA8\xE7\xA4\xBA"));

    metricReporter->setLanguage(lang);

    if (tabs && tabs->count() > 0) {
        QFileInfo info(tabs->tabToolTip(tabs->currentIndex()));
        updateSyntaxLabel(info.suffix(), hasCurrentHighlighting);
    } else {
        updateSyntaxLabel(QString(), hasCurrentHighlighting);
    }
}


void kamakura::addRecentFile(const QString& path)
{
    QString absPath = QFileInfo(path).absoluteFilePath();
    recentFiles.removeAll(absPath);
    recentFiles.prepend(absPath);
    while (recentFiles.size() > MaxRecentFiles)
        recentFiles.removeLast();
    updateRecentFilesMenu();
    saveRecentFiles();
}

void kamakura::updateRecentFilesMenu()
{
    if (!recentFilesMenu)
        return;
    recentFilesMenu->clear();
    for (const QString& path : recentFiles) {
        QAction* act = recentFilesMenu->addAction(QFileInfo(path).fileName());
        act->setToolTip(path);
        connect(act, &QAction::triggered, this, [this, path]() {
            openFileByPath(path);
        });
    }
    recentFilesMenu->setEnabled(!recentFiles.isEmpty());
}

void kamakura::loadRecentFiles()
{
    QSettings settings("Kamakura", "Kamakura");
    recentFiles = settings.value("recentFiles").toStringList();
}

void kamakura::saveRecentFiles()
{
    QSettings settings("Kamakura", "Kamakura");
    settings.setValue("recentFiles", recentFiles);
}


QString kamakura::syntaxNameForExtension(const QString& extension) const
{
    QString ext = extension.toLower();
    if (ext == "i" || ext == "inp") return trLang("PHITS", "PHITS");
    if (ext == "py" || ext == "pyw") return trLang("Python", "Python");
    if (ext == "c" || ext == "cc" || ext == "cpp" || ext == "h" || ext == "hpp") return trLang("C/C++", "C/C++");
    if (ext == "rs") return trLang("Rust", "Rust");
    if (ext == "hs") return trLang("Haskell", "Haskell");
    if (ext == "f90" || ext == "f95" || ext == "f") return trLang("Fortran", "Fortran");
    if (ext == "html" || ext == "htm") return trLang("HTML", "HTML");
    if (ext == "css") return trLang("CSS", "CSS");
    return QString();
}

void kamakura::updateSyntaxLabel(const QString& extension, bool hasHighlighting)
{
    if (!syntaxLabel)
        return;

    QString name = syntaxNameForExtension(extension);
    if (name.isEmpty())
        name = trLang("Plain Text", "\xE3\x83\x97\xE3\x83\xAC\xE3\x83\xBC\xE3\x83\x86\xE3\x82\xAD\xE3\x82\xB9\xE3\x83\x88");

    QString prefix = trLang("Syntax: ", "\xE3\x82\xB7\xE3\x83\xB3\xE3\x82\xBF\xE3\x83\x83\xE3\x82\xAF\xE3\x82\xB9: ");
    if (!hasHighlighting && !extension.isEmpty()) {
        name += trLang(" (no highlighting)", " (\xE3\x83\x8F\xE3\x82\xA4\xE3\x83\xA9\xE3\x82\xA4\xE3\x83\x88\xE3\x81\xAA\xE3\x81\x97)");
    }
    syntaxLabel->setText(prefix + name);
}

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
