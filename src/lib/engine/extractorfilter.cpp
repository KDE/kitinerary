/*
   SPDX-FileCopyrightText: 2017-2021 Volker Krause <vkrause@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "extractorfilter.h"
#include "extractordocumentnode.h"
#include "extractordocumentprocessor.h"
#include "extractorresult.h"
#include "logging.h"

#include <QJsonObject>
#include <QMetaEnum>
#include <QRegularExpression>

using namespace KItinerary;

namespace KItinerary {
class ExtractorFilterPrivate : public QSharedData
{
public:
    QString m_mimeType;
    QString m_fieldName;
    QRegularExpression m_exp;
    ExtractorFilter::Scope m_scope = ExtractorFilter::Current;
};
}

ExtractorFilter::ExtractorFilter()
    : d(new ExtractorFilterPrivate)
{
}

ExtractorFilter::ExtractorFilter(const ExtractorFilter&) = default;
ExtractorFilter::ExtractorFilter(ExtractorFilter&&) noexcept = default;
ExtractorFilter::~ExtractorFilter() = default;
ExtractorFilter& ExtractorFilter::operator=(const ExtractorFilter&) = default;
ExtractorFilter& ExtractorFilter::operator=(ExtractorFilter&&) = default;

ExtractorInput::Type ExtractorFilter::type() const
{
    return ExtractorInput::typeFromMimeType(d->m_mimeType);
}

void ExtractorFilter::setType(ExtractorInput::Type type)
{
    setMimeType(ExtractorInput::typeToMimeType(type));
}

QString ExtractorFilter::mimeType() const
{
    return d->m_mimeType;
}

void ExtractorFilter::setMimeType(const QString &mimeType)
{
    d.detach();
    d->m_mimeType = mimeType;
}

QString ExtractorFilter::fieldName() const
{
    return d->m_fieldName;
}

void ExtractorFilter::setFieldName(const QString &fieldName)
{
    d.detach();
    d->m_fieldName = fieldName;
}

bool ExtractorFilter::matches(const QString &data) const
{
    if (!d->m_exp.isValid()) {
        qCDebug(Log) << d->m_exp.errorString() << d->m_exp.pattern();
    }
    return d->m_exp.match(data).hasMatch();
}

static bool needsFieldName(const QString &mimeType)
{
    return mimeType != QLatin1String("text/plain") && mimeType != QLatin1String("application/octet-stream");
}

template <typename T>
static T readEnum(const QJsonValue &v, T defaultValue = {})
{
    if (!v.isString()) {
        return defaultValue;
    }

    const auto me = QMetaEnum::fromType<T>();
    bool success = false;
    const auto result = static_cast<T>(me.keyToValue(v.toString().toUtf8().constData(), &success));
    return success ? result : defaultValue;
}

bool ExtractorFilter::load(const QJsonObject &obj)
{
    d.detach();
    d->m_mimeType = obj.value(QLatin1String("mimeType")).toString();
    if (d->m_mimeType.isEmpty()) {
        setType(ExtractorInput::typeFromName(obj.value(QLatin1String("type")).toString()));
    }
    if (d->m_mimeType.isEmpty()) {
        qCDebug(Log) << "unspecified filter type";
    }
    d->m_fieldName = obj.value(QLatin1String("field")).toString();
    d->m_exp.setPattern(obj.value(QLatin1String("match")).toString());
    d->m_scope = readEnum<ExtractorFilter::Scope>(obj.value(QLatin1String("scope")), ExtractorFilter::Current);
    return !d->m_mimeType.isEmpty() && (!d->m_fieldName.isEmpty() || !needsFieldName(d->m_mimeType)) && d->m_exp.isValid();
}

QJsonObject ExtractorFilter::toJson() const
{
    QJsonObject obj;
    obj.insert(QLatin1String("mimeType"), d->m_mimeType);
    if (needsFieldName(d->m_mimeType)) {
        obj.insert(QLatin1String("field"), d->m_fieldName);
    }
    obj.insert(QLatin1String("match"), pattern());
    obj.insert(QLatin1String("scope"), QLatin1String(QMetaEnum::fromType<ExtractorFilter::Scope>().valueToKey(d->m_scope)));
    return obj;
}

QString ExtractorFilter::pattern() const
{
    return d->m_exp.pattern();
}

void ExtractorFilter::setPattern(const QString &pattern)
{
    d.detach();
    d->m_exp.setPattern(pattern);
}

ExtractorFilter::Scope ExtractorFilter::scope() const
{
    return d->m_scope;
}

void ExtractorFilter::setScope(Scope scope)
{
    d.detach();
    d->m_scope = scope;
}

static QString valueForJsonPath(const QJsonObject &obj, const QString &path)
{
    const auto pathSections = path.splitRef(QLatin1Char('.'));
    QJsonValue v(obj);
    for (const auto &pathSection : pathSections) {
        if (!v.isObject()) {
            return {};
        }
        v = v.toObject().value(pathSection.toString());
    }
    return v.toString();
}

enum MatchMode { Any, All };

static bool filterMachesNode(const ExtractorFilter &filter, ExtractorFilter::Scope scope, const ExtractorDocumentNode &node,
                             std::vector<ExtractorDocumentNode> &matches, MatchMode matchMode)
{
    if (node.isNull()) {
        return false;
    }

    if (filter.mimeType() == node.mimeType() && node.processor()->matches(filter, node)) {
        if (matchMode == All) {
            matches.push_back(node);
        }
        return true;
    }

    if (scope != ExtractorFilter::Ancestors && filter.mimeType() == QLatin1String("application/ld+json") && !node.result().isEmpty()) {
        const auto res = node.result().jsonLdResult();
        for (const auto &elem : res) {
            const auto property = valueForJsonPath(elem.toObject(), filter.fieldName());
            if (filter.matches(property)) {
                if (matchMode == All) {
                    matches.push_back(node);
                } else {
                    return true;
                }
            }
        }
    }

    if (scope == ExtractorFilter::Ancestors) {
        return filterMachesNode(filter, scope, node.parent(), matches, matchMode);
    }
    if (scope == ExtractorFilter::Descendants) {
        for (const auto &child : node.childNodes()) {
            const auto m = filterMachesNode(filter, ExtractorFilter::Descendants, child, matches, matchMode);
            if (m && matchMode == Any) {
                return true;
            }
        }
    }

    return !matches.empty();
}

bool ExtractorFilter::matches(const ExtractorDocumentNode &node) const
{
    std::vector<ExtractorDocumentNode> matches;
    switch (d->m_scope) {
        case ExtractorFilter::Current:
            return filterMachesNode(*this, ExtractorFilter::Current, node, matches, Any);
        case ExtractorFilter::Parent:
            return filterMachesNode(*this, ExtractorFilter::Current, node.parent(), matches, Any);
        case ExtractorFilter::Ancestors:
            return filterMachesNode(*this, ExtractorFilter::Ancestors, node.parent(), matches, Any);
        case ExtractorFilter::Children:
        case ExtractorFilter::Descendants:
            for (const auto &child : node.childNodes()) {
                if (filterMachesNode(*this, d->m_scope == ExtractorFilter::Descendants ? d->m_scope : ExtractorFilter::Current, child, matches, Any)) {
                    return true;
                }
            }
    }
    return false;
}

void ExtractorFilter::allMatches(const ExtractorDocumentNode &node, std::vector<ExtractorDocumentNode>& matches) const
{
    switch (d->m_scope) {
        case ExtractorFilter::Current:
            filterMachesNode(*this, ExtractorFilter::Current, node, matches, All);
            return;
        case ExtractorFilter::Parent:
            filterMachesNode(*this, ExtractorFilter::Current, node.parent(), matches, All);
            return;
        case ExtractorFilter::Ancestors:
            filterMachesNode(*this, ExtractorFilter::Ancestors, node.parent(), matches, All);
            return;
        case ExtractorFilter::Children:
        case ExtractorFilter::Descendants:
            for (const auto &child : node.childNodes()) {
                filterMachesNode(*this, d->m_scope == ExtractorFilter::Descendants ? d->m_scope : ExtractorFilter::Current, child, matches, All);
            }
            return;
    }
}