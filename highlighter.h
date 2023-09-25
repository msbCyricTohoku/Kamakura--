#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QString>
#include <QVector>
#include <QRegExp>
#include <QTextCharFormat>
#include <QDomDocument>
#include <QHash>

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

class Highlighter : public QSyntaxHighlighter {
public:
    explicit Highlighter(QString _filename, QObject *parent = 0);
    void highlightBlock(const QString &text) override;
    bool setExtension(const QString &_filename);
private:
    QString xml_filename;
    QRegExp current_extension;
    struct HighlightFormat{
        QRegExp pattern;
        QRegExp pattern2;
        QTextCharFormat format;
        QTextCharFormat format2;
    };
    struct HighlightLang{
        QVector <HighlightFormat> for_keywords;
        QVector <HighlightFormat> for_keywords2;
        QRegExp comment_beg_exp;
        QRegExp comment_end_exp;
        QTextCharFormat single_comment;
        QTextCharFormat single_comment2;
        QTextCharFormat multi_comment;
        QTextCharFormat keyword_format;
         QTextCharFormat keyword_format2;
        QTextCharFormat classname_format;
        QTextCharFormat function_format;
        QTextCharFormat quotation_format;
        QTextCharFormat preproc_format;
        QTextCharFormat preproc_format2;
    };

    QHash<QRegExp, HighlightLang> langs;
};

#endif // HIGHLIGHTER_H
