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

Q_DECLARE_METATYPE(KItinerary::Internal::OwnedPtr<KMime::Content>)

namespace {
bool contentMightBeEmail(const QByteArray &data)
{
    // raw email
    for (const auto c : data) {
        if (std::isalpha(c) || c == '-') {
            continue;
        }
        if (c == ':') {
            return true;
        }
        break;
    }

    // mbox format
    return data.startsWith("From ");
}

template <typename T>
const T* findHeader(KMime::Content *content)
{
    auto header = content->header<T>();
    if (header || !content->parent()) {
        return header;
    }
    return findHeader<T>(content->parent());
}

const KMime::Headers::Base* findHeader(KMime::Content *content, const char *headerType)
{
    const auto header = content->headerByType(headerType);
    if (header || !content->parent()) {
        return header;
    }
    return findHeader(content->parent(), headerType);
}
}

bool MimeDocumentProcessor::canHandleData(const QByteArray &encodedData, QStringView fileName) const
{
  return contentMightBeEmail(encodedData) ||
         fileName.endsWith(QLatin1StringView(".eml"), Qt::CaseInsensitive) ||
         fileName.endsWith(QLatin1StringView(".mbox"), Qt::CaseInsensitive);
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

    const auto dateHdr = findHeader<KMime::Headers::Date>(msg);
    if (dateHdr) {
        node.setContextDateTime(dateHdr->dateTime());
    }

    return node;
}

ExtractorDocumentNode MimeDocumentProcessor::createNodeFromContent(const QVariant &decodedData) const
{
    auto *content = decodedData.value<KMime::Content*>();
    if (!content) {
        content = decodedData.value<KMime::Message*>();
    }
    if (!content) {
        return {};
    }

    ExtractorDocumentNode node;
    node.setContent(content);

    const auto dateHdr = findHeader<KMime::Headers::Date>(content);
    if (dateHdr) {
        node.setContextDateTime(dateHdr->dateTime());
    }

    return node;
}

static ExtractorDocumentNode expandContentNode(ExtractorDocumentNode &node, KMime::Content *content, const ExtractorEngine *engine)
{
    QString fileName;
    const auto contentType = content->contentType(false);
    if (contentType) {
        fileName = contentType->name();
    }
    const auto contentDisposition = content->contentDisposition(false);
    if (fileName.isEmpty() && contentDisposition) {
        fileName = contentDisposition->filename();
    }

    ExtractorDocumentNode child;
    if ((contentType && contentType->isPlainText() && fileName.isEmpty()) || (!contentType && content->isTopLevel())) {
        child = engine->documentNodeFactory()->createNode(content->decodedText(), u"text/plain");
    } else if (contentType && contentType->isHTMLText()) {
        child = engine->documentNodeFactory()->createNode(content->decodedText(), u"text/html");
    } else if (content->bodyIsMessage()) {
        child = engine->documentNodeFactory()->createNode(QVariant::fromValue(content->bodyAsMessage().get()), u"message/rfc822");
    } else {
        child = engine->documentNodeFactory()->createNode(content->decodedContent(), fileName);
    }
    node.appendChild(child);
    return child;
}

static void expandContentNodeRecursive(ExtractorDocumentNode &node, KMime::Content *content, const ExtractorEngine *engine)
{
    const auto ct = content->contentType(false);
    const auto children = content->contents();
    if (!ct || children.empty()) {
        expandContentNode(node, content, engine);
        return;
    }

    // special handling of multipart/related to add images to the corresponding HTML document
    if (ct && ct->isMultipart() && ct->isSubtype("related") &&
        ct->parameter(QLatin1StringView("type")) ==
            QLatin1String("text/html") &&
        children.size() >= 2) {
      const auto child = children.front();
      if (child->contentType(false) &&
          child->contentType(false)->isHTMLText()) {
        auto htmlNode = expandContentNode(node, child, engine);
        for (auto it = std::next(children.begin()); it != children.end();
             ++it) {
          auto imgNode = expandContentNode(htmlNode, (*it), engine);
          const auto cid = (*it)->contentID(false);
          if (cid) {
            imgNode.setLocation(cid->identifier());
          }
        }
        return;
      }
    }

    for (const auto child : children) {
        if (child->bodyIsMessage()) {
            expandContentNode(node, child, engine); // do not recurse into nested emails, we want those as dedicated nodes
        } else {
            expandContentNodeRecursive(node, child, engine);
        }
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
    const auto header = findHeader(content, filter.fieldName().toUtf8().constData());
    return header ? filter.matches(header->asUnicodeString()) : false;
}

void MimeDocumentProcessor::destroyNode(ExtractorDocumentNode &node) const
{
    destroyIfOwned<KMime::Content>(node);
}
