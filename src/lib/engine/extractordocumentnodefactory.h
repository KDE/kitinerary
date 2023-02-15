/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QString>
#include <QStringView>

#include <memory>

class QByteArray;
class QVariant;

namespace KItinerary {

class ExtractorDocumentNode;
class ExtractorDocumentNodeFactoryPrivate;
class ExtractorDocumentProcessor;

/** Instantiates KItinerary::ExtractorDocumentNode instances using the type-specific document processor. */
class KITINERARY_EXPORT ExtractorDocumentNodeFactory
{
public:
    explicit ExtractorDocumentNodeFactory();
    ~ExtractorDocumentNodeFactory();

    /** Create a new document node from @p data.
     *  @param fileName Optional hint for MIME-type auto-detection.
     *  @param mimeType MIME type of @p data if known, auto-detected otherwise.
     */
    ExtractorDocumentNode createNode(const QByteArray &data, QStringView fileName = {}, QStringView mimeType = {}) const;
    /** Create a node for an already decoded content object. */
    ExtractorDocumentNode createNode(const QVariant &decodedData, QStringView mimeType) const;

    /** Register a new document processor. */
    void registerProcessor(std::unique_ptr<ExtractorDocumentProcessor> &&processor, QStringView canonicalMimeType,
                           std::initializer_list<QStringView> aliasMimeTypes = {});

    /** Perform extraction of "risky" content such as PDF files in a separate process.
     *  This is safer as it isolates the using application from crashes/hangs due to corrupt files.
     *  It is however slower, and not available on all platforms.
     *  This is off by default.
     */
    void setUseSeparateProcess(bool separateProcess);

private:
    std::unique_ptr<ExtractorDocumentNodeFactoryPrivate> d;
};

}

