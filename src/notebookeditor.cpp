#include "notebookeditor.h"
#include <QBuffer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QTextCursor>
#include <QUrl>
#include <QDateTime>
#include <QTextBlock>
// Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
NotebookEditor::NotebookEditor(QWidget* parent) : QTextEdit(parent) {
    setAcceptDrops(true);
    setAcceptRichText(true);
}

void NotebookEditor::insertImage(const QImage& img) {
    QImage scaled = img.width() > 800 ? img.scaledToWidth(800, Qt::SmoothTransformation) : img;

    //image
    QString imgName = QString("img_%1.png").arg(QDateTime::currentMSecsSinceEpoch());
    document()->addResource(QTextDocument::ImageResource, QUrl(imgName), scaled);

    QTextCursor cursor = textCursor();
    QTextImageFormat imgFmt;
    imgFmt.setName(imgName);
    imgFmt.setWidth(scaled.width());
    imgFmt.setHeight(scaled.height());
    cursor.insertImage(imgFmt);
    cursor.insertText("\n");
}

bool NotebookEditor::canInsertFromMimeData(const QMimeData* source) const {
    return source->hasImage() || source->hasUrls() || QTextEdit::canInsertFromMimeData(source);
}

void NotebookEditor::insertFromMimeData(const QMimeData* source) {
    if (source->hasImage()) {
        insertImage(qvariant_cast<QImage>(source->imageData()));
    } else if (source->hasUrls()) {
        foreach (QUrl url, source->urls()) {
            QImage img(url.toLocalFile());
            if (!img.isNull()) insertImage(img);
        }
    } else {
        QTextEdit::insertFromMimeData(source);
    }
}

void NotebookEditor::mouseDoubleClickEvent(QMouseEvent *event) {
    QTextCursor cursor = cursorForPosition(event->pos());
    QTextCursor checkCursor = cursor;

    //char under cursor
    checkCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    if (!checkCursor.charFormat().isImageFormat()) {
        checkCursor = cursor;
        checkCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);
    }

    if (checkCursor.charFormat().isImageFormat()) {
        QTextImageFormat imgFmt = checkCursor.charFormat().toImageFormat();
        bool ok;
        int newWidth = QInputDialog::getInt(this, "Resize Image", "New Width (px):", imgFmt.width(), 10, 4000, 10, &ok);
        if (ok && newWidth > 0) {
            double ratio = (double)imgFmt.height() / (double)imgFmt.width();
            imgFmt.setWidth(newWidth);
            imgFmt.setHeight(newWidth * ratio);
            checkCursor.setCharFormat(imgFmt);

            //redraw image
            document()->markContentsDirty(checkCursor.position(), checkCursor.selectedText().length());
            document()->setModified(true);
        }
        event->accept();
        return;
    }
    QTextEdit::mouseDoubleClickEvent(event);
}

//uncompressed robust JSON writing mapping resources back to the HTML
bool NotebookEditor::saveKmkDoc(const QString& path) {
    QJsonObject docObj;
    docObj["version"] = "1.0";
    docObj["html_content"] = toHtml();

    QJsonObject imagesObj;
    for (QTextBlock block = document()->begin(); block.isValid(); block = block.next()) {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.isValid() && fragment.charFormat().isImageFormat()) {
                QString imgName = fragment.charFormat().toImageFormat().name();
                QVariant res = document()->resource(QTextDocument::ImageResource, QUrl(imgName));
                QImage img = qvariant_cast<QImage>(res);

                QByteArray ba;
                QBuffer buffer(&ba);
                buffer.open(QIODevice::WriteOnly);
                img.save(&buffer, "PNG");
                imagesObj[imgName] = QString::fromLatin1(ba.toBase64());
            }
        }
    }
    docObj["images"] = imagesObj;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(QJsonDocument(docObj).toJson(QJsonDocument::Indented));
        file.close();
        document()->setModified(false);
        return true;
    }
    return false;
}

bool NotebookEditor::loadKmkDoc(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isNull() && doc.isObject()) {
        QJsonObject docObj = doc.object();
        QJsonObject imagesObj = docObj["images"].toObject();

        //push images back into memory before generating HTML
        for (auto it = imagesObj.begin(); it != imagesObj.end(); ++it) {
            QByteArray ba = QByteArray::fromBase64(it.value().toString().toLatin1());
            QImage img;
            img.loadFromData(ba, "PNG");
            document()->addResource(QTextDocument::ImageResource, QUrl(it.key()), img);
        }
        setHtml(docObj["html_content"].toString());
        document()->setModified(false);
        return true;
    }
    return false;
}
// Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023