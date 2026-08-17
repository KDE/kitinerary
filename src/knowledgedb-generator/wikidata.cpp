/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "wikidata.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

using namespace Qt::Literals;
using namespace KItinerary;
using namespace KItinerary::Generator;

QString WikiData::value(const QJsonObject &obj, QLatin1StringView name)
{
    return obj.value(name).toObject().value("value"_L1).toString();
}

KnowledgeDb::Coordinate WikiData::parseCoordinate(QStringView value)
{
    const auto idx = value.indexOf(QLatin1Char(' '));
    bool latOk = false, longOk = false;
    KnowledgeDb::Coordinate c;
    c.longitude = value.mid(6, idx - 6).toFloat(&latOk);
    c.latitude = value.mid(idx + 1, value.size() - idx - 2).toFloat(&longOk);
    if (!latOk || !longOk) {
        c.longitude = NAN;
        c.latitude = NAN;
    }
    return c;
}

QJsonArray WikiData::query(const char *sparqlQuery, const char *cacheFileName)
{
    return query(QString::fromUtf8(sparqlQuery), QString::fromUtf8(cacheFileName));
}

QJsonArray WikiData::query(const QString &sparqlQuery, const QString &cacheFileName)
{
    QDir().mkdir(QStringLiteral("data"));
    QFile cacheFile(QLatin1StringView("data/") + cacheFileName);
    QByteArray data;
    if (qEnvironmentVariableIsSet("KITINERARY_USE_WIKIDATA_CACHE") && cacheFile.open(QFile::ReadOnly)) {
        data = cacheFile.readAll();
        cacheFile.close();
    }

    if (data.isEmpty()) {
        // write out SPARQL next to the cache data, convenient for debugging/developing queries
        QFile sparqlFile("data/"_L1 + cacheFileName + ".sparql"_L1);
        if (sparqlFile.open(QFile::WriteOnly | QFile::Truncate)) {
            sparqlFile.write(sparqlQuery.toUtf8());
            sparqlFile.close();
        } else {
            qWarning() << sparqlFile.errorString() << sparqlFile.fileName();
        }

        QUrl url(QStringLiteral("https://query.wikidata.org/sparql"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("query"), sparqlQuery.trimmed().simplified());
        query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
        url.setQuery(query);

        QNetworkAccessManager nam;
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::UserAgentHeader, u"org.kde.kitinerary/KnowledgeDbGenerator (kde-pim@kde.org)"_s);
        req.setDecompressedSafetyCheckThreshold(100ll * 1024 * 1024);
        auto reply = nam.get(req);
        QObject::connect(reply, &QNetworkReply::finished, QCoreApplication::instance(), &QCoreApplication::quit);
        QCoreApplication::exec();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << reply->errorString();
            return {};
        }

        data = reply->readAll();
        if (cacheFile.open(QFile::WriteOnly)) {
            cacheFile.write(data);
        } else {
            qWarning() << cacheFile.errorString() << cacheFile.fileName();
        }
    }

    const auto doc = QJsonDocument::fromJson(data);
    const auto resultArray = doc.object()
                                 .value(QLatin1StringView("results"))
                                 .toObject()
                                 .value(QLatin1StringView("bindings"))
                                 .toArray();
    return resultArray;
}
