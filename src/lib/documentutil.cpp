/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentutil.h"

#include <KItinerary/JsonLdDocument>

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

bool DocumentUtil::addDocumentId(QVariant &obj, const QString &id)
{
    auto l = documentIds(obj);
    if (!l.contains(id)) {
        l.push_back(id);
        setDocumentIds(obj, l);
        return true;
    }
    return false;
}

bool DocumentUtil::removeDocumentId(QVariant &obj, const QString &id)
{
    auto l = documentIds(obj);
    if (l.contains(id)) {
        l.removeAll(id);
        setDocumentIds(obj, l);
        return true;
    }
    return false;
}

QVariantList DocumentUtil::documentIds(const QVariant &obj)
{
    return JsonLdDocument::readProperty(obj, "subjectOf").toList();
}

void DocumentUtil::setDocumentIds(QVariant &obj, const QVariantList &docIds)
{
    JsonLdDocument::writeProperty(obj, "subjectOf", docIds);
}
