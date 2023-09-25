#include "kamakura.h"
#include "ui_kamakura.h"
#include <iostream>
#include <string>
#include <QProcess>
#include <QDebug>
#include <QtDebug>
#include <QtPrintSupport/QPrinter>      // printing
#include <QtPrintSupport/QPrintDialog>  // printing
#include <QFileDialog>                  // file open/save dialogs
#include <QFile>                        // file descriptors, IO
#include <QTextStream>                  // file IO
#include <QStandardPaths>               // default open directory
#include <QDateTime>                    // current time
#include <QApplication>                 // quit
#include <QShortcut>

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

using namespace std;

kamakura::kamakura(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::kamakura)
{
    ui->setupUi(this);
        setAcceptDrops(true);

         SetupTabWidget();
        SetupOpenedDocsDock();

         // Add metric reporter and simulate a tab switch
         metricReporter = new MetricReporter();
         ui->statusbar->addPermanentWidget(metricReporter);

         // connect(file_system_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OpenFile(QModelIndex)));

        connect(tabs->tabBar(),      SIGNAL(tabCloseRequested(int)),        this, SLOT(UpdateCurrentIndexOnDelete(int)));
        on_actionNew_triggered();
       //on_actionOpen_triggered();

        setCentralWidget(tabs);
        tabs->currentWidget()->setFocus();

        findDialog = new FindDialog();

       connect(findDialog, SIGNAL(startFinding(QString, bool, bool)),  ((CodeEditor*)tabs->currentWidget()), SLOT(find(QString,bool,bool)));
      //  connect(findDialog, SIGNAL(startReplacing(QString, QString, bool, bool)),  ((CodeEditor*)tabs->currentWidget()), SLOT(replace(QString, QString, bool, bool)));
      //  connect(findDialog, SIGNAL(startReplacingAll(QString, QString, bool, bool)),  ((CodeEditor*)tabs->currentWidget()), SLOT(replaceAll(QString, QString, bool, bool)));
        connect( ((CodeEditor*)tabs->currentWidget()), SIGNAL(findResultReady(QString)), findDialog, SLOT(onFindResultReady(QString)));

       // connect(editor, SIGNAL(currentChanged(int)), this, SLOT(on_currentTabChanged(int)));


   // reconnectsignals();

          highlighter = new Highlighter("phits_commands.xml", this);

}

kamakura::~kamakura()
{
    delete ui;
}

void kamakura::SetupTabWidget() {
    tabs = new QTabWidget(this);
    tabs->setMovable(true);
    tabs->setTabsClosable(true);
    tabs->setUsesScrollButtons(true);
  connect(tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(CloseFile(int)));

    QToolButton* new_tab_button = new QToolButton;
   // new_tab_button->setText("N");
   // connect(new_tab_button, SIGNAL(clicked()), this, SLOT(on_actionNew_triggered()));
  //  tabs->setCornerWidget(new_tab_button, Qt::TopRightCorner);
    // connect(tabs->tabBar(),      SIGNAL(tabCloseRequested(int)),        this, SLOT(UpdateCurrentIndexOnDelete(int)));
}

void kamakura::SetupOpenedDocsDock()
{    opened_docs_widget = new QListWidget;

    // update on opening/creating new file
    // delete on deleting tab provided by DeleteTabFromList(int) function
    // update position in list
    connect(tabs->tabBar(),      SIGNAL(tabMoved(int, int)),            this, SLOT(ChangeTabIndexInList(int, int)));
    connect(opened_docs_widget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(UpdateCurrentIndex(QListWidgetItem*)));
    connect(opened_docs_widget, SIGNAL(currentRowChanged(int)),        tabs, SLOT(setCurrentIndex(int)));
    connect(tabs->tabBar(),      SIGNAL(currentChanged(int)),           this, SLOT(UpdateCurrentIndex(int)));
    connect(tabs->tabBar(),      SIGNAL(tabCloseRequested(int)),        this, SLOT(UpdateCurrentIndexOnDelete(int)));

    opened_docs_dock  = new QDockWidget("Opened files", this);

    opened_docs_dock->setWidget(opened_docs_widget);
    opened_docs_dock->setFeatures(QDockWidget::DockWidgetClosable);
    opened_docs_dock->hide();
    addDockWidget(Qt::RightDockWidgetArea, opened_docs_dock);




}

