/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "jsonldfilterengine.h"

#include <QJsonArray>
#include <QJsonObject>

#include <cstring>

using namespace KItinerary;

void KItinerary::JsonLd::renameProperty(QJsonObject &obj, const char *oldName, const char *newName)
{
    const auto value = obj.value(QLatin1String(oldName));
    if (!value.isUndefined() && !obj.contains(QLatin1String(newName))) {
        obj.insert(QLatin1String(newName), value);
        obj.remove(QLatin1String(oldName));
    }
}

JsonLdFilterEngine::JsonLdFilterEngine() = default;
JsonLdFilterEngine::~JsonLdFilterEngine() = default;

void JsonLdFilterEngine::filterRecursive(QJsonObject &obj)
{
    auto type = obj.value(QLatin1String("@type")).toString().toUtf8();

    // normalize type
    if (m_typeMappings)  {
        const auto it = std::lower_bound(m_typeMappings, m_typeMappings + m_typeMappingsSize, type, [](const auto &lhs, const auto &rhs) {
            return std::strcmp(lhs.fromType, rhs.constData()) < 0;
        });
        if (it != (m_typeMappings + m_typeMappingsSize) && std::strcmp((*it).fromType, type.constData()) == 0) {
            type = it->toType;
            obj.insert(QStringLiteral("@type"), QLatin1String(type));
        }
    }

    for (auto it = obj.begin(); it != obj.end(); ++it) {
        if ((*it).type() == QJsonValue::Object) {
            QJsonObject subObj = (*it).toObject();
            filterRecursive(subObj);
            *it = subObj;
        } else if ((*it).type() == QJsonValue::Array) {
            QJsonArray array = (*it).toArray();
            filterRecursive(array);
            *it = array;
        }
    }

    // rename properties
    if (m_propertyMappings) {
        const auto [pBegin, pEnd] = std::equal_range(m_propertyMappings, m_propertyMappings + m_propertyMappingsSize, type, [](const auto &lhs, const auto &rhs) {
            if constexpr (std::is_same_v<std::decay_t<decltype(lhs)>, QByteArray>) {
                return std::strcmp(lhs.constData(), rhs.type) < 0;
            } else {
                return std::strcmp(lhs.type, rhs.constData()) < 0;
            }
        });
        for (auto it = pBegin; it != pEnd; ++it) {
            JsonLd::renameProperty(obj, (*it).fromName, (*it).toName);
        }
    }

    // apply filter functions
    if (m_typeFilters) {
        const auto filterIt = std::lower_bound(m_typeFilters, m_typeFilters + m_typeFiltersSize, type, [](const auto &lhs, const auto &rhs) {
            return std::strcmp(lhs.type, rhs.constData()) < 0;
        });
        if (filterIt != (m_typeFilters + m_typeFiltersSize) && std::strcmp((*filterIt).type, type.constData()) == 0) {
            (*filterIt).filterFunc(obj);
        }
    }
}

void JsonLdFilterEngine::filterRecursive(QJsonArray &array)
{
    for (auto it = array.begin(); it != array.end(); ++it) {
        if ((*it).type() == QJsonValue::Object) {
            QJsonObject subObj = (*it).toObject();
            filterRecursive(subObj);
            *it = subObj;
        } else if ((*it).type() == QJsonValue::Array) {
            QJsonArray array = (*it).toArray();
            filterRecursive(array);
            *it = array;
        }
    }
}

void JsonLdFilterEngine::setTypeMappings(const JsonLdFilterEngine::TypeMapping *typeMappings, std::size_t count)
{
    m_typeMappings = typeMappings;
    m_typeMappingsSize = count;
}

void JsonLdFilterEngine::setTypeFilters(const JsonLdFilterEngine::TypeFilter *typeFilters, std::size_t count)
{
    m_typeFilters = typeFilters;
    m_typeFiltersSize = count;
}

void JsonLdFilterEngine::setPropertyMappings(const JsonLdFilterEngine::PropertyMapping *propertyMappings, std::size_t count)
{
    m_propertyMappings = propertyMappings;
    m_propertyMappingsSize = count;
}
