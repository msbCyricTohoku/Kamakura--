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
#include "notebookeditor.h"


//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

kamakura::kamakura(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::kamakura)
{
    ui->setupUi(this); 
    setAcceptDrops(true);

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

    on_actionNew_triggered();
    updateWindowTitle("");
    updateSyntaxLabel(QString(), false);
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


    //**************************************************************8
    viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(opened_docs_dock->toggleViewAction());

    QActionGroup* themeGroup = new QActionGroup(this);

    lightThemeAction = viewMenu->addAction("Light Theme");
    darkThemeAction = viewMenu->addAction("Dark Theme");
    solarizedLightAction = viewMenu->addAction("Solarized Light Theme");
    solarizedDarkAction = viewMenu->addAction("Solarized Dark Theme");
    monokaiThemeAction = viewMenu->addAction("Monokai Theme");
    nordThemeAction = viewMenu->addAction("Nord Theme");

    lightThemeAction->setCheckable(true);
    darkThemeAction->setCheckable(true);
    solarizedLightAction->setCheckable(true);
    solarizedDarkAction->setCheckable(true);
    monokaiThemeAction->setCheckable(true);
    nordThemeAction->setCheckable(true);

    themeGroup->addAction(lightThemeAction);
    themeGroup->addAction(darkThemeAction);
    themeGroup->addAction(solarizedLightAction);
    themeGroup->addAction(solarizedDarkAction);
    themeGroup->addAction(monokaiThemeAction);
    themeGroup->addAction(nordThemeAction);

    switch (currentTheme) {
    case Theme::Dark:
        darkThemeAction->setChecked(true);
        break;
    case Theme::SolarizedLight:
        solarizedLightAction->setChecked(true);
        break;
    case Theme::SolarizedDark:
        solarizedDarkAction->setChecked(true);
        break;
    case Theme::Monokai:
        monokaiThemeAction->setChecked(true);
        break;
    case Theme::Nord:
        nordThemeAction->setChecked(true);
        break;
    default:
        lightThemeAction->setChecked(true);
        break;
    }

    wordWrapAction = viewMenu->addAction("Word Wrap");
    lineNumbersAction = viewMenu->addAction("Show Line Numbers");

    wordWrapAction->setCheckable(true);
    wordWrapAction->setChecked(wordWrapEnabled);
    lineNumbersAction->setCheckable(true);
    lineNumbersAction->setChecked(lineNumbersEnabled);

    connect(lightThemeAction, &QAction::triggered, this, &kamakura::setLightTheme);
    connect(darkThemeAction, &QAction::triggered, this, &kamakura::setDarkTheme);
    connect(solarizedLightAction, &QAction::triggered, this, &kamakura::setSolarizedLightTheme);
    connect(solarizedDarkAction, &QAction::triggered, this, &kamakura::setSolarizedDarkTheme);
    connect(monokaiThemeAction, &QAction::triggered, this, &kamakura::setMonokaiTheme);
    connect(nordThemeAction, &QAction::triggered, this, &kamakura::setNordTheme);
    //*************************************************************




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


    consoleDock = new QDockWidget(trLang("Data Science Output", "データサイエンス出力"), this);
    consoleOutput = new QPlainTextEdit(consoleDock);
    consoleOutput->setReadOnly(true);
    consoleOutput->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4; font-family: Courier;");
    consoleDock->setWidget(consoleOutput);
    addDockWidget(Qt::BottomDockWidgetArea, consoleDock);
    consoleDock->hide();

    scriptProcess = new QProcess(this);
    connect(scriptProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        consoleOutput->moveCursor(QTextCursor::End);
        consoleOutput->insertPlainText(scriptProcess->readAllStandardOutput());
    });
    connect(scriptProcess, &QProcess::readyReadStandardError, this, [this]() {
        consoleOutput->moveCursor(QTextCursor::End);
        consoleOutput->insertPlainText(scriptProcess->readAllStandardError());
    });

    /* the new addition for notebook and runscripts */

    /*
    QMenu* dsMenu = menuBar()->addMenu(trLang("Data Science", "データサイエンス"));

    QAction* runAct = dsMenu->addAction(trLang("Run Script (Python)", "スクリプト実行"));
    runAct->setShortcut(QKeySequence("F5"));
    connect(runAct, &QAction::triggered, this, &kamakura::runScript);

    dsMenu->addSeparator();
    QAction* setPyAct = dsMenu->addAction(trLang("Select Global Python Interpreter...", "Pythonインタープリターを選択..."));
    connect(setPyAct, &QAction::triggered, this, &kamakura::selectPythonInterpreter);

    QAction* createVenvAct = dsMenu->addAction(trLang("Create Virtual Environment (.venv) here", "仮想環境(.venv)を作成"));
    connect(createVenvAct, &QAction::triggered, this, &kamakura::createVenv);

    QAction* newNbAct = ui->menuFile->addAction(trLang("New Notebook (.kmk)", "新規ノートブック (.kmk)"));
    connect(newNbAct, &QAction::triggered, this, &kamakura::createNewNotebook);

    QAction* installPipAct = dsMenu->addAction(trLang("Install Pip Packages...", "Pipパッケージをインストール..."));
    connect(installPipAct, &QAction::triggered, this, &kamakura::installPipPackage);
    */



    dsMenu = menuBar()->addMenu(trLang("Data Science", "データサイエンス"));

    runAct = dsMenu->addAction(trLang("Run Script (Python)", "スクリプトを実行 (Python)"));
    runAct->setShortcut(QKeySequence("F5"));
    connect(runAct, &QAction::triggered, this, &kamakura::runScript);

    dsMenu->addSeparator();
    setPyAct = dsMenu->addAction(trLang("Select Global Python Interpreter...", "グローバル Python インタープリターを選択..."));
    connect(setPyAct, &QAction::triggered, this, &kamakura::selectPythonInterpreter);

    createVenvAct = dsMenu->addAction(trLang("Create Virtual Environment (.venv) here", "ここに仮想環境 (.venv) を作成"));
    connect(createVenvAct, &QAction::triggered, this, &kamakura::createVenv);

    newNbAct = ui->menuFile->addAction(trLang("New Notebook (.kmk)", "新規ノートブック (.kmk)"));
    connect(newNbAct, &QAction::triggered, this, &kamakura::createNewNotebook);

    installPipAct = dsMenu->addAction(trLang("Install Pip Packages...", "Pip パッケージをインストール..."));
    connect(installPipAct, &QAction::triggered, this, &kamakura::installPipPackage);
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


