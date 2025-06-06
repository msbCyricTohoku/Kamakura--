#include "finddialog.h"
#include <QHBoxLayout>
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    initializeWidgets();

    setFocusProxy(findLineEdit);

    initializeLayout();

    setWindowTitle(tr("Search and Replace Tool"));


    connect(FindKeywordButton, SIGNAL(clicked()), this, SLOT(on_FindKeywordButton_clicked()));
    connect(replaceButton, SIGNAL(clicked()), this, SLOT(on_replaceOperation_initiated()));
    connect(replaceAllButton, SIGNAL(clicked()), this, SLOT(on_replaceOperation_initiated()));
}


FindDialog::~FindDialog()
{
    delete findLabel;
    delete replaceLabel;
    delete findLineEdit;
    delete replaceLineEdit;
    delete FindKeywordButton;
    delete replaceButton;
    delete replaceAllButton;
    delete findHorizontalLayout;
    delete replaceHorizontalLayout;
    delete optionsLayout;
    delete verticalLayout;
}

void FindDialog::initializeWidgets()
{
    findLabel = new QLabel(tr("Keyword to search:    "));
    QFont f( "Arial", 14, QFont::Normal);
    findLabel->setFont(f);
    //textLabel->setFont( f);
    //findLabel->setPixmap()
    replaceLabel = new QLabel(tr("Replacement word:    "));
    replaceLabel->setFont(f);
    findLineEdit = new QLineEdit();
    findLineEdit->setFont(f);
    replaceLineEdit = new QLineEdit();
    replaceLineEdit->setFont(f);
    FindKeywordButton = new QPushButton(tr("&Find keyword"));
    FindKeywordButton->setFont(f);
    replaceButton = new QPushButton(tr("&Replace keyword"));
    replaceButton->setFont(f);
    replaceAllButton = new QPushButton(tr("&Replace all keywords"));
    replaceAllButton->setFont(f);
}


void FindDialog::initializeLayout()
{
    findHorizontalLayout = new QHBoxLayout();
    replaceHorizontalLayout = new QHBoxLayout();
    optionsLayout = new QHBoxLayout();
    verticalLayout = new QVBoxLayout();

    verticalLayout->addLayout(findHorizontalLayout);
    verticalLayout->addLayout(replaceHorizontalLayout);
    verticalLayout->addLayout(optionsLayout);

    findHorizontalLayout->addWidget(findLabel);
    findHorizontalLayout->addWidget(findLineEdit);
    replaceHorizontalLayout->addWidget(replaceLabel);
    replaceHorizontalLayout->addWidget(replaceLineEdit);

    optionsLayout->addWidget(FindKeywordButton);
    optionsLayout->addWidget(replaceButton);
    optionsLayout->addWidget(replaceAllButton);

    setLayout(verticalLayout);
}

void FindDialog::on_FindKeywordButton_clicked()
{
    QString query = findLineEdit->text();

    if (query.isEmpty())
    {
        QMessageBox::information(this, tr("No keyword"), tr("Please enter a keyword"));
        return;
    }

    bool caseSensitive = false;
    bool wholeWords = true;
    emit(startFinding(query, caseSensitive, wholeWords));
}


void FindDialog::on_replaceOperation_initiated()
{
    QString what = findLineEdit->text();

    if (what.isEmpty())
    {
        QMessageBox::information(this, tr("No keyword"), tr("Please enter a keyword"));
        return;
    }

    QString with = replaceLineEdit->text();
    bool caseSensitive = false;
    bool wholeWords = true;
    bool replace = sender() == replaceButton;

    if (replace)
    {
        emit(startReplacing(what, with, caseSensitive, wholeWords));
    }
    else
    {
        emit(startReplacingAll(what, with, caseSensitive, wholeWords));
    }

}
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
