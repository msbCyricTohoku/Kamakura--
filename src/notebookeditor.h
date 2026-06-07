#ifndef NOTEBOOKEDITOR_H
#define NOTEBOOKEDITOR_H

#include <QTextEdit>
#include <QMimeData>
#include <QImage>
#include <QMouseEvent>
#include <QInputDialog>

class NotebookEditor : public QTextEdit {
    Q_OBJECT
public:
    explicit NotebookEditor(QWidget* parent = nullptr);
    bool saveKmkDoc(const QString& filePath);
    bool loadKmkDoc(const QString& filePath);

protected:
    bool canInsertFromMimeData(const QMimeData* source) const override;
    void insertFromMimeData(const QMimeData* source) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void insertImage(const QImage& image);
};

#endif // NOTEBOOKEDITOR_H