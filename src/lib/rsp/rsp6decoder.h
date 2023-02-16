/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_RSP6DECODER_H
#define KITINERARY_RSP6DECODER_H

class QByteArray;

namespace KItinerary {

/**
 * UK RSP-6 ticket decoder.
 *
 * @see https://eta.st/2023/01/31/rail-tickets.html
 * @see https://git.eta.st/eta/rsp6-decoder
 */
class Rsp6Decoder
{
public:
     /** Decodes base26 transport encoding and decrypts the ticket payload. */
     static QByteArray decode(const QByteArray &data);
};

}

#endif // KITINERARY_RSP6DECODER_H
