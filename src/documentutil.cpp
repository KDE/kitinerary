/*
    Copyright (C) 2019 Volker Krause <vkrause@kde.org>

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
