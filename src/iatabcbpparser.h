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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_IATABCBPPARSER_H
#define KITINERARY_IATABCBPPARSER_H

#include "kitinerary_export.h"

#include <QDate>
#include <QVector>

#include <memory>

class QDate;
class QString;
class QVariant;

namespace KItinerary {

/**
 *  Parser for IATA Bar Coded Boarding Pass messages.
 *  @see https://www.iata.org/whatwedo/stb/Documents/BCBP-Implementation-Guide-5th-Edition-June-2016.pdf
 */
namespace IataBcbpParser
{
/** Parses the bar coded boarding pass message @p message into
 *  a list of FlightReservation instances.
 *  @param message The message.
 *  @param externalIssueDate The date the boarding pass was issued (or a sufficiently close approximation).
 *  This is necessary as by default the BCBP data only contains day and month of the flight, not the year.
 */
KITINERARY_EXPORT QVector<QVariant> parse(const QString &message, const QDate &externalIssueDate = QDate());

/** Quickly test if @p message could be a IATA BCBP code.
 *  This optimizes for speed rather than correctness, for use in barcode content auto-detection.
 */
KITINERARY_EXPORT bool maybeIataBcbp(const QString &message);
}

}

#endif // KITINERARY_IATABCBPPARSER_H
