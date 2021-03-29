/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_EXTRACTORDOCUMENTNODE_H
#define KITINERARY_EXTRACTORDOCUMENTNODE_H

#include "kitinerary_export.h"

#include <QDateTime>
#include <QJsonArray>
#include <QMetaType>
#include <QVariant>

#include <memory>
#include <type_traits>

class QJSEngine;
class QJSValue;

namespace KItinerary {

///@cond internal
namespace Internal {
template <typename T>
struct OwnedPtr {
    inline OwnedPtr() = default;
    inline OwnedPtr(T* _ptr) : ptr(_ptr) {}
    inline operator T*() const { return ptr; }
    T *ptr = nullptr;
};
}
///@endcond

class ExtractorDocumentNodePrivate;
class ExtractorDocumentProcessor;
class ExtractorResult;
class ExtractorScriptEngine;

/** A node in the extracted document object tree.
 *  Essentially this models a tree of variants representing the input document,
 *  Each node being associated with and managed by the KItinerary::ExtractorDocumentProcessor
 *  for its corresponding type.
 *  Each nodes also carries the result of data extraction on itself and/or its children.
 *  This is meant for consumption in both C++ and JS code.
 */
class KITINERARY_EXPORT ExtractorDocumentNode
{
    Q_GADGET
    Q_PROPERTY(bool isNull READ isNull)

    /** The parent node, or a null node if this is the root node. */
    Q_PROPERTY(KItinerary::ExtractorDocumentNode parent READ parent)
    /** Child nodes, for QJSEngine access. */
    Q_PROPERTY(QVariantList childNodes READ childNodesVariant)

    /** The MIME type of this node. */
    Q_PROPERTY(QString mimeType READ mimeType)
    /** The decoded content of this node.
     *  The exact type in here depends on the MIME type, adapted for QJSEngine consumption.
     */
    Q_PROPERTY(QJSValue content READ contentJsValue)
    /** The best known context date/time at this point in the document tree.
     *  If not set on this node, the context date/time of the parent node is returned.
     */
    Q_PROPERTY(QDateTime contextDateTime READ contextDateTime)
    /** Result access for QJSEngine. */
    Q_PROPERTY(QJsonArray result READ jsonLdResult)
    /** Information about the location of this node in relation to one of its
     *  ancestors.
     *  The exact meaning of this depends on the type of the node, one example
     *  would be a page number an image is found on in a PDF document.
     */
    Q_PROPERTY(QVariant location READ location)

public:
    /** Creates a null node.
     *  @see KItinerary::ExtractorDocumentNodeFactory on how to create proper instances.
     */
    ExtractorDocumentNode();
    ExtractorDocumentNode(const ExtractorDocumentNode &other);
    ExtractorDocumentNode(ExtractorDocumentNode &&other);
    ~ExtractorDocumentNode();
    ExtractorDocumentNode& operator=(const ExtractorDocumentNode &other);
    ExtractorDocumentNode& operator=(ExtractorDocumentNode &&other);

    /** Returns @c true if this is a null instance. */
    bool isNull() const;

    ExtractorDocumentNode parent() const;
    ///@cond internal
    void setParent(const ExtractorDocumentNode &parent);
    ///@endcond

    /** The MIME type of this node. */
    QString mimeType() const;
    ///@cond internal
    void setMimeType(const QString &mimeType);
    ///@endcond

    /** Returns the decoded content of this node.
     *  The content of the QVariant depends on the MIME type.
     */
    QVariant content() const;
    /** Set decoded content.
     *  Only to be used from KItinerary::ExtractorDocumentProcessor::createNodeFromData.
     */
    void setContent(const QVariant &content);

    /** Checks if the content of this node is of type @p T. */
    template <typename T>
    inline bool isA() const
    {
        return content().userType() == qMetaTypeId<T>();
    }

    /** Returns the content of this node converted to type @p T. */
    template <typename T>
    inline typename std::enable_if<!std::is_pointer<T>::value || !QMetaTypeId2<Internal::OwnedPtr<typename std::remove_pointer<T>::type>>::Defined, T>::type
    content() const
    {
        return content().value<T>();
    }
    template <typename T>
    inline typename std::enable_if<std::is_pointer<T>::value && QMetaTypeId2<Internal::OwnedPtr<typename std::remove_pointer<T>::type>>::Defined, T>::type
    content() const
    {
        if (isA<T>()) {
            return content().value<T>();
        }
        return content().value<Internal::OwnedPtr<typename std::remove_pointer<T>::type>>();
    }

    template <typename T>
    inline void setContent(const T& value)
    {
        setContent(QVariant::fromValue(value));
    }

    /** The best known context date/time at this point in the document tree. */
    QDateTime contextDateTime() const;
    /** Set the context date/time.
     *  Only use this from KItinerary::ExtractorDocumentProcessor.
     */
    void setContextDateTime(const QDateTime &contextDateTime);


    /* Information about the location of this node in relation to one of its ancestors. */
    QVariant location() const;
    /** Set the location information.
     *  Only use this from KItinerary::ExtractorDocumentProcessor.
     */
    void setLocation(const QVariant &location);

    ///@cond internal
    const ExtractorDocumentProcessor* processor() const;
    void setProcessor(const ExtractorDocumentProcessor *processor);
    ///@endcond

    /** The child nodes of this node. */
    const std::vector<ExtractorDocumentNode>& childNodes() const;
    /** Add another child node.
     *  Do not use this outside of KItinerary::ExtractorDocumentProcessor::expandNode().
     */
    void appendChild(ExtractorDocumentNode &child);

    /** JS API for finding child nodes given an KItinerary::ExtractorFilter. */
    Q_INVOKABLE QVariantList findChildNodes(const QJSValue &jsFilter) const;

    /** Returns the results that have accumulated so far from this node or its children. */
    ExtractorResult result() const;
    /** Add additional results from an extraction step. */
    void addResult(ExtractorResult &&result);
    /** Replace the existing results by @p result. */
    void setResult(ExtractorResult &&result);

private:
    explicit ExtractorDocumentNode(const std::shared_ptr<ExtractorDocumentNodePrivate> &dd);
    QJsonArray jsonLdResult() const;
    QVariantList childNodesVariant() const;
    QJSValue contentJsValue() const;
    std::shared_ptr<ExtractorDocumentNodePrivate> d;

    friend class ExtractorScriptEngine;
    void setScriptEngine(QJSEngine *jsEngine) const;
};

}

Q_DECLARE_METATYPE(KItinerary::ExtractorDocumentNode)

#endif // KITINERARY_EXTRACTORDOCUMENTNODE_H