void kamakura::reconnectsignals()
{

    connect(findDialog, SIGNAL(startFinding(QString, bool, bool)),  ((CodeEditor*)tabs->currentWidget()), SLOT(find(QString,bool,bool)));
    connect(findDialog, SIGNAL(startReplacing(QString, QString, bool, bool)),  ((CodeEditor*)tabs->currentWidget()), SLOT(replace(QString, QString, bool, bool)));
    connect(findDialog, SIGNAL(startReplacingAll(QString, QString, bool, bool)),  ((CodeEditor*)tabs->currentWidget()), SLOT(replaceAll(QString, QString, bool, bool)));
    connect( ((CodeEditor*)tabs->currentWidget()), SIGNAL(findResultReady(QString)), findDialog, SLOT(onFindResultReady(QString)));

       connect(((CodeEditor*)tabs->currentWidget()), SIGNAL(wordCountChanged(int)), metricReporter, SLOT(updateWordCount(int)));

}

void kamakura::disconnectsignals()
{
    disconnect(findDialog, SIGNAL(startFinding(QString, bool, bool)),  ((CodeEditor*)tabs->currentWidget()), SLOT(find(QString,bool,bool)));
    disconnect(findDialog, SIGNAL(startReplacing(QString, QString, bool, bool)),  ((CodeEditor*)tabs->currentWidget()), SLOT(replace(QString, QString, bool, bool)));
    disconnect(findDialog, SIGNAL(startReplacingAll(QString, QString, bool, bool)),  ((CodeEditor*)tabs->currentWidget()), SLOT(replaceAll(QString, QString, bool, bool)));
    disconnect( ((CodeEditor*)tabs->currentWidget()), SIGNAL(findResultReady(QString)), findDialog, SLOT(onFindResultReady(QString)));

}


void kamakura::dragEnterEvent(QDragEnterEvent* drag_event) {
    if (drag_event->mimeData()->hasUrls())
        drag_event->acceptProposedAction();
}

void kamakura::dropEvent(QDropEvent* drop_event) {
  // QString dummy;
    QList<QUrl> url_list = drop_event->mimeData()->urls();
    foreach (QUrl url, url_list) {
        OpenFile(url.url(QUrl::RemoveScheme));
    }
}

void kamakura::on_actionQuit_triggered()
{
        QApplication::quit();
}


void kamakura::on_actionKamakura_triggered()
{
        QMessageBox msgAbout;
         msgAbout.setInformativeText("<span style='text-align: center'><p><b><font size = 20>Kamakura - -</font><p><font size = 14>PHITS lightweight code editor</font><p><font size = 14>Version 0.1</font><p><font size = 14>by CYRIC, Tohoku University</font><p>Mehrdad S. Beni & Hiroshi Watabe | 2023</span><span style='text-align: left'><p>Kamakura (かまくら or カマクラ) is a type of traditional snow dome in snowy regions of Japan. Winter events with kamakura snow huts are currently held in a number of locations in Japan. For example, in Yokote City in central Akita prefecture such events are celebrated every February 14 to 16. This event likely dates to a time when the Onodera clan ruled that area during the Sengoku period. After visiting Yokote City in Akita prefecture during winter, I felt in love with Kamakura and therefore the present multifunctional code editor for PHITS Monte Carlo package is named after these beautiful snow domes.</span><p>");
        QPixmap pixAbout;
        pixAbout.load("resources/kamakura.png");
        msgAbout.setIconPixmap(pixAbout);
        msgAbout.setStandardButtons(QMessageBox::Close);
        msgAbout.setDefaultButton(QMessageBox::Close);
        msgAbout.exec();


}

void kamakura::on_actionOpen_triggered()
{
        QString filepath = QFileDialog::getOpenFileName(this, "Choose file", "/home");
        if (filepath.isEmpty())
        return;
        OpenFile(filepath);

         ui->statusbar->showMessage("File opened", 2000);

        // tabs->currentWidget()->setFocus();

}


