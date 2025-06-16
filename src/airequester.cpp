#include "airequester.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QEventLoop>
#include <QDebug>

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
AIRequester::AIRequester(QObject* parent) : QObject(parent) {}

QString AIRequester::askChatGPT(const QString& prompt)
{
    if (m_chatGPTKey.isEmpty()) {
        qDebug() << "ChatGPT API key is not set.";
        return {};
    }

    QUrl url("https://api.openai.com/v1/chat/completions");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QByteArray("Bearer ") + m_chatGPTKey.toUtf8());

    QJsonObject userMsg{{"role", "user"}, {"content", prompt}};
    QJsonArray  messages{userMsg};
    QJsonObject body{
        {"model", "gpt-3.5-turbo"},
        {"messages", messages},
        {"max_tokens", 256},
        {"temperature", 0.7}         
    };


    QNetworkReply* reply = m_manager.post(req, QJsonDocument(body).toJson());
    QEventLoop    loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();


    QByteArray data = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "OpenAI error:" << reply->errorString()
                 << "HTTP status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                 << "Body:" << data;
        reply->deleteLater();
        return QString("API Error: %1").arg(reply->errorString());
    }
    reply->deleteLater();


    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON from OpenAI:" << data;
        return {};
    }
    QJsonArray  choices = doc.object().value("choices").toArray();
    if (choices.isEmpty()) {
        qDebug() << "No choices in response:" << data;
        return {};
    }
    QJsonObject message = choices.first().toObject()
                              .value("message").toObject();
    return message.value("content").toString().trimmed();
}



QString AIRequester::askGemini(const QString& prompt)
{
    if (m_geminiKey.isEmpty()) {
        qDebug() << "Gemini API key is not set.";
        return {};
    }

    //v1beta REST endpoint for Gemini 2.0 Flash
    QUrl url(
        QString("https://generativelanguage.googleapis.com/v1beta/"
                "models/gemini-2.0-flash:generateContent?key=%1")
        .arg(m_geminiKey)
    );
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");


    QJsonObject part{{"text", prompt}};
    QJsonArray  parts;    parts.append(part);
    QJsonObject content{{"parts", parts}};
    QJsonArray  contents; contents.append(content);

    QJsonObject body;
    body["contents"] = contents;

    QJsonObject genCfg;
    genCfg["temperature"]       = 0.7;   //randomness
    genCfg["topP"]              = 0.95;  //nucleus sampling
    genCfg["topK"]              = 40;    //top-k sampling
    genCfg["candidateCount"]    = 1;     //how many completions
    genCfg["maxOutputTokens"]   = 512;   //length cap
    body["generation_config"] = genCfg;

    QNetworkReply* reply = m_manager.post(req, QJsonDocument(body).toJson());
    QEventLoop    loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray data = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Gemini error:" << reply->errorString()
                 << "HTTP status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                 << "Body:" << data;
        reply->deleteLater();
        return QString("API Error: %1").arg(reply->errorString());
    }
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON from Gemini:" << data;
        return {};
    }
    QJsonArray  cands      = doc.object().value("candidates").toArray();
    if (cands.isEmpty()) {
        qDebug() << "No candidates in Gemini response:" << data;
        return {};
    }
    QJsonObject firstCand  = cands.first().toObject();
    QJsonObject contentObj = firstCand.value("content").toObject();
    QJsonArray  outParts   = contentObj.value("parts").toArray();
    if (outParts.isEmpty()) {
        qDebug() << "No parts in Gemini candidate:" << data;
        return {};
    }

    return outParts.first().toObject()
                   .value("text").toString().trimmed();
}

//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023