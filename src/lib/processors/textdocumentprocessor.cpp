/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "textdocumentprocessor.h"
#include "genericpriceextractorhelper_p.h"

#include <KItinerary/ExtractorFilter>

#include <QByteArray>
#include <QStringView>

#include <algorithm>
#include <cctype>

using namespace Qt::Literals::StringLiterals;
using namespace KItinerary;

bool TextDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return std::none_of(encodedData.begin(), encodedData.end(), [](unsigned char c) { return std::iscntrl(c) && !std::isspace(c); })
        || fileName.endsWith(".txt"_L1, Qt::CaseInsensitive);
}

ExtractorDocumentNode TextDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    ExtractorDocumentNode node;
    node.setContent(QString::fromUtf8(encodedData));
    return node;
}

bool TextDocumentProcessor::matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const
{
    return filter.matches(node.content<QString>());
}

void TextDocumentProcessor::postExtract(ExtractorDocumentNode &node, [[maybe_unused]] const ExtractorEngine *engine) const
{
    GenericPriceExtractorHelper::postExtract(node.content<QString>(), node);
}
