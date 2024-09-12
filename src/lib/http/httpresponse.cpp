/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "httpresponse.h"
#include "logging.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#ifdef QT_NETWORK_LIB
#include <QNetworkReply>
#endif

using namespace KItinerary;

namespace KItinerary {
class HttpResponsePrivate : public QSharedData {
public:
    QUrl url;
    QByteArray content;
    QDateTime requestDateTime;
    // expand with HTTP headers, status code, etc as necessary
};
}

HttpResponse::HttpResponse()
    : d(new HttpResponsePrivate)
{
}

HttpResponse::~HttpResponse() = default;
HttpResponse::HttpResponse(const HttpResponse&) = default;
HttpResponse& HttpResponse::operator=(const HttpResponse&) = default;

HttpResponse::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

QUrl HttpResponse::url() const
{
    return d->url;
}

QByteArray HttpResponse::content() const
{
    return d->content;
}

QDateTime HttpResponse::requestDateTime() const
{
    return d->requestDateTime;
}

HttpResponse HttpResponse::fromNetworkReply(QNetworkReply *reply)
{
    HttpResponse r;
#ifdef QT_NETWORK_LIB
    r.d->url = reply->url();
    r.d->content = reply->readAll();
    r.d->requestDateTime = QDateTime::currentDateTime();
#endif
    return r;
}

HttpResponse HttpResponse::fromHarEntry(const QJsonObject &harEntry)
{
    HttpResponse r;
    r.d->url = QUrl(harEntry.value(QLatin1StringView("request"))
                        .toObject()
                        .value(QLatin1StringView("url"))
                        .toString());
    const auto content = harEntry.value(QLatin1StringView("response"))
                             .toObject()
                             .value(QLatin1StringView("content"))
                             .toObject();

    r.d->content = content.value(QLatin1StringView("text")).toString().toUtf8();
    if (content.value(QLatin1StringView("encoding")).toString() ==
        QLatin1StringView("base64")) {
      r.d->content = QByteArray::fromBase64(r.d->content);
    }

    r.d->requestDateTime = QDateTime::fromString(
        harEntry.value(QLatin1StringView("startedDateTime")).toString(),
        Qt::ISODateWithMs);
    return r;
}

QList<HttpResponse> HttpResponse::fromHarFile(const QByteArray &harFile)
{
    QJsonParseError error;
    const auto doc = QJsonDocument::fromJson(harFile, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(Log) << error.errorString() << error.offset;
        return {};
    }

    const auto entries = QJsonDocument::fromJson(harFile)
                             .object()
                             .value(QLatin1StringView("log"))
                             .toObject()
                             .value(QLatin1StringView("entries"))
                             .toArray();
    QList<HttpResponse> result;
    result.reserve(entries.size());
    for (const auto &entry : entries) {
        result.push_back(HttpResponse::fromHarEntry(entry.toObject()));
    }
    return result;
}

#include "moc_httpresponse.cpp"
