/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kitinerary_version.h>

#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorResult>
#include <KItinerary/HttpResponse>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <iostream>

using namespace KItinerary;

static void harPostRequest(const QNetworkRequest &req, const QByteArray &postData, QJsonObject &harEntry)
{
    QJsonArray headers;
    const auto rawHeaders = req.rawHeaderList();
    for (const auto &h : rawHeaders) {
        headers.push_back(QJsonObject({
            {QLatin1String("name"), QString::fromUtf8(h)},
            {QLatin1String("value"), QString::fromUtf8(req.rawHeader(h))},
        }));
    }

    harEntry.insert(QLatin1String("request"), QJsonObject({
        {QLatin1String("method"), QLatin1String("POST")},
        {QLatin1String("url"), req.url().toString()},
        {QLatin1String("headers"), headers},
        {QLatin1String("postData"), QJsonObject({
            {QLatin1String("text"), QString::fromUtf8(postData)},
        })},
    }));
}

static void harResponse(const HttpResponse &response, QJsonObject &harEntry)
{
    harEntry.insert(QLatin1String("response"), QJsonObject({
        {QLatin1String("content"), QJsonObject({
            {QLatin1String("text"), QString::fromUtf8(response.content().toBase64())},
            {QLatin1String("encoding"), QLatin1String("base64")},
        })}
    }));
}

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("online-ticket-dump"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KITINERARY_VERSION_STRING));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Dump online ticket data."));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption nameOpt(QStringLiteral("name"), QStringLiteral("Passenger last name."), QStringLiteral("name"));
    parser.addOption(nameOpt);
    QCommandLineOption refOpt(QStringLiteral("ref"), QStringLiteral("Ticket reference number."), QStringLiteral("ref"));
    parser.addOption(refOpt);
    QCommandLineOption sourceOpt(QStringLiteral("source"), QStringLiteral("Ticket provider (db or sncf)."), QStringLiteral("provider"));
    parser.addOption(sourceOpt);
    QCommandLineOption harOpt(QStringLiteral("har"), QStringLiteral("File to write HTTP communication to."), QStringLiteral("file"));
    parser.addOption(harOpt);
    parser.process(app);

    if (!parser.isSet(nameOpt) || !parser.isSet(refOpt) || !parser.isSet(sourceOpt)) {
        parser.showHelp(1);
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = nullptr;

    QJsonObject harEntry{ {QLatin1String("startDateTime"), QDateTime::currentDateTime().toString(Qt::ISODateWithMs)} };
    if (parser.value(sourceOpt) == QLatin1String("db")) {
        QNetworkRequest req(QUrl(QStringLiteral("https://fahrkarten.bahn.de/mobile/dbc/xs.go?")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/x-www-form-urlencoded"));
        QByteArray postData("<rqorderdetails version=\"1.0\"><rqheader v=\"19120000\" os=\"KCI\" app=\"KCI-Webservice\"/><rqorder on=\"" + parser.value(refOpt).toUtf8() + "\"/><authname tln=\"" + parser.value(nameOpt).toUtf8() + "\"/></rqorderdetails>");
        reply = nam.post(req, postData);
        harPostRequest(req, postData, harEntry);
    } else if (parser.value(sourceOpt) == QLatin1String("sncf")) {
        // based on https://www.sncf-connect.com/app/trips/search and stripped to the bare minimum that works
        QNetworkRequest req(QUrl(QStringLiteral("https://www.sncf-connect.com/bff/api/v1/trips/trips-by-criteria")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/json"));
        req.setHeader(QNetworkRequest::UserAgentHeader, QByteArray("Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/111.0"));
        req.setRawHeader("Accept", "application/json, text/plain, */*");
        req.setRawHeader("x-bff-key", "ah1MPO-izehIHD-QZZ9y88n-kku876");
        QByteArray postData("{\"reference\":\"" + parser.value(refOpt).toUtf8() + "\",\"name\":\"" + parser.value(nameOpt).toUtf8() + "\"}");
        reply = nam.post(req, postData);
        harPostRequest(req, postData, harEntry);
    }

    if (!reply) {
        parser.showHelp(1);
    }

    QObject::connect(reply, &QNetworkReply::finished, &app, [&app, &parser, &harOpt, &harEntry, reply]() {
        reply->deleteLater();
        const auto response = HttpResponse::fromNetworkReply(reply);
        ExtractorEngine engine;
        engine.setContent(response, u"internal/http-response");
        qDebug().noquote() << QJsonDocument(engine.extract()).toJson();

        if (const auto harPath = parser.value(harOpt); !harPath.isEmpty()) {
            QFile f(harPath);
            if (!f.open(QFile::WriteOnly)) {
                qWarning() << f.errorString();
            }

            harResponse(response, harEntry);
            QJsonObject har({
                {QLatin1String("log"), QJsonObject({
                    {QLatin1String("entries"), QJsonArray({harEntry})}
                })},
            });
            f.write(QJsonDocument(har).toJson());
        }

        app.quit();
    });

    return app.exec();
}