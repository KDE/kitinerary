/*
    Copyright (C) 2018 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KITINERARY_HTMLDOCUMENT_H
#define KITINERARY_HTMLDOCUMENT_H

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
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(KItinerary::HtmlElement firstChild READ firstChild)
    Q_PROPERTY(KItinerary::HtmlElement nextSibling READ nextSibling)
    Q_PROPERTY(QString content READ content)
public:
    HtmlElement();
    ~HtmlElement();

    /** Check if the element is null. */
    bool isNull() const;
    /** The element name. */
    QString name() const;
    /** Value of the attribute @p attr. */
    Q_INVOKABLE QString attribute(const QString &attr) const;
    /** Returns the first child element of this node. */
    HtmlElement firstChild() const;
    /** Returns the next sibling element of this node. */
    HtmlElement nextSibling() const;
    /** Returns the content of this element. */
    QString content() const;

    /** Evaluate an XPath expression relative to this node. */
    Q_INVOKABLE QVariant eval(const QString &xpath) const;

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

#endif // KITINERARY_HTMLDOCUMENT_H
