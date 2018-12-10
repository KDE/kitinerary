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
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "config-kitinerary.h"
#include "htmldocument.h"

#include <QDebug>
#include <QVariant>

#ifdef HAVE_LIBXML2
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#endif

using namespace KItinerary;

namespace KItinerary {
class HtmlDocumentPrivate {
public:
#ifdef HAVE_LIBXML2
    ~HtmlDocumentPrivate() {
        xmlFreeDoc(m_doc);
    }

    xmlDocPtr m_doc;
#endif
};
}

HtmlElement::HtmlElement()
    : d(nullptr)
{
}

HtmlElement::~HtmlElement() = default;

#ifdef HAVE_LIBXML2
HtmlElement::HtmlElement(xmlNode *dd)
    : d(dd)
{
}
#endif

HtmlDocument::HtmlDocument(QObject *parent)
    : QObject(parent)
    , d(new HtmlDocumentPrivate)
{
}

HtmlDocument::~HtmlDocument() = default;

bool HtmlElement::isNull() const
{
    return d == nullptr;
}

QString HtmlElement::name() const
{
#ifdef HAVE_LIBXML2
    if (d) {
        return QString::fromUtf8(reinterpret_cast<const char*>(d->name));
    }
#endif
    return {};
}

QString HtmlElement::attribute(const QString &attr) const
{
#ifdef HAVE_LIBXML2
    if (d) {
        const auto val = std::unique_ptr<xmlChar, decltype(xmlFree)>(xmlGetProp(d, reinterpret_cast<const xmlChar*>(attr.toUtf8().constData())), xmlFree);
        return QString::fromUtf8(reinterpret_cast<const char*>(val.get()));
    }
#else
    Q_UNUSED(attr);
#endif
    return {};
}

HtmlElement HtmlElement::parent() const
{
#ifdef HAVE_LIBXML2
    if (d && d->parent && d->parent->type == XML_ELEMENT_NODE) {
        return HtmlElement(d->parent);
    }
#endif
    return {};
}

HtmlElement HtmlElement::firstChild() const
{
#ifdef HAVE_LIBXML2
    if (d) {
        return HtmlElement(xmlFirstElementChild(d));
    }
#endif
    return {};
}

HtmlElement HtmlElement::nextSibling() const
{
#ifdef HAVE_LIBXML2
    if (d) {
        return HtmlElement(xmlNextElementSibling(d));
    }
#endif
    return {};
}

QString HtmlElement::content() const
{
#ifdef HAVE_LIBXML2
    if (!d) {
        return {};
    }

    QString s;
    auto node = d->children;
    while (node) {
        switch (node->type) {
            case XML_TEXT_NODE:
            case XML_CDATA_SECTION_NODE:
                s += QString::fromUtf8(reinterpret_cast<const char*>(node->content));
                break;
            case XML_ENTITY_REF_NODE:
            {
                const auto val = std::unique_ptr<xmlChar, decltype(xmlFree)>(xmlNodeGetContent(node), xmlFree);
                s += QString::fromUtf8(reinterpret_cast<const char*>(val.get()));
                break;
            }
            case XML_ELEMENT_NODE:
                if (qstricmp(reinterpret_cast<const char*>(node->name), "br") == 0) {
                    s += QLatin1Char('\n');
                }
                break;
            default:
                break;

        }
        node = node->next;
    }

    // convert non-breaking spaces to normal ones, technically not correct
    // but way too often this confuses our regular expressions
    s.replace(QString::fromUtf8(" "), QLatin1String(" "));

    return s.trimmed();
#endif
    return {};
}

#ifdef HAVE_LIBXML2
static void recursiveContent(_xmlNode *node, QString &s)
{
    switch (node->type) {
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
            s += QString::fromUtf8(reinterpret_cast<const char*>(node->content));
            return;
        case XML_ENTITY_REF_NODE:
        {
            const auto val = std::unique_ptr<xmlChar, decltype(xmlFree)>(xmlNodeGetContent(node), xmlFree);
            s += QString::fromUtf8(reinterpret_cast<const char*>(val.get()));
            break;
        }
        case XML_ELEMENT_NODE:
        {
            if (qstricmp(reinterpret_cast<const char*>(node->name), "br") == 0) {
                s += QLatin1Char('\n');
            } else {
                s += QLatin1Char(' ');
            }
            break;
        }
        case XML_ATTRIBUTE_NODE:
        case XML_COMMENT_NODE:
            return;
        default:
            break;
    }

    auto child = node->children;
    while (child) {
        recursiveContent(child, s);
        child = child->next;
    }
}
#endif

