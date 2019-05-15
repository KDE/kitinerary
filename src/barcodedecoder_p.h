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

#ifndef KITINERARY_BARCODEDECODER_H
#define KITINERARY_BARCODEDECODER_H

#include "kitinerary_export.h"

class QByteArray;
class QImage;
class QString;

namespace KItinerary {

/** Barcode decoding functions.
 *  @note These functions are only functional if zxing is available.
 *  @internal Only exported for unit tests.
 */
namespace BarcodeDecoder
{
    /** Decode a PDF417 barcode. */
    KITINERARY_EXPORT QString decodePdf417(const QImage &img);

    /** Decode an Aztec barcode containing text data. */
    KITINERARY_EXPORT QString decodeAztec(const QImage &img);

    /** Decode an Aztec barcode containing binary data (such as UIC 918.3 codes). */
    KITINERARY_EXPORT QByteArray decodeAztecBinary(const QImage &img);

    /** Decode an QRCode barcode containing text data. */
    KITINERARY_EXPORT QString decodeQRCode(const QImage &img);
}

}

#endif // KITINERARY_BARCODEDECODER_H
