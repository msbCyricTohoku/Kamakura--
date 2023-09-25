#include "metrics.h"

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
MetricReporter::MetricReporter(QWidget *parent) : QFrame(parent)
{
    wordLabel = new QLabel("Words: ");
    wordCountLabel = new QLabel();
    charLabel = new QLabel("Chars: ");
    charCountLabel = new QLabel();
    lineLabel = new QLabel("Line: ");
    lineCountLabel = new QLabel();
    columnLabel = new QLabel("Column: ");
    columnCountLabel = new QLabel();

    QHBoxLayout *layout = new QHBoxLayout();
//    layout->addWidget(wordLabel);
 //   layout->addWidget(wordCountLabel);
 //   layout->addWidget(charLabel);
 ////   layout->addWidget(charCountLabel);
 //   layout->addWidget(lineLabel);
 //   layout->addWidget(lineCountLabel);
//    layout->addWidget(columnLabel);
//    layout->addWidget(columnCountLabel);
    setLayout(layout);
}


void MetricReporter::updateWordCount(int wordCount)
{
//    wordCountLabel->setText(QString::number(wordCount));
}


void MetricReporter::updateCharCount(int charCount)
{
 //   charCountLabel->setText(QString::number(charCount));
}

void MetricReporter::updateLineCount(int current, int total)
{
//    lineCountLabel->setText(QString::number(current) + tr("/") + QString::number(total));
}


void MetricReporter::updateColumnCount(int columnCount)
{
 //   columnCountLabel->setText(QString::number(columnCount));
}
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
