/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-kitinerary.h"
#include "htmldocument.h"

#include <QDebug>
#include <QVariant>

#if HAVE_LIBXML2
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#endif

using namespace KItinerary;

namespace KItinerary {
class HtmlDocumentPrivate {
public:
#if HAVE_LIBXML2
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

#if HAVE_LIBXML2
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
#if HAVE_LIBXML2
    if (d) {
        return QString::fromUtf8(reinterpret_cast<const char*>(d->name));
    }
#endif
    return {};
}

QString HtmlElement::attribute(const QString &attr) const
{
#if HAVE_LIBXML2
    if (d) {
        const auto val = std::unique_ptr<xmlChar, decltype(xmlFree)>(xmlGetProp(d, reinterpret_cast<const xmlChar*>(attr.toUtf8().constData())), xmlFree);
        return QString::fromUtf8(reinterpret_cast<const char*>(val.get()));
    }
#else
    Q_UNUSED(attr)
#endif
    return {};
}

HtmlElement HtmlElement::parent() const
{
#if HAVE_LIBXML2
    if (d && d->parent && d->parent->type == XML_ELEMENT_NODE) {
        return HtmlElement(d->parent);
    }
#endif
    return {};
}

HtmlElement HtmlElement::firstChild() const
{
#if HAVE_LIBXML2
    if (d) {
        return HtmlElement(xmlFirstElementChild(d));
    }
#endif
    return {};
}

HtmlElement HtmlElement::nextSibling() const
{
#if HAVE_LIBXML2
    if (d) {
        return HtmlElement(xmlNextElementSibling(d));
    }
#endif
    return {};
}

#if HAVE_LIBXML2
static void normalizingAppend(QString &out, const QString &in)
{
    if (in.isEmpty()) {
        return;
    }

    const bool needsLeadingSpace = !out.isEmpty() && !out.back().isSpace();
    out.reserve(out.size() + in.size() + (needsLeadingSpace ? 1 : 0));
    if (needsLeadingSpace) {
        out.push_back(QChar::Space);
    }

    // convert non-breaking spaces and windows line break to normal ones, technically not correct
    // but way too often this confuses our regular expressions
    bool leadingTrim = true;
    bool foundCR = false;
    for (const auto c : in) {
        // trim leading spaces while we are at it
        if (leadingTrim && c.isSpace()) {
            continue;
        }
        leadingTrim = false;

        // normalize CRs
        if (c == QChar::CarriageReturn) {
            foundCR = true;
            continue;
        }
        if (foundCR && c != QChar::LineFeed) {
            out.push_back(QChar::LineFeed);
        }
        foundCR = false;

        // normalize space variations
        if (c == QChar::Nbsp) {
            out.push_back(QChar::Space);
        } else {
            out.push_back(c);
        }
    }
}

static void normalizingLineBreakAppend(QString &s)
{
    s = s.trimmed();
    s.push_back(QChar::LineFeed);
}
#endif

QString HtmlElement::content() const
{
#if HAVE_LIBXML2
    if (!d) {
        return {};
    }

    QString s;
    auto node = d->children;
    while (node) {
        switch (node->type) {
            case XML_TEXT_NODE:
            case XML_CDATA_SECTION_NODE:
                normalizingAppend(s, QString::fromUtf8(reinterpret_cast<const char*>(node->content)));
                break;
            case XML_ENTITY_REF_NODE:
            {
                const auto val = std::unique_ptr<xmlChar, decltype(xmlFree)>(xmlNodeGetContent(node), xmlFree);
                normalizingAppend(s, QString::fromUtf8(reinterpret_cast<const char*>(val.get())));
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

    return s.trimmed(); // trailing trim can be done without copying
#endif
    return {};
}

#if HAVE_LIBXML2
static void recursiveContent(_xmlNode *node, QString &s)
{
    switch (node->type) {
        case XML_TEXT_NODE:
        case XML_CDATA_SECTION_NODE:
            normalizingAppend(s, QString::fromUtf8(reinterpret_cast<const char*>(node->content)));
            return;
        case XML_ENTITY_REF_NODE:
        {
            const auto val = std::unique_ptr<xmlChar, decltype(xmlFree)>(xmlNodeGetContent(node), xmlFree);
            normalizingAppend(s, QString::fromUtf8(reinterpret_cast<const char*>(val.get())));
            break;
        }
        case XML_ELEMENT_NODE:
        {
            if (qstricmp(reinterpret_cast<const char*>(node->name), "style") == 0) {
                return;
            } else if (qstricmp(reinterpret_cast<const char*>(node->name), "table") == 0) {
                normalizingLineBreakAppend(s);
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

    if (node->type == XML_ELEMENT_NODE) {
        for (const auto elemName : { "br", "p", "tr" }) {
            if (qstricmp(reinterpret_cast<const char*>(node->name), elemName) == 0) {
                normalizingLineBreakAppend(s);
                break;
            }
        }
    }
}
#endif

QString HtmlElement::recursiveContent() const
{
#if HAVE_LIBXML2
    if (!d) {
        return {};
    }

    QString s;
    ::recursiveContent(d, s);
    return s.trimmed(); // trailing trim can be done without copying
#else
    return {};
#endif
}

QVariant HtmlElement::eval(const QString &xpath) const
{
#if HAVE_LIBXML2
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
    Q_UNUSED(xpath)
#endif
    return {};
}

bool HtmlElement::hasAttribute(const QString& attr) const
{
#if HAVE_LIBXML2
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
    Q_UNUSED(attr)
#endif
    return false;
}

QStringList HtmlElement::attributes() const
{
    QStringList l;
#if HAVE_LIBXML2
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

bool HtmlElement::operator==(const HtmlElement &other) const
{
    return d == other.d;
}


HtmlElement HtmlDocument::root() const
{
#if HAVE_LIBXML2
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
#if HAVE_LIBXML2
    auto tree = htmlReadMemory(data.constData(), data.size(), nullptr, nullptr, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NOBLANKS | HTML_PARSE_NONET | HTML_PARSE_COMPACT);
    if (!tree) {
        return nullptr;
    }

    auto doc = new HtmlDocument(parent);
    doc->d->m_doc = tree;
    return doc;
#else
    Q_UNUSED(data)
    Q_UNUSED(parent)
    return nullptr;
#endif
}

HtmlDocument* HtmlDocument::fromString(const QString &data, QObject *parent)
{
#if HAVE_LIBXML2
    const auto utf8Data = data.toUtf8();
    auto tree = htmlReadMemory(utf8Data.constData(), utf8Data.size(), nullptr, "utf-8", HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NOBLANKS | HTML_PARSE_NONET | HTML_PARSE_COMPACT);
    if (!tree) {
        return nullptr;
    }

    auto doc = new HtmlDocument(parent);
    doc->d->m_doc = tree;
    return doc;
#else
    Q_UNUSED(data)
    Q_UNUSED(parent)
    return nullptr;
#endif
}

#include "moc_htmldocument.cpp"
