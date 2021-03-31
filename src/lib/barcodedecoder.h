/*
    SPDX-FileCopyrightText: 2018-2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QFlags>
#include <QVariant>

#include <unordered_map>

class QByteArray;
class QImage;
class QString;

namespace KItinerary {

/** Barcode decoding with result caching.
 *  All non-static functions are using heuristics and cached results before actually
 *  performing an expensive barcode decoding operation, so repreated calls or calls with
 *  implausible arguments are cheap-ish.
 *
 *  @note This is only functional if zxing is available.
 *  @internal Only exported for unit tests and KItinerary Workbench.
 */
class KITINERARY_EXPORT BarcodeDecoder
{
public:
    BarcodeDecoder();
    ~BarcodeDecoder();

    enum BarcodeType {
        Aztec = 1,
        QRCode = 2,
        PDF417 = 4,
        DataMatrix = 8,
        Any = 15,
        AnySquare = 11,
        None = 0
    };
    Q_DECLARE_FLAGS(BarcodeTypes, BarcodeType)

    /** Checks if @p img contains a barcode of type @p hint. */
    bool isBarcode(const QImage &img, BarcodeTypes hint = Any) const;

    /** Decodes a binary payload barcode in @p img of type @p hint. */
    QByteArray decodeBinary(const QImage &img, BarcodeTypes hint = Any) const;

    /** Decodes a textual payload barcode in @p img of type @p hint. */
    QString decodeString(const QImage &img, BarcodeTypes hint = Any) const;

    /** Clears the internal cache. */
    void clearCache();

    /** Checks if the given image dimensions are plausible for a barcode.
     *  These checks are done first by BarcodeDecoder, it might however useful
     *  to perform them manually if a cheaper way to obtain the image dimension exists
     *  that does not require a full QImage creation.
     */
    static bool isPlausibleSize(int width, int height);

    /** Checks if the given image dimensions are a barcode of type @p hint.
     *  See above.
     */
    static bool isPlausibleAspectRatio(int width, int height, BarcodeTypes hint = Any);

    /** The combination of the above. */
    static bool maybeBarcode(int width, int height, BarcodeTypes hint = Any);

private:
    struct Result {
        BarcodeTypes positive = BarcodeDecoder::None;
        BarcodeTypes negative = BarcodeDecoder::None;
        enum ContentType {
            None = 0,
            ByteArray = 1,
            String = 2,
            Any = 3
        };
        int contentType = None;
        QVariant content;
    };

    void decodeIfNeeded(const QImage &img, BarcodeTypes hint, Result &result) const;
    void decodeZxing(const QImage &img, BarcodeDecoder::BarcodeTypes format, BarcodeDecoder::Result &result) const;

    mutable std::unordered_map<qint64, Result> m_cache;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KItinerary::BarcodeDecoder::BarcodeTypes)