QString HtmlElement::recursiveContent() const
{
#ifdef HAVE_LIBXML2
    if (!d) {
        return {};
    }

    QString s;
    ::recursiveContent(d, s);
    // convert non-breaking spaces to normal ones, technically not correct
    // but way too often this confuses our regular expressions
    s.replace(QString::fromUtf8(" "), QLatin1String(" "));
    return s.trimmed();
#else
    return {};
#endif
}

QVariant HtmlElement::eval(const QString &xpath) const
{
#ifdef HAVE_LIBXML2
    if (!d) {
        return {};
    }

    const auto ctx = std::unique_ptr<xmlXPathContext, decltype(&xmlXPathFreeContext)>(xmlXPathNewContext(d->doc), &xmlXPathFreeContext);
    if (!ctx) {
        return {};
    }
    xmlXPathSetContextNode(d, ctx.get());
    const auto xpathObj = std::unique_ptr<xmlXPathObject, decltype(&xmlXPathFreeObject)>(xmlXPathEvalExpression(reinterpret_cast<const xmlChar*>(xpath.toUtf8().constData()), ctx.get()), &xmlXPathFreeObject);
    if (!xpathObj) {
        return {};
    }

    switch (xpathObj->type) {
        case XPATH_NODESET:
        {
            QVariantList l;
            if (!xpathObj->nodesetval) {
                return l;
            }
            l.reserve(xpathObj->nodesetval->nodeNr);
            for (int i = 0; i < xpathObj->nodesetval->nodeNr; ++i) {
                l.push_back(QVariant::fromValue<HtmlElement>(xpathObj->nodesetval->nodeTab[i]));
            }
            return l;
        }
        case XPATH_BOOLEAN:
            return QVariant::fromValue<bool>(xpathObj->boolval);
        case XPATH_NUMBER:
            return xpathObj->floatval;
        case XPATH_STRING:
            return QString::fromUtf8(reinterpret_cast<const char*>(xpathObj->stringval));
        default:
            return {};
    }
#else
    Q_UNUSED(xpath);
#endif
    return {};
}

bool HtmlElement::hasAttribute(const QString& attr) const
{
#ifdef HAVE_LIBXML2
    if (!d) {
        return false;
    }

    auto attribute = d->properties;
    while(attribute)
    {
        if (qstricmp(attr.toUtf8().constData(), reinterpret_cast<const char*>(attribute->name)) == 0) {
            return true;
        }
        attribute = attribute->next;
    }
#else
    Q_UNUSED(attr);
#endif
    return false;
}

QStringList HtmlElement::attributes() const
{
    QStringList l;
#ifdef HAVE_LIBXML2
    if (!d) {
        return l;
    }

    auto attribute = d->properties;
    while(attribute)
    {
        l.push_back(QString::fromUtf8(reinterpret_cast<const char*>(attribute->name)));
        attribute = attribute->next;
    }
#endif
    return l;
}


HtmlElement HtmlDocument::root() const
{
#ifdef HAVE_LIBXML2
    if (!d->m_doc) {
        return {};
    }
    return HtmlElement(xmlDocGetRootElement(d->m_doc));
#else
    return {};
#endif
}

QVariant HtmlDocument::eval(const QString &xpath) const
{
    return root().eval(xpath);
}

HtmlDocument* HtmlDocument::fromData(const QByteArray &data, QObject *parent)
{
#ifdef HAVE_LIBXML2
    auto tree = htmlReadMemory(data.constData(), data.size(), nullptr, nullptr, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NOBLANKS | HTML_PARSE_NONET | HTML_PARSE_COMPACT);
    if (!tree) {
        return nullptr;
    }

    auto doc = new HtmlDocument(parent);
    doc->d->m_doc = tree;
    return doc;
#else
    Q_UNUSED(data);
    Q_UNUSED(parent);
    return nullptr;
#endif
}

#include "moc_htmldocument.cpp"