void kamakura::on_actionNew_triggered()
{
    CodeEditor* editor = new CodeEditor;
    setupEditor(editor);
    
    int index = tabs->addTab(editor, "Untitled");
    tabs->setTabToolTip(index, ""); 
    tabs->setCurrentIndex(index);

    opened_docs_widget->addItem("Untitled");
    syncListSelectionWithTab(index);

    onCurrentTabChanged(index);

    
}

void kamakura::on_actionOpen_triggered()
{
    //QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath());
    QString filePath = QFileDialog::getOpenFileName(this,
                                                   trLang("Open File", "ファイルを開く"),
                                                   QDir::homePath());

    if (!filePath.isEmpty()) {
        openFileByPath(filePath);
    }
}

void kamakura::openFileByPath(const QString& path)
{
    //prevent opening the same file twice
    for (int i = 0; i < tabs->count(); ++i) {
        if (tabs->tabToolTip(i) == path) {
            tabs->setCurrentIndex(i);
            return;
        }
    }

    QString ext = QFileInfo(path).suffix().toLower();
    QWidget* newEditor = nullptr;


        if (ext == "kmk") {
            NotebookEditor* notebook = new NotebookEditor(this);
            if (!notebook->loadKmkDoc(path)) {
                QMessageBox::warning(this, trLang("Error", "エラー"), trLang("Could not load notebook.", "ノートブックを読み込めませんでした。"));
                delete notebook; return;
            }
            connect(notebook->document(), &QTextDocument::modificationChanged, this, &kamakura::updateTabDirtyStatus);
            newEditor = notebook;
        }
    else {
        QFile file(path);
        if (!file.open(QFile::ReadOnly | QFile::Text)) {
            QMessageBox::warning(this, trLang("Error", "エラー"), trLang("Could not open file: ", "ファイルを開けません: ") + file.errorString());
            return;
        }
        QString contents = file.readAll();
        file.close();

        CodeEditor* editor = new CodeEditor(this);
        setupEditor(editor);
        editor->setPlainText(contents);
        newEditor = editor;
    }

    // if only open tab is an empty "untitled" tab, close it
    QWidget* currentWidget = tabs->currentWidget();
    if (tabs->count() == 1 && tabs->tabText(0) == "Untitled") {
        if (CodeEditor* ce = qobject_cast<CodeEditor*>(currentWidget)) {
            if (!ce->document()->isModified()) {
                tabs->removeTab(0);
                delete opened_docs_widget->takeItem(0);
                ce->deleteLater();
            }
        }
    }

    QFileInfo fileInfo(path);

    int index = tabs->addTab(newEditor, fileInfo.fileName());

    tabs->setTabToolTip(index, path);
    tabs->setCurrentIndex(index);

    opened_docs_widget->addItem(fileInfo.fileName());

    syncListSelectionWithTab(index);

    if (ext != "kmk") {
        onCurrentTabChanged(index);
    }

    addRecentFile(path);
}

