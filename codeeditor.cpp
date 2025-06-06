#include "codeeditor.h"

#include "linenumberarea.h"
#include <QPainter>
#include <QTextBlock>
#include <QFontDialog>
#include <QTextDocumentFragment>
#include <QPalette>
#include <QStack>
#include <QFileInfo>
#include <QtDebug>
#include <QRegularExpression>

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::handleTextChanged);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &CodeEditor::handleCursorPositionChanged);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    handleTextChanged();
    handleCursorPositionChanged();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 5 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::cyan).lighter(170);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(Qt::yellow).lighter(170));
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::red);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

bool CodeEditor::find(QString query, bool caseSensitive, bool wholeWords)
{
    int cursorPositionBeforeCurrentSearch = textCursor().position();

    QTextDocument::FindFlags searchOptions = getSearchOptionsFromFlags(caseSensitive, wholeWords);

    bool matchFound = QPlainTextEdit::find(query, searchOptions);

    if (!matchFound)
    {
        moveCursor(QTextCursor::Start);
        matchFound = QPlainTextEdit::find(query, searchOptions);
    }
    if (matchFound)
    {
       // int foundPosition = textCursor().position();

    }
    else
    {
        moveCursorTo(cursorPositionBeforeCurrentSearch);

      //  emit(findResultReady("No keyword found."));
    }

    return matchFound;
}


void CodeEditor::replace(QString what, QString with, bool caseSensitive, bool wholeWords)
{
    bool found = find(what, caseSensitive, wholeWords);

    if (found)
    {
        QTextCursor cursor = textCursor();
        cursor.beginEditBlock();
        cursor.insertText(with);
        cursor.endEditBlock();
    }
}

void CodeEditor::replaceAll(QString what, QString with, bool caseSensitive, bool wholeWords)
{
    disconnect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
    disconnect(this, SIGNAL(textChanged()), this, SLOT(on_textChanged()));

    moveCursorTo(0);

    QTextDocument::FindFlags searchOptions = getSearchOptionsFromFlags(caseSensitive, wholeWords);
    bool found = QPlainTextEdit::find(what, searchOptions);
    int replacements = 0;

    QTextCursor cursor(document());
    cursor.beginEditBlock();
    while (found)
    {
        QTextCursor currentPosition = textCursor();
        currentPosition.insertText(with);
        replacements++;
        found = QPlainTextEdit::find(what, searchOptions);
    }
    cursor.endEditBlock();

    if (replacements == 0)
    {
     //  emit(findResultReady("No keyword found."));
    }
    else
    {
        emit(findResultReady("Input file searched. Replaced " + QString::number(replacements) + " keywords."));
    }

    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
    connect(this, SIGNAL(textChanged()), this, SLOT(on_textChanged()));
}


void CodeEditor::moveCursorTo(int positionInText)
{
    QTextCursor newCursor = textCursor();
    newCursor.setPosition(positionInText);
    setTextCursor(newCursor);
}

QTextDocument::FindFlags CodeEditor::getSearchOptionsFromFlags(bool caseSensitive, bool wholeWords)
{
    QTextDocument::FindFlags searchOptions = QTextDocument::FindFlags();
    if (caseSensitive)
    {
        searchOptions |= QTextDocument::FindCaseSensitively;
    }
    if (wholeWords)
    {
        searchOptions |= QTextDocument::FindWholeWords;
    }
    return searchOptions;
}

void CodeEditor::handleTextChanged()
{
    QString text = toPlainText();
    metrics.wordCount = text.split(QRegularExpression("\\b\\w+\\b"), Qt::SkipEmptyParts).size();
    metrics.charCount = text.length();
    metrics.totalLines = blockCount();
    emit wordCountChanged(metrics.wordCount);
    emit charCountChanged(metrics.charCount);
    emit lineChanged(metrics.currentLine, metrics.totalLines);
}

void CodeEditor::handleCursorPositionChanged()
{
    QTextCursor c = textCursor();
    metrics.currentLine = c.blockNumber() + 1;
    metrics.currentColumn = c.position() - c.block().position() + 1;
    metrics.totalLines = blockCount();
    emit lineChanged(metrics.currentLine, metrics.totalLines);
    emit columnChanged(metrics.currentColumn);
}
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
