/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

using namespace KItinerary;
using namespace KItinerary::Generator;

KnowledgeDb::Coordinate WikiData::parseCoordinate(const QString& value)
{
    const auto idx = value.indexOf(QLatin1Char(' '));
    bool latOk = false, longOk = false;
    KnowledgeDb::Coordinate c;
    c.longitude = value.midRef(6, idx - 6).toFloat(&latOk);
    c.latitude = value.midRef(idx + 1, value.size() - idx - 2).toFloat(&longOk);
    if (!latOk || !longOk) {
        c.longitude = NAN;
        c.latitude = NAN;
    }
    return c;
}

QJsonArray WikiData::query(const char *sparqlQuery, const char *cacheFileName)
{
    QDir().mkdir(QStringLiteral("data"));
    QFile cacheFile(QLatin1String("data/") + QString::fromUtf8(cacheFileName));
    QByteArray data;
    if (cacheFile.exists() && qEnvironmentVariableIsSet("KITINERARY_USE_WIKIDATA_CACHE")) {
        cacheFile.open(QFile::ReadOnly);
        data = cacheFile.readAll();
        cacheFile.close();
    }

    if (data.isEmpty()) {
        QUrl url(QStringLiteral("https://query.wikidata.org/sparql"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("query"), QString::fromUtf8(sparqlQuery).trimmed().simplified());
        query.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
        url.setQuery(query);

        QNetworkAccessManager nam;
        auto reply = nam.get(QNetworkRequest(url));
        QObject::connect(reply, &QNetworkReply::finished, QCoreApplication::instance(), &QCoreApplication::quit);
        QCoreApplication::exec();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << reply->errorString();
            return {};
        }

        data = reply->readAll();
        cacheFile.open(QFile::WriteOnly);
        cacheFile.write(data);
    }

    const auto doc = QJsonDocument::fromJson(data);
    const auto resultArray = doc.object().value(QLatin1String("results")).toObject().value(QLatin1String("bindings")).toArray();
    return resultArray;
}
