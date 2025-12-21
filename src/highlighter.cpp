#include "highlighter.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

Highlighter::Highlighter(const QStringList& languageFiles, QObject *parent)
    : QSyntaxHighlighter(parent)
{
    for (const QString& file : languageFiles) {
        loadLanguage(file);
    }
}

void Highlighter::loadLanguage(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Highlighter Error: Could not open language file:" << path;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "Could not parse XML from file:" << path;
        file.close();
        return;
    }
    file.close();

    QDomElement rootElem = doc.documentElement();
    for(QDomElement langElement = rootElem.firstChildElement("language");
        !langElement.isNull();
        langElement = langElement.nextSiblingElement("language"))
    {
        Language lang;
        lang.extensionRegex = langElement.attribute("extentions");

    // Define Text Formats
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);

    QTextCharFormat paramFormat;
    paramFormat.setForeground(QColor("#008080"));
    paramFormat.setFontWeight(QFont::DemiBold);

    QTextCharFormat extraParamFormat;
    extraParamFormat.setForeground(Qt::darkMagenta);

    QTextCharFormat matParamFormat;
    matParamFormat.setForeground(QColor("#228B22"));

    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGreen);

    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(QColor(Qt::green).darker(150));
    
    QTextCharFormat functionFormat;
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(QColor("#9B227B"));

    // Function Rule
    HighlightingRule funcRule;
    funcRule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    funcRule.format = functionFormat;
    lang.functionRules.append(funcRule);

    // Keyword Rules
    QDomElement keywordsElement = langElement.firstChildElement("keywords");
    for (QDomElement elem = keywordsElement.firstChildElement(); !elem.isNull(); elem = elem.nextSiblingElement()) {
        QString tagName = elem.tagName();
        QString text = elem.text();
        lang.keywords.append(text);

        HighlightingRule rule;
        rule.pattern = QRegularExpression(tagName == "extraparam" ? text : "\\b" + text + "\\b");

        if (tagName == "keyword") rule.format = keywordFormat;
        else if (tagName == "param") rule.format = paramFormat;
        else if (tagName == "extraparam") rule.format = extraParamFormat;
        else if (tagName == "matparam") rule.format = matParamFormat;
        else continue;

        lang.keywordRules.append(rule);
    }
    
    // Single-Line Comment Rules
    QDomElement singleComment = langElement.firstChildElement("single_comment");
    if (!singleComment.isNull()) {
        HighlightingRule r;
        r.pattern = QRegularExpression(singleComment.attribute("expression"));
        r.format = singleLineCommentFormat;
        lang.singleLineCommentRules.append(r);
    }
    QDomElement singleComment2 = langElement.firstChildElement("single_comment2");
    if (!singleComment2.isNull()) {
        HighlightingRule r;
        r.pattern = QRegularExpression(singleComment2.attribute("expression2"));
        r.format = singleLineCommentFormat;
        lang.singleLineCommentRules.append(r);
    }

    // Quotation Rule (non-greedy)
    HighlightingRule quoteRule;
    quoteRule.pattern = QRegularExpression("\".*?\"");
    quoteRule.format = quotationFormat;
    lang.quotationRules.append(quoteRule);

    // Multi-line Comment Rules
    QDomElement multiComment = langElement.firstChildElement("multiline_comment");
    if (!multiComment.isNull()) {
        lang.multiLineCommentStart = QRegularExpression(multiComment.attribute("begin"));
        lang.multiLineCommentEnd = QRegularExpression(multiComment.attribute("end"));
        lang.multiLineCommentFormat.setForeground(QColor(Qt::green).darker(150));
    }

    // Store the processed language definition
    QString cleanExts = lang.extensionRegex;
    cleanExts.remove('(').remove(')');
    QStringList exts = cleanExts.split('|');
    for(const QString& ext : exts) {
        QString trimmedExt = ext.trimmed().toLower();
        if (!trimmedExt.isEmpty()) {
            languageByExtension.insert(trimmedExt, lang);
        }
    }
    }
}


bool Highlighter::setExtension(const QString &fileExtension)
{
    QString key = fileExtension.toLower();
    currentLanguage = languageByExtension.contains(key) ? &languageByExtension[key] : nullptr;
    return currentLanguage != nullptr;
}

QStringList Highlighter::getKeywordsForExtension(const QString& fileExtension)
{
    return languageByExtension.value(fileExtension.toLower()).keywords;
}

// Helper function to apply a set of rules
void Highlighter::applyRules(const QString& text, const QVector<HighlightingRule>& rules)
{
    for (const HighlightingRule &rule : rules) {
        QRegularExpressionMatchIterator i = rule.pattern.globalMatch(text);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

void Highlighter::highlightBlock(const QString& text) {
    if (!currentLanguage) {
        return;
    }

    // Apply rules in a specific order of precedence
    applyRules(text, currentLanguage->functionRules);
    applyRules(text, currentLanguage->keywordRules);
    applyRules(text, currentLanguage->quotationRules);
    applyRules(text, currentLanguage->singleLineCommentRules);


    const bool hasMultiline =
        currentLanguage->multiLineCommentStart.isValid() &&
        currentLanguage->multiLineCommentEnd.isValid() &&
        !currentLanguage->multiLineCommentStart.pattern().isEmpty() &&
        !currentLanguage->multiLineCommentEnd.pattern().isEmpty();

    if (!hasMultiline) {
        setCurrentBlockState(0);
        return;
    }


    // Handle multi-line comments last
    setCurrentBlockState(0);

    //int startIndex = (previousBlockState() != 1) ? currentLanguage->multiLineCommentStart.match(text).capturedStart() : 0;

    int startIndex = (previousBlockState() != 1)
                         ? currentLanguage->multiLineCommentStart.match(text).capturedStart()
                         : 0;

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = currentLanguage->multiLineCommentEnd.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, currentLanguage->multiLineCommentFormat);
        startIndex = currentLanguage->multiLineCommentStart.match(text, startIndex + commentLength).capturedStart();
    }
}