void kamakura::on_actionSave_triggered()
{
    CodeEditor* editor = currentEditor();
    if (!editor) return;

    //QString filePath = QFileDialog::getSaveFileName(this, "Save File As", QDir::homePath());

    QString filePath = QFileDialog::getSaveFileName(this,
                                                   trLang("Save File As", "名前を付けて保存"),
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
    QWidget* currentWidget = tabs->currentWidget();
    if (!currentWidget) return;

    QString filePath = tabs->tabToolTip(tabs->currentIndex());

    //if the file is "Untitled" (no path), redirect to "Save As"
    if (filePath.isEmpty()) {
        on_actionSave_triggered();
        return;
    }

    //saving a notebook (.kmk)
    if (auto ne = qobject_cast<NotebookEditor*>(currentWidget)) {
        if (!filePath.endsWith(".kmk", Qt::CaseInsensitive)) {
            filePath = QFileDialog::getSaveFileName(this, "Save Notebook", QDir::homePath(), "Kamakura Doc (*.kmk)");
            if (filePath.isEmpty()) return;

            if (!filePath.endsWith(".kmk", Qt::CaseInsensitive)) {
                filePath += ".kmk";
            }

            tabs->setTabToolTip(tabs->currentIndex(), filePath);
            tabs->setTabText(tabs->currentIndex(), QFileInfo(filePath).fileName());

            if (opened_docs_widget->item(tabs->currentIndex())) {
                opened_docs_widget->item(tabs->currentIndex())->setText(QFileInfo(filePath).fileName());
            }
        }
        ne->saveKmkDoc(filePath);
        ui->statusbar->showMessage(trLang("Notebook saved", "保存しました"), 2000);
        return;
    }

    //saving a standard code script (.py, .cpp, .txt, etc.)
    else if (auto ce = qobject_cast<CodeEditor*>(currentWidget)) {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {

            file.write(ce->toPlainText().toUtf8());
            file.close();

            ce->document()->setModified(false);

            QString fileName = QFileInfo(filePath).fileName();
            tabs->setTabText(tabs->currentIndex(), fileName);
            if (opened_docs_widget->item(tabs->currentIndex())) {
                opened_docs_widget->item(tabs->currentIndex())->setText(fileName);
            }

            ui->statusbar->showMessage(trLang("File saved", "保存しました"), 2000);
        } else {
            QMessageBox::warning(this, trLang("Error", "エラー"),
                                 trLang("Could not save file: ", "保存できませんでした: ") + file.errorString());
        }
    }
}



void kamakura::closeTab(int index)
{
    QWidget* widgetToClose = tabs->widget(index);
    if (!widgetToClose) return;

    bool isModified = false;

    //check for unsaved changes
    if (CodeEditor* editor = qobject_cast<CodeEditor*>(widgetToClose)) {
        isModified = editor->document()->isModified();
    } else if (NotebookEditor* notebook = qobject_cast<NotebookEditor*>(widgetToClose)) {
        isModified = notebook->document()->isModified();
    }

    if (isModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                  trLang("Unsaved Changes", "保存されていない変更"),
                                                                  trLang("'" + tabs->tabText(index) + "' has been modified. Save changes?",
                                                                         "'" + tabs->tabText(index) + "' は編集されています。保存しますか？"),
                                                                  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            tabs->setCurrentIndex(index);
            on_actionSave_triggered();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    tabs->removeTab(index);
    delete opened_docs_widget->takeItem(index);

    widgetToClose->deleteLater();

    if (tabs->count() == 0) {
        on_actionNew_triggered();
    }
}


void kamakura::dragEnterEvent(QDragEnterEvent* drag_event)
{
    if (drag_event->mimeData()->hasUrls())
        drag_event->acceptProposedAction();
}

/*
void kamakura::dropEvent(QDropEvent* drop_event)
{
    foreach (const QUrl &url, drop_event->mimeData()->urls()) {
        QString filePath = url.toLocalFile();
        if (!filePath.isEmpty()) {
            openFileByPath(filePath);
        }
    }
}
*/

void kamakura::dropEvent(QDropEvent* drop_event)
{
    foreach (const QUrl &url, drop_event->mimeData()->urls()) {
        QString filePath = url.toLocalFile();
        if (!filePath.isEmpty()) {
            QString ext = QFileInfo(filePath).suffix().toLower();

            //if image and if notebook open then allow drop event
            if ((ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "gif") &&
                qobject_cast<NotebookEditor*>(tabs->currentWidget())) {
                continue;
            }
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

void kamakura::runScript()
{
    CodeEditor* editor = currentEditor();
    if (!editor) {
        QMessageBox::information(this, "Info", "Please open a code script (.py) to run.");
        return;
    }

    QString filePath = tabs->tabToolTip(tabs->currentIndex());
    if (filePath.isEmpty() || editor->document()->isModified()) {
        QMessageBox::information(this, "Save Required", "Please save the file before running.");
        on_actionSave_triggered();
        filePath = tabs->tabToolTip(tabs->currentIndex());
        if (filePath.isEmpty()) return;
    }

    QString dirPath = QFileInfo(filePath).absolutePath();
    QString ext = QFileInfo(filePath).suffix().toLower();
    QString cmd;

    //if (ext == "r") {
    //    cmd = "Rscript";
    //}
    if (ext == "py" || ext == "pyw") {

        //reading global python settings
        QSettings settings("Kamakura", "Kamakura");
        cmd = settings.value("PythonExec", "python").toString();

        //here we check for local venv
        QString winVenv1 = QDir(dirPath).filePath(".venv/Scripts/python.exe");
        QString unixVenv1 = QDir(dirPath).filePath(".venv/bin/python");
        QString winVenv2 = QDir(dirPath).filePath("venv/Scripts/python.exe");
        QString unixVenv2 = QDir(dirPath).filePath("venv/bin/python");

        QString detectedVenv = "";

        if (QFile::exists(winVenv1)) detectedVenv = winVenv1;
        else if (QFile::exists(unixVenv1)) detectedVenv = unixVenv1;
        else if (QFile::exists(winVenv2)) detectedVenv = winVenv2;
        else if (QFile::exists(unixVenv2)) detectedVenv = unixVenv2;

        if (!detectedVenv.isEmpty()) {
            QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                      "Virtual Environment Detected",
                                                                      "A local virtual environment was found in this directory.\n\nDo you want to run the script using this isolated environment instead of the global default?",
                                                                      QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                cmd = detectedVenv;
            }
        }
    } else {
        QMessageBox::information(this, "Info", "Unsupported script type. Open a .py file to run.");
        return;
    }

    consoleOutput->clear();
    consoleDock->show();
    consoleOutput->appendPlainText(QString("--- Running %1 using [%2] ---\n").arg(QFileInfo(filePath).fileName(), cmd));

    scriptProcess->setWorkingDirectory(dirPath);
    scriptProcess->start(cmd, QStringList() << filePath);
}

void kamakura::createNewNotebook()
{
    NotebookEditor* ne = new NotebookEditor(this);
    connect(ne->document(), &QTextDocument::modificationChanged, this, &kamakura::updateTabDirtyStatus);
    int index = tabs->addTab(ne, "Untitled.kmk");
    tabs->setTabToolTip(index, "");
    tabs->setCurrentIndex(index);
    opened_docs_widget->addItem("Untitled.kmk");
}

void kamakura::selectPythonInterpreter()
{
    QSettings settings("Kamakura", "Kamakura");
    QString current = settings.value("PythonExec", "python").toString();

    QString newPath = QFileDialog::getOpenFileName(this,
                                                   trLang("Select Python Executable", "Python実行ファイルを選択"),
                                                   current);

    if (!newPath.isEmpty()) {
        settings.setValue("PythonExec", newPath);
        QMessageBox::information(this, "Success", "Global Python interpreter updated to:\n" + newPath);
    }
}

//here Kamakura opens .venv when users asks
void kamakura::createVenv()
{
    QString filePath = tabs->tabToolTip(tabs->currentIndex());
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please save your script in a directory first so the environment knows where to install.");
        return;
    }

    QString dirPath = QFileInfo(filePath).absolutePath();
    QSettings settings("Kamakura", "Kamakura");
    QString pythonExec = settings.value("PythonExec", "python").toString();

    consoleOutput->clear();
    consoleDock->show();
    consoleOutput->appendPlainText("--- Creating Virtual Environment (.venv) in: " + dirPath + " ---\n");
    consoleOutput->appendPlainText("Please wait... This may take up to a minute.\n");
    QCoreApplication::processEvents();

    scriptProcess->setWorkingDirectory(dirPath);
    scriptProcess->start(pythonExec, QStringList() << "-m" << "venv" << ".venv");
}

void kamakura::installPipPackage()
{
    QString filePath = tabs->tabToolTip(tabs->currentIndex());
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please save your script in a directory first.");
        return;
    }

    QString dirPath = QFileInfo(filePath).absolutePath();
    QString winVenv = QDir(dirPath).filePath(".venv/Scripts/python.exe");
    QString unixVenv = QDir(dirPath).filePath(".venv/bin/python");

    QString detectedVenv = "";
    if (QFile::exists(winVenv)) detectedVenv = winVenv;
    else if (QFile::exists(unixVenv)) detectedVenv = unixVenv;

    if (detectedVenv.isEmpty()) {
        QMessageBox::warning(this, "Error", "No .venv found in this directory. Create one first.");
        return;
    }

    bool ok;
    QString packages = QInputDialog::getText(this, "Install Packages",
                                             "Enter pip package names (e.g., numpy pandas matplotlib):",
                                             QLineEdit::Normal, "", &ok);
    if (ok && !packages.isEmpty()) {
        consoleOutput->clear();
        consoleDock->show();
        consoleOutput->appendPlainText("--- Installing packages via pip: " + packages + " ---\n");

        scriptProcess->setWorkingDirectory(dirPath);
        QStringList args;
        args << "-m" << "pip" << "install";
        args.append(packages.split(" ", Qt::SkipEmptyParts));

        scriptProcess->start(detectedVenv, args);
    }
}

void kamakura::on_actionCut_triggered() { if (currentEditor()) currentEditor()->cut(); }
void kamakura::on_actionCopy_triggered() { if (currentEditor()) currentEditor()->copy(); }
void kamakura::on_actionPaste_triggered() { if (currentEditor()) currentEditor()->paste(); }
//void kamakura::on_actionZoom_triggered() { if (currentEditor()) currentEditor()->zoomIn(2); }
//void kamakura::on_actionZoom_2_triggered() { if (currentEditor()) currentEditor()->zoomOut(2); }


void kamakura::on_actionZoom_triggered() {
    QWidget* currentWidget = tabs->currentWidget();
    if (auto ce = qobject_cast<CodeEditor*>(currentWidget)) ce->zoomIn(2);
    else if (auto ne = qobject_cast<NotebookEditor*>(currentWidget)) ne->zoomIn(2);
}

void kamakura::on_actionZoom_2_triggered() {
    QWidget* currentWidget = tabs->currentWidget();
    if (auto ce = qobject_cast<CodeEditor*>(currentWidget)) ce->zoomOut(2);
    else if (auto ne = qobject_cast<NotebookEditor*>(currentWidget)) ne->zoomOut(2);
}

/*
void kamakura::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0)
            currentEditor()->zoomIn(1);
        else
            currentEditor()->zoomOut(1);
        event->accept();
    }
}
*/

void kamakura::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        QWidget* currentWidget = tabs->currentWidget();
        auto ce = qobject_cast<CodeEditor*>(currentWidget);
        auto ne = qobject_cast<NotebookEditor*>(currentWidget);

        if (event->angleDelta().y() > 0) {
            if (ce) ce->zoomIn(1);
            else if (ne) ne->zoomIn(1);
        } else {
            if (ce) ce->zoomOut(1);
            else if (ne) ne->zoomOut(1);
        }
        event->accept();
    }
}


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
                                    trLang("Go to Line", "行へ移動"),
                                    trLang("Line number:", "行番号:"),
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
         ui->statusbar->showMessage(trLang("Removed trailing spaces", "末尾の空白を削除しました"), 2000);
    }
}


