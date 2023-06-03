/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractorengine.h"
#include "logging.h"

#include <KItinerary/ExtractorDocumentNodeFactory>

#include <QScopeGuard>

using namespace KItinerary;

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
    const auto preHints = m_engine->hints();
    const auto prevNode = m_currentNode;

    m_engine->setHints(preHints | KItinerary::ExtractorEngine::ExtractFullPageRasterImages);
    auto node = m_engine->documentNodeFactory()->createNode(data);
    m_engine->processNode(node);

    m_engine->setHints(preHints);
    m_currentNode = prevNode;

    m_currentNode.appendChild(node); // for debug tooling
    return node;
}

ExtractorDocumentNode JsApi::ExtractorEngine::extract(const QVariant &content, const QString &mimeType)
{
    const auto preHints = m_engine->hints();
    const auto prevNode = m_currentNode;

    m_engine->setHints(preHints | KItinerary::ExtractorEngine::ExtractFullPageRasterImages);
    auto node = m_engine->documentNodeFactory()->createNode(content, mimeType);
    m_engine->processNode(node);

    m_engine->setHints(preHints);
    m_currentNode = prevNode;

    m_currentNode.appendChild(node); // for debug tooling
    return node;
}
