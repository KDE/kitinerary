/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_HTTPRESPONSE_H
#define KITINERARY_HTTPRESPONSE_H

#include "kitinerary_export.h"

#include <QDateTime>
#include <QExplicitlySharedDataPointer>
#include <QMetaType>
#include <QUrl>

class QJsonObject;
class QNetworkReply;

namespace KItinerary {

class HttpResponsePrivate;

/**
 * Container for an HTTP response to be passed into the extractor engine.
 * This is primarily needed for extracting data from arbitrary API responses
 * or for custom website extractors, as we cannot trigger extractor scripts
 * on HTTP request or response parameters (such as the URL) otherwise.
 *
 * For operational use this can be created from a QNetworkReply, the support for
 * HAR files is mainly intended for development use.
 */
class KITINERARY_EXPORT HttpResponse
{
    Q_GADGET
    Q_PROPERTY(QUrl url READ url)
    Q_PROPERTY(QByteArray content READ content)
    Q_PROPERTY(QDateTime requestDateTime READ requestDateTime)

public:
    HttpResponse();
    ~HttpResponse();
    HttpResponse(const HttpResponse&);
    HttpResponse& operator=(const HttpResponse&);
    operator QVariant() const;

    QUrl url() const;
    QByteArray content() const;
    QDateTime requestDateTime() const;

    /** Create a HttpResponse object from an active QNetworkReply. */
    static HttpResponse fromNetworkReply(QNetworkReply *reply);
    /** Read from an HAR entry.
     *  @see https://w3c.github.io/web-performance/specs/HAR/Overview.html
     */
    Q_DECL_HIDDEN static HttpResponse fromHarEntry(const QJsonObject &harEntry);
    /** Read a set of HTTP responses from an HAR file. */
    Q_DECL_HIDDEN static QVector<HttpResponse> fromHarFile(const QByteArray &harFile);

private:
    QExplicitlySharedDataPointer<HttpResponsePrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::HttpResponse)

#endif // KITINERARY_HTTPRESPONSE_H
