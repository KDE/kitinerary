/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "documentutil.h"

#include <KItinerary/JsonLdDocument>

#include <QCryptographicHash>
#include <QDebug>
#include <QString>
#include <QUrl>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

QString DocumentUtil::idForContent(const QByteArray &data)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data);
    return QString::fromLatin1(hash.result().toHex());
}

QString DocumentUtil::idForPkPass(const QString &passTypeIdentifier, const QString &serialNumber)
{
    QUrl url;
    url.setScheme("pkpass"_L1);
    url.setHost(passTypeIdentifier); // TODO can this contain problematic characters?
    url.setPath('/'_L1 + serialNumber);
    return url.toString();
}

bool DocumentUtil::addDocumentId(QVariant &obj, const QString &id)
{
    auto l = documentIds(obj);
    if (!l.contains(id) && !id.isEmpty()) {
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

QUrl DocumentUtil::pkPassId(const QVariant &obj)
{
    const auto docIds = DocumentUtil::documentIds(obj);
    for (const auto &docIdV : docIds) {
        if (docIdV.typeId() != QMetaType::QString) {
            continue;
        }
        auto docId = QUrl(docIdV.toString());
        if (docId.scheme() == "pkpass"_L1) {
            return docId;
        }
    }
    return {};
}
