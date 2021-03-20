/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractordocumentnode.h"
#include "extractordocumentprocessor.h"
#include "extractorresult.h"

#include <QJSEngine>
#include <QJSValue>

#include <cassert>

using namespace KItinerary;

namespace KItinerary {
class ExtractorDocumentNodePrivate
{
public:
    std::weak_ptr<ExtractorDocumentNodePrivate> parent;
    std::vector<ExtractorDocumentNode> childNodes;
    QString mimeType;
    QVariant content;
    QDateTime contextDateTime;
    const ExtractorDocumentProcessor *processor;
    ExtractorResult result;
    QVariant location;
    QJSEngine *m_jsEngine = nullptr;

    QJSEngine *jsEngine() const;
};
}

QJSEngine* ExtractorDocumentNodePrivate::jsEngine() const
{
    if (m_jsEngine) {
        return m_jsEngine;
    }
    const auto p = parent.lock();
    return p ? p->jsEngine() : nullptr;
}

ExtractorDocumentNode::ExtractorDocumentNode()
    : d(std::make_shared<ExtractorDocumentNodePrivate>())
{
}

ExtractorDocumentNode::ExtractorDocumentNode(const std::shared_ptr<ExtractorDocumentNodePrivate> &dd)
    : d(dd ? dd : std::make_shared<ExtractorDocumentNodePrivate>())
{
}

ExtractorDocumentNode::ExtractorDocumentNode(const ExtractorDocumentNode &other) = default;
ExtractorDocumentNode::ExtractorDocumentNode(ExtractorDocumentNode &&other) = default;

ExtractorDocumentNode::~ExtractorDocumentNode()
{
    if (d && d.use_count() == 1 && d->processor) {
        d->processor->destroyNode(*this);
    }
}

ExtractorDocumentNode& ExtractorDocumentNode::operator=(const ExtractorDocumentNode &other)
{
    if (d && d.use_count() == 1 && d->processor) {
        d->processor->destroyNode(*this);
    }
    d = other.d;
    return *this;
}

ExtractorDocumentNode& ExtractorDocumentNode::operator=(ExtractorDocumentNode &&other)
{
    if (d && d.use_count() == 1 && d->processor) {
        d->processor->destroyNode(*this);
    }
    d = std::move(other.d);
    return *this;
}

bool ExtractorDocumentNode::isNull() const
{
    return d->content.isNull() || !d->processor || d->mimeType.isEmpty();
}

ExtractorDocumentNode ExtractorDocumentNode::parent() const
{
    return ExtractorDocumentNode(d->parent.lock());
}

void ExtractorDocumentNode::setParent(const ExtractorDocumentNode &parent)
{
    d->parent = std::weak_ptr(parent.d);
}

QString ExtractorDocumentNode::mimeType() const
{
    return d->mimeType;
}

void ExtractorDocumentNode::setMimeType(const QString &mimeType)
{
    d->mimeType = mimeType;
}

QVariant ExtractorDocumentNode::content() const
{
    return d->content;
}

void ExtractorDocumentNode::setContent(const QVariant &content)
{
    d->content = content;
}

const ExtractorDocumentProcessor* ExtractorDocumentNode::processor() const
{
    return d->processor;
}

void ExtractorDocumentNode::setProcessor(const ExtractorDocumentProcessor *processor)
{
    assert(!d->processor);
    d->processor = processor;
}

const std::vector<ExtractorDocumentNode>& ExtractorDocumentNode::childNodes() const
{
    return d->childNodes;
}

void ExtractorDocumentNode::appendChild(ExtractorDocumentNode &child)
{
    child.setParent(*this);
    d->childNodes.push_back(child);
}

ExtractorResult ExtractorDocumentNode::result() const
{
    return d->result;
}

void ExtractorDocumentNode::addResult(ExtractorResult &&result)
{
    d->result.append(std::move(result));
}

void ExtractorDocumentNode::setResult(ExtractorResult &&result)
{
    d->result = std::move(result);
}

QDateTime ExtractorDocumentNode::contextDateTime() const
{
    if (!d->contextDateTime.isValid() && !d->parent.expired()) {
        return parent().contextDateTime();
    }
    return d->contextDateTime;
}

void ExtractorDocumentNode::setContextDateTime(const QDateTime &contextDateTime)
{
    d->contextDateTime = contextDateTime;
}

QVariant ExtractorDocumentNode::location() const
{
    if (d->location.isNull() && !d->parent.expired()) {
        return parent().location();
    }
    return d->location;
}

void ExtractorDocumentNode::setLocation(const QVariant &location)
{
    d->location = location;
}

QJsonArray ExtractorDocumentNode::jsonLdResult() const
{
    return d->result.jsonLdResult();
}

QVariantList ExtractorDocumentNode::childNodesVariant() const
{
    QVariantList l;
    l.reserve(d->childNodes.size());
    std::transform(d->childNodes.begin(), d->childNodes.end(), std::back_inserter(l), [](const auto &c) { return QVariant::fromValue(c); });
    return l;
}

QJSValue ExtractorDocumentNode::contentJsValue() const
{
    if (!d || !d->processor) {
        return {};
    }
    if (auto jsEngine = d->jsEngine()) {
        return d->processor->contentToScriptValue(*this, jsEngine);
    }
    return {};
}

void ExtractorDocumentNode::setScriptEngine(QJSEngine* jsEngine) const
{
    if (!d->parent.expired()) {
        parent().setScriptEngine(jsEngine);
    } else {
        d->m_jsEngine = jsEngine;
    }
}

#include "moc_extractordocumentnode.cpp"
