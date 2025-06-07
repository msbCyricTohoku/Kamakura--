#ifndef FINDDIALOG_H
#define FINDDIALOG_H
#include <QDialog>
#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QFont>

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

class FindDialog : public QDialog
{
    Q_OBJECT

public:

    FindDialog(QWidget *parent = nullptr);
    ~FindDialog();

signals:

    void startFinding(QString queryText, bool caseSensitive, bool wholeWords);
    void startFindingBackward(QString queryText, bool caseSensitive, bool wholeWords);
    void startReplacing(QString what, QString with, bool caseSensitive, bool wholeWords);
    void startReplacingAll(QString what, QString with, bool caseSensitive, bool wholeWords);

public slots:

    void on_FindKeywordButton_clicked();
    void on_FindBackwardButton_clicked();
    void on_exitButton_clicked();
    void on_replaceOperation_initiated();
    void onFindResultReady(QString message) { QMessageBox::information(this, "Search and Replace", message); }

private:

    void initializeWidgets();
    void initializeLayout();

    QLabel *findLabel;
    QLabel *replaceLabel;
    QPushButton *FindKeywordButton;
    QPushButton *FindBackwardButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
    QPushButton *exitButton;
    QLineEdit *findLineEdit;
    QLineEdit *replaceLineEdit;
    QHBoxLayout *findHorizontalLayout;
    QHBoxLayout *replaceHorizontalLayout;
    QHBoxLayout *optionsLayout;
    QVBoxLayout *verticalLayout;
};

#endif // FINDDIALOG_H
