#include "airequester.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QEventLoop>

AIRequester::AIRequester(QObject* parent) : QObject(parent) {}

QString AIRequester::askChatGPT(const QString& prompt)
{
    if (m_chatGPTKey.isEmpty())
        return QString();

    QUrl url("https://api.openai.com/v1/chat/completions");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QByteArray("Bearer ") + m_chatGPTKey.toUtf8());

    QJsonObject body;
    body["model"] = "gpt-3.5-turbo";
    QJsonArray messages;
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = prompt;
    messages.append(userMsg);
    body["messages"] = messages;
    body["max_tokens"] = 256;

    QNetworkReply* reply = m_manager.post(req, QJsonDocument(body).toJson());
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return QString();

    QJsonArray choices = doc.object().value("choices").toArray();
    if (choices.isEmpty())
        return QString();

    QJsonObject message = choices.first().toObject().value("message").toObject();
    return message.value("content").toString();
}

QString AIRequester::askGemini(const QString& prompt)
{
    if (m_geminiKey.isEmpty())
        return QString();

    QUrl url(QString("https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=%1").arg(m_geminiKey));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject part;
    part["text"] = prompt;
    QJsonArray parts;
    parts.append(part);
    QJsonObject content;
    content["parts"] = parts;
    QJsonArray contents;
    contents.append(content);
    QJsonObject body;
    body["contents"] = contents;

    QNetworkReply* reply = m_manager.post(req, QJsonDocument(body).toJson());
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return QString();

    QJsonArray candidates = doc.object().value("candidates").toArray();
    if (candidates.isEmpty())
        return QString();

    QJsonArray outParts = candidates.first().toObject().value("content").toObject().value("parts").toArray();
    if (outParts.isEmpty())
        return QString();

    return outParts.first().toObject().value("text").toString();
}
