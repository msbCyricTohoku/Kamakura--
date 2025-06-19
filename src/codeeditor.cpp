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
#include <QString>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QDateTime>



//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);
    aiRequester = new AIRequester(this);

    lineNumberArea->setVisible(showLineNumbers);
    updateLineNumberAreaWidth(0);

    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    
    wordWrapEnabled = true;

    applyDarkTheme();

    //applyLightTheme();

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
    //setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    if (showLineNumbers)
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    else
        setViewportMargins(0, 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (!showLineNumbers)
        return;

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
    //lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));

    if (showLineNumbers)
        lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    else
        lineNumberArea->setGeometry(QRect());
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Tab && textCursor().hasSelection()) {
        bool unindent = event->modifiers() & Qt::ShiftModifier;
        indentSelection(unindent);
        return;
    }

    if (!event->text().isEmpty() && !textCursor().hasSelection()) {
        QChar ch = event->text().at(0);
        QChar closing;
        if (ch == '(') closing = ')';
        else if (ch == '[') closing = ']';
        else if (ch == '{') closing = '}';
        else if (ch == '"') closing = '"';
        else if (ch == '\'') closing = '\'';
        if (!closing.isNull()) {
            QPlainTextEdit::keyPressEvent(event);
            insertPlainText(QString(closing));
            QTextCursor c = textCursor();
            c.movePosition(QTextCursor::Left);
            setTextCursor(c);
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    if (textCursor().hasSelection()) {
        QMenu *aiMenu = menu->addMenu(tr("Ask AI"));
        QAction *chatAct = aiMenu->addAction(tr("Ask ChatGPT"));
        QAction *geminiAct = aiMenu->addAction(tr("Ask Gemini"));
        connect(chatAct, &QAction::triggered, this, &CodeEditor::handleAskChatGPT);
        connect(geminiAct, &QAction::triggered, this, &CodeEditor::handleAskGemini);
    }
    menu->exec(event->globalPos());
    delete menu;
}


void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(lineHighlightColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
        highlightMatchingBracket(extraSelections);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::highlightMatchingBracket(QList<QTextEdit::ExtraSelection>& selections)
{
    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    int docLen = document()->characterCount();

    auto addSelection = [&](int atPos) {
        QTextEdit::ExtraSelection sel;
        QTextCursor c(document());
        c.setPosition(atPos);
        c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        sel.cursor = c;
        sel.format.setBackground(bracketMatchColor);
        sel.format.setForeground(Qt::black);
        selections.append(sel);
    };

    auto findForward = [&](QChar openC, QChar closeC, int start){
        int depth = 1;
        for(int i=start; i<docLen; ++i){
            QChar ch = document()->characterAt(i);
            if(ch==openC) depth++;
            else if(ch==closeC) depth--;
            if(depth==0) return i;
        }
        return -1;
    };

    auto findBackward = [&](QChar openC, QChar closeC, int start){
        int depth = 1;
        for(int i=start; i>=0; --i){
            QChar ch = document()->characterAt(i);
            if(ch==closeC) depth++;
            else if(ch==openC) depth--;
            if(depth==0) return i;
        }
        return -1;
    };

    QChar before = pos > 0 ? document()->characterAt(pos-1) : QChar();
    QChar after  = pos < docLen ? document()->characterAt(pos) : QChar();

    int openPos = -1;
    int closePos = -1;

    auto matchPair = [&](QChar openC){
        if(openC=='(') return QChar(')');
        if(openC=='[') return QChar(']');
        if(openC=='{') return QChar('}');
        return QChar();
    };

    if(QString("([{" ).contains(before)) {
        openPos = pos-1;
        QChar closeC = matchPair(before);
        closePos = findForward(before, closeC, pos);
    } else if(QString("([{" ).contains(after)) {
        openPos = pos;
        QChar closeC = matchPair(after);
        closePos = findForward(after, closeC, pos+1);
    } else if(QString(")]}" ).contains(before)) {
        closePos = pos-1;
        QChar openC = matchPair(before==')'?'(':(before==']'?'[':'{'));
        openPos = findBackward(openC, before, pos-2);
    } else if(QString(")]}" ).contains(after)) {
        closePos = pos;
        QChar openC = matchPair(after==')'?'(':(after==']'?'[':'{'));
        openPos = findBackward(openC, after, pos-1);
    }

    if(openPos!=-1 && closePos!=-1) {
        addSelection(openPos);
        addSelection(closePos);
    }
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), lineNumberAreaBgColor);
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(lineNumberAreaTextColor);
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

bool CodeEditor::findBackward(QString query, bool caseSensitive, bool wholeWords)
{
    int cursorPositionBeforeCurrentSearch = textCursor().position();

    QTextDocument::FindFlags searchOptions = getSearchOptionsFromFlags(caseSensitive, wholeWords);
    searchOptions |= QTextDocument::FindBackward;

    bool matchFound = QPlainTextEdit::find(query, searchOptions);

    if (!matchFound)
    {
        moveCursor(QTextCursor::End);
        matchFound = QPlainTextEdit::find(query, searchOptions);
    }
    if (!matchFound)
    {
        moveCursorTo(cursorPositionBeforeCurrentSearch);
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

void CodeEditor::updateMetrics()
{
    handleTextChanged();
    handleCursorPositionChanged();
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

void CodeEditor::applyLightTheme()
{
    setStyleSheet("background-color:#ffffff;color:#000000;");
    lineHighlightColor = QColor("#e0e0e0");
    lineNumberAreaBgColor = QColor("#f0f0f0");
    lineNumberAreaTextColor = QColor("#555555");
    bracketMatchColor = QColor("#ffd966");
    highlightCurrentLine();
    //lineNumberArea->update();
    if (showLineNumbers)
        lineNumberArea->update();
}

void CodeEditor::applyDarkTheme()
{
    setStyleSheet("background-color:#2b2b2b;color:#ffffff;");
    lineHighlightColor = QColor("#333333");
    lineNumberAreaBgColor = QColor("#444444");
    lineNumberAreaTextColor = QColor("#aaaaaa");
    bracketMatchColor = QColor("#806000");
    highlightCurrentLine();
    //lineNumberArea->update();
    if (showLineNumbers)
        lineNumberArea->update();
}

void CodeEditor::applySolarizedLightTheme()
{
    setStyleSheet("background-color:#fdf6e3;color:#657b83;");
    lineHighlightColor = QColor("#eee8d5");
    lineNumberAreaBgColor = QColor("#eee8d5");
    lineNumberAreaTextColor = QColor("#586e75");
    bracketMatchColor = QColor("#b58900");
    highlightCurrentLine();
    //lineNumberArea->update();
    if (showLineNumbers)
        lineNumberArea->update();
}

void CodeEditor::applySolarizedDarkTheme()
{
    setStyleSheet("background-color:#002b36;color:#839496;");
    lineHighlightColor = QColor("#073642");
    lineNumberAreaBgColor = QColor("#073642");
    lineNumberAreaTextColor = QColor("#586e75");
    bracketMatchColor = QColor("#b58900");
    highlightCurrentLine();
    //lineNumberArea->update();
    if (showLineNumbers)
        lineNumberArea->update();
}

void CodeEditor::setWordWrap(bool enable)
{
    wordWrapEnabled = enable;
    setLineWrapMode(enable ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}


void CodeEditor::setLineNumbersVisible(bool visible)
{
    showLineNumbers = visible;
    lineNumberArea->setVisible(visible);
    updateLineNumberAreaWidth(0);
    update();
}


void CodeEditor::indentSelection(bool unindent)
{
    QTextCursor cursor = textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();
    QTextBlock startBlock = document()->findBlock(start);
    QTextBlock endBlock = document()->findBlock(end - 1);

    cursor.beginEditBlock();
    for (QTextBlock block = startBlock; ; block = block.next()) {
        QTextCursor lineCursor(block);
        lineCursor.movePosition(QTextCursor::StartOfBlock);
        if (unindent) {
            QString text = block.text();
            int removeCount = 0;
            for (int i = 0; i < NUM_CHARS_FOR_TAB && i < text.length(); ++i) {
                if (text.at(i) == QLatin1Char(' '))
                    removeCount++;
                else
                    break;
            }
            if (removeCount > 0) {
                lineCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, removeCount);
                lineCursor.removeSelectedText();
            }
        } else {
            lineCursor.insertText(QString(NUM_CHARS_FOR_TAB, QLatin1Char(' ')));
        }
        if (block == endBlock)
            break;
    }
    cursor.endEditBlock();

    QTextCursor newCursor(document());
    newCursor.setPosition(startBlock.position());
    QTextBlock afterEnd = endBlock.next();
    int endPos = afterEnd.isValid() ? afterEnd.position() - 1 : document()->characterCount() - 1;
    newCursor.setPosition(endPos, QTextCursor::KeepAnchor);
    setTextCursor(newCursor);
}


void CodeEditor::handleAskChatGPT()
{
    QString selected = textCursor().selectedText();
    if (selected.isEmpty())
        return;

    if (aiRequester->chatGPTKey().isEmpty()) {
        bool ok = false;
        QString key = QInputDialog::getText(this, tr("ChatGPT API Key"),
                                            tr("Enter your OpenAI API key"), QLineEdit::Normal,
                                            QString(), &ok);
        if (!ok || key.isEmpty())
            return;
        aiRequester->setChatGPTKey(key);
    }

    QString response = aiRequester->askChatGPT(selected);
    if (!response.isEmpty()) {
        QTextCursor c = textCursor();
        c.setPosition(c.selectionEnd());
        c.insertText("\n" + response + "\n");
    }
}

void CodeEditor::handleAskGemini()
{
    QString selected = textCursor().selectedText();
    if (selected.isEmpty())
        return;

    if (aiRequester->geminiKey().isEmpty()) {
        bool ok = false;
        QString key = QInputDialog::getText(this, tr("Gemini API Key"),
                                            tr("Enter your Google Gemini API key"), QLineEdit::Normal,
                                            QString(), &ok);
        if (!ok || key.isEmpty())
            return;
        aiRequester->setGeminiKey(key);
    }

    QString response = aiRequester->askGemini(selected);
    if (!response.isEmpty()) {
        QTextCursor c = textCursor();
        c.setPosition(c.selectionEnd());
        c.insertText("\n" + response + "\n");
    }
}


void CodeEditor::insertDateTime()
{
    QString dt = QDateTime::currentDateTime().toString(Qt::ISODate);
    textCursor().insertText(dt);
}


void CodeEditor::duplicateLine()
{
    QTextCursor cursor = textCursor();
    QString text;
    if (cursor.hasSelection()) {
        text = cursor.selection().toPlainText();
        cursor.setPosition(cursor.selectionEnd());
    } else {
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        text = cursor.selection().toPlainText();
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::EndOfLine);
    }
    cursor.insertBlock();
    cursor.insertText(text);
    setTextCursor(cursor);
}

void CodeEditor::toggleComment()
{
    QTextCursor cursor = textCursor();
    int start = cursor.selectionStart();
    int end = cursor.selectionEnd();

    QTextBlock startBlock = document()->findBlock(start);
    QTextBlock endBlock = document()->findBlock(end == start ? start : end - 1);

    cursor.beginEditBlock();
    for (QTextBlock block = startBlock; block.isValid(); block = block.next()) {
        QTextCursor lineCursor(block);
        QString text = block.text();

        int firstNonSpace = 0;
        while (firstNonSpace < text.length() && text.at(firstNonSpace).isSpace())
            ++firstNonSpace;

        bool commented = text.mid(firstNonSpace, commentPrefix.length()) == commentPrefix;
        lineCursor.setPosition(block.position() + firstNonSpace);
        if (commented) {
            lineCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, commentPrefix.length());
            lineCursor.removeSelectedText();
        } else {
            lineCursor.insertText(commentPrefix);
        }
        if (block == endBlock)
            break;
    }
    cursor.endEditBlock();
}

void CodeEditor::setCommentPrefix(const QString& prefix)
{
    commentPrefix = prefix;
}



//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
