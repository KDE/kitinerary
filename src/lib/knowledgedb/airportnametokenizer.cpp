/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "airportnametokenizer_p.h"

using namespace KItinerary;

AirportNameTokenizer::AirportNameTokenizer(QStringView text)
    : m_text(text)
{
    advance();
}

AirportNameTokenizer::~AirportNameTokenizer() = default;

bool AirportNameTokenizer::hasNext()
{
    return m_end > m_begin && m_begin >= 0 && m_end >= 0 && m_end <= m_text.size();
}

QStringView AirportNameTokenizer::next()
{
    Q_ASSERT(hasNext());
    const auto s = m_text.mid(m_begin, m_end - m_begin);
    advance();
    return s;
}

void AirportNameTokenizer::advance()
{
    m_begin = m_end;
    for (;m_begin < m_text.size(); ++m_begin) {
        if (!isSeparator(m_text.at(m_begin))) {
            break;
        }
    }

    m_end = m_begin + 1;
    for (;m_end < m_text.size(); ++m_end) {
        if (isSeparator(m_text.at(m_end))) {
            break;
        }
    }

    if ((m_end - m_begin) < MIN_LENGTH) {
        m_begin = m_end;
    }
    if (!hasNext() && m_end < m_text.size()) {
        advance();
    }
}

bool AirportNameTokenizer::isSeparator(QChar c) const
{
    return c.isSpace() || c.isNumber() || c.isPunct() || !c.isPrint();
}

QStringList AirportNameTokenizer::toStringList()
{
    QStringList l;
    while (hasNext()) {
        l.push_back(next().toString());
    }
    return l;
}
