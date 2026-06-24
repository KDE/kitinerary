/*
    SPDX-FileCopyrightText: 2018-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "token.h"

#include <QStringView>
#include <QVariant>

using namespace Qt::Literals;
using namespace KItinerary;

struct {
    QLatin1StringView name;
    Token::TokenType type;
    bool supportsBinary;
} static constexpr const token_types[] = {
    { "qrCode"_L1, Token::QRCode, true },
    { "aztecCode"_L1, Token::AztecCode, false },
    { "aztec"_L1, Token::AztecCode, true }, // technically aztec: is not valid, this only supports binary (inconcistent for historic reasons)
    { "barcode128"_L1, Token::Code128, false },
    { "datamatrix"_L1, Token::DataMatrix, false }, // ### binary support?
    { "pdf417"_L1, Token::PDF417, true },
    { "code39"_L1, Token::Code39, false },
    { "ean13"_L1, Token::EAN13, false },
    { "itf"_L1, Token::ITF, false },
    { "codabar"_L1, Token::Codabar, false },
};


Token::TokenType Token::tokenType(QStringView token)
{
    for (const auto &type : token_types) {
        if (!token.startsWith(type.name, Qt::CaseInsensitive)) {
            continue;
        }
        const auto s = token.mid(type.name.size());
        if (s.startsWith(':'_L1) || (s.startsWith("bin:"_L1, Qt::CaseInsensitive) && type.supportsBinary)) {
            return type.type;
        }
    }

    if (token.startsWith("http"_L1, Qt::CaseInsensitive)) {
      return Token::Url;
    }

    return Unknown;
}

QVariant Token::tokenData(const QString &token)
{
    for (const auto &type : token_types) {
        if (!token.startsWith(type.name, Qt::CaseInsensitive)) {
            continue;
        }
        const auto s = QStringView(token).mid(type.name.size());
        if (s.startsWith(':'_L1)) {
            return s.mid(1).toString();
        }
        if (s.startsWith("bin:"_L1, Qt::CaseInsensitive)) {
            return QByteArray::fromBase64(s.mid(4).toLatin1());
        }
    }

    // Qt 6 only does shallow isNull checks, so make sure empty strings produce empty variants
    return token.isEmpty() ? QVariant() : QVariant(token);
}

QString Token::encodeTokenData(Token::TokenType type, const QString &token)
{
    const auto it = std::ranges::find_if(token_types, [type](const auto &t) {
        return t.type == type;
    });
    if (it == std::end(token_types)) {
        return token;
    }
    return (*it).name + ':'_L1 + token;
}

#include "moc_token.cpp"
