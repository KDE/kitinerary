/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentutil.h"

#include <KItinerary/JsonLdDocument>
#include <KItinerary/Reservation>

#include <QCryptographicHash>
#include <QDebug>
#include <QString>

using namespace KItinerary;

QString DocumentUtil::idForContent(const QByteArray &data)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    return QString::fromLatin1(hash.result().toHex());
}

bool DocumentUtil::addDocumentId(QVariant &res, const QString &id)
{
    if (!JsonLd::canConvert<Reservation>(res)) {
        return false;
    }

    auto l = documentIds(res);
    if (!l.contains(id)) {
        l.push_back(id);
        setDocumentIds(res, l);
        return true;
    }
    return false;
}

bool DocumentUtil::removeDocumentId(QVariant &res, const QString &id)
{
    if (!JsonLd::canConvert<Reservation>(res)) {
        return false;
    }

    auto l = documentIds(res);
    if (l.contains(id)) {
        l.removeAll(id);
        setDocumentIds(res, l);
        return true;
    }
    return false;
}

QVariantList DocumentUtil::documentIds(const QVariant &res)
{
    if (!JsonLd::canConvert<Reservation>(res)) {
        return {};
    }
    return JsonLd::convert<Reservation>(res).subjectOf();
}

void DocumentUtil::setDocumentIds(QVariant &res, const QVariantList &docIds)
{
    JsonLdDocument::writeProperty(res, "subjectOf", docIds);
}
