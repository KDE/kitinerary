/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractorengine.h"
#include "logging.h"

#include <text/pricefinder_p.h>

#include <KItinerary/ExtractorDocumentNodeFactory>

#include <QJSValue>
#include <QJSValueIterator>
#include <QScopeGuard>

using namespace KItinerary;

static constexpr inline auto RecursionDepthLimit = 10;

JsApi::ExtractorEngine::ExtractorEngine(QObject *parent)
    : QObject(parent)
{
}

JsApi::ExtractorEngine::~ExtractorEngine() = default;

void JsApi::ExtractorEngine::setEngine(KItinerary::ExtractorEngine *engine)
{
    m_engine = engine;
}

void JsApi::ExtractorEngine::setCurrentNode(const ExtractorDocumentNode& node)
{
    m_currentNode = node;
}

void JsApi::ExtractorEngine::clear()
{
    m_currentNode = {};
}

ExtractorDocumentNode JsApi::ExtractorEngine::extract(const QByteArray &data)
{
    if (m_recursionDepth > RecursionDepthLimit) {
        qCWarning(Log) << "Recursion depth limit reached, aborting";
        return {};
    }

    const auto preHints = m_engine->hints();
    const auto prevNode = m_currentNode;

    m_engine->setHints(preHints | KItinerary::ExtractorEngine::ExtractFullPageRasterImages);
    auto node = m_engine->documentNodeFactory()->createNode(data);
    m_currentNode.appendChild(node);

    ++m_recursionDepth;
    m_engine->processNode(node);
    --m_recursionDepth;

    m_engine->setHints(preHints);
    m_currentNode = prevNode;

    return node;
}

ExtractorDocumentNode JsApi::ExtractorEngine::extract(const QVariant &content, const QString &mimeType)
{
    if (m_recursionDepth > RecursionDepthLimit) {
        qCWarning(Log) << "Recursion depth limit reached, aborting";
        return {};
    }

    const auto preHints = m_engine->hints();
    const auto prevNode = m_currentNode;

    m_engine->setHints(preHints | KItinerary::ExtractorEngine::ExtractFullPageRasterImages);
    auto node = m_engine->documentNodeFactory()->createNode(content, mimeType);
    m_currentNode.appendChild(node);

    ++m_recursionDepth;
    m_engine->processNode(node);
    --m_recursionDepth;

    m_engine->setHints(preHints);
    m_currentNode = prevNode;

    return node;
}

static void applyPrice(const PriceFinder::Result &price, QJSValue obj)
{
    if (!obj.isObject()) {
        return;
    }
    obj.setProperty(QStringLiteral("totalPrice"), price.value);
    obj.setProperty(QStringLiteral("priceCurrency"), price.currency);
}

void JsApi::ExtractorEngine::extractPrice(const QString &text, QJSValue result) const
{
    PriceFinder finder;
    const auto price = finder.findHighest(text);
    if (!price.hasResult()) {
        return;
    }

    if (result.isArray()) {
        QJSValueIterator it(result);
        while (it.hasNext()) {
            it.next();
            applyPrice(price, it.value());
        }
    } else {
        applyPrice(price, result);
    }
}

#include "moc_extractorengine.cpp"
