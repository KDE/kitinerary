/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_EXTRACTORFILTER_H
#define KITINERARY_EXTRACTORFILTER_H

#include "kitinerary_export.h"

#include "extractorinput.h"

#include <QExplicitlySharedDataPointer>
#include <qobjectdefs.h>

class QJsonObject;
class QJSValue;

namespace KItinerary {

class ExtractorDocumentNode;
class ExtractorFilterPrivate;

/** Determines whether an extractor is applicable to a given email. */
class KITINERARY_EXPORT ExtractorFilter
{
    Q_GADGET
public:
    ExtractorFilter();
    ~ExtractorFilter();
    ExtractorFilter(const ExtractorFilter&);
    ExtractorFilter(ExtractorFilter&&) noexcept;
    ExtractorFilter& operator=(const ExtractorFilter&);
    ExtractorFilter& operator=(ExtractorFilter&&);

    /** The filter type. */
    [[deprecated("use mimeType()")]] ExtractorInput::Type type() const;
    /** MIME type of the document part this filter can match. */
    QString mimeType() const;
    /** The field to filter on. */
    QString fieldName() const;
    /** Check if @p data matches this filter. */
    bool matches(const QString &data) const;
    /** Pattern to match field value against. */
    QString pattern() const;

    /** Specifies which document nodes should match this filter, relative to the one being extracted. */
    enum Scope {
        Current, ///< match the node being extracted
        Parent, ///< match the direct parent node
        Children, ///< match the direct child nodes
        Ancestors, ///< match any direct or indirect parent nodes
        Descendants, ///< match any direct or indirect child nodes
    };
    Q_ENUM(Scope)
    /** Evaluation scope of this filter, in relation to the node being extracted. */
    Scope scope() const;

    /** Checks whether this filter applies to @p node. */
    bool matches(const ExtractorDocumentNode &node) const;

    /** Checks whether this filter applies to @p node.
     *  Unlike matches() this returns all nodes triggering this filter.
     *  This matters in particular for matching child nodes, where multiple
     *  ones can match the filter.
     */
    void allMatches(const ExtractorDocumentNode &node, std::vector<ExtractorDocumentNode> &matches) const;

    ///@cond internal
    /** Load filter from @p obj. */
    bool load(const QJsonObject &obj);
    /** Serialize filter to a JSON object. */
    QJsonObject toJson() const;
    /** Create a filter from a JS object value. */
    static ExtractorFilter fromJSValue(const QJSValue &js);

    [[deprecated("use setMimeType()")]]  void setType(ExtractorInput::Type type);
    void setMimeType(const QString &mimeType);
    void setFieldName(const QString &fieldName);
    void setPattern(const QString &pattern);
    void setScope(Scope scope);
    ///@endcond

private:
    QExplicitlySharedDataPointer<ExtractorFilterPrivate> d;
};

}

#endif // KITINERARY_EXTRACTORFILTER_H
