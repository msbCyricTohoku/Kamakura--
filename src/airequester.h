#ifndef AIREQUESTER_H
#define AIREQUESTER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>

//simple helper to communicate with ChatGPT (OpenAI) and Google Gemini.
//users must provide their own API keys. Requests are made synchronously
//and the returned text is extracted from the JSON response.
class AIRequester : public QObject
{
    Q_OBJECT
public:
    explicit AIRequester(QObject* parent = nullptr);

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
