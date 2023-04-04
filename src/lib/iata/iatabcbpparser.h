/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QDateTime>
#include <QList>

#include <memory>

class QString;
class QVariant;

namespace KItinerary {

class IataBcbp;

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
QList<QVariant> parse(const QString &message,
                      const QDateTime &externalIssueDateTime = QDateTime());
KITINERARY_EXPORT QList<QVariant> parse(const IataBcbp &bcbp,
                                        const QDateTime &contextDate);
}

}

