/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uic9183head.h"

using namespace KItinerary;

Uic9183Head::Uic9183Head() = default;

// U_HEAD Block (version 1, size 53)
// 4x issuing carrier id
// 6x PNR
// 20x unique ticket key
// 12x issuing date/time as ddMMyyyyHHMM, as UTC
// 1x flags
// 2x ticket language
// 2x secondary ticket language

Uic9183Head::Uic9183Head(const Uic9183Block &block)
{
    if (block.version() == 1 && block.size() == 53) {
        m_data = block;
    }
}

Uic9183Head::~Uic9183Head() = default;

bool Uic9183Head::isValid() const
{
    return !m_data.isNull();
}

QDateTime Uic9183Head::issuingDateTime() const
{
    return isValid() ? QDateTime::fromString(Uic9183Utils::readUtf8String(m_data, 24, 12), QStringLiteral("ddMMyyyyhhmm")) : QDateTime();
}

#include "moc_uic9183head.cpp"
