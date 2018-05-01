/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_UIC9183PARSER_H
#define KITINERARY_UIC9183PARSER_H

#include "kitinerary_export.h"

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

namespace KItinerary {

class Person;
class Uic9183ParserPrivate;

/** Parser for UIC 918-3 and 918-3* train tickets.
 *  The specification for this is unfortunately not publicly available.
 */
class KITINERARY_EXPORT Uic9183Parser
{
    Q_GADGET
    Q_PROPERTY(QString pnr READ pnr)
    Q_PROPERTY(KItinerary::Person person READ person)
public:
    Uic9183Parser();
    Uic9183Parser(const Uic9183Parser&);
    ~Uic9183Parser();
    Uic9183Parser& operator=(Uic9183Parser&);

    void parse(const QByteArray &data);
    bool isValid() const;

    /** The booking reference. */
    QString pnr() const;
    /** The person this ticket is issued to. */
    Person person() const;

private:
    QExplicitlySharedDataPointer<Uic9183ParserPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::Uic9183Parser)

#endif // KITINERARY_UIC9183PARSER_H
