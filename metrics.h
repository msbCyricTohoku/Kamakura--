#ifndef METRICREPORTER_H
#define METRICREPORTER_H
#include <QSplitter>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QFrame>

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

class MetricReporter : public QFrame
{
    Q_OBJECT

public:
    explicit MetricReporter(QWidget *parent = nullptr);

public slots:
    void updateWordCount(int wordCount);
    void updateCharCount(int charCount);
    void updateLineCount(int current, int total);
    void updateColumnCount(int columnCount);

private:
    QLabel *wordLabel;
    QLabel *wordCountLabel;
    QLabel *lineLabel;
    QLabel *lineCountLabel;
    QLabel *charLabel;
    QLabel *charCountLabel;
    QLabel *columnLabel;
    QLabel *columnCountLabel;
};

#endif // METRICREPORTER_H

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
