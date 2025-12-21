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
    layout->setContentsMargins(5, 0, 5, 0);
    layout->addWidget(wordLabel);
    layout->addWidget(wordCountLabel);
    layout->addWidget(charLabel);
    layout->addWidget(charCountLabel);
    layout->addWidget(lineLabel);
    layout->addWidget(lineCountLabel);
    layout->addWidget(columnLabel);
    layout->addWidget(columnCountLabel);
    setLayout(layout);
}

void MetricReporter::setLanguage(Language lang)
{
    if (lang == Language::Japanese) {
        wordLabel->setText("単語: ");
        charLabel->setText("文字: ");
        lineLabel->setText("行: ");
        columnLabel->setText("列: ");
    } else {
        wordLabel->setText("Words: ");
        charLabel->setText("Chars: ");
        lineLabel->setText("Line: ");
        columnLabel->setText("Column: ");
    }
}


void MetricReporter::updateWordCount(int wordCount)
{
    wordCountLabel->setText(QString::number(wordCount));
}


void MetricReporter::updateCharCount(int charCount)
{
    charCountLabel->setText(QString::number(charCount));
}

void MetricReporter::updateLineCount(int current, int total)
{
    lineCountLabel->setText(QString::number(current) + tr("/") + QString::number(total));
}


void MetricReporter::updateColumnCount(int columnCount)
{
    columnCountLabel->setText(QString::number(columnCount));
}

void MetricReporter::clearMetrics()
{
    wordCountLabel->clear();
    charCountLabel->clear();
    lineCountLabel->clear();
    columnCountLabel->clear();
}
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
