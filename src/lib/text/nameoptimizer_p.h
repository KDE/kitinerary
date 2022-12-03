/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_NAMEOPTIMIZER_H
#define KITINERARY_NAMEOPTIMIZER_H

class QString;
class QVariant;

namespace KItinerary {

class Person;

/** Searches a given text for a better form of a given Person object.
 *  Useful to improve ASCII-only or all uppercase names extracted from
 *  ticket barcodes for example.
 */
class NameOptimizer
{
public:
    static Person optimizeName(const QString &text, const Person &person);
    static QVariant optimizeNameRecursive(const QString &text, QVariant object);

private:
    static QString optimizeNameString(const QString &text, const QString &name);
};

}

#endif // KITINERARY_NAMEOPTIMIZER_H
