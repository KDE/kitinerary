/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "nameoptimizer_p.h"
#include "stringutil.h"

#include <KItinerary/Person>

#include <QDebug>
#include <QMetaProperty>
#include <QRegularExpression>

using namespace KItinerary;

static const char* name_truncation_pattern[] = {
    "(?:^|\\s)(%1\\w+) %2(?:$|\\s)",
    "(?:^|\\s)%2 ?/ ?(%1\\w+)(?:$|\\s)",
    "(?:^|\\s)%2, (%1\\w+)(?:$|\\s)",
};

Person NameOptimizer::optimizeName(const QString &text, const Person &person)
{
    Person p(person);
    if (p.givenName().isEmpty() && p.familyName().isEmpty()) {
        p.setName(optimizeNameString(text, p.name().trimmed()));
        return p;
    }

    p.setFamilyName(optimizeNameString(text, p.familyName().trimmed()));
    p.setGivenName(optimizeNameString(text, p.givenName().trimmed()));

    // check for IATA BCBP truncation effects
    // IATA BCBP has a 20 character size limit, with one character used for separating name parts
    if (person.givenName() == p.givenName() && (person.familyName().size() + person.givenName().size()) == 19 && person.givenName().size() >= 3) {
        for (auto pattern : name_truncation_pattern) {
            QRegularExpression rx(QLatin1String(pattern).arg(QRegularExpression::escape(p.givenName()), QRegularExpression::escape(p.familyName())), QRegularExpression::CaseInsensitiveOption);
            const auto match = rx.match(text);
            if (match.hasMatch()) {
                p.setGivenName(match.captured(1));
                break;
            }
        }
    }

    return p;
}

static const char* name_prefixes[] = {
    "DR", "MR", "MRS", "MS"
};

static bool isNamePrefix(QStringView s)
{
    s = s.trimmed();
    return std::any_of(std::begin(name_prefixes), std::end(name_prefixes), [s](const char *prefix) { return s == QLatin1String(prefix); });
}

QString NameOptimizer::optimizeNameString(const QString &text, const QString &name)
{
    if (name.size() < 2) {
        return name;
    }

    for (int i = 0; i < text.size(); ++i) {
        bool mismatch = false;
        auto nameLen = name.size();
        for (int j = 0; j < name.size(); ++j) {
            // reached the end of text
            if (i + j >= text.size()) {
                // remainder is either a prefix placed as suffix (see below), or we are unsuccessful
                if (isNamePrefix(QStringView(name).mid(j))) {
                    nameLen = j;
                } else {
                    return name;
                }
                break;
            }

            auto c1 = text.at(i+j).toCaseFolded();
            auto c2 = name.at(j).toCaseFolded();

            if (c1 == c2) {
                continue;
            }

            if (c1.decompositionTag() != c2.decompositionTag()) {
                if (c1.decompositionTag() == QChar::Canonical) {
                    c1 = c1.decomposition().at(0);
                }
                if (c2.decompositionTag() == QChar::Canonical) {
                    c2 = c2.decomposition().at(0);
                }
                if (c1 == c2) {
                    continue;
                }
            }

            // mismatch: check if the remainder is a name prefix (yes, those also occur frequently as suffixes of name parts in IATA BCBP for example)
            if (isNamePrefix(QStringView(name).mid(j))) {
                nameLen = QStringView(name).left(j).trimmed().size();
                break;
            }

            mismatch = true;
            break;
        }
        if (mismatch) {
            continue;
        }

        // test for word boundaries
        if (i > 0 && text.at(i-1).isLetter()) {
            continue;
        }
        if (i + nameLen < text.size() && text.at(i + nameLen).isLetter()) {
            continue;
        }

        if (StringUtil::betterString(QStringView(text).mid(i, nameLen), name) != name) {
            return text.mid(i, nameLen);
        }
    }

    return name;
}

QVariant NameOptimizer::optimizeNameRecursive(const QString &text, QVariant object)
{
    if (JsonLd::isA<Person>(object)) {
        return optimizeName(text, object.value<Person>());
    }

    const auto mo = QMetaType(object.userType()).metaObject();
    if (!mo) {
        return object;
    }

    for (int i = 0; i < mo->propertyCount(); ++i) {
        const auto prop = mo->property(i);
        const auto subMo = QMetaType(prop.userType()).metaObject();
        if (!prop.isStored() || prop.isEnumType()|| (!subMo && prop.userType() != QMetaType::QVariant)) {
            continue;
        }
        const auto value = optimizeNameRecursive(text, prop.readOnGadget(object.constData()));
        prop.writeOnGadget(object.data(), value);
    }

    return object;
}