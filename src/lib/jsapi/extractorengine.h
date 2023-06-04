/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_JSAPI_EXTRACTORENGINE_H
#define KITINERARY_JSAPI_EXTRACTORENGINE_H

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorEngine>

namespace KItinerary {
namespace JsApi {

/** API to access the extractor engine for JS extractor scripts. */
class ExtractorEngine : public QObject
{
    Q_OBJECT
public:
    explicit ExtractorEngine(QObject *parent = nullptr);
    ~ExtractorEngine();

    void setEngine(KItinerary::ExtractorEngine *engine);
    void setCurrentNode(const ExtractorDocumentNode &node);
    void clear();

    Q_INVOKABLE KItinerary::ExtractorDocumentNode extract(const QByteArray &data);
    Q_INVOKABLE KItinerary::ExtractorDocumentNode extract(const QVariant &content, const QString &mimeType);

private:
    KItinerary::ExtractorEngine *m_engine = nullptr;
    KItinerary::ExtractorDocumentNode m_currentNode;
    int m_recursionDepth = 0;
};

}
}

#endif // KITINERARY_JSAPI_EXTRACTORENGINE_H
