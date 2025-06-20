#ifndef METRICREPORTER_H
#define METRICREPORTER_H
#include <QSplitter>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QFrame>
#include "language.h"


//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

class MetricReporter : public QFrame
{
    Q_OBJECT

public:
    explicit MetricReporter(QWidget *parent = nullptr);
    void setLanguage(Language lang);


public slots:
    void updateWordCount(int wordCount);
    void updateCharCount(int charCount);
    void updateLineCount(int current, int total);
    void updateColumnCount(int columnCount);
    void clearMetrics();

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
