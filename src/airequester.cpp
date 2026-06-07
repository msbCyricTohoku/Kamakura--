#include "airequester.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

AIRequester::AIRequester(QObject* parent) : QObject(parent) {}

void AIRequester::askChatGPT(const QString& prompt) {
    if (m_chatGPTKey.isEmpty()) { emit errorOccurred("ChatGPT API key is not set."); return; }

    QUrl url("https://api.openai.com/v1/chat/completions");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QByteArray("Bearer ") + m_chatGPTKey.toUtf8());

    QJsonObject body{
        {"model", "gpt-3.5-turbo"},
        {"messages", QJsonArray{QJsonObject{{"role", "user"}, {"content", prompt}}}},
        {"max_tokens", 512}, {"temperature", 0.7}
    };

    QNetworkReply* reply = m_manager.post(req, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("API Error: %1").arg(reply->errorString()));
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray choices = doc.object().value("choices").toArray();
        if (!choices.isEmpty()) {
            QString content = choices.first().toObject().value("message").toObject().value("content").toString().trimmed();
            emit responseReady(content);
        }
    });
}

void AIRequester::askGemini(const QString& prompt)
{
    if (m_geminiKey.isEmpty()) {
        emit errorOccurred("Gemini API key is not set.");
        return;
    }

    QUrl url(QString("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=%1").arg(m_geminiKey));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject part{{"text", prompt}};
    QJsonArray  parts;    parts.append(part);
    QJsonObject content{{"parts", parts}};
    QJsonArray  contents; contents.append(content);

    QJsonObject genCfg{
        {"temperature", 0.7},
        {"topP", 0.95},
        {"topK", 40},
        {"candidateCount", 1},
        {"maxOutputTokens", 512}
    };

    QJsonObject body{
        {"contents", contents},
        {"generation_config", genCfg}
    };

    QNetworkReply* reply = m_manager.post(req, QJsonDocument(body).toJson());

    //async call
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("Gemini API Error: %1").arg(reply->errorString()));
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isObject()) {
            emit errorOccurred("Invalid JSON from Gemini.");
            return;
        }

        QJsonArray cands = doc.object().value("candidates").toArray();
        if (cands.isEmpty()) {
            emit errorOccurred("No candidates in Gemini response.");
            return;
        }

        QJsonObject firstCand = cands.first().toObject();
        QJsonObject contentObj = firstCand.value("content").toObject();
        QJsonArray outParts = contentObj.value("parts").toArray();

        if (outParts.isEmpty()) {
            emit errorOccurred("No parts in Gemini candidate.");
            return;
        }

        QString responseText = outParts.first().toObject().value("text").toString().trimmed();
        emit responseReady(responseText);
    });
}

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023