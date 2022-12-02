/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "nameoptimizer_p.h"
#include "stringutil.h"

#include <KItinerary/Person>

#include <QDebug>
#include <QMetaProperty>

using namespace KItinerary;

Person NameOptimizer::optimizeName(const QString &text, Person person)
{
    if (person.givenName().isEmpty() && person.familyName().isEmpty()) {
        person.setName(optimizeNameString(text, person.name().trimmed()));
        return person;
    }

    person.setFamilyName(optimizeNameString(text, person.familyName().trimmed()));
    person.setGivenName(optimizeNameString(text, person.givenName().trimmed()));
    return person;
}

QString NameOptimizer::optimizeNameString(const QString &text, const QString &name)
{
    if (name.size() < 2) {
        return name;
    }

    for (int i = 0; i < text.size() - name.size() + 1; ++i) {
        bool mismatch = false;
        for (int j = 0; j < name.size(); ++j) {
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
        if (i + name.size() < text.size() && text.at(i + name.size()).isLetter()) {
            continue;
        }

        if (StringUtil::betterString(name, QStringView(text).mid(i, name.size())) != name) {
            return text.mid(i, name.size());
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
        if (!prop.isStored() || (!subMo && prop.userType() != QMetaType::QVariant)) {
            continue;
        }
        const auto value = optimizeNameRecursive(text, prop.readOnGadget(object.constData()));
        prop.writeOnGadget(object.data(), value);
    }

    return object;
}
