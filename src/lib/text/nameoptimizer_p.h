/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_NAMEOPTIMIZER_H
#define KITINERARY_NAMEOPTIMIZER_H

class QString;

namespace KItinerary {

class Person;

/** Searches a given text for a better form of a given Person object.
 *  Useful to improve ASCII-only or all uppercase names extracted from
 *  ticket barcodes for example.
 */
class NameOptimizer
{
public:
    static Person optimizeName(const QString &text, Person person);

private:
    static QString optimizeNameString(const QString &text, const QString &name);
};

}

#endif // KITINERARY_NAMEOPTIMIZER_H
