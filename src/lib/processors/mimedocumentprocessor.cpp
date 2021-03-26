/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mimedocumentprocessor.h"

#include <KItinerary/ExtractorDocumentNodeFactory>
#include <KItinerary/ExtractorEngine>
#include <KItinerary/ExtractorFilter>

#include <KMime/Message>

#include <QDebug>

using namespace KItinerary;

Q_DECLARE_METATYPE(Internal::OwnedPtr<KMime::Content>)

static bool contentMightBeEmail(const QByteArray &data)
{
    // raw email
    for (const auto c : data) {
        if (std::isalpha(c) || c == '-') {
            continue;
        }
        if (c == ':') {
            return true;
        } else {
            break;
        }
    }

    // mbox format
    return data.startsWith("From ");
}

bool MimeDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
    return contentMightBeEmail(encodedData) ||
        fileName.endsWith(QLatin1String(".eml"), Qt::CaseInsensitive) ||
        fileName.endsWith(QLatin1String(".mbox"), Qt::CaseInsensitive);
}

ExtractorDocumentNode MimeDocumentProcessor::createNodeFromData(const QByteArray &encodedData) const
{
    auto msg = new KMime::Message;
    msg->setContent(KMime::CRLFtoLF(encodedData));
    if (msg->head().isEmpty() || msg->body().isEmpty()) {
        delete msg;
        return {};
    }
    msg->parse();

    ExtractorDocumentNode node;
    node.setContent<Internal::OwnedPtr<KMime::Content>>(msg);

    auto dateHdr = msg->header<KMime::Headers::Date>();
    if (dateHdr) {
        node.setContextDateTime(dateHdr->dateTime());
    }

    return node;
}

ExtractorDocumentNode MimeDocumentProcessor::createNodeFromContent(const QVariant &decodedData) const
{
    KMime::Content *content = decodedData.value<KMime::Content*>();
    if (!content) {
        content = decodedData.value<KMime::Message*>();
    }
    if (!content) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent(content);

    auto dateHdr = content->header<KMime::Headers::Date>();
    if (dateHdr) {
        node.setContextDateTime(dateHdr->dateTime());
    }

    return node;
}

static void expandContentNode(ExtractorDocumentNode &node, KMime::Content *content, const ExtractorEngine *engine)
{
    QString fileName;
    const auto ct = content->contentType(false);
    if (ct) {
        fileName = ct->name();
    }
    const auto cd = content->contentDisposition(false);
    if (fileName.isEmpty() && cd) {
        fileName = cd->filename();
    }

    ExtractorDocumentNode child;
    if ((ct && ct->isPlainText() && fileName.isEmpty()) || (!ct && content->isTopLevel())) {
        child = engine->documentNodeFactory()->createNode(content->decodedText(), u"text/plain");
    } else {
        child = engine->documentNodeFactory()->createNode(content->decodedContent(), fileName);
    }
    node.appendChild(child);
}

static void expandContentNodeRecursive(ExtractorDocumentNode &node, KMime::Content *content, const ExtractorEngine *engine)
{
    const auto children = content->contents();
    if (!content->contentType(false) || children.empty()) {
        expandContentNode(node, content, engine);
        return;
    }

    // TODO special handling of multipart/related to add images to the corresponding HTML document

    for (const auto child : children) {
        expandContentNodeRecursive(node, child, engine);
    }
}

void MimeDocumentProcessor::expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const
{
    const auto content = node.content<KMime::Content*>();
    expandContentNodeRecursive(node, content, engine);
}

bool MimeDocumentProcessor::matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const
{
    const auto content = node.content<KMime::Content*>();
    const auto header = content->headerByType(filter.fieldName().toUtf8().constData());
    return header ? filter.matches(header->asUnicodeString()) : false;
}

void MimeDocumentProcessor::destroyNode(ExtractorDocumentNode &node) const
{
    destroyIfOwned<KMime::Content>(node);
}
