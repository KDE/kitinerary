/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QExplicitlySharedDataPointer>
#include <QMetaType>

#include <functional>

class QImage;
class QTransform;

namespace KItinerary {
class PdfImagePrivate;
class PdfImageRef;
}

namespace std {
template<>
struct hash<KItinerary::PdfImageRef>
{
    inline std::size_t operator()(const KItinerary::PdfImageRef &ref) const noexcept;
};
}

namespace KItinerary {

/** PDF image element type. */
enum class PdfImageType {
    Image,
    Mask,
    SMask
};

/** PDF object reference for an image, with the ability to address attached masks as well. */
class PdfImageRef
{
public:
    constexpr PdfImageRef() = default;
    constexpr inline PdfImageRef(int num, int gen, PdfImageType type = PdfImageType::Image)
        : m_refNum(num)
        , m_refGen(gen)
        , m_type(type)
    {}

    [[nodiscard]] constexpr inline bool isNull() const
    {
        return m_refNum < 0;
    }

    [[nodiscard]] constexpr inline bool operator==(const PdfImageRef &other) const
    {
        return m_refNum == other.m_refNum && m_refGen == other.m_refGen && m_type == other.m_type;
    }

private:
    int m_refNum = -1;
    int m_refGen = -1;
    PdfImageType m_type = PdfImageType::Image;
    friend class PdfImage;
    friend class PdfImagePrivate;
    friend std::size_t std::hash<PdfImageRef>::operator ()(const PdfImageRef&) const noexcept;
};

/** An image in a PDF document.
 */
class KITINERARY_EXPORT PdfImage
{
    Q_GADGET
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
public:
    PdfImage();
    PdfImage(const PdfImage&);
    ~PdfImage();
    PdfImage& operator=(const PdfImage&);

    /** Width of the image in PDF 1/72 dpi coordinates. */
    [[nodiscard]] int width() const;
    /** Height of the image in PDF 1/72 dpi coordinates. */
    [[nodiscard]] int height() const;

    /** Height of the source image. */
    [[nodiscard]] int sourceHeight() const;
    /** Width of the source image. */
    [[nodiscard]] int sourceWidth() const;

    /** Transformation from source image to final size/position on the page.
     *  Values are 1/72 inch.
     */
    [[nodiscard]] QTransform transform() const;

    /** Hints for loading image data. */
    enum LoadingHint {
        NoHint = 0, ///< Load image data as-is. The default.
        AbortOnColorHint = 1, ///< Abort loading when encountering a non black/white pixel, as a shortcut for barcode detection.
        ConvertToGrayscaleHint = 2, ///< Convert to QImage::Format_Grayscale8 during loading. More efficient than converting later if all you need is grayscale.
    };
    Q_DECLARE_FLAGS(LoadingHints, LoadingHint)

    /** Sets image loading hints. */
    void setLoadingHints(LoadingHints hints);

    /** The source image without display transformations applied. */
    [[nodiscard]] QImage image() const;

    /** Returns whether this image has an object id.
     *  Vector graphic "images" don't have that.
     */
    [[nodiscard]] bool hasObjectId() const;

    /** PDF-internal unique identifier of this image.
     *  Use this to detect multiple occurrences of the same image in different
     *  places, if that reduces e.g. computation cost.
     */
    [[nodiscard]] PdfImageRef objectId() const;

    /** Returns whether this is a raster or vector image. */
    [[nodiscard]] bool isVectorImage() const;

    /** If this is a vector image, this returns the number
     *  of vector path elemets.
     */
    [[nodiscard]] int pathElementsCount() const;

    /** Returns @c true if this image has an aspect-ratio changing transform.
     *  That might need to be applied before doing barcode decoding for example.
     */
    [[nodiscard]] bool hasAspectRatioTransform() const;

    /** Applies the aspect ratio changing part of the transform
     *  to the given image (which typically should be the one returned
     *  by image()).
     */
    [[nodiscard]] QImage applyAspectRatioTransform(const QImage &image) const;

    /** PDF image type. */
    [[nodiscard]] PdfImageType type() const;

private:
    friend class PdfExtractorOutputDevice;
    friend class PdfPagePrivate;
    friend class PdfPage;
    QExplicitlySharedDataPointer<PdfImagePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PdfImage::LoadingHints)

}

std::size_t std::hash<KItinerary::PdfImageRef>::operator()(const KItinerary::PdfImageRef &ref) const noexcept
{
    // first part is how poppler hashes its Ref type
    return std::hash<int>{}(ref.m_refNum) ^ (std::hash<int>{}(ref.m_refGen) << 1) ^ std::hash<int>{}((int)ref.m_type);
}
