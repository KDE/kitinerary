/*
    SPDX-FileCopyrightText: 2018-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "token.h"

#include <QStringView>
#include <QVariant>

using namespace KItinerary;

Token::TokenType Token::tokenType(QStringView token)
{
    if (token.startsWith(QLatin1String("qrcode:"), Qt::CaseInsensitive)) {
        return QRCode;
    } else if (token.startsWith(QLatin1String("aztec"), Qt::CaseInsensitive)) {
        return AztecCode;
    } else if (token.startsWith(QLatin1String("barcode128:"), Qt::CaseInsensitive)) {
        return Code128;
    } else if (token.startsWith(QLatin1String("datamatrix:"), Qt::CaseInsensitive)) {
        return DataMatrix;
    } else if (token.startsWith(QLatin1String("pdf417"), Qt::CaseInsensitive)) {
        return PDF417;
    } else if (token.startsWith(QLatin1String("code39:"), Qt::CaseInsensitive)) {
        return Code39;
    } else if (token.startsWith(QLatin1String("http"), Qt::CaseInsensitive)) {
        return Url;
    }
    return Unknown;
}

QVariant Token::tokenData(const QString &token)
{
    if (token.startsWith(QLatin1String("qrcode:"), Qt::CaseInsensitive)) {
        return token.mid(7);
    } else if (token.startsWith(QLatin1String("azteccode:"), Qt::CaseInsensitive)) {
        return token.mid(10);
    } else if (token.startsWith(QLatin1String("aztecbin:"), Qt::CaseInsensitive)) {
        return QByteArray::fromBase64(QStringView(token).mid(9).toLatin1());
    } else if (token.startsWith(QLatin1String("barcode128:"), Qt::CaseInsensitive)) {
        return token.mid(11);
    } else if (token.startsWith(QLatin1String("datamatrix:"), Qt::CaseInsensitive)) {
        return token.mid(11);
    } else if (token.startsWith(QLatin1String("pdf417:"), Qt::CaseInsensitive)) {
        return token.mid(7);
    } else if (token.startsWith(QLatin1String("pdf417bin:"), Qt::CaseInsensitive)) {
        return QByteArray::fromBase64(QStringView(token).mid(10).toLatin1());
    } else if (token.startsWith(QLatin1String("code39:"), Qt::CaseInsensitive)) {
        return token.mid(7);
    }

    // Qt 6 only does shallow isNull checks, so make sure empty strings produce empty variants
    return token.isEmpty() ? QVariant() : QVariant(token);
}
