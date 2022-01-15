/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_AIRPORTNAMETOKENIZER_H
#define KITINERARY_AIRPORTNAMETOKENIZER_H

#include <QStringList>
#include <QStringView>

namespace KItinerary {

/** Split airport names into the tokens used by the airport database. */
class AirportNameTokenizer
{
public:
    explicit AirportNameTokenizer(QStringView text);
    ~AirportNameTokenizer();

    /** Returns @true if next() can be called one more time. */
    bool hasNext();
    /** Returns the next token and advances the tokenizer. */
    QStringView next();

    /** Returns a list containing all tokens. */
    QStringList toStringList();

private:
    void advance();

    bool isSeparator(QChar c) const;
    static constexpr const int MIN_LENGTH = 3;

    QStringView m_text;
    int m_begin = 0;
    int m_end = 0;
};

}

#endif // KITINERARY_AIRPORTNAMETOKENIZER_H
