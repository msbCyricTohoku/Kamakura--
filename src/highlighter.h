#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QString>
#include <QVector>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QDomDocument>
#include <QHash>

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit Highlighter(const QStringList& languageFiles, QObject *parent = 0);
    bool setExtension(const QString &fileExtension);
    QStringList getKeywordsForExtension(const QString& fileExtension);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    struct Language {
        QString extensionRegex;
        // Rules are now separated by type for precedence
        QVector<HighlightingRule> keywordRules;
        QVector<HighlightingRule> functionRules;
        QVector<HighlightingRule> singleLineCommentRules;
        QVector<HighlightingRule> quotationRules;
        
        QRegularExpression multiLineCommentStart;
        QRegularExpression multiLineCommentEnd;
        QTextCharFormat multiLineCommentFormat;
        QStringList keywords;
    };

    QHash<QString, Language> languageByExtension;
    Language* currentLanguage = nullptr;

    void loadLanguage(const QString& path);
    void applyRules(const QString& text, const QVector<HighlightingRule>& rules);
};

#endif // HIGHLIGHTER_H