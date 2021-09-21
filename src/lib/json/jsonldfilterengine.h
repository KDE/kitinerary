/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KITINERARY_JSONLDFILTERENGINE_H
#define KITINERARY_JSONLDFILTERENGINE_H

#include "kitinerary_export.h"

#include <cstddef>

class QJsonArray;
class QJsonObject;

namespace KItinerary {

namespace JsonLd {
    /** Rename a property, if present and the new name isn't in use already. */
    void renameProperty(QJsonObject &obj, const char *oldName, const char *newName);
}

/** JSON-LD filtering for input normalization or type transforms.
 *  @internal only exported for unit tests
 */
class KITINERARY_EXPORT JsonLdFilterEngine
{
public:
    explicit JsonLdFilterEngine();
    ~JsonLdFilterEngine();

    /** Recursively apply filtering rules to @p obj. */
    void filterRecursive(QJsonObject &obj);
    void filterRecursive(QJsonArray &array);

    /** Type mappings.
     *  Has to be sorted by @c fromType.
     */
    struct TypeMapping {
        const char *fromType;
        const char *toType;
    };
    void setTypeMappings(const TypeMapping *typeMappings, std::size_t count);
    template <std::size_t N>
    inline void setTypeMappings(const TypeMapping (&typeMappings)[N])
    {
        setTypeMappings(typeMappings, N);
    }

    /** Type filter functions.
     *  Has to be sorted by @c type.
     */
    struct TypeFilter {
        const char* type;
        void(*filterFunc)(QJsonObject&);
    };
    void setTypeFilters(const TypeFilter *typeFilters, std::size_t count);
    template <std::size_t N>
    inline void setTypeFilters(const TypeFilter (&typeFilters)[N])
    {
        setTypeFilters(typeFilters, N);
    }

    /** Property mappings.
     *  Has to be sorted by @c type.
     */
    struct PropertyMapping {
        const char* type;
        const char* fromName;
        const char* toName;
    };
    void setPropertyMappings(const PropertyMapping *propertyMappings, std::size_t count);
    template <std::size_t N>
    inline void setPropertyMappings(const PropertyMapping (&propertyMappings)[N])
    {
        setPropertyMappings(propertyMappings, N);
    }

private:
    const TypeMapping *m_typeMappings = nullptr;
    std::size_t m_typeMappingsSize;
    const TypeFilter *m_typeFilters = nullptr;
    std::size_t m_typeFiltersSize;
    const PropertyMapping *m_propertyMappings = nullptr;
    std::size_t m_propertyMappingsSize;
};

}

#endif // KITINERARY_JSONLDFILTERENGINE_H
