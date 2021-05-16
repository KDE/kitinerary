/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_SSBTICKETREADER_H
#define KITINERARY_SSBTICKETREADER_H

class QByteArray;
class QVariant;

namespace KItinerary {

/** Factory function for the various ERA SSB variants. */
namespace SSBTicketReader
{
/** Attempt to read an SSB ticket.
 *  The variant is auto-detected, unless @p versionOverride is specified.
 */
QVariant read(const QByteArray &data, int versionOverride = 0);
}

}

#endif // KITINERARY_SSBTICKETREADER_H
