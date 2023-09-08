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
        Code39 = 16,
        Code93 = 32,
        Code128 = 64,
        IgnoreAspectRatio = 128, /// search for barcodes anywhere in the image, rather than assuming the image is primarily containing the barcode
        AnySquare = Aztec | QRCode | DataMatrix,
        Any2D = AnySquare | PDF417,
        Any1D = Code39 | Code93 | Code128,
        Any = Any1D | Any2D,
        None = 0
    };
    Q_DECLARE_FLAGS(BarcodeTypes, BarcodeType)

    /** Barcode decoding result.
     *  Can be a QByteArray, a QString or empty.
     */
    class KITINERARY_EXPORT Result {
    public:
        enum ContentType {
            None = 0,
            ByteArray = 1,
            String = 2,
            Any = 3
        };
        int contentType = None;
        QVariant content;

        QByteArray toByteArray() const;
        QString toString() const;

        ///@cond internal
        BarcodeTypes positive = BarcodeDecoder::None;
        BarcodeTypes negative = BarcodeDecoder::None;
        ///@endcond
    };

    /** Decodes a barcode in @p img based on @p hint.
     *  @param hint has to be validated by something of the likes of maybeBarcode()
     *  before.
     */
    Result decode(const QImage &img, BarcodeTypes hint) const;

    /** Decodes multiple barcodes in @p img based on @p hint.
     *  @param hint IgnoreAspectRatio is implied here
     */
    std::vector<Result> decodeMulti(const QImage &img, BarcodeTypes hint) const;

    /** Decodes a binary payload barcode in @p img of type @p hint.
     *  @param hint has to be validated by something of the likes of maybeBarcode()
     *  before.
     */
    [[deprecated("use decode()")]] QByteArray decodeBinary(const QImage &img, BarcodeTypes hint) const;

    /** Decodes a textual payload barcode in @p img of type @p hint.
     *  @param hint has to be validated by something of the likes of maybeBarcode()
     *  before.
     */
    [[deprecated("use decode()")]] QString decodeString(const QImage &img, BarcodeTypes hint) const;

    /** Clears the internal cache. */
    void clearCache();

    /** Checks if the given image dimensions are plausible for a barcode.
     *  These checks are done first by BarcodeDecoder, it might however useful
     *  to perform them manually if a cheaper way to obtain the image dimension exists
     *  that does not require a full QImage creation.
     */
    static BarcodeTypes isPlausibleSize(int width, int height, BarcodeTypes hint);

    /** Checks if the given image dimensions are a barcode of type @p hint.
     *  See above.
     */
    static BarcodeTypes isPlausibleAspectRatio(int width, int height, BarcodeTypes hint);

    /** The combination of the above. */
    static BarcodeTypes maybeBarcode(int width, int height, BarcodeTypes hint);

private:
    void decodeIfNeeded(const QImage &img, BarcodeTypes hint, Result &result) const;
    void decodeMultiIfNeeded(const QImage &img, BarcodeTypes hint, std::vector<Result> &results) const;

    mutable std::unordered_map<qint64, std::vector<Result>> m_cache;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BarcodeDecoder::BarcodeTypes)

}