void kamakura::OpenFile(const QString& filepath) {

      //  disconnectsignals();


        for (int i = 0; i < tabs->count(); ++i)
        if (tabs->tabToolTip(i) == filepath) {
            QMessageBox::warning(this, "Duplicate", "File is open already",
                                 QMessageBox::Ok);
            return;
        }

        QString filename = filepath.section("/",-1,-1);
        QFile file(filepath);

        if (file.open(QIODevice::ReadOnly)) {
        CodeEditor* temp_text = (CodeEditor*)tabs->currentWidget();
        if (temp_text->document()->isEmpty() &&
            tabs->tabToolTip(tabs->currentIndex()) == "" &&
            tabs->tabText(tabs->currentIndex()) == "OpenFile") {
            DeleteTabFromList(tabs->currentIndex());
            delete tabs->widget(tabs->currentIndex());
        }



        CodeEditor* new_text_edit = new CodeEditor;

      // QTextStream in(&file);
        QString content = file.readAll();
        // editor->setPlainText(content);
        file.close();
       // new_text_edit->setPlainText(content);
        new_text_edit->appendPlainText(content);
        int index = tabs->addTab(new_text_edit, filename);
        tabs->setCurrentIndex(index);
        tabs->setTabToolTip(index, filepath);
        tabs->setTabWhatsThis(index, "No changes");
        connect(new_text_edit, SIGNAL(textChanged()), this, SLOT(UpdateParameter()));

        UpdateCurrentIndex(index);

        reconnectsignals();


        QListWidgetItem* new_item = new QListWidgetItem;
        new_item->setText(tabs->tabText(index));
        new_item->setToolTip(tabs->tabToolTip(index));
        opened_docs_widget->addItem(new_item);


        QString file_extension = QFileInfo(filename).suffix();
        if (highlighter->setExtension(file_extension)) {
            highlighter->setDocument(new_text_edit->document());
            highlighter->highlightBlock(new_text_edit->toPlainText());
        }

        tabs->setTabWhatsThis(index, "No changes");
        UpdateCurrentIndex(index);
        } else {
        (new QErrorMessage(this))->showMessage("Cannot open file");
        return;
        }
}


void kamakura::DeleteTabFromList(int index) {
            QListWidgetItem* temp_item = opened_docs_widget->takeItem(index);
            delete temp_item;
}



void kamakura::on_actionSave_triggered()
{
            QString filename = tabs->tabText(tabs->currentIndex());
            QString filepath = QFileDialog::getSaveFileName(this, "Save " + filename, "/home/" + filename);
            if (filepath.isEmpty())
            return;
            if (QFileInfo(filepath).suffix().isEmpty())
            filepath.append(".inp");

            QFile file(filepath);
            if (file.open(QIODevice::WriteOnly)) {
            file.write(((CodeEditor*)tabs->currentWidget())->document()->toPlainText().toUtf8());
            file.close();
            }

            filename = filepath.section("/",-1,-1);
            tabs->tabBar()->setTabText(tabs->currentIndex(), filename);
            tabs->tabBar()->setTabToolTip(tabs->currentIndex(), filepath);

            QString file_extension = QFileInfo(filename).suffix();
            if (highlighter->setExtension(file_extension)) {
            highlighter->setDocument(((CodeEditor*)tabs->currentWidget())->document());
            highlighter->highlightBlock(((CodeEditor*)tabs->currentWidget())->toPlainText());
            }

            tabs->setTabWhatsThis(tabs->currentIndex(), "No changes");

             ui->statusbar->showMessage("Script saved", 2000);
}




void kamakura::on_actionZoom_triggered()
{
    ((CodeEditor*)tabs->currentWidget())->zoomIn(1);
   //  ui->plainTextEdit->zoomIn(1);

     ui->statusbar->showMessage("Zoomed in", 2000);
}


void kamakura::on_actionZoom_2_triggered()
{
    ((CodeEditor*)tabs->currentWidget())->zoomOut(1);

     ui->statusbar->showMessage("Zoomed out", 2000);

}


void kamakura::on_actionSave_2_triggered()
{

    if (tabs->tabToolTip(tabs->currentIndex()) == "") {
            on_actionSave_triggered();
            return;
    }
    QString filepath = tabs->tabToolTip(tabs->currentIndex());
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly)) {
            file.write(((CodeEditor*)tabs->currentWidget())->document()->toPlainText().toUtf8()); // unsafe getting!
            file.close();
            tabs->setTabWhatsThis(tabs->currentIndex(), "No changes");
    } else {
            (new QErrorMessage(this))->showMessage("Cannot save file!");
            return;
    }


    ui->statusbar->showMessage("Script saved", 2000);


}



