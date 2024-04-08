/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QByteArray;
class QVariant;

namespace KItinerary {

/** Factory function for the various ERA SSB variants. */
namespace SSBTicketReader
{
/** Returns whether @p data could be an ERA SSB ticket. */
[[nodiscard]] bool maybeSSB(const QByteArray &data);

/** Attempt to read an SSB ticket.
 *  The variant is auto-detected, unless @p versionOverride is specified.
 */
[[nodiscard]] QVariant read(const QByteArray &data, int versionOverride = 0);
}

}

