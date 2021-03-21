/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_EXTERNALPROCESSOR_H
#define KITINERARY_EXTERNALPROCESSOR_H

#include <KItinerary/ExtractorDocumentProcessor>

#include <QString>

namespace KItinerary {

/** Dummy node to delegate to an external extractor process. */
class ExternalProcessor : public ExtractorDocumentProcessor
{
public:
    ExternalProcessor();
    ~ExternalProcessor();

    bool canHandleData(const QByteArray &encodedData, QStringView fileName) const override;
    ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const override;
    void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const override;

private:
    QString m_externalExtractor;
};

}

#endif // KITINERARY_EXTERNALPROCESSOR_H
