/*
    SPDX-FileCopyrightText: 2018-2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "stationidentifier.h"

#include <QString>

#include <cstring>

using namespace KItinerary::KnowledgeDb;

UICIdentiferBase::UICIdentiferBase(const QString &id)
{
    const auto n = id.toUInt();
    setValue(n > 9999999 ? n / 10 : n);
}

static bool containsOnlyLetters(const QString &s)
{
    for (const auto c : s) {
        if (c < QLatin1Char('A') || c > QLatin1Char('Z')) {
            return false;
        }
    }
    return true;
}

FiveAlphaId::FiveAlphaId(const QString &id)
{
    if (id.size() != 5 || !containsOnlyLetters(id))  {
        return;
    }

    setValue(fromChars(id.toUpper().toUtf8().constData()));
}

QString FiveAlphaId::toString() const
{
    auto id = value();
    if (id == 0) {
        return {};
    }

    QString s;
    s.resize(5);
    for (int i = 0; i < 5; ++i) {
        s[4 - i] = QLatin1Char('@' + (id % 27));
        id /= 27;
    }

    return s;
}


[[nodiscard]] static bool isValidVRCodeLetter(QChar c)
{
    return c.row() == 0 && ((c.cell() >= 'A' && c.cell() <= 'Z') || c.cell() == 0xC4 || c.cell() == 0xD6);
}

VRStationCode::VRStationCode(const QString &id)
{
    if (id.size() < 2 || id.size() > 4 || !std::all_of(id.begin(), id.end(), isValidVRCodeLetter)) {
        return;
    }

    char buffer[4];
    std::memset(buffer, 0, 4);
    std::memcpy(buffer, id.toLatin1().constData(), id.size());
    setValue(fromChars(buffer));
}

QString VRStationCode::toString() const
{
    auto id = value();
    if (id == 0) {
        return {};
    }

    QString s;
    for (int i = 3; i >= 0; --i) {
        const auto v = ((id >> (i*6)) % 32);
        switch (v) {
            case 0: return s;
            case 27:
                s += QChar(0xC4);
                break;
            case 28:
                s += QChar(0xD6);
                break;
            default:
                s += QLatin1Char('@' + v);
        }
    }

    return s;
}

static_assert(sizeof(ViaRailStationCode) == 3);
