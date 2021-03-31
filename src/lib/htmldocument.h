/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kitinerary_export.h"

#include <QObject>

#include <memory>

struct _xmlNode;

namespace KItinerary {

class HtmlDocument;
class HtmlDocumentPrivate;

/** HTML document element. */
class KITINERARY_EXPORT HtmlElement
{
    Q_GADGET
    Q_PROPERTY(bool isNull READ isNull)
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(KItinerary::HtmlElement parent READ parent)
    Q_PROPERTY(KItinerary::HtmlElement firstChild READ firstChild)
    Q_PROPERTY(KItinerary::HtmlElement nextSibling READ nextSibling)
    Q_PROPERTY(QString content READ content)
    Q_PROPERTY(QString recursiveContent READ recursiveContent)
public:
    HtmlElement();
    ~HtmlElement();

    /** Check if the element is null. */
    bool isNull() const;
    /** The element name. */
    QString name() const;
    /** Value of the attribute @p attr. */
    Q_INVOKABLE QString attribute(const QString &attr) const;
    /** Returns the parent element of this node. */
    HtmlElement parent() const;
    /** Returns the first child element of this node. */
    HtmlElement firstChild() const;
    /** Returns the next sibling element of this node. */
    HtmlElement nextSibling() const;
    /** Returns the content of this element.
     *  That is, all text nodes that are immediate children of this element.
     *  The content is trimmed from leading or trailing whitespaces.
     */
    QString content() const;
    /** Returns the content of this element and all its children. */
    QString recursiveContent() const;
    /** Checks whether an attribute with name @p attr exists. */
    bool hasAttribute(const QString &attr) const;
    /** Returns the list of all attributes of this node. */
    QStringList attributes() const;

    /** Evaluate an XPath expression relative to this node. */
    Q_INVOKABLE QVariant eval(const QString &xpath) const;

    /** Checks if two HtmlElement instances refer to the same DOM node. */
    bool operator==(const HtmlElement &other) const;

private:
    friend class HtmlDocument;
    HtmlElement(_xmlNode *dd);
    _xmlNode *d;
};

/** HTML document for extraction.
 *  This is used as input for ExtractorEngine and the JS extractor scripts.
 *  @note This class is only functional if libxml is available as a dependency,
 *  otherwise all methods return empty values.
 */
class KITINERARY_EXPORT HtmlDocument : public QObject
{
    Q_OBJECT
    Q_PROPERTY(KItinerary::HtmlElement root READ root)
public:
    ~HtmlDocument();

    /** Creates a HtmlDocument from the given raw data.
     *  @returns @c nullptr if loading fails or libxml was not found.
     */
    static HtmlDocument* fromData(const QByteArray &data, QObject *parent = nullptr);

    /** Returns the root element of the document. */
    HtmlElement root() const;

    /** Evaluate an XPath expression relative to the document root. */
    Q_INVOKABLE QVariant eval(const QString &xpath) const;

private:
    explicit HtmlDocument(QObject *parent = nullptr);
    std::unique_ptr<HtmlDocumentPrivate> d;
};

}

Q_DECLARE_METATYPE(KItinerary::HtmlElement)