void kamakura::on_actionNew_triggered()
{
       //int systemRet = system("rm openedfile.tmp");
       //if(systemRet == -1){}

    //   ui->plainTextEdit->setPlainText("");

   // disconnectsignals();


       CodeEditor* new_text_edit = new CodeEditor;
       int index = tabs->addTab(new_text_edit, "OpenFile");
       tabs->setCurrentIndex(index);
       tabs->setTabToolTip(index, "");
       tabs->setTabWhatsThis(index, "No changes");
       connect(new_text_edit, SIGNAL(textChanged()), this, SLOT(UpdateParameter()));


       QListWidgetItem* new_item = new QListWidgetItem;
       new_item->setText(tabs->tabText(index));
       new_item->setToolTip(tabs->tabToolTip(index));
      opened_docs_widget->addItem(new_item);

       UpdateCurrentIndex(index);

   //   reconnectsignals();

}



void kamakura::on_actionCut_triggered()
{
       ((CodeEditor*)tabs->currentWidget())->cut();
     //  ui->plainTextEdit->cut();

        ui->statusbar->showMessage("Text cut", 2000);
}


void kamakura::on_actionPaste_triggered()
{
       ((CodeEditor*)tabs->currentWidget())->paste();
      // ui->plainTextEdit->paste();

       ui->statusbar->showMessage("Text paste", 2000);
}


void kamakura::on_actionCopy_triggered()
{
       ((CodeEditor*)tabs->currentWidget())->copy();
     //  ui->plainTextEdit->copy();


      ui->statusbar->showMessage("Text copy", 2000);
}


void kamakura::OpenFile(QModelIndex file_index) {
       if (!file_system_model->isDir(file_index))
            OpenFile(file_system_model->filePath(file_index));

}





void kamakura::on_actionSearch_and_Replace_triggered()
{
    //   disconnectsignals();
        findDialog->show();
        findDialog->activateWindow();
        findDialog->raise();
        findDialog->setFocus();

      //  reconnectsignals();

}


void kamakura::ChangeTabIndexInList(int old_index, int new_index) {
        QListWidgetItem* first_item  = opened_docs_widget->takeItem(old_index);
        opened_docs_widget->insertItem(new_index, first_item);

}

void kamakura::UpdateCurrentIndex(QListWidgetItem* current_item) {

        int index = current_item->listWidget()->row(current_item);
        tabs->setCurrentIndex(index);
}

void kamakura::UpdateCurrentIndex(int new_selection_index) {
        opened_docs_widget->setCurrentRow(new_selection_index);

        // + highlight update
        QString file = tabs->tabBar()->tabText(new_selection_index);
        QString file_extension = QFileInfo(file).suffix();
        if (!file_extension.isEmpty()) {
            // reconnectsignals();
            if (highlighter->setExtension(file_extension)) {
            highlighter->setDocument(((CodeEditor*)tabs->currentWidget())->document()); // unsafe getting!
            highlighter->highlightBlock(((CodeEditor*)tabs->currentWidget())->toPlainText()); // unsafe getting!
            }
        }

}



void kamakura::UpdateCurrentIndexOnDelete(int) {

        opened_docs_widget->setCurrentRow(opened_docs_widget->count() - 1);

}


void kamakura::CloseFile(int index_) {

        delete tabs->widget(index_);

        DeleteTabFromList(index_);

        if (!tabs->count())
            on_actionNew_triggered();
        tabs->currentWidget()->setFocus();
}

void kamakura::CloseFile() {
        CloseFile(tabs->currentIndex());
}

void::kamakura::UpdateParameter() {

        QString file = tabs->tabBar()->tabText(tabs->currentIndex());
        QString file_extension = QFileInfo(file).suffix();
        if (!file_extension.isEmpty()) {
            if (highlighter->setExtension(file_extension)) {
          //  reconnectsignals();
            tabs->setTabWhatsThis(tabs->currentIndex(), "Changed");

            return;
            }
        }
        tabs->setTabWhatsThis(tabs->currentIndex(), "Changed");

        disconnect(sender(), SIGNAL(textChanged()), this, SLOT(UpdateParameter()));

       //disconnectsignals();


}

void kamakura::on_actionHowTo_triggered()
{

        QMessageBox msgAbout;
        msgAbout.setInformativeText("<span style='text-align: left'><p><font size = 12>Open a PHITS script with extension of .inp or .i then you can use the syntax highlighing. Make sure you save your files. You can open as many scripts as you wish. Enjoy!</span><p>");
        msgAbout.setStandardButtons(QMessageBox::Close);
        msgAbout.setDefaultButton(QMessageBox::Close);
        msgAbout.exec();

}

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
