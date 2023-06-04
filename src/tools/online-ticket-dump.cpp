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
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <iostream>

using namespace KItinerary;

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
    parser.process(app);

    if (!parser.isSet(nameOpt) || !parser.isSet(refOpt) || !parser.isSet(sourceOpt)) {
        parser.showHelp(1);
    }

    QNetworkAccessManager nam;
    nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = nullptr;

    if (parser.value(sourceOpt) == QLatin1String("db")) {
        QNetworkRequest req(QUrl(QStringLiteral("https://fahrkarten.bahn.de/mobile/dbc/xs.go?")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/x-www-form-urlencoded"));
        reply = nam.post(req, "<rqorderdetails version=\"1.0\"><rqheader v=\"19120000\" os=\"KCI\" app=\"KCI-Webservice\"/><rqorder on=\"" + parser.value(refOpt).toUtf8() + "\"/><authname tln=\"" + parser.value(nameOpt).toUtf8() + "\"/></rqorderdetails>");
    } else if (parser.value(sourceOpt) == QLatin1String("sncf")) {
        // based on https://www.sncf-connect.com/app/trips/search and stripped to the bare minimum that works
        QNetworkRequest req(QUrl(QStringLiteral("https://www.sncf-connect.com/bff/api/v1/trips/trips-by-criteria")));
        req.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/json"));
        req.setHeader(QNetworkRequest::UserAgentHeader, QByteArray("Mozilla/5.0 (X11; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/111.0"));
        req.setRawHeader("Accept", "application/json, text/plain, */*");
        req.setRawHeader("x-bff-key", "ah1MPO-izehIHD-QZZ9y88n-kku876");
        reply = nam.post(req, "{\"reference\":\"" + parser.value(refOpt).toUtf8() + "\",\"name\":\"" + parser.value(nameOpt).toUtf8() + "\"}");
    }

    if (!reply) {
        parser.showHelp(1);
    }

    QObject::connect(reply, &QNetworkReply::finished, &app, [&app, reply]() {
        reply->deleteLater();
        // qDebug() << reply->readAll();
        ExtractorEngine engine;
        engine.setContent(HttpResponse::fromNetworkReply(reply), u"internal/http-response");
        qDebug().noquote() << QJsonDocument(engine.extract()).toJson();
        app.quit();
    });

    return app.exec();
}