void kamakura::on_actionKamakura_triggered()
{
    //QMessageBox::about(this, "About Kamakura",
       // "<p><b>Kamakura Code Editor</b></p>"
        //"<p>Version 4.5</p>"
        //"<p>A lightweight, extensible code editor.</p>"
        //"<p>By Mehrdad S. Beni & Hiroshi Watabe, 2025.</p>");
QMessageBox::about(
    this,
    trLang("About Kamakura", "Kamakuraについて"),
    trLang(
        "<p><b>Kamakura Code Editor</b></p>"
        "<p>Version 4.5</p>"
        "<p>A lightweight, extensible code editor.</p>"
        "<p>By Mehrdad S. Beni & Hiroshi Watabe, 2025.</p>",
            "<p><b>Kamakuraコードエディタ</b></p>"
            "<p>バージョン 4.5</p>"
            "<p>軽量で拡張性のあるコードエディタです。</p>"
            "<p>Mehrdad S. Beni & Hiroshi Watabe, 2025.</p>"
    )
);

}

void kamakura::on_actionHowTo_triggered()
{
QMessageBox::information(
    this,
    trLang("How To", "使い方"),
    trLang(
        "Open a file (.inp, .i, .py) to see syntax highlighting and get code completion suggestions. "
        "More language definitions can be added by creating new XML files.",
            "ファイル（.inp、.i、.py）を開くとシンタックスハイライトが表示され、コード補完候補が提示されます。"
            "新しいXMLファイルを作成することで、対応する言語を追加できます。"
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

void kamakura::setMonokaiTheme()
{
    qApp->setStyle("Fusion");
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#272822"));
    palette.setColor(QPalette::WindowText, QColor("#F8F8F2"));
    palette.setColor(QPalette::Base, QColor("#1E1F1C"));
    palette.setColor(QPalette::AlternateBase, QColor("#272822"));
    palette.setColor(QPalette::ToolTipBase, QColor("#F8F8F2"));
    palette.setColor(QPalette::ToolTipText, QColor("#272822"));
    palette.setColor(QPalette::Text, QColor("#F8F8F2"));
    palette.setColor(QPalette::Button, QColor("#3E3D32"));
    palette.setColor(QPalette::ButtonText, QColor("#F8F8F2"));
    palette.setColor(QPalette::BrightText, QColor("#F92672"));
    palette.setColor(QPalette::Highlight, QColor("#49483E"));
    palette.setColor(QPalette::HighlightedText, QColor("#A6E22E"));

    currentTheme = Theme::Monokai;
    qApp->setPalette(palette);
    for (int i = 0; i < tabs->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {
            editor->applyMonokaiTheme();
        }
    }
}

void kamakura::setNordTheme()
{
    qApp->setStyle("Fusion");
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#2E3440"));
    palette.setColor(QPalette::WindowText, QColor("#D8DEE9"));
    palette.setColor(QPalette::Base, QColor("#242933"));
    palette.setColor(QPalette::AlternateBase, QColor("#2E3440"));
    palette.setColor(QPalette::ToolTipBase, QColor("#3B4252"));
    palette.setColor(QPalette::ToolTipText, QColor("#E5E9F0"));
    palette.setColor(QPalette::Text, QColor("#D8DEE9"));
    palette.setColor(QPalette::Button, QColor("#434C5E"));
    palette.setColor(QPalette::ButtonText, QColor("#E5E9F0"));
    palette.setColor(QPalette::BrightText, QColor("#BF616A"));
    palette.setColor(QPalette::Highlight, QColor("#4C566A"));
    palette.setColor(QPalette::HighlightedText, QColor("#88C0D0"));

    currentTheme = Theme::Nord;
    qApp->setPalette(palette);
    for (int i = 0; i < tabs->count(); ++i) {

        if (auto editor = qobject_cast<CodeEditor*>(tabs->widget(i))) {

            editor->applyNordTheme();
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

    opened_docs_dock->setWindowTitle(trLang("Opened Files", "開いているファイル"));

    if (dsMenu) {
        dsMenu->setTitle(trLang("Data Science", "データサイエンス"));
        runAct->setText(trLang("Run Script (Python)", "スクリプトを実行 (Python)"));
        setPyAct->setText(trLang("Select Global Python Interpreter...", "グローバル Python インタープリターを選択..."));
        createVenvAct->setText(trLang("Create Virtual Environment (.venv) here", "ここに仮想環境 (.venv) を作成"));
        installPipAct->setText(trLang("Install Pip Packages...", "Pip パッケージをインストール..."));
    }

    if (newNbAct) {
        newNbAct->setText(trLang("New Notebook (.kmk)", "新規ノートブック (.kmk)"));
    }

    if (consoleDock) {
        consoleDock->setWindowTitle(trLang("Data Science Output", "データサイエンス出力"));
    }

    ui->menuFile->setTitle(trLang("File", "ファイル"));
    ui->menuEdit->setTitle(trLang("Edit", "編集"));
    ui->menuAbout->setTitle(trLang("About", "情報"));

    ui->actionOpen->setText(trLang("Open", "開く"));
    ui->actionNew->setText(trLang("New", "新規"));
    ui->actionQuit->setText(trLang("Quit", "終了"));
    ui->actionSave->setText(trLang("Save As", "名前を付けて保存"));
    ui->actionSave_2->setText(trLang("Save", "保存"));
    ui->actionCopy->setText(trLang("Copy", "コピー"));
    ui->actionPaste->setText(trLang("Paste", "貼り付け"));
    ui->actionCut->setText(trLang("Cut", "切り取り"));
    ui->actionDuplicate_Line->setText(trLang("Duplicate Line", "行を複製"));
    ui->actionZoom->setText(trLang("Zoom+", "ズーム+"));
    ui->actionZoom_2->setText(trLang("Zoom-", "ズーム-"));
    ui->actionSearch_and_Replace->setText(trLang("Search and Replace", "検索と置換"));
    ui->actionHowTo->setText(trLang("HowTo", "使い方"));
    ui->actionKamakura->setText(trLang("Kamakura", "Kamakura"));

    ui->actionGoToLine->setText(trLang("Go to Line", "行へ移動"));

    ui->actionToggle_Comment->setText(trLang("Toggle Comment","コメント化／解除"));

    ui->actionInsert_DateTime->setText(trLang("Insert Date/Time","日付と時刻を挿入"));


    ui->actionTrim_Trailing_Spaces->setText(trLang("Trim Trailing Spaces", "末尾の空白を削除"));


    if (viewMenu) {
        viewMenu->setTitle(trLang("View", "表示"));
    }
    if (lightThemeAction) {
        lightThemeAction->setText(trLang("Light Theme", "ライトテーマ"));
    }
    if (darkThemeAction) {
        darkThemeAction->setText(trLang("Dark Theme", "ダークテーマ"));
    }
    if (solarizedLightAction) {
        solarizedLightAction->setText(trLang("Solarized Light Theme", "ソーラライズド ライト"));
    }
    if (solarizedDarkAction) {
        solarizedDarkAction->setText(trLang("Solarized Dark Theme", "ソーラライズド ダーク"));
    }
    if (monokaiThemeAction) {
        monokaiThemeAction->setText(trLang("Monokai Theme", "Monokai テーマ"));
    }
    if (nordThemeAction) {
        nordThemeAction->setText(trLang("Nord Theme", "Nord テーマ"));
    }

    englishAction->setText(trLang("English", "英語"));
    japaneseAction->setText(trLang("Japanese", "日本語"));
    if(languageMenu)
        languageMenu->setTitle(trLang("Language", "言語"));

    wordWrapAction->setText(trLang("Word Wrap", "自動折り返し"));
    if(lineNumbersAction)
        lineNumbersAction->setText(trLang("Show Line Numbers", "行番号を表示"));

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
        name = trLang("Plain Text", "プレーンテキスト");

    QString prefix = trLang("Syntax: ", "構文: ");
    if (!hasHighlighting && !extension.isEmpty()) {
          name += trLang(" (no highlighting)", " (ハイライトなし)");
    }
    syntaxLabel->setText(prefix + name);
}

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
