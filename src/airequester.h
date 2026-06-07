#ifndef AIREQUESTER_H
#define AIREQUESTER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

class AIRequester : public QObject {
    Q_OBJECT
public:
    explicit AIRequester(QObject* parent = nullptr);

    void setChatGPTKey(const QString& key) { m_chatGPTKey = key; }
    void setGeminiKey(const QString& key) { m_geminiKey = key; }
    QString chatGPTKey() const { return m_chatGPTKey; }
    QString geminiKey() const { return m_geminiKey; }

    void askChatGPT(const QString& prompt);
    void askGemini(const QString& prompt);

signals:
    void responseReady(const QString& response);
    void errorOccurred(const QString& error);

private:
    QNetworkAccessManager m_manager;
    QString m_chatGPTKey;
    QString m_geminiKey;
};
#endif // AIREQUESTER_H