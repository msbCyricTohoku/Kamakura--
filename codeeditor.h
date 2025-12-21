#ifndef CODEEDITOR_H
#define CODEEDITOR_H
#include <QWidget>
#include <QRect>
#include <QPainter>
#include <QTextBlock>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPlainTextEdit>
#include <QFont>
#include <QTextFormat>
#include <QTextCursor>
#include <QMessageBox>
#include <QKeyEvent>
#include <QColor>
#include <QList>
#include "finddialog.h"
#include "documentmetrics.h"
#include "airequester.h"

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);

    void applyLightTheme();
    void applyDarkTheme();
    void applySolarizedLightTheme();
    void applySolarizedDarkTheme();


     void setWordWrap(bool enable);
    bool isWordWrapEnabled() const { return wordWrapEnabled; }

    inline DocumentMetrics getDocumentMetrics() const { return metrics; }

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setLineNumbersVisible(bool visible);
    bool lineNumbersVisible() const { return showLineNumbers; }
    inline bool isUnsaved() const { return document()->isModified(); }

        const static int NUM_CHARS_FOR_TAB = 5;
         inline QString getFileName() { return getFileNameFromPath(); }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
     void keyPressEvent(QKeyEvent *event) override;

signals:
    void findResultReady(QString message);
    void gotoResultReady(QString message);
    void startFinding(QString queryText, bool caseSensitive, bool wholeWords);
    void startReplacing(QString what, QString with, bool caseSensitive, bool wholeWords);
    void startReplacingAll(QString what, QString with, bool caseSensitive, bool wholeWords);
    void interpTest(QString text);
    void wordCountChanged(int count);
    void charCountChanged(int count);
    void lineChanged(int current, int total);
    void columnChanged(int column);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void moveCursorTo(int positionInText);
    QTextDocument::FindFlags getSearchOptionsFromFlags(bool caseSensitive, bool wholeWords);
    void handleTextChanged();
    void handleCursorPositionChanged();
    void highlightMatchingBracket(QList<QTextEdit::ExtraSelection>& selections);

public slots:
    bool find(QString query, bool caseSensitive, bool wholeWords);
    bool findBackward(QString query, bool caseSensitive, bool wholeWords);
    void replace(QString what, QString with, bool caseSensitive, bool wholeWords);
    void replaceAll(QString what, QString with, bool caseSensitive, bool wholeWords);
    void updateMetrics();
    void handleAskChatGPT();
    void handleAskGemini();
    void duplicateLine();
    void toggleComment();
    void insertDateTime();
    void setCommentPrefix(const QString& prefix);
     void trimTrailingWhitespace();

private:
    QWidget *lineNumberArea;
    void indentSelection(bool unindent);
    QString indentationForNewLine(const QTextBlock& block) const;
    bool shouldIncreaseIndent(const QString& line) const;
     DocumentMetrics metrics;
     QString getFileNameFromPath();
     QColor lineHighlightColor;
     QColor lineNumberAreaBgColor;
     QColor lineNumberAreaTextColor;
     QColor bracketMatchColor;

     AIRequester* aiRequester{nullptr};

     QString commentPrefix{"#"};
    
     bool wordWrapEnabled{true};
     bool showLineNumbers{true};

};

#endif // CODEEDITOR_H
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
