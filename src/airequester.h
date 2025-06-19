#ifndef AIREQUESTER_H
#define AIREQUESTER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>

class AIRequester : public QObject
{
    Q_OBJECT
public:
    explicit AIRequester(QObject* parent = nullptr);

    //setters and getters for API keys (now non-static)
    void setChatGPTKey(const QString& key) { m_chatGPTKey = key; }
    void setGeminiKey(const QString& key) { m_geminiKey = key; }
    QString chatGPTKey() const { return m_chatGPTKey; }
    QString geminiKey() const { return m_geminiKey; }

    QString askChatGPT(const QString& prompt);
    QString askGemini(const QString& prompt);

private:
    QNetworkAccessManager m_manager;
     QString m_chatGPTKey;
    QString m_geminiKey;
};

#endif // AIREQUESTER_H