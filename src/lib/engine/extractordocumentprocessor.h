/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "extractordocumentnode.h"

class QByteArray;
class QJSEngine;
class QJSValue;
class QStringView;
class QVariant;

namespace KItinerary {

class ExtractorEngine;
class ExtractorFilter;

/** Abstract base class of a document type processor. */
class ExtractorDocumentProcessor
{
public:
    virtual ~ExtractorDocumentProcessor();

    /** Fast check whether the given encoded data can possibly be processed by this instance.
     *  The default implementation returns false, relying on QMimeDatabase for detection.
     */
    virtual bool canHandleData(const QByteArray &encodedData, QStringView fileName) const;

    /** Create a document node from raw data.
     *  The default implementation produces in invalid node.
     */
    virtual ExtractorDocumentNode createNodeFromData(const QByteArray &encodedData) const;
    /** Create a document node from an already decoded data type.
     *  The default implementation just sets @p decodedData on the resulting node.
     */
    virtual ExtractorDocumentNode createNodeFromContent(const QVariant &decodedData) const;

    /** Create child nodes for @p node, as far as that's necessary for this document type. */
    virtual void expandNode(ExtractorDocumentNode &node, const ExtractorEngine *engine) const;

    /** Propagate results from child nodes up to @p node.
     *  The default implementation just appends results.
     */
    virtual void reduceNode(ExtractorDocumentNode &node) const;

    /** Called before extractors are applied to @p node.
     *  This can be used for performing document type specific extraction steps.
     *  The default implementation does nothing.
     */
    virtual void preExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const;

    /** Checks whether the given @p filter matches @p node.
     *  The default implementation can handle QObject and Q_GADGET types via Qt's property system.
     */
    virtual bool matches(const ExtractorFilter &filter, const ExtractorDocumentNode &node) const;

    /** Called after extractors have been applied to @p node.
     *  This can be used for applying document type specific data to the extracted result.
     *  The default implementation does nothing.
     */
    virtual void postExtract(ExtractorDocumentNode &node, const ExtractorEngine *engine) const;

    /** Create a QJSValue for the node content.
     *  The default implementation is sufficient if the content is a QObject pointer, QGadget value,
     *  or any built-in type.
     */
    virtual QJSValue contentToScriptValue(const ExtractorDocumentNode &node, QJSEngine *engine) const;

    /** Destroys type-specific data in @p node.
     *  The default implementation does nothing.
     */
    virtual void destroyNode(ExtractorDocumentNode &node) const;

protected:
    template <typename T>
    inline void destroyIfOwned(ExtractorDocumentNode &node) const
    {
        delete static_cast<T*>(node.content<Internal::OwnedPtr<T>>());
    }

    static bool matchesGadget(const ExtractorFilter &filter, const QMetaObject *mo, const void *obj);
    template <typename T>
    static inline bool matchesGadget(const ExtractorFilter &filter, const T *obj)
    {
        return matchesGadget(filter, &T::staticMetaObject, obj);
    }
};

}

