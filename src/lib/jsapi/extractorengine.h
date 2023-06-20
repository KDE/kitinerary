/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_JSAPI_EXTRACTORENGINE_H
#define KITINERARY_JSAPI_EXTRACTORENGINE_H

#include <KItinerary/ExtractorDocumentNode>
#include <KItinerary/ExtractorEngine>

namespace KItinerary {

/** JavaScript API available to extractor scripts. */
namespace JsApi {

/** API to access the extractor engine for JS extractor scripts. */
class ExtractorEngine : public QObject
{
    Q_OBJECT
public:
    ///@cond internal
    explicit ExtractorEngine(QObject *parent = nullptr);
    ~ExtractorEngine();

    void setEngine(KItinerary::ExtractorEngine *engine);
    void setCurrentNode(const ExtractorDocumentNode &node);
    void clear();
    ///@endcond

    /** Run the extractor on @p data.
     *  Use this if the data to extract needs to be decoded by an extractor script first
     *  and is available as raw byte array.
     *  You do not need to call this for document parts that the extractor recognizes itself.
     *
     *  A new document node for @p data is added below the node currently processed,
     *  and that node is returned.
     *
     *  @see KItinerary::ExtractorEngine
     */
    Q_INVOKABLE KItinerary::ExtractorDocumentNode extract(const QByteArray &data);
    /** Run the extractor on @p content of type @p mimeType.
     *  Use this if the data to extract needs to be decoded by an extractor script first
     *  and is available already in decoded form in a suitable data type.
     *  You do not need to call this for document parts that the extractor recognizes itself.
     *
     *  A new document node for @p data is added below the node currently processed,
     *  and that node is returned.
     *
     *  @see KItinerary::ExtractorEngine
     */
    Q_INVOKABLE KItinerary::ExtractorDocumentNode extract(const QVariant &content, const QString &mimeType);

private:
    KItinerary::ExtractorEngine *m_engine = nullptr;
    KItinerary::ExtractorDocumentNode m_currentNode;
    int m_recursionDepth = 0;
};

}
}

#endif // KITINERARY_JSAPI_EXTRACTORENGINE_H
