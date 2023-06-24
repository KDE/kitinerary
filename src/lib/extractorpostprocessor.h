/*
   SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QList>
#include <QVariant>

#include <memory>

namespace KItinerary {

class ExtractorPostprocessorPrivate;

/** Post-process extracted data to filter out garbage and augment data from other sources.
 *
 *  In detail, this performs the tasks listed below for all data elements fed into it.
 *
 *  @section postproc_normalize Normalization
 *
 *  Basic normalization for e.g. renamed properties of older schema.org versions is already
 *  covered by JsonLdImportFilter, post-processing covers the more elaborate normalization steps,
 *  such as:
 *  - translate human readable and possibly localized country names into ISO 3166-1 codes.
 *  - expand IATA BCBP ticket tokens (see IataParser).
 *
 *  @section postproc_augment Augmentation
 *
 *  That is, add additional information derived from a built-in knowledge base (see KnowledgeDb).
 *  This includes:
 *  - Add timezone information to arrival and departure times.
 *  - Add geo coordinates and country information to known airports or train stations.
 *
 *  @section postproc_merge Merge Duplicates
 *
 *  Duplicate elements that might have been the result of two overlapping extractors (e.g. when
 *  extracting two different MIME parts of an email referring to the same reservation) are merged.
 *
 *  @section postproc_validation Validation
 *
 *  At this point, all invalid elements are discarded. The definition of invalid is fairly loose though,
 *  and typically only covers elements that are explicitly considered unusable. Examples:
 *  - A Flight missing a departure day or destination.
 *  - A LodigingReservation without an attached LodgingBusiness.
 *  - etc.
 *
 *  Validation can be disabled and done separately using KItinerary::ExtractorValidator, in case you
 *  want more control over which elements are considered valid. See setValidationEnabled().
 *
 *  @section postproc_sort Sorting
 *
 *  Finally the remaining elements are sorted based on their relevant date (see SortUtil). This
 *  makes the data usable for basic display right away, but it for example doesn't do multi-traveler
 *  aggregation, that's still left for the display layer.
 */
class KITINERARY_EXPORT ExtractorPostprocessor
{
public:
    ExtractorPostprocessor();
    ExtractorPostprocessor(const ExtractorPostprocessor&) = delete;
    ExtractorPostprocessor(ExtractorPostprocessor&&) noexcept;
    ~ExtractorPostprocessor();

    /** This will normalize and augment the given data elements and merge them with
     *  already added data elements if applicable.
     */
    void process(const QList<QVariant> &data);

    /** This returns the final result of all previously executed processing steps
     *  followed by sorting and filtering out all invalid data elements.
     */
    QList<QVariant> result() const;

    /** The date the reservation(s) processed here have been made, if known.
     *  This is used for determining the year of incomplete dates provided by
     *  various sources. Therefore this has to be somewhen before the reservation
     *  becomes due.
     */
    void setContextDate(const QDateTime &dt);

    /** Enable or disable validation.
     *  By default this is enabled, and will discard all unknown types
     *  and incomplete items. If you need more control over this, disable
     *  this here and pass the items through ExtractorValidator yourself
     *  (or even use an entirely different validation mechanism entirely).
     *  @see ExtractorValidator.
     *  @deprecated Has no functionality anymore, remove and use ExtractorValidator
     *  explicitly instead.
     */
    [[deprecated("has no functionality anymore")]] void setValidationEnabled(bool validate);

private:
    std::unique_ptr<ExtractorPostprocessorPrivate> d;
};

}

